//
// Created by kamil on 18.05.18.
//

#ifndef ZEST8_APPLICATION_HELPER_H
#define ZEST8_APPLICATION_HELPER_H
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>

#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); exit(EXIT_FAILURE);}
#define TO_INT(arg) (int)strtol(arg, NULL, 10);

#ifndef max
#define max(a, b) ( ((a) > (b)) ? (a) : (b) )
#endif

#ifndef min
#define min( a, b ) ( ((a) < (b)) ? (a) : (b) )
#endif


void display_real_time(struct timespec *before, struct timespec *after){
    time_t real_time =
            (after->tv_sec - before->tv_sec) * 1000000000 +
            (after->tv_nsec - before->tv_nsec);

    printf("Elapsed real time: %ld ns \r\n", real_time);
}

void display_system_time(struct rusage *before, struct rusage *after) {
    time_t user_time =
            (after->ru_utime.tv_sec - before->ru_utime.tv_sec) * 1000000 +
            after->ru_utime.tv_usec - before->ru_utime.tv_usec;
    time_t system_time =
            (after->ru_stime.tv_sec - before->ru_stime.tv_sec) * 1000000 +
            after->ru_stime.tv_usec - before->ru_stime.tv_usec;
    printf("Elapsed user time: %ld µs \r\n"
           "Elapsed system time: %ld µs \r\n\r\n",
           user_time, system_time);
}

#endif //ZEST8_APPLICATION_HELPER_H
