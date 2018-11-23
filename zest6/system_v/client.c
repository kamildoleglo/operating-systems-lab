//
// Created by kamil on 24.04.18.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <zconf.h>
#include <string.h>
#include <signal.h>
#include "header.h"

#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE);   }
#define FAILURE_RETURN(format, ...) { printf(format, ##__VA_ARGS__); return; }

int session_id = -1;
int server_queue_id = -1;
int private_queue_id = -1;

void int_handler(int _);
void rm_queue();
int get_queue_id(char* path, int id, int flag);
void register_client(key_t private_key);
void mirror_cmd(Msg *message);
void calc_cmd(Msg *message);
void time_cmd(Msg *message);
void end_cmd(Msg *message);


int main(int argc, char** argv) {
    if (atexit(rm_queue) == -1)
        FAILURE_EXIT("CLIENT: failed to register atexit() \r\n")
    if(signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("CLIENT: failed to register INT handler \r\n")

    char* path = getenv("HOME");
    if(path == 0) FAILURE_EXIT("CLIENT: failed to obtain value of $HOME \r\n")

    server_queue_id = get_queue_id(path, PROJECT_ID, 0);
    printf("CLIENT: server queue id: %d \r\n", server_queue_id);

    key_t private_key = ftok(path, getpid());
    if(private_key == -1) FAILURE_EXIT("CLIENT: failed to generate private key \r\n");

    private_queue_id = msgget(private_key, IPC_CREAT | IPC_EXCL | 0666);

    register_client(private_key);

    char command[20];
    Msg message;

    while(1) {
        message.sender_PID = getpid();
        printf("CLIENT: > ");

        if(fgets(command, 20, stdin) == 0) {
            printf("CLIENT: error while reading command \r\n");
            continue;
        }

        size_t n = strlen(command);
        while(command[n - 1] == '\n' || command[n - 1] == '\r'){ command[n-1] = 0; n--; }

        if(strcmp(command, "MIRROR") == 0) {
            mirror_cmd(&message);
        } else if (strcmp(command, "CALC") == 0) {
            calc_cmd(&message);
        } else if (strcmp(command, "TIME") == 0) {
            time_cmd(&message);
        } else if (strcmp(command, "END") == 0) {
            end_cmd(&message);
            exit(EXIT_SUCCESS);
        } else {
            printf("CLIENT: unrecognized command \r\n");
        }
    }
}


void int_handler(int _) {
    printf("CLIENT: killed by INT \r\n");
    exit(EXIT_SUCCESS);
}

void rm_queue() {
    if(private_queue_id <= -1) return;

    if (msgctl(private_queue_id, IPC_RMID, 0) == -1) {
        printf("CLIENT: error deleting queue \r\n");
        return;
    }

    printf("CLIENT: queue deleted successfully \r\n");
}

int get_queue_id(char *path, int id, int flag) {
    key_t key = ftok(path, id);
    if(key == -1) FAILURE_EXIT("CLIENT: failed to generate key \r\n")

    int queue_id = msgget(key, flag);
    if (queue_id == -1) FAILURE_EXIT("CLIENT: failed to open queue \r\n")

    return queue_id;
}

void register_client(key_t private_key) {
    Msg message;
    message.type = LOGIN;
    message.sender_PID = getpid();
    sprintf(message.content, "%d", private_key);

    if(msgsnd(server_queue_id, &message, MESSAGE_SIZE, 0) == -1)
        FAILURE_EXIT("CLIENT: LOGIN request failed \r\n");
    if(msgrcv(private_queue_id, &message, MESSAGE_SIZE, 0, 0) == -1)
        FAILURE_EXIT("CLIENT: error while receiving LOGIN response \r\n");
    if(sscanf(message.content, "%d", &session_id) < 1)
        FAILURE_EXIT("CLIENT: error while reading LOGIN response \r\n");
    if(session_id < 0)
        FAILURE_EXIT("CLIENT: server reached maximum clients \r\n");

    printf("CLIENT: registered with session no %i \r\n", session_id);
}

void mirror_cmd(Msg *message) {
    message->type = MIRROR;
    printf("CLIENT: enter line to mirror: \r\n");
    if(fgets(message->content, MAX_CONTENT_SIZE, stdin) == 0) {
        printf("CLIENT: too long \r\n");
        return;
    }
    if(msgsnd(server_queue_id, message, MESSAGE_SIZE, 0) == -1)
    FAILURE_RETURN("CLIENT: MIRROR request failed \r\n");
    if(msgrcv(private_queue_id, message, MESSAGE_SIZE, 0, 0) == -1)
    FAILURE_RETURN("CLIENT: error while receiving MIRROR response \r\n");

    printf("%s", message->content);
}

void calc_cmd(Msg *message) {
    message->type = CALC;
    printf("Enter expression to calculate in format <number> <operator> <number>: \r\n");

    if(fgets(message->content, MAX_CONTENT_SIZE, stdin) == 0) {
        printf("Too many characters \r\n");
        return;
    }

    if(msgsnd(server_queue_id, message, MESSAGE_SIZE, 0) == -1)
    FAILURE_RETURN("CLIENT: CALC request failed \r\n")
    if(msgrcv(private_queue_id, message, MESSAGE_SIZE, 0, 0) == -1)
    FAILURE_RETURN("CLIENT: error while receiving CALC response \r\n")

    printf("%s \r\n", message->content);
}

void time_cmd(Msg *message) {
    message->type = TIME;

    if(msgsnd(server_queue_id, message, MESSAGE_SIZE, 0) == -1)
    FAILURE_RETURN("CLIENT: TIME request failed \r\n")
    if(msgrcv(private_queue_id, message, MESSAGE_SIZE, 0, 0) == -1)
    FAILURE_RETURN("CLIENT: error while receiving TIME response \r\n")

    printf("%s \r\n", message->content);
}

void end_cmd(Msg *message) {
    message->type = END;

    if(msgsnd(server_queue_id, message, MESSAGE_SIZE, 0) == -1)
    FAILURE_RETURN("CLIENT: END request failed \r\n");
}