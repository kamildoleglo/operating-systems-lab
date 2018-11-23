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

#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE);   }
#define FAILURE_RETURN(format, ...) { printf(format, ##__VA_ARGS__); return; }

int session_id = -1;
mqd_t server_queue_id = -1;
mqd_t private_queue_id = -1;
char client_path[64];

void int_handler(int _);
void rm_queue();
int get_queue_id(char* path, int id, int flag);
void register_client(void);
void mirror_cmd(Msg *message);
void calc_cmd(Msg *message);
void time_cmd(Msg *message);
void end_cmd(Msg *message);


int main(int argc, char** argv) {
    if (atexit(rm_queue) == -1)
        FAILURE_EXIT("CLIENT: failed to register atexit() \r\n")
    if(signal(SIGINT, int_handler) == SIG_ERR)
        FAILURE_EXIT("CLIENT: failed to register INT handler \r\n")

    sprintf(client_path, "/%d", getpid());

    server_queue_id = mq_open(serverPath, O_WRONLY);
    if(server_queue_id == -1)
        FAILURE_EXIT("CLIENT: failed to open server's queue \r\n")

    struct mq_attr posixAttr;
    posixAttr.mq_maxmsg = MAX_MQ_SIZE;
    posixAttr.mq_msgsize = MESSAGE_SIZE;

    private_queue_id = mq_open(client_path, O_RDONLY | O_CREAT | O_EXCL, 0666, &posixAttr);
    if(private_queue_id == -1)
        FAILURE_EXIT("CLIENT: failed to open queue \r\n")

    register_client();

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
    exit(EXIT_SUCCESS);
}


void int_handler(int _) {
    printf("CLIENT: killed by INT \r\n");
    exit(EXIT_SUCCESS);
}

void rm_queue() {
    if(private_queue_id <= -1) return;

    if(server_queue_id >= 0){
        Msg message;
        message.sender_PID = getpid();
        message.type = QUIT;
        if(mq_send(server_queue_id, (char*) &message, MESSAGE_SIZE, 1) == -1)
            printf("CLIENT: QUIT request failed \r\n");
    }

    if(mq_close(server_queue_id) == -1)
        printf("CLIENT: error closing server's queue \r\n");
    else
        printf("CLIENT: server's queue closed successfully \r\n");

    if(mq_close(private_queue_id) == -1)
        printf("CLIENT: error closing queue \r\n");
    else
        printf("CLIENT: queue closed successfully \r\n");

    if(mq_unlink(client_path) == -1)
        printf("CLIENT: error deleting queue \r\n");
    else
        printf("CLIENT: queue deleted successfully \r\n");
}



void register_client() {
    Msg message;
    message.type = LOGIN;
    message.sender_PID = getpid();
    sprintf(message.content, "%s", client_path);

    if(mq_send(server_queue_id, (char*) &message, MESSAGE_SIZE, 1) == -1)
        FAILURE_EXIT("CLIENT: LOGIN request failed \r\n");
    if(mq_receive(private_queue_id,(char*) &message, MESSAGE_SIZE, NULL) == -1)
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
    if(mq_send(server_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_RETURN("CLIENT: MIRROR request failed \r\n");
    if(mq_receive(private_queue_id, (char*) message, MESSAGE_SIZE, NULL) == -1)
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

    if(mq_send(server_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_RETURN("CLIENT: CALC request failed \r\n")
    if(mq_receive(private_queue_id, (char*) message, MESSAGE_SIZE, NULL) == -1)
        FAILURE_RETURN("CLIENT: error while receiving CALC response \r\n")

    printf("%s \r\n", message->content);
}

void time_cmd(Msg *message) {
    message->type = TIME;

    if(mq_send(server_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_RETURN("CLIENT: TIME request failed \r\n")
    if(mq_receive(private_queue_id, (char*) message, MESSAGE_SIZE, NULL) == -1)
        FAILURE_RETURN("CLIENT: error while receiving TIME response \r\n")

    printf("%s \r\n", message->content);
}

void end_cmd(Msg *message) {
    message->type = END;

    if(mq_send(server_queue_id, (char*) message, MESSAGE_SIZE, 1) == -1)
        FAILURE_RETURN("CLIENT: END request failed \r\n")
}