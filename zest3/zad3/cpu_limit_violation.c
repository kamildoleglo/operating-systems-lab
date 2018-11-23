//
// Created by kamil on 27.03.18.
//

#include <zconf.h>

int main(void){
    int sum = 0;
    for(int i = 0; i < INT_MAX; i++){
        sum += i / 3;
    }

    return 0;
}