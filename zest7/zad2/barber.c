//
// Created by kamil on 15.05.18.
//

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <zconf.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <semaphore.h>

#include "common.h"

int shared_memory_fd;
sem_t* semaphore;

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
    if (semaphore != 0) sem_unlink(PROJECT_PATH);
    if (shared_memory_fd != 0) shm_unlink(PROJECT_PATH);
}

void init(int argc, char** argv) {
    signal(SIGTERM, handle_signal);
    signal(SIGINT, handle_signal);
    atexit(clean_up);

    if (argc < 2) FAILURE_EXIT("Not enough arguments! \r\n")

    int waiting_room_size = (int) strtol(argv[1], 0, 10);
    if (waiting_room_size > MAX_QUEUE_SIZE)
        FAILURE_EXIT("Room size is too big \r\n")


    shared_memory_fd = shm_open(
            PROJECT_PATH,
            O_RDWR | O_CREAT | O_EXCL,
            S_IRWXU | S_IRWXG
    );


    if (shared_memory_fd == -1)
    FAILURE_EXIT("Could not create shared memory segment \r\n")

    int error = ftruncate(shared_memory_fd, sizeof(*barbershop));

    if (error == -1)
        FAILURE_EXIT("Could not truncate shared memory \r\n");

    barbershop = mmap(
            NULL,                   // address
            sizeof(*barbershop),    // length
            PROT_READ | PROT_WRITE, // prot (memory segment security)
            MAP_SHARED,             // flags
            shared_memory_fd,       // file descriptor
            0                       // offset
    );

    if (barbershop == (void*) -1)
        FAILURE_EXIT("Could not access shared memory \r\n")

    semaphore = sem_open(
            PROJECT_PATH,                // path
            O_WRONLY | O_CREAT | O_EXCL, // flags
            S_IRWXU | S_IRWXG,           // mode
            0                            // value
    );

    if (semaphore == (void*) -1)
        FAILURE_EXIT("Could not create semaphore \r\n")


    barbershop->barber_status = SLEEPING;
    barbershop->waiting_room_size = waiting_room_size;
    barbershop->client_count = 0;
    barbershop->selected_client = 0;

    for(int i = 0; i < MAX_QUEUE_SIZE; ++i)
        barbershop->queue[i] = 0;
}


int main(int argc, char** argv) {

    init(argc, argv);

    release_semaphore(semaphore);

    while(1) {
        get_semaphore(semaphore);

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

        release_semaphore(semaphore);
    }
}