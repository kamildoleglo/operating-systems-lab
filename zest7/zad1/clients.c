//
// Created by kamil on 15.05.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <zconf.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include "common.h"

enum Client_status status;
int shared_memory_id;
int semaphore_id;

void init() {
    key_t project_key = ftok(PROJECT_PATH, PROJECT_ID);
    if (project_key == -1)
    FAILURE_EXIT("Couldn't obtain a project key\n")

    shared_memory_id = shmget(project_key, sizeof(struct Barbershop), 0);
    if (shared_memory_id == -1)
    FAILURE_EXIT("Couldn't create shared memory\n")

    barbershop = shmat(shared_memory_id, 0, 0);
    if (barbershop == (void*) -1)
    FAILURE_EXIT("Couldn't access shared memory\n")

    semaphore_id = semget(project_key, 0, 0);
    if (semaphore_id == -1)
    FAILURE_EXIT("Couldn't create semaphore\n")
}

void claim_chair() {
    pid_t pid = getpid();

    if (status == INVITED) {
        pop_queue();
    } else if (status == NEWCOMER) {
        while (1) {
            release_semaphore(semaphore_id);
            get_semaphore(semaphore_id);
            if (barbershop->barber_status == READY) break;
        }
        status = INVITED;
    }
    barbershop->selected_client = pid;
    printf("%lo CLIENT %i: sat in the chair \r\n", get_time(), pid);
}

void run_client(int S) {
    pid_t pid = getpid();
    int cuts = 0;

    while (cuts < S) {
        status = NEWCOMER;

        get_semaphore(semaphore_id);

        if (barbershop->barber_status == SLEEPING) {
            printf("%lo CLIENT %i: woke up the barber \r\n", get_time(), pid);
            barbershop->barber_status = AWOKEN;
            claim_chair();
            barbershop->barber_status = BUSY;
        } else if (!is_queue_full()) {
            enter_queue(pid);
            printf("%lo CLIENT %i: entering the queue \r\n", get_time(), pid);
        } else {
            printf("%lo CLIENT %i: the queue is full \r\n", get_time(), pid);
            release_semaphore(semaphore_id);
            return;
        }

        release_semaphore(semaphore_id);

        while(status == NEWCOMER) {
            get_semaphore(semaphore_id);
            if (barbershop->selected_client == pid) {
                status = INVITED;
                claim_chair();
                barbershop->barber_status = BUSY;
            }
            release_semaphore(semaphore_id);
        }

        while(status == INVITED) {
            get_semaphore(semaphore_id);
            if (barbershop->selected_client != pid) {
                status = SHAVED;
                printf("%lo CLIENT %i: shaved \r\n", get_time(), pid);
                barbershop->barber_status = IDLE;
                cuts++;
            }
            release_semaphore(semaphore_id);
        }
    }
    printf("%lo CLIENT %i: left barbershop after %i cuts \r\n", get_time(), pid, S);
}


int main(int argc, char** argv) {
    if(argc < 3) FAILURE_EXIT("Not enough arguments \r\n")

    int clients_number = (int) strtol(argv[1], 0, 10);
    int S = (int) strtol(argv[2], 0, 10);
    init();

    for(int i = 0; i < clients_number; ++i) {
        if (fork() == 0) {
            run_client(S);
            exit(0);
        }
    }
    while(wait(0)) if (errno != ECHILD) break;
}