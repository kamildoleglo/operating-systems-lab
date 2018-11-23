#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <zconf.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>


volatile pid_t pid, parent;
int L, Type;
volatile bool got_signal = false;
volatile int signals_got = 0;


const char tears_in_rain[] = "I've seen things you people wouldn't believe. \r\n"
                             "Attack ships on fire off the shoulder of Orion. \r\n"
                             "I watched C-beams glitter in the dark near the Tannhäuser Gate. \r\n"
                             "All those moments will be lost in time, like tears in rain. Time to die.\r\n";


void sigusr_handler(int signo, siginfo_t *info, void *context);
void sigint_handler(int signo, siginfo_t *info, void *context);
void sigrt_handler(int signum, siginfo_t *info, void *context);
void sigchld_handler(int signo, siginfo_t *info, void *context);

void mode1(void);
void mode2(void);
void mode3(void);

int main(int argc, char **argv) {
    if(argc < 3) exit(EXIT_FAILURE);
    L = atoi(argv[1]);
    Type = atoi(argv[2]);
    parent = getpid();

    sigset_t mask;
    sigfillset(&mask);

    struct sigaction sigint, sigusr, sigchld, sigrt;
    sigint.sa_flags = SA_SIGINFO;
    sigint.sa_sigaction = sigint_handler;
    sigusr.sa_flags = SA_SIGINFO;
    sigusr.sa_sigaction = sigusr_handler;
    sigchld.sa_flags = SA_SIGINFO;
    sigchld.sa_sigaction = sigchld_handler;
    sigrt.sa_flags = SA_SIGINFO;
    sigrt.sa_sigaction = sigrt_handler;

    sigaction(SIGINT, &sigint, NULL);
    sigdelset(&mask, SIGINT);
    sigaction(SIGUSR1, &sigusr, NULL);
    sigdelset(&mask, SIGUSR1);
    sigaction(SIGUSR2, &sigusr, NULL);
    sigdelset(&mask, SIGUSR2);
    sigaction(SIGCHLD, &sigchld, NULL);
    sigdelset(&mask, SIGCHLD);
    sigaction(SIGRTMIN, &sigrt, NULL);
    sigdelset(&mask, SIGRTMIN);
    sigaction(SIGRTMIN+1, &sigrt, NULL);
    sigdelset(&mask, SIGRTMIN+1);

    pid = fork();
    if (pid > 0) {

    } else if (pid == 0) {
        write(1, "Good morning, Vietnam!\r\n", 24);
        while(true) sigsuspend(&mask);
        return 0;
    }else{
        printf("Cannot fork\r\n");
        exit(EXIT_FAILURE);
    }

    switch (Type){
        case 1:
            mode1();
            break;
        case 2:
            mode2();
            break;
        case 3:
            mode3();
            break;
    }

    while(true);

    return 0;
}

void mode1(void){
    for(int i = 0 ; i < L; i++){
        printf("Sending %i SIGUSR1 \r\n", i);
        kill(pid, SIGUSR1);
    }
    printf("Sending SIGUSR2\r\n");
    kill(pid, SIGUSR2);
}

void mode2(void){
    for(int i = 0 ; i < L; i++){
        printf("Sending %i SIGUSR1 \r\n", i);
        kill(pid, SIGUSR1);
        if(!got_signal) pause();
        got_signal = false;
    }
    printf("Sending SIGUSR2\r\n");
    kill(pid, SIGUSR2);
}

void mode3(void){
    for(int i = 0 ; i < L; i++){
        printf("Sending %i SIGRT+%d \r\n", i, SIGRTMIN);
        kill(pid, SIGRTMIN);
        if(!got_signal) pause();
        got_signal = false;
    }
    printf("Sending SIGRT+%d\r\n", SIGRTMIN+1);
    kill(pid, SIGRTMIN+1);
}

void sigrt_handler(int signo, siginfo_t *info, void *context){
    if(signo == SIGRTMIN){
        if(getpid() == parent) {
            write(1, "I am your father \r\n", 19);
            signals_got++;
            got_signal = true;
            return;
        }
        signals_got++;
        kill(info->si_pid, SIGRTMIN);
    }else{
        printf("Signals got = %d\r\n", signals_got);
        printf(tears_in_rain);
        exit(EXIT_SUCCESS);
    }

}



void sigusr_handler(int signo, siginfo_t *info, void *context){
    if(signo == SIGUSR2){
        printf("Signals got = %d\r\n", signals_got);
        printf(tears_in_rain); //write(1, tears_in_rain, 239);
        exit(EXIT_SUCCESS);
    }
    if(getpid() == parent) {
        write(1, "I am your father \r\n", 19);
        signals_got++;
        got_signal = true;
        return;
    }
    signals_got++;
    kill(info->si_pid, SIGUSR1);
}

void sigchld_handler(int signo, siginfo_t *info, void *context){
    printf("Parent signals got = %d\r\n", signals_got);
    printf("Exiting...\r\n");
    exit(EXIT_SUCCESS);
}

void sigint_handler(int signo, siginfo_t *info, void *context){
    kill(pid, SIGUSR2);
    printf("Exiting\r\n");
    exit(EXIT_SUCCESS);
}
