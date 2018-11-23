//
// Created by kamil on 16.06.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <string.h>
#include <zconf.h>
#include <signal.h>
#include <errno.h>
#include "server.h"

char* Name;
int Socket_s;

int create_inet4_socket(char* address, uint16_t port) {
    int soc = socket(AF_INET, SOCK_DGRAM, 0);
    if (soc == -1)
        FAILURE_EXIT("Cannot create IPv4 socket \r\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, address, &addr.sin_addr) == 0)
        FAILURE_EXIT("Cannot convert address \r\n");

    addr.sin_port = htons(port);

    if (connect(soc, (const struct sockaddr*) &addr, sizeof(addr)) != 0)
        FAILURE_EXIT("Cannot create connection to server \r\n");

    return soc;
}

int create_local_socket(char* server_address) {
    int soc = socket(AF_UNIX, SOCK_DGRAM, 0);

    if (soc == -1)
        FAILURE_EXIT("Cannot create UNIX socket \r\n");

    struct sockaddr_un client_addr;
    struct sockaddr_un server_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strcpy(client_addr.sun_path, Name);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strcpy(server_addr.sun_path, server_address);


    if (bind(soc, (struct sockaddr*) &client_addr, sizeof(client_addr)) == -1) {
        perror(strerror(errno));
        FAILURE_EXIT("Cannot bind local socket \r\n");
    }


    if (connect(soc, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
        perror(strerror(errno));
        FAILURE_EXIT("Cannot create connection to server \r\n");
    }

    return soc;
}

void login() {
    message_t message;
    strcpy(message.name, Name);
    message.type = LOGIN;
    if ((write(Socket_s, &message, sizeof(message))) <= 0)
        FAILURE_EXIT("Cannot login to server, exiting \r\n");
    printf("SENT LOGIN \r\n");
}

void cleanup() {
    shutdown(Socket_s, SHUT_RDWR);
    close(Socket_s);
    unlink(Name);
}

void reply(message_t incoming_message) {
    int result;
    switch (incoming_message.expr.operation) {
        case MUL:
            result = incoming_message.expr.arg1 * incoming_message.expr.arg2;
            break;
        case DIV:
            result = incoming_message.expr.arg1 / incoming_message.expr.arg2;
            break;
        case SUB:
            result = incoming_message.expr.arg1 - incoming_message.expr.arg2;
            break;
        case ADD:
            result = incoming_message.expr.arg1 + incoming_message.expr.arg2;
            break;
        default:
            printf("Unrecognized operation \r\n");
            return;
    }
    message_t message;
    strcpy(message.name, Name);
    message.id = incoming_message.id;
    message.type = RESULT;
    message.expr.arg1 = result;
    if (write(Socket_s, &message, sizeof(message)) < 0) {
        perror("Failed to send result to server \r\n");
    }
}

void serve() {
    while (1) {
        message_t message;
        ssize_t bytes = recv(Socket_s, & message, sizeof(message), MSG_WAITALL);
        if (bytes == 0) {
            printf("Server went down \r\n");
            fflush(stdout);
            exit(EXIT_SUCCESS);
        }

        switch (message.type) {
            case EVAL:
                fflush(stdout);
                reply(message);
                break;
            case PING:
                fflush(stdout);
                write(Socket_s, &message, sizeof(message));
                break;
            case OK:
                printf("Successfully logged in \r\n");
                break;
            case REJECT:
                printf("Cannot login to server, exiting \r\n");
                exit(EXIT_FAILURE);
            case KILL:
                printf("Server send KILL message \r\n");
                exit(EXIT_SUCCESS);
            default:
                printf("Unrecognized message type: %d", message.type);

        }
    }
}

void sigint_handler(int _) {
    exit(0);
}

int main(int argc, char* argv[]) {
    if (argc < 4)
        FAILURE_EXIT("Incorrect usage. Usage: ./client <name> <IPv4=0, UNIX=1> <address> [port]\r\n")

    struct sigaction sa;
    sa.sa_flags = 0;
    sigemptyset(&(sa.sa_mask));
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    if(atexit(cleanup) != 0)
        FAILURE_EXIT("Cannot register atexit function \r\n");

    Name = argv[1];

    if (strtol(argv[2], NULL, 10) == 0)
        Socket_s = create_inet4_socket(argv[3], (uint16_t) strtoul(argv[4], NULL, 0));
    else
        Socket_s = create_local_socket(argv[3]);


    login();
    serve();

    return 0;
}