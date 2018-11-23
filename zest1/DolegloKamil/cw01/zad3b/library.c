#include "library.h"

#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <stdlib.h>



char **create_static_array_with_blocks(int size, int block_size){
    char **ptr = (char**) calloc((size_t) size, sizeof(char*));
    ptr[0] = (char*) calloc((size_t) size * block_size, sizeof(char));

    size_t i;
    char* const data = ptr[0];
    for(i = 0; i < size; i++)
        ptr[i] = data + i * block_size;

    return ptr;
}


char **create_dynamic_array_with_blocks(int size, int block_size){
    char ** array = create_dynamic_array(size);
    for(int i = 0; i < size; i++){
        create_dynamic_block(i, block_size, array);
    }
    return array;
}

char **create_dynamic_array(int size) {
    return (char **) calloc((size_t) size, sizeof(char *));
}


void create_dynamic_block(int position, int block_size, char **array) {
    char * temp = (char*) calloc((size_t) block_size, sizeof(char));
    array[position] = temp;
}


void delete_dynamic_block(int position, char **array) {
    free(array[position]);
    array[position] = 0;
}

void delete_static_block(int position, int block_size, char **array){
    for(int i = 0; i < block_size; i++){
        array[position][i] = 0;
    }
}


char *find_closest_block(int block_int, char **array, int size){
    char *block = array[block_int];

    int sum = 0;
    for (int i = 0; i < strlen(block); i++){
        sum += block[i];
    }

    char *closest = NULL;
    int closest_sum = 0, temp_sum = 0;

    for (int i = 0; i < size; i++){
        if(i == block_int || array[i] == 0){
            continue;
        }

        for (int j = 0; j < strlen(array[i]); j++){
            temp_sum += array[i][j];
        }

        if(abs(temp_sum - sum) < abs(closest_sum - sum)){
            closest_sum = temp_sum;
            closest = array[i];
        }
        temp_sum = 0;
    }

    return closest;
}



void delete_dynamic_array(char **array, int size) {
    for(int i = 0; i < size; i++){
        if(array[i] != 0){
           free(array[i]);
        }
    }
    free(array);
}

void delete_static_array(char **array){
    free(array);
}



