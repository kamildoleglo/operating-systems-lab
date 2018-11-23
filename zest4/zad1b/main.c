#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <zconf.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>

bool sleeping = false;
pid_t pid;

void catcher(int signo);
void int_catcher(int sigint);
void conceive_a_child(void);

int main(void) {
    time_t rawtime;
    struct tm * timeinfo;
    struct sigaction new_action;
    new_action.sa_handler = int_catcher;
    sigemptyset (&new_action.sa_mask);
    new_action.sa_flags = 0;
    signal(SIGTSTP, &catcher);
    sigaction(SIGINT, &new_action, NULL);

    conceive_a_child();
    while(true);

    return 0;
}

void conceive_a_child(void){
    pid = fork();
    if (pid == 0) {
        execlp("./date.sh", "./date.sh", NULL);
        exit(EXIT_SUCCESS);
    }
}

void int_catcher(int sigint){
    printf("Odebrano sygnał SIGINT \r\n");
    if(pid != 0) kill(pid, SIGTERM);
    exit(EXIT_SUCCESS);
}

void catcher(int signo){
    if(sleeping) {
        sleeping = false;
        conceive_a_child();
        return;
    } else {
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTRL+C - zakonczenie programu\r\n");
        sleeping = true;
        kill(pid, SIGTERM);
        pid = 0;

        sigset_t myset;
        (void) sigemptyset(&myset);

        while (sleeping) {
            (void) sigsuspend(&myset);
        }
    }
}