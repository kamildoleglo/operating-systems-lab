//
// Created by kamil on 24.04.18.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h>
#include <signal.h>
#include <time.h>
#include "header.h"

#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define FAILURE_RETURN(format, ...) { printf(format, ##__VA_ARGS__); return; }


mqd_t server_queue_id = -1;
bool end = false;
int clients[MAX_CLIENTS][2];
int clients_count = 0;

void int_handler(int _);
void rm_queue();
int find_queue_id(pid_t sender_pid);
int prepare_message(struct Msg *message);
void login_cmd(struct Msg* message);
void mirror_cmd(struct Msg* message);
void calc_cmd(struct Msg* message);
void time_cmd(struct Msg* message);
void end_cmd(struct Msg* _);
void quit_cmd(struct Msg* msg);


int main(int argc, char ** argv){
    if(atexit(rm_queue) == -1)
        FAILURE_EXIT("SERVER: failed to register atexit() \r\n")
    if(signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("SERVER: failed to register INT handler \r\n")

    struct mq_attr currentState;

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MQ_SIZE;
    posixAttr.mq_msgsize = MESSAGE_SIZE;

    server_queue_id = mq_open(serverPath, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(server_queue_id == -1) FAILURE_EXIT("SERVER: failed to create public queue \r\n");

    Msg message;
    while(1) {
        if(end) {
            if(mq_getattr(server_queue_id, &currentState) == -1)
                FAILURE_EXIT("SERVER: getting current state of queue failed \r\n");
            if(currentState.mq_curmsgs == 0) break;
        }

        if(mq_receive(server_queue_id,(char*) &message, MESSAGE_SIZE, NULL) == -1)
            FAILURE_EXIT("SERVER: failed to receive message \r\n");

        switch(message.type){
            case LOGIN:
                login_cmd(&message);
                break;
            case MIRROR:
                mirror_cmd(&message);
                break;
            case CALC:
                calc_cmd(&message);
                break;
            case TIME:
                time_cmd(&message);
                break;
            case END:
                end_cmd(&message);
                break;
            case QUIT:
                quit_cmd(&message);
                break;
            default:
                break;
        }
    }

    exit(EXIT_SUCCESS);
}

void end_cmd(struct Msg *_) { end = true; }

void int_handler(int _) {
    printf("SERVER: killed by INT \r\n");
    exit(EXIT_SUCCESS);
}

void rm_queue() {
    for(int i = 0; i < clients_count; i++){
        if(mq_close(clients[i][1]) == -1){
            printf("SERVER: error closing %d client queue \r\n", i);
        }
        if(kill(clients[i][0], SIGINT) == -1){
            printf("SERVER: error stopping %d client \r\n", i);
        }
    }

    if(server_queue_id <= -1) return;

    if(mq_close(server_queue_id) == -1)
        printf("SERVER: error closing queue \r\n");
    else
        printf("SERVER: queue closed successfully \r\n");

    if(mq_unlink(serverPath) == -1)
        printf("SERVER: error deleting queue \r\n");
    else
        printf("SERVER: queue deleted successfully \r\n");
}

int find_queue_id(pid_t sender_pid) {
    for(int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i][0] == sender_pid) return clients[i][1];
    }
    return -1;
}

int prepare_message(struct Msg *message) {
    int client_queue_id = find_queue_id(message->sender_PID);
    if(client_queue_id == -1) {
        printf("SERVER: client not found\n");
        return -1;
    }

    message->type = message->sender_PID;
    message->sender_PID = getpid();

    return client_queue_id;
}

void login_cmd(struct Msg *message) {
    int client_PID = message->sender_PID;
    char client_path[15];
    sprintf(client_path, "/%d", client_PID);

    int client_queue_id = mq_open(client_path, O_WRONLY);
    if(client_queue_id == -1 )
        FAILURE_RETURN("SERVER: reading client queue id failed \r\n");

    int client_pid = message->sender_PID;
    message->type = LOGIN;
    message->sender_PID = getpid();

    if(clients_count < MAX_CLIENTS){
        clients[clients_count][0] = client_pid;
        clients[clients_count][1] = client_queue_id;
        sprintf(message->content, "%d", clients_count);
        clients_count++;
    }else{
        printf("SERVER: reached maximum number of clients \r\n");
        sprintf(message->content, "%d", -1);
    }

    if(mq_send(client_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("SERVER: LOGIN response failed \r\n");
}

void mirror_cmd(struct Msg *message) {
    int client_queue_id = prepare_message(message);
    if(client_queue_id == -1) return;

    int message_length = (int) strlen(message->content);
    while(message->content[message_length-1] == '\n' || message->content[message_length-1] == '\r') message_length--;

    for(int i = 0; i < message_length / 2; i++) {
        char buff = message->content[i];
        message->content[i] = message->content[message_length - i - 1];
        message->content[message_length - i - 1] = buff;
    }

    if(mq_send(client_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("SERVER: MIRROR response failed \r\n");
}

void calc_cmd(struct Msg *message) {
    int client_queue_id = prepare_message(message);
    if(client_queue_id == -1) return;

    char command[4096];
    sprintf(command, "echo '%s' | bc", message->content);
    FILE* calc = popen(command, "r");
    fgets(message->content, MESSAGE_SIZE, calc);
    pclose(calc);

    if(mq_send(client_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("SERVER: CALC response failed \r\n");
}

void time_cmd(struct Msg *message) {
    int client_queue_id = prepare_message(message);
    if(client_queue_id == -1) return;

    time_t raw_time;
    time(&raw_time);
    char* time_str = ctime(&raw_time);

    sprintf(message->content, "%s", time_str);

    if(mq_send(client_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("SERVER: TIME response failed \r\n");
}

void quit_cmd(struct Msg *msg) {
    int i;
    for(i = 0; i < clients_count; i++){
        if(clients[i][0] == msg->sender_PID) break;
    }

    if(i >= clients_count){
        printf("SERVER: client not found \r\n");
        return;
    }

    if(mq_close(clients[i][1]) == -1)
        FAILURE_RETURN("SERVER: error while closing client's queue \r\n");
    for(; i+1 < clients_count; i++){
        clients[i][0] = clients[i+1][0];
        clients[i][1] = clients[i+1][1];
    }
    clients_count--;
    printf("SERVER: successfully removed client \r\n");
}

