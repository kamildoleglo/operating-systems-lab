#include <stdio.h>

#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>
#include <stdbool.h>

#define NUMBER_OF_THREADS 200
bool found = false;

bool try(char* buffer){
    if(buffer != NULL && buffer[0] == 'Z')
        return true;
    else
        return false;
}


FILE * text;
void *find_in_file() {

    char* buffer = calloc(sizeof(char), 200);
    fgets(buffer, 200, text);
    if(try(buffer)) {
        printf("%s \r\n", buffer);
       // found = true;
        pthread_cancel(pthread_self());
    }

    free(buffer);
    return NULL;
}


int main(int argc, char** argv){
    srand(time(NULL));

    text = fopen("pan-tadeusz.txt", "r");

    pthread_t thread[NUMBER_OF_THREADS] = {0};

    /*creating all threads*/
    int i;
    for(i = 0; feof(text) == 0; i = (i+1) % NUMBER_OF_THREADS){
        int result = pthread_create(&thread[i], NULL, &find_in_file, NULL);
        if(found)
            break;
        if (result != 0)
            printf("Error creating thread %d. Return code: %d \r\n", i, result);
    }
    if(found)
        for(; i >= 0; i--)
            if(thread[i] != 0) pthread_cancel(thread[i]);

    for(i = 0; i < NUMBER_OF_THREADS; i++)
        if(thread[i] != 0) pthread_join(thread[i], NULL);

    return 0;
}

