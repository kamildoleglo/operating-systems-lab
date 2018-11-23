//
// Created by kamil on 05.06.18.
//

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <zconf.h>
#include <signal.h>
#include <semaphore.h>

#include "application_helper.h"

#define MAX_FILENAME 512
#define MAX_LINE 128

char** Buffer;
sem_t *Buffer_sem;;

int Readers, Writers, Line_length, Ttl;
size_t Buffer_size;
bool Verbose = false, Eof = false, Finished = false;
short int Search_mode;
char Filename[MAX_FILENAME];
FILE* File;

int read_index = 0;
int write_index = 0;

pthread_t* Writers_threads;
pthread_t* Readers_threads;

void sig_handler(int signo) {
    printf("Received signal %s, cancelling threads \r\n", signo == SIGALRM ? "SIGALRM" : "SIGINT");
    for (int i = 0; i < Writers; i++)
        pthread_cancel(Writers_threads[i]);
    for (int i = 0; i < Readers; i++)
        pthread_cancel(Readers_threads[i]);
    exit(EXIT_SUCCESS);
}

void cleanup() {
    fclose(File);
}

void read_config_file(char *config_file_path) {
    FILE* config_file;
    if ((config_file = fopen(config_file_path, "r")) == NULL) FAILURE_EXIT("Opening config file failed");

    char search_mode;
    fscanf(config_file, "%d %d %zu %s %d %c %d %d",
           &Writers, &Readers, &Buffer_size, Filename, &Line_length, &search_mode, (int*) &Verbose, &Ttl);

    if(search_mode == '=')
        Search_mode = 0;
    else if(search_mode == '<')
        Search_mode = -1;
    else if(search_mode == '>')
        Search_mode = 1;
    else
        FAILURE_EXIT("Bad search mode \r\n");

    printf("CONFIGURATION \r\n P: %d \r\n K: %d \r\n N: %zu \r\n Filename: %s \r\n L: %d \r\n Search mode: %c \r\n "
           "Verbose: %d \r\n nk: %d \r\n",
           Writers, Readers, Buffer_size, Filename, Line_length, search_mode, Verbose, Ttl);

    fclose(config_file);
}

void initialize(){
    File = fopen(Filename, "r");
    if(File == NULL) FAILURE_EXIT("Cannot open file \r\n");
    Buffer = calloc(Buffer_size, sizeof(char*));
    Writers_threads = calloc((size_t) Writers, sizeof(pthread_t));
    Readers_threads = calloc((size_t) Readers, sizeof(pthread_t));
    Buffer_sem = malloc((Buffer_size + 3) * sizeof(sem_t));

    for (int i = 0; i < Buffer_size + 2; i++)
        sem_init(&Buffer_sem[i], 0, 1);
    sem_init(&Buffer_sem[Buffer_size+2], 0, (unsigned int) Buffer_size);

    if(Buffer == NULL || Writers_threads == NULL || Readers_threads == NULL)
        FAILURE_EXIT("Cannot allocate memory \r\n");

    signal(SIGINT, sig_handler);
    if (Ttl > 0) signal(SIGALRM, sig_handler);
    atexit(cleanup);
}

bool required_length(int length){
    if((Search_mode == 0 && length == Line_length) ||
       (Search_mode < 0 && length < Line_length) ||
       (Search_mode > 0 && length > Line_length))
        return true;
    return false;
}

void* reader(void* _){
    char* line;
    int index;
    while(true) {
        sem_wait(&Buffer_sem[Buffer_size + 1]);
        while (Buffer[read_index] == NULL) {
            sem_post(&Buffer_sem[Buffer_size + 1]);
            if(Finished){
                return NULL;
            }
            sem_wait(&Buffer_sem[Buffer_size + 1]);
        }

        index = read_index;
        read_index = (read_index + 1) % (int) Buffer_size;

        sem_wait(&Buffer_sem[index]);

        if (Verbose)
            printf("Reader[%ld]: obtained lock for buffer[%d]  \r\n", pthread_self(),
                   index);
        line = Buffer[index];
        Buffer[index] = NULL;
        if (Verbose)
            printf("Reader[%ld]: took line from buffer[%d] \r\n", pthread_self(),
                   index);

        sem_post(&Buffer_sem[Buffer_size + 2]);
        sem_post(&Buffer_sem[Buffer_size + 1]);
        sem_post(&Buffer_sem[index]);

        if(line == NULL) continue;

        if (Verbose)
            printf("Reader[%ld]: line at buffer[%d] with length %d \r\n", pthread_self(), index, (int) strlen(line));

        if (required_length((int) strlen(line)))
            printf("Reader[%ld]: found matching line at buffer[%d]: \r\n %s \r\n", pthread_self(), index, line);

        free(line);
    }
}

void* writer(void* _){
    char line[MAX_LINE];
    int index;
    while(true) {
        sem_wait(&Buffer_sem[Buffer_size]);
        while (Buffer[write_index] != NULL) {
            sem_post(&Buffer_sem[Buffer_size]);
            sem_wait(&Buffer_sem[Buffer_size]);
        }
        sem_wait(&Buffer_sem[Buffer_size + 2]);

        index = write_index;
        write_index = (write_index + 1) % (int) Buffer_size;


        sem_wait(&Buffer_sem[index]);
        sem_post(&Buffer_sem[Buffer_size]);
        if (Verbose)
            printf("Writer[%ld]: obtained lock for buffer[%d]  \r\n", pthread_self(),
                   index);

        if(fgets(line, MAX_LINE, File) == NULL || feof(File)) {
            if(Verbose) printf("Writer[%ld]: reached EOF, quitting \r\n", pthread_self());
            Finished = true;
            return NULL;
        }

        Buffer[index] = malloc((strlen(line) + 1) * sizeof(char));
        strcpy(Buffer[index], line);

        if (Verbose) printf("Writer[%ld]: copied line to buffer[%d] \r\n", pthread_self(), index);

        sem_post(&Buffer_sem[index]);
    }
}

void start_threads() {
    for (int i = 0; i < Writers; i++)
        pthread_create(&Writers_threads[i], NULL, writer, NULL);
    for (int i = 0; i < Readers; i++)
        pthread_create(&Readers_threads[i], NULL, reader, NULL);

    if (Ttl > 0) alarm(Ttl);
}

void join_threads(){
    for (int i = 0; i < Writers; i++)
        pthread_join(Writers_threads[i], NULL);
    
    for (int i = 0; i < Readers; i++)
        pthread_join(Readers_threads[i], NULL);
}

int main(int argc, char** argv){

    if(argc < 2) FAILURE_EXIT("Usage: ./main <config file> \r\n")

    read_config_file(argv[1]);
    initialize();
    start_threads();
    join_threads();

    exit(EXIT_SUCCESS);

}

