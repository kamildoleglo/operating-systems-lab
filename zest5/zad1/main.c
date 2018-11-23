//
// Created by kamil on 15.04.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>
#include <wait.h>
#include <errno.h>
#include <fcntl.h>


#define IN 1
#define OUT 0
#define NOT(mode) (mode) == 0 ? 1 : 0
#define SET_PIPE(mode){ \
    close(pipes[(i + (mode)) % 2][(mode)]); \
    if(dup2(pipes[(i + (mode)) % 2][NOT(mode)], NOT(mode)) < 0){ \
        fprintf(stderr, "Error setting pipe \r\n"); \
        exit(EXIT_FAILURE); \
    }\
}



char **split_line(char *line, char *delimiter);

int main(int argc, char **argv){

    if(argc < 2){
        printf("Too few arguments, exiting \r\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Cannot open file, exiting \r\n");
        exit(EXIT_FAILURE);
    }

    char* line = NULL; size_t length = 0;
    int pipes[2][2];

    while ((getline(&line, &length, file)) != -1) {
        char **commands = split_line(line, "|");

        int commands_length = 0;
        for(; commands[commands_length] != '\0'; commands_length++);

        int i = 0;
        for(; i < commands_length; i++) {
            if (i > 1) {
                close(pipes[i % 2][0]);
                close(pipes[i % 2][1]);
            }//close pipe if opened

            if (pipe(pipes[i % 2]) != 0) { //open new pipe
                printf("Error creating pipe\r\n");
                exit(EXIT_FAILURE);
            }

            pid_t pid = fork();

            if (pid == 0) {
                char **args = split_line(commands[i], " ");

                if (i > 0) SET_PIPE(IN)
                if (i < commands_length - 1) SET_PIPE(OUT)

                for(int j = 0; args[j] != '\0'; j++){
                    if(strcmp(args[j], ">") == 0){
                        int fd = open(args[j+1], O_WRONLY | O_CREAT);
                        if(fd > 0) dup2(fd, STDOUT_FILENO);
                        strcpy(args[j], " ");
                        strcpy(args[j+1], " ");
                    }
                }


                if (execvp(args[0], args) < 0) {
                    fprintf(stderr, "Error while executing %s \r\n", line);
                    exit(EXIT_FAILURE);
                }
            } else if (pid == -1) {
                printf("Fork failed\r\n");
                exit(EXIT_FAILURE);
            }
        }
        /*
            printf("Waiting for %i \r\n", pid);
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status)) {
                printf("Command `%s` exited with nonzero value \r\n", commands[i]);
                exit(EXIT_FAILURE);
            }
            printf("Ended\r\n");
            */
        close(pipes[i % 2][0]); close(pipes[i % 2][1]);
        while (wait(0)) if (errno != ECHILD) break;
        free(commands);
        line = NULL; length = 0;
    }

    free(line);
    fclose(file);
    return 0;
}


char **split_line(char *line, char *delimiter) {
    for(size_t i = strlen(line) - 1; line[i] == '\r' || line[i] == '\n'; i++){
        line[i] = '\0';
    }
    int size = 0;
    char **args = NULL;
    char *token = strtok(line, delimiter);
    while(token != NULL) {
        size++;
        args = realloc(args, sizeof (char*) * size);
        if (args == 0) exit(EXIT_FAILURE);
        args[size - 1] = token;
        token = strtok (NULL, delimiter);
    }
    size++;
    args = realloc(args, sizeof (char*) * size);
    args[size - 1] = '\0';
    return args;
}
