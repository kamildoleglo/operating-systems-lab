#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include <string.h>
#include <wait.h>
#include <sys/time.h>
#include <sys/resource.h>


#define TIMEVAL_DIFF(r1, r2) \
    ((double) ((r1).tv_sec - (r2).tv_sec) + (double) ((r1).tv_usec - (r2).tv_usec)/(double)1E6)

char **get_args(char *line);
void print_usage(char*, struct rusage, struct rusage);

int main(int argc, char **argv){

    if(argc < 3){
        printf("Too few arguments, exiting \r\n");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(argv[1], "r");
    if(file == NULL){
        printf("Cannot open file, exiting \r\n");
        exit(EXIT_FAILURE);
    }

    char* line; size_t length;
    int cpu_limit = atoi(argv[2]);
    int mem_limit = atoi(argv[3]);
    struct rlimit cpu, mem;

    if(cpu_limit <= 0 ){
        cpu.rlim_cur = RLIM_INFINITY;
        cpu.rlim_max = RLIM_INFINITY;
    }else{
        cpu.rlim_cur = cpu.rlim_max = (rlim_t) cpu_limit;
    }
    if(mem_limit <= 0 ){
        mem.rlim_cur = RLIM_INFINITY;
        mem.rlim_max = RLIM_INFINITY;
    }else{
        mem.rlim_cur = mem.rlim_max = (rlim_t) mem_limit * 1024 * 1024;
    }


    while ((getline(&line, &length, file)) != -1) {
        line[strlen(line)-1] = '\0'; //requires last blank line

        struct rusage before, after;
        char **args = get_args(line);

        getrusage(RUSAGE_CHILDREN, &before);
        pid_t pid = fork();
        if(pid == 0) {
            setrlimit(RLIMIT_CPU, &cpu);
            setrlimit(RLIMIT_AS, &mem);

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
        getrusage(RUSAGE_CHILDREN, &after);
        print_usage(line, before, after);
    }

    free(line);
    fclose(file);
    return 0;
}

void print_usage(char *line, struct rusage before, struct rusage after){
    printf("Executing %s \r\n", line);
    printf("User time = %lf \r\n", TIMEVAL_DIFF(after.ru_utime, before.ru_utime));
    printf("System time = %lf \r\n", TIMEVAL_DIFF(after.ru_stime, before.ru_stime));
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