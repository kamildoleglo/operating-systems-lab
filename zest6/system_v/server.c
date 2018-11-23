//
// Created by kamil on 24.04.18.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdbool.h>
#include <zconf.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "header.h"

#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define FAILURE_RETURN(format, ...) { printf(format, ##__VA_ARGS__); return; }


int server_queue_id = -1;
bool end = false;
int clients[MAX_CLIENTS][2];
int clients_count = 0;

void end_cmd(struct Msg* _);
void int_handler(int _);
void rm_queue();
int find_queue_id(pid_t sender_pid);
int prepare_message(struct Msg *message);
void login_cmd(struct Msg* message);
void mirror_cmd(struct Msg* message);
void calc_cmd(struct Msg* message);
void time_cmd(struct Msg* message);


int main(int argc, char ** argv){

    if (atexit(rm_queue) == -1)
        FAILURE_EXIT("SERVER: failed to register atexit() \r\n")
    if(signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("SERVER: failed to register INT handler \r\n")


    char* path = getenv("HOME");
    if(path == 0) FAILURE_EXIT("SERVER: failed to obtain value of $HOME \r\n");

    key_t public_key = ftok(path, PROJECT_ID);
    if(public_key == -1) FAILURE_EXIT("SERVER: failed to generate public key \r\n");

    server_queue_id = msgget(public_key, IPC_CREAT | IPC_EXCL | 0666);
    if(server_queue_id == -1) FAILURE_EXIT("SERVER: failed to create public queue \r\n");
    printf("SERVER: server queue id: %d \r\n", server_queue_id);

    struct msqid_ds current_state;
    Msg message;

    while(1) {
        if(end) {
            if(msgctl(server_queue_id, IPC_STAT, &current_state) == -1)
                FAILURE_EXIT("SERVER: getting current state of queue failed \r\n");
            if(current_state.msg_qnum == 0) break;
        }

        if(msgrcv(server_queue_id, &message, MESSAGE_SIZE, 0, 0) < 0)
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
            default:
                break;
        }
    }

    return 0;
}

void end_cmd(struct Msg *_) { end = true; }

void int_handler(int _) {
    printf("SERVER: killed by INT \r\n");
    exit(EXIT_SUCCESS);
}

void rm_queue() {
    if(server_queue_id <= -1) return;

    if (msgctl(server_queue_id, IPC_RMID, 0) == -1) {
        printf("SERVER: error deleting queue \r\n");
        return;
    }

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
    key_t client_queue_key;
    if(sscanf(message->content, "%d", &client_queue_key) < 0)
        FAILURE_RETURN("SERVER: reading client key failed \r\n");

    int client_queue_id = msgget(client_queue_key, 0);
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


    if(msgsnd(client_queue_id, message, MESSAGE_SIZE, 0) == -1)
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

    if(msgsnd(client_queue_id, message, MESSAGE_SIZE, 0) == -1)
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

    if(msgsnd(client_queue_id, message, MESSAGE_SIZE, 0) == -1)
    FAILURE_EXIT("SERVER: CALC response failed \r\n");
}

void time_cmd(struct Msg *message) {
    int client_queue_id = prepare_message(message);
    if(client_queue_id == -1) return;

    time_t raw_time;
    time(&raw_time);
    char* time_str = ctime(&raw_time);

    sprintf(message->content, "%s", time_str);

    if(msgsnd(client_queue_id, message, MESSAGE_SIZE, 0) == -1)
    FAILURE_EXIT("SERVER: TIME response failed \r\n");
}

