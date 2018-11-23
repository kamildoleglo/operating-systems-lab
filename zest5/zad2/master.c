//
// Created by kamil on 17.04.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <zconf.h>
#include <string.h>

#define CHARS 512

int main(int argc, char **argv){

    if(argc < 2){
        printf("Too few arguments. Usage: master <path>\r\n");
        exit(EXIT_FAILURE);
    }

    if(mkfifo(argv[1], 0666) < 0){
        printf("Cannot create FIFO\r\n");
        exit(EXIT_FAILURE);
    }


    FILE *fifo = fopen(argv[1], "r");
    if (fifo == NULL) {
        printf ("Error opening FIFO\r\n");
        exit(EXIT_FAILURE);
    }


    char buffer[CHARS];
    while (fgets(buffer, CHARS, fifo) != NULL) {
        printf("%s\r\n", buffer);
    }

    if (remove(argv[1]) != 0){
        printf("Error deleting FIFO\r\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}