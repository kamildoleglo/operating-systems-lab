//
// Created by kamil on 27.03.18.
//

#include <stdlib.h>
#include <memory.h>

int main(void){
    const int times = 100;
    const int c = 1000;
    for(int i = 0; i < times; i++) {
        char *array = malloc(c * 1024 * 1024);
        memset(array, 0, c * 1024 * 1024);
    }
    return 0;
}