//
// Created by kamil on 16.04.18.
//

#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <memory.h>
#include <errno.h>
#include <wait.h>

#define WRITING 0
#define READING 1
#define REVERSE(n) (n) == 0 ? 1 : 0
#define CLOSE_PIPES() { close(pipes[i % 2][0]); close(pipes[i % 2][1]); }
#define FAILURE_EXIT(format, ...) { printf(format, ##__VA_ARGS__); free(line); fclose(file); exit(-1); }
#define SET_PIPES(mode) {                                                               \
    close(pipes[(i + (mode)) % 2][(mode)]);                                             \
    int status = dup2(pipes[(i + (mode)) % 2][REVERSE(mode)], REVERSE(mode));           \
    if (status < 0) FAILURE_EXIT("Error %s in %i\n", (mode) ? "reading" : "writing", i) }

int pipes[2][2];
FILE* file = 0; char* line = 0; size_t length;

char** line_to_args(char *line, char *delimeter) {
    size_t last_index = strlen(line) - 1;
    if (line[last_index] == '\n') line[last_index] = '\0';
    int size = 0; char** args = 0;
    char* p = strtok(line, delimeter);
    while (p) {
        args = realloc(args, sizeof(char*) * ++size);
        args[size - 1] = p;
        p = strtok(0, delimeter);
    }
    args = realloc(args, sizeof(char*) * (size + 1));
    args[size] = 0;
    return args;
}

int main(int argc, char** argv) {
    if (argc < 2) FAILURE_EXIT("Not enough arguments\n")
    file = fopen(argv[1], "r");
    if (file == 0) FAILURE_EXIT("Couldn't open file\n")

    while ((getline(&line, &length, file)) != -1) {
        char **commands = line_to_args(line, "|");
        int count; for (count = 0; commands[count] != 0; ++count);
        int i; for (i = 0; i < count; ++i) {
            if (i > 1) CLOSE_PIPES()
            if (pipe(pipes[i % 2])) FAILURE_EXIT("Could not pipe at %i\n", i)

            pid_t pid = fork();
            if (pid == 0) {
                char **args = line_to_args(commands[i], " ");
                if (i < count - 1) SET_PIPES(WRITING)
                if (i > 0)         SET_PIPES(READING)
                execvp(args[0], args);
            } else if (pid == -1) {
                FAILURE_EXIT("fork() failed\n")
            }
        }
        CLOSE_PIPES()
        while (wait(0)) if (errno != ECHILD) break;
    }
    free(line);
    fclose(file);
    return 0;
}