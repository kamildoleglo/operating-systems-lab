#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <mqueue.h>


#define QUEUE_NAME  "/test_queue"
#define MAX_SIZE    1024
#define MSG_STOP    "exit"
mqd_t mq;


void cleanup(void){
    mq_close(mq);
    printf("Exiting");
}

int main(int argc, char** argv) {
    atexit(cleanup);

    char buffer[MAX_SIZE];

    /* open the mail queue */
    mq = mq_open(QUEUE_NAME, O_WRONLY);

    int count = 0;
    while(1) {
        snprintf(buffer, sizeof(buffer), "MESSAGE %d", count++);

        printf("Client: Send message %i\r\n", count - 1);
        mq_send(mq, buffer, MAX_SIZE, 0);

        fflush(stdout);
        usleep(7.33 * 1e6);
    }

    return (0);
}

//ipcs
//iprm
//atexit
//posix - serwer zamyka po swojej stronie kolejkę klienta, po tym klient zamyka swoją