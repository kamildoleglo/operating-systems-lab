//
// Created by kamil on 15.06.18.
//

#ifndef ZEST10_SERVER_H
#define ZEST10_SERVER_H
#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE);}

#define CLIENT_MAX_NAME 20

enum message_type{
    PING = 0, OK = 1, REJECT = 2, EVAL = 3, LOGIN = 4, RESULT = 5, KILL = 6
};

enum operations{
    ADD = 0, SUB = 1, MUL = 2, DIV = 3
};

typedef struct {
    char name[CLIENT_MAX_NAME];
    int file_descriptor;
    int pings;
    struct sockaddr addr;
    socklen_t addr_size;
} client_t;

typedef struct {
  int arg1;
  int arg2;
  int operation;
} expression_t;

typedef struct {
    int id;
    int type;
    char name[CLIENT_MAX_NAME];
    expression_t expr;
} message_t;

#endif //ZEST10_SERVER_H
