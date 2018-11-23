//
// Created by kamil on 12.06.18.
//


#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/epoll.h>
#include <zconf.h>
#include <pthread.h>
#include <signal.h>
#include "server.h"

#define MAX_CLIENTS 15
#define MAX_EVENTS 5

int Inet4_s, Local_s;
pthread_mutex_t Client_mutex[MAX_CLIENTS];
client_t Client[MAX_CLIENTS];
int Epoll_fd;
char* Path;

// Creates IPv4 socket and starts listening
void create_inet4_socket(uint16_t port) {
    Inet4_s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(Inet4_s, (struct sockaddr*) &addr, sizeof(addr)) != 0)
        FAILURE_EXIT("Cannot bind inet socket, exitting \r\n");


    if(listen(Inet4_s, 12) != 0)
        FAILURE_EXIT("Cannot start listening on inet socket, exitting \r\n");

}

// Creates UNIX socket and starts listening
void create_local_socket(const char* name) {
    Local_s = socket(AF_UNIX, SOCK_STREAM, 0);
    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, name);

    if (bind(Local_s, (struct sockaddr*) &addr, sizeof(addr)) != 0)
        FAILURE_EXIT("Cannot bind local socket, exitting \r\n");

    if(listen(Local_s, 12) != 0)
        FAILURE_EXIT("Cannot start listening on local socket, exitting \r\n");

}

void epoll_init() {
    Epoll_fd = epoll_create1(0);
    if (Epoll_fd == -1)
        FAILURE_EXIT("Failed to create epoll file descriptor \r\n");

    struct epoll_event e;
    e.events = EPOLLIN | EPOLLET;
    e.data.fd = Local_s;
    if (epoll_ctl(Epoll_fd, EPOLL_CTL_ADD, Local_s, &e) == -1)
        FAILURE_EXIT("Failed to add UNIX socket to epoll \r\n");

    e.events = EPOLLIN | EPOLLET;
    e.data.fd = Inet4_s;
    if (epoll_ctl(Epoll_fd, EPOLL_CTL_ADD, Inet4_s, &e) == -1)
        FAILURE_EXIT("Failed to add IPv4 socket to epoll \r\n");
}

void add_client(struct epoll_event event) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_lock(&Client_mutex[i]);
        if (Client[i].file_descriptor <= 0) {
            Client[i].pings = 0;
            struct sockaddr new_addr;
            socklen_t new_addr_len = sizeof(new_addr);
            Client[i].file_descriptor = accept(event.data.fd, &new_addr, &new_addr_len);

            struct epoll_event e;
            e.events = EPOLLIN | EPOLLET;
            e.data.fd = Client[i].file_descriptor;
            if (epoll_ctl(Epoll_fd, EPOLL_CTL_ADD, Client[i].file_descriptor, & e) == -1) {
                printf("Failed to create epoll file descriptor for client \r\n");
                fflush(stdout);
                Client[i].pings = -1;
            }
            pthread_mutex_unlock(&Client_mutex[i]);
            return;
        }
        pthread_mutex_unlock(&Client_mutex[i]);
    }
    return;
}


void close_connection(struct epoll_event event) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_lock(&Client_mutex[i]);
        if (event.data.fd == Client[i].file_descriptor) {
            shutdown(event.data.fd, SHUT_RDWR);
            close(event.data.fd);
            for(int j = 0; j < CLIENT_MAX_NAME; j++) Client[i].name[j] = 0;
            Client[i].pings = 0;
            Client[i].file_descriptor = -1;
            pthread_mutex_unlock(&Client_mutex[i]);
            break;
        }
        pthread_mutex_unlock(&Client_mutex[i]);
    }
}

void send_message(client_t* client, message_t message){
    write(client->file_descriptor, &message, sizeof(message));
}

void send_message_to_random_client(message_t message){
    int i = (rand() * MAX_CLIENTS) % (MAX_CLIENTS - 1);
    for(int j = i+1; j != i; j = (j + 1) % MAX_CLIENTS){
        pthread_mutex_lock(&Client_mutex[j]);
        if(Client[j].file_descriptor > 2){
            send_message(&Client[j], message);
            pthread_mutex_unlock(&Client_mutex[j]);
            return;
        }
        pthread_mutex_unlock(&Client_mutex[j]);
    }
    printf("No client to send \r\n");
}

void receive_message(struct epoll_event event) {
    message_t message;
    ssize_t bytes = read(event.data.fd, &message, sizeof(message));
    if (bytes == 0) {
        printf("Closing connection %d \r\n", event.data.fd);
        fflush(stdout);
        close_connection(event);
        return;
    } else {
        switch(message.type) {
            case LOGIN:
            {
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    pthread_mutex_lock(&Client_mutex[i]);
                    if ((strcmp(message.name, Client[i].name) == 0) && Client[i].file_descriptor > 2) {
                        message.type = REJECT;
                        send_message(&Client[i], message);
                        pthread_mutex_unlock(&Client_mutex[i]);
                        return;
                    }
                    pthread_mutex_unlock(&Client_mutex[i]);
                }
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    pthread_mutex_lock(&Client_mutex[i]);
                    if (Client[i].file_descriptor == event.data.fd) {
                        strcpy(Client[i].name, message.name);
                        Client[i].pings = 0;
                        message.type = OK;
                        send_message(&Client[i], message);
                        pthread_mutex_unlock(&Client_mutex[i]);
                        printf("Client %s logged in \r\n", message.name);
                        return;
                    }
                    pthread_mutex_unlock(&Client_mutex[i]);
                }
                break;
            }
            case RESULT:
                printf("CLIENT %s: Result of exp num: %d is: %d \r\n", message.name, message.id, message.expr.arg1);
                fflush(stdout);
                break;
            case PING:
                pthread_mutex_lock(&Client_mutex[message.id]);
                Client[message.id].pings = 0;
                pthread_mutex_unlock(&Client_mutex[message.id]);
                break;
        }
    }
}

void* network_handler(void* _){
    struct epoll_event event[MAX_EVENTS];

    while (1) {
        int event_count = epoll_wait(Epoll_fd, event, MAX_EVENTS, -1);
        for (int i = 0; i < event_count; i++) {
            if (event[i].data.fd == Local_s || event[i].data.fd == Inet4_s) {
                add_client(event[i]);
            } else {
                receive_message(event[i]);
            }
        }
    }
    return NULL;
}



// Sends a ping approx. every 12 seconds and then removes not responding clients
void* ping_sender(void* _){
    message_t message;
    message.type = PING;
    while(1){
        for(int i = 0; i < MAX_CLIENTS; i++){
            pthread_mutex_lock(&Client_mutex[i]);
            if(Client[i].file_descriptor > 2) {
                message.id = i;
                Client[i].pings++;
                send_message(&Client[i], message);
            }
            pthread_mutex_unlock(&Client_mutex[i]);
        }
        sleep(2);
        for(int i = 0; i < MAX_CLIENTS; i++){
            pthread_mutex_lock(&Client_mutex[i]);
            if(Client[i].pings > 0) {
                printf("Deregistered client \r\n");
                shutdown(Client[i].file_descriptor, SHUT_RDWR);
                close(Client[i].file_descriptor);
                Client[i].file_descriptor = -1;
                Client[i].pings = 0;
                for(int j = 0; j < CLIENT_MAX_NAME; j++) Client[i].name[j] = 0;
            }
            pthread_mutex_unlock(&Client_mutex[i]);
        }
        sleep(10);
    }
    return NULL;
}

// Handles commands from user and sends message to random client
void* command_handler(void* _){
    int a, b, counter = 0;
    char c;
    while(1){
        printf("%d> ", counter);
        int items = scanf("%d %c %d", &a, &c, &b);
        if(items != 3){
            printf("Not enough arguments \r\n");
            while ((c = getchar()) != EOF && c != '\n')
                continue;
            continue;
        }
        message_t message;
        message.id = counter;
        message.type = EVAL;
        message.expr.arg1 = a;
        message.expr.arg2 = b;
        switch(c){
            case '+':
                message.expr.operation = ADD;
                break;
            case '-':
                message.expr.operation = SUB;
                break;
            case '*':
                message.expr.operation = MUL;
                break;
            case '/':
                message.expr.operation = DIV;
                break;
            default:
                printf("Unrecognized operation: %c \r\n", c);
                continue;
        }
        send_message_to_random_client(message);
        counter++;
    }
    return NULL;
}

// Initializes mutexes
void init_mutexes(){
    for(int i = 0; i < MAX_CLIENTS; i++){
        if(pthread_mutex_init(&Client_mutex[i], NULL) != 0)
            FAILURE_EXIT("Cannot create mutex, exiting \r\n");
    }
}

// Sends KILL message to clients and destroys mutexes
void cleanup(){

    message_t message;
    message.type = KILL;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_mutex_lock(&Client_mutex[i]);
        if (Client[i].file_descriptor > 2)
            send_message(&Client[i], message);
        pthread_mutex_unlock(&Client_mutex[i]);
    }

    for(int i = 0; i < MAX_CLIENTS; i++){
        if(pthread_mutex_destroy(&Client_mutex[i]) != 0)
            printf("Cannot destroy mutex \r\n");
    }

    shutdown(Inet4_s, SHUT_RDWR);
    shutdown(Local_s, SHUT_RDWR);
    close(Epoll_fd);
    close(Inet4_s);
    close(Local_s);
    unlink(Path);
}

void sigint_handler(int _){
    exit(0);
}

int main(int argc, char **argv){
    if(argc != 3)
        FAILURE_EXIT("Incorrect usage. Usage: ./server <port> <path> \r\n");

    if(atexit(cleanup) != 0)
        FAILURE_EXIT("Cannot register atexit function \r\n");

    srand((unsigned int)time(NULL));
    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&(sa.sa_mask));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);


    init_mutexes();
    for(int i = 0; i < MAX_CLIENTS; i++) {
        Client[i].file_descriptor = -1;
        Client[i].pings = 0;
    }

    uint16_t port_no = (uint16_t)strtol(argv[1], NULL, 10);
    Path = argv[2];
    create_inet4_socket(port_no);
    create_local_socket(Path);

    epoll_init();

    pthread_t command, ping, network;
    pthread_create(&command, NULL, command_handler, NULL);
    pthread_create(&ping, NULL, ping_sender, NULL);
    pthread_create(&network, NULL, network_handler, NULL);
    pthread_join(command, NULL);
    pthread_join(ping, NULL);
    pthread_join(network, NULL);

    return 0;
}