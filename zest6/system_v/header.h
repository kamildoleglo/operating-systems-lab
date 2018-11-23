//
// Created by kamil on 24.04.18.
//

#ifndef ZEST6_HEADER_H
#define ZEST6_HEADER_H

#define PROJECT_ID 0xC0FFEE
#define MAX_CLIENTS 44
#define MAX_CONTENT_SIZE 512

typedef enum Msg_type {
    LOGIN  = 1,
    MIRROR = 2,
    CALC   = 3,
    TIME   = 4,
    END    = 5,
} Msg_type;

typedef struct Msg{
    long type;
    pid_t sender_PID;
    char content[MAX_CONTENT_SIZE];
} Msg;

const size_t MESSAGE_SIZE = sizeof(Msg) - sizeof(long);

#endif //ZEST6_HEADER_H
