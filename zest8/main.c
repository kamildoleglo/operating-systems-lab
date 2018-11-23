#include <stdio.h>

#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>

#define NUMBER_OF_THREADS 200


FILE * text;
void *print_file() {

    char* buffer = calloc(sizeof(char), 200);
    fgets(buffer, 200, text);


    usleep(rand() % 111);

    printf("%s \n", buffer);

    free(buffer);
    return NULL;
}

int main(int argc, char** argv){
    srand(time(NULL));

    text = fopen("pan-tadeusz.txt", "r");

    pthread_t thread[NUMBER_OF_THREADS];

    /*creating all threads*/
    int i;
    for(i = 0; feof(text) == 0; i = (i+1) % NUMBER_OF_THREADS){
        int result = pthread_create(&thread[i], NULL, &print_file, NULL);

        if (result != 0)
            printf("Error creating thread %d. Return code: %d \r\n", i, result);
    }
    sleep(1);

    return 0;
}

