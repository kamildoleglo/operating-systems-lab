#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/resource.h>

char **get_args(char *line);
void print_usage(char*, struct rusage, struct rusage);

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
   


    while ((getline(&line, &length, file)) != -1) {
  	line[strlen(line) -1] = '\0';
        char **args = get_args(line);

        pid_t pid = fork();
        if(pid == 0) {
            if (execvp(args[0], args) < 0){
                printf("Error while executing %s \r\n", line);
                exit(EXIT_FAILURE);
            }
            exit(EXIT_SUCCESS);
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (WEXITSTATUS(status)) {
                exit(EXIT_FAILURE);
            }
        }
	line = NULL; length = 0;
    }

    free(line);
    fclose(file);
    return 0;
}


char **get_args(char *line) {
    int size = 0;
    char **args = NULL;
    char *token = strtok(line, " ");
    while(token != NULL) {
        size++;
        args = realloc(args, sizeof (char*) * size);
        if (args == 0) exit(EXIT_FAILURE);
        args[size - 1] = token;
        token = strtok (NULL, " ");
    }
    size++;
    args = realloc(args, sizeof (char*) * size);
    args[size - 1] = '\0';
    return args;
}
