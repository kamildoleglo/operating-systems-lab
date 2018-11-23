//
// Created by kamil on 15.05.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>

#include "common.h"

int shared_memory_id;
int semaphore_id;

void handle_signal(int _) {
    printf("BARBER: closing \r\n");
    exit(EXIT_SUCCESS);
}

void invite_client() {
    pid_t client_pid = barbershop->queue[0];
    barbershop->selected_client = client_pid;
    printf("%lo BARBER: invited client %i \r\n", get_time(), client_pid);
}

void serve_client() {
    printf("%lo BARBER: started serving client %i \r\n",
           get_time(),
           barbershop->selected_client);

    printf("%lo BARBER: finished serving client %i \r\n",
           get_time(),
           barbershop->selected_client);

    barbershop->selected_client = 0;
}

void clean_up() {
    if(semaphore_id != 0) {
        semctl(semaphore_id, 0, IPC_RMID);
    }
    if(shared_memory_id != 0) {
        shmctl(shared_memory_id, IPC_RMID, NULL);
    }
}

void init(int argc, char** argv) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    atexit(clean_up);

    if (argc < 2) FAILURE_EXIT("Not enough arguments! \r\n")

    int waiting_room_size = (int) strtol(argv[1], 0, 10);
    if (waiting_room_size > MAX_QUEUE_SIZE)
        FAILURE_EXIT("Room size is too big \r\n")

    key_t project_key = ftok(PROJECT_PATH, PROJECT_ID);
    if (project_key == -1)
        FAILURE_EXIT("Couldn't get project key \r\n")

    // Create shared memory segment
    shared_memory_id = shmget(
            project_key,
            sizeof(struct Barbershop),
            S_IRWXU | IPC_CREAT
    );

    if (shared_memory_id == -1)
        FAILURE_EXIT("Could not create shared memory segment \r\n")

    // Access shared memory
    barbershop = shmat(shared_memory_id, 0, 0);
    if (barbershop == (void*) -1)
        FAILURE_EXIT("Could not access shared memory \r\n")

    semaphore_id = semget(project_key, 1, IPC_CREAT | S_IRWXU);

    if (semaphore_id == -1)
        FAILURE_EXIT("Could not create semaphore \r\n")

    semctl(semaphore_id, 0, SETVAL, 0);

    // Initialize the barbershop
    barbershop->barber_status = SLEEPING;
    barbershop->waiting_room_size = waiting_room_size;
    barbershop->client_count = 0;
    barbershop->selected_client = 0;

    // Initialize empty clients queue
    for(int i = 0; i < MAX_QUEUE_SIZE; ++i)
        barbershop->queue[i] = 0;
}


int main(int argc, char** argv) {

    init(argc, argv);

    release_semaphore(semaphore_id);

    while(1) {
        get_semaphore(semaphore_id);

        switch (barbershop->barber_status) {
            case IDLE:
                if (is_queue_empty()) {
                    printf("%lo BARBER: ...zzzZZZZZzzzzzZZZz... \r\n", get_time());
                    barbershop->barber_status = SLEEPING;
                } else {
                    invite_client();
                    barbershop->barber_status = READY;
                }
                break;
            case AWOKEN:
                printf("%lo BARBER: I HAVE AWOKEN \r\n", get_time());
                barbershop->barber_status = READY;
                break;
            case BUSY:
                serve_client();
                barbershop->barber_status = READY;
                break;
            default:
                break;
        }

        release_semaphore(semaphore_id);
    }
}