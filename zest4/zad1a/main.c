#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <zconf.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>

bool sleeping = false;

void catcher(int signo);
void int_catcher(int sigint);

int main(void) {
    time_t rawtime;
    struct tm * timeinfo;
    struct sigaction new_action;
    new_action.sa_handler = int_catcher;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    signal(SIGTSTP, &catcher);
    sigaction(SIGINT, &new_action, NULL);
/*
    while(true) {
        pid_t pid = fork();
        if (pid == 0) {
            execlp("date", "date", NULL);
            exit(EXIT_SUCCESS);
        }
    }
*/


    while(true){
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );
        printf ( "Current local time and date: %s \r\n", asctime (timeinfo) );
    }


    return 0;
}

void int_catcher(int sigint){
    printf("Odebrano sygnał SIGINT \r\n");
    exit(EXIT_SUCCESS);
}

void catcher(int signo){
    if(sleeping) {
        sleeping = false;
        return;
    } else {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\r\n");
        sleeping = true;

        sigset_t myset;
        (void) sigemptyset(&myset);

        while (sleeping) {
            (void) sigsuspend(&myset);
        }
    }
}