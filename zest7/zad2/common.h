//
// Created by kamil on 15.05.18.
//

#ifndef ZEST7_COMMON_H
#define ZEST7_COMMON_H

#include <errno.h>
#include <string.h>
#include <time.h>

#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE);   }


#define PROJECT_PATH "/barber"


#define MAX_QUEUE_SIZE 64


enum Barber_status {
    SLEEPING,
    AWOKEN,
    READY,
    IDLE,
    BUSY
};

enum Client_status {
    NEWCOMER,
    INVITED,
    SHAVED
};

struct Barbershop {
    enum Barber_status barber_status;
    int client_count;
    int waiting_room_size;
    pid_t selected_client;
    pid_t queue[MAX_QUEUE_SIZE];
} *barbershop;


long get_time() {
    struct timespec buffer;
    clock_gettime(CLOCK_MONOTONIC, &buffer);
    return buffer.tv_nsec / 1000;
}

void get_semaphore(sem_t* semaphore) {
    int error = sem_wait(semaphore);
    if (error == -1) FAILURE_EXIT("Semaphore error: %s \r\n", strerror(errno))
}

void release_semaphore(sem_t* semaphore) {
    int error = sem_post(semaphore);
    if (error == -1) FAILURE_EXIT("Semaphore error: %s \r\n", strerror(errno))
}


int is_queue_full() {
    if (barbershop->client_count < barbershop->waiting_room_size) return 0;
    return 1;
}

int is_queue_empty() {
    if (barbershop->client_count == 0) return 1;
    return 0;
}

void enter_queue(pid_t pid) {
    barbershop->queue[barbershop->client_count] = pid;
    barbershop->client_count += 1;
}

void pop_queue() {
    for (int i = 0; i < barbershop->client_count - 1; i++) {
        barbershop->queue[i] = barbershop->queue[i + 1];
    }

    barbershop->queue[barbershop->client_count - 1] = 0;
    barbershop->client_count -= 1;
}


#endif //ZEST7_COMMON_H
