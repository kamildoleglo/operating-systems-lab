#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <zconf.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>


const char tears_in_rain[] = "I've seen things you people wouldn't believe. \r\n"
                             "Attack ships on fire off the shoulder of Orion. \r\n"
                             "I watched C-beams glitter in the dark near the Tannhäuser Gate. \r\n"
                             "All those moments will be lost in time, like tears in rain. Time to die.\r\n\0";

volatile pid_t pid;
volatile pid_t *children, *requests;
volatile int received_signals = 0, alive_children = 0, K, N;
volatile pid_t parent;


void child_exit_handler(int signo, siginfo_t *info, void *context);
void sigusr_handler(int signo, siginfo_t *info, void *context);
void child_request_handler(int signo, siginfo_t *info, void *context);
void sigint_handler(int signo, siginfo_t *info, void *context);
void sigrt_handler(int signum, siginfo_t *info, void *context);



int main(int argc, char **argv) {
    if(argc < 3) exit(EXIT_FAILURE);

    srand((unsigned int)time(NULL));

    N = atoi(argv[1]);
    K = atoi(argv[2]);

    parent = getpid();

    struct sigaction sigint_h, sigusr1_h, sigusr2_h, sigchld_h, sigrt_h;
    sigint_h.sa_flags = SA_SIGINFO;
    sigusr1_h.sa_flags = SA_SIGINFO;
    sigusr2_h.sa_flags = SA_SIGINFO;
    sigchld_h.sa_flags = SA_SIGINFO;
    sigrt_h.sa_flags = SA_SIGINFO;

    sigint_h.sa_sigaction = sigint_handler;
    sigfillset(&sigint_h.sa_mask);
    sigusr1_h.sa_sigaction = child_request_handler;
    sigfillset(&sigusr1_h.sa_mask);
    sigusr2_h.sa_sigaction = sigusr_handler;
    sigfillset(&sigusr2_h.sa_mask);
    sigchld_h.sa_sigaction = child_exit_handler;
    sigfillset(&sigchld_h.sa_mask);
    sigrt_h.sa_sigaction = sigrt_handler;
    sigfillset(&sigrt_h.sa_mask);

    sigaction(SIGINT, &sigint_h, NULL);
    sigaction(SIGUSR1, &sigusr1_h, NULL);
    sigaction(SIGUSR2, &sigusr2_h, NULL);
    sigaction(SIGCHLD, &sigchld_h, NULL);

    for (int i = SIGRTMIN; i <= SIGRTMAX; i++) sigaction(i, &sigrt_h, NULL);


    sigset_t sigusr2_mask, sigusr1_mask, empty_mask;
    sigfillset(&sigusr2_mask);
    sigfillset(&sigusr1_mask);
    sigdelset(&sigusr2_mask, SIGUSR2);
    sigdelset(&sigusr1_mask, SIGUSR1);
    sigdelset(&sigusr1_mask, SIGCHLD);
    sigemptyset(&empty_mask);

    sigprocmask(SIG_SETMASK, &sigusr1_mask, NULL);

    children = calloc((size_t) N, sizeof(pid_t));
    requests = calloc((size_t) K + 1, sizeof(pid_t));

    
    for (int i = 0; i < N; i++) {
        pid = fork();
        if (pid != 0) {
            children[i] = pid;
            alive_children++;

        } else if (pid == 0) {
            sigprocmask(SIG_SETMASK, &empty_mask, NULL);
            unsigned int sleep_time = (unsigned int)(rand() % 10 + 1);
            sleep(sleep_time);
            /**/ printf("%i: Good morning, Vietnam!\n", getpid());
            kill(getppid(), SIGUSR1);
            /**/ printf("%i: Sent kill to %i\n", getpid(), getppid());
            sigsuspend(&sigusr2_mask);
            /**/ printf("%i: Dying with time %d\n", getpid(), sleep_time);
            return sleep_time;
        } else {
            printf("Cannot fork\r\n");
            exit(EXIT_FAILURE);
        }
    }
    sigprocmask(SIG_SETMASK, &empty_mask, NULL);

    while(true){
        sigsuspend(&empty_mask);
        if(alive_children < 1) sigint_handler(0, NULL, NULL);
    }

    return 0;
}

void child_exit_handler(int signo, siginfo_t *info, void *context){
    if (getpid() != parent) return;
    /**/ printf("It's too bad %i won't live! But then again, who does? %i\r\n", info->si_pid, info->si_status);
    alive_children--;
    printf("%d \r\n", alive_children);
    for (int i = 0; i < N; i++) {
        if (children[i] == info->si_pid) children[i] = 0;
    }
}

void sigusr_handler(int signo, siginfo_t *info, void *context){
    if (getpid() == parent) return;
    /**/ printf("%i: My parent has accepted me.\r\n", getpid());
    kill(getppid(), SIGRTMIN + (rand() % 32));
}

void child_request_handler(int signo, siginfo_t *info, void *context){
    if (getpid() != parent) return;
    /**/ printf("%i, I am your father. \r\n", info->si_pid);

    received_signals++;

    if (received_signals < K) {
        requests[received_signals] = info->si_pid;
    } else if (received_signals == K) {
        kill(info->si_pid, SIGUSR2);
        for (int i = 1; i <= K; i++) kill(requests[i], SIGUSR2);
    } else if (received_signals > K) {
        kill(info->si_pid, SIGUSR2);
    }

}

void sigint_handler(int signo, siginfo_t *info, void *context){
    if (getpid() != parent) return;
    /**/ printf("%i: Sigint handler!\n", getpid());

    if(alive_children > 0){
        for(int i = 0; i < N; i++) kill(children[i], SIGTERM);
    }
    printf("%s \r\n", tears_in_rain);
    exit(EXIT_SUCCESS);
}

void sigrt_handler(int signum, siginfo_t *info, void *context) {
    if (getpid() != parent) return;
    printf("Got signal SIGMIN+%i, from child pid: %i\n", signum - SIGRTMIN, info->si_pid);
}
