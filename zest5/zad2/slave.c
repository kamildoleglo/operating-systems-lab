//
// Created by kamil on 17.04.18.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <zconf.h>
#include <time.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 512

int main(int argc, char **argv){
    if(argc < 3){
        printf("Too few arguments. Usage: slave <path> <count>\r\n");
        exit(EXIT_FAILURE);
    }
    int fifo = open(argv[1], O_WRONLY);
    if (fifo < 0) {
        printf ("Error opening fifo\r\n");
        exit(EXIT_FAILURE);
    }

    printf("PID: %i\r\n", getpid());

    int n = atoi(argv[2]);
    srand((unsigned int)time(NULL));

    FILE *date;
    char date_buffer[sizeof(char)*40];
    char buffer[BUFFER_SIZE];

    for(int i = 0; i < n; i++){
        date = popen("date", "r");
        fgets(date_buffer, sizeof(char)*40, date);
        fclose(date);

        sprintf(buffer, "%i - %s", getpid(), date_buffer);
        write(fifo, buffer, strlen(buffer));
        sleep((unsigned int)(rand() % 4 + 2));
    }
    close(fifo);

    return 0;
}