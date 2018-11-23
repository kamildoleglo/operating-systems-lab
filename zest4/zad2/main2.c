#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <zconf.h>
#include <signal.h>
#ifdef __linux
#include <wait.h>
#endif

// Ugly way to make CLion color syntax on MacOS X (i.e. don't try to run it there!)
#ifndef __linux__
#define SIGRTMIN 1
#define SIGRTMAX 8
#endif

#define N 3
#define K 2
#define SIGDIFF (SIGRTMAX - SIGRTMIN)

#define DECLARE_SIGACT                                                         \
    struct sigaction sig_act;                                                  \
    sig_act.sa_flags = SA_SIGINFO;                                             \

#define REG_HANDLER(signal, function) {                                        \
    sig_act.sa_sigaction = &(function);                                        \
    sigaction((signal), &sig_act, 0);                                          \
}

volatile pid_t children[N];
volatile pid_t requests[N];
volatile pid_t parent;

volatile sig_atomic_t signals_received = 0;
volatile sig_atomic_t alive_children = 0;
volatile sig_atomic_t exit_flag = 0;

void terminate_children();

// HANDLE SIGNALS //////////////////////////////////////////////////////////////

void sigrt_action(int, siginfo_t*, void*);
void handle_sigint(int, siginfo_t*, void*);
void handle_requests(int, siginfo_t*, void*);
void handle_sigchld(int, siginfo_t*, void*);
void handle_sigusr(int, siginfo_t*, void*);

void sigrt_action(int signum, siginfo_t *siginfo, void *_context) {
    if (getpid() != parent) return;
    printf(
            "SIGMIN+%i, from child pid: %i\n",
            signum - SIGRTMIN,
            siginfo->si_pid
    );
}

void handle_sigint(int _signum, siginfo_t *_siginfo, void *_context) {
    if (getpid() != parent) return;
    /**/ printf("SIGINT to parent\n");
    terminate_children();
    exit_flag = 1;
}

void handle_requests(int _signum, siginfo_t *siginfo, void *_context) {
    if (getpid() != parent) return;

    /**/ printf("SIGUSR1 from %i\n", siginfo->si_pid);
    signals_received++;
    if (signals_received < K) {
        requests[signals_received] = siginfo->si_pid;
    } else if (signals_received == K) {
        requests[signals_received] = siginfo->si_pid;
        int i; for (i = 0; i < K; ++i) kill(requests[i], SIGUSR1);
    } else if (signals_received > K) {
        kill(siginfo->si_pid, SIGUSR1);
    }
}

void handle_sigchld(int _signum, siginfo_t *siginfo, void *_context) {
    if (getpid() != parent) return;
    /**/ printf("%i: stopped\n", siginfo->si_pid);

    int i; for (i = 0; i < N; ++i) {
        if (children[i] == siginfo->si_pid) children[i] = 0;
    }

    int exit_status = 0;
    pid_t child_pid = 0;
    while (1) {
        child_pid = wait(&exit_status);
        if (!WIFEXITED(exit_status) || child_pid <= 0) return;

        alive_children--;
        printf(
                "child %i exited with status %i\n",
                siginfo->si_pid,
                WEXITSTATUS(exit_status)
        );
    }
}

void handle_sigusr(int _signum, siginfo_t *siginfo, void *_context) {
    int sig_diff = rand() % SIGDIFF;
    /**/ printf(
            "%i: received SIGUSR1 and sending SIRTMIN+%i\n",
            getpid(),
            sig_diff
    );
    kill(getppid(), SIGRTMIN + sig_diff);
}

// UTILITY FUNCTIONS ///////////////////////////////////////////////////////////

void terminate_children() {
    /**/ printf("terminating children; alive left: %i.\n", alive_children);
    int i; for (i = 0; i < N; ++i) {
        if (children[i]) {
            kill(children[i], SIGINT);
            children[i] = 0;
            alive_children--;
        }
    }
}

void set_mask() {
    sigset_t used_signals;
    sigemptyset(&used_signals);
    sigaddset(&used_signals, SIGUSR1);
    sigaddset(&used_signals, SIGCHLD);
    sigaddset(&used_signals, SIGINT);
    int i; for (i = SIGRTMIN; i <= SIGRTMAX; ++i) sigaddset(&used_signals, i);
}

void register_parent_signals() {
    DECLARE_SIGACT
    sig_act.sa_sigaction = &sigrt_action;
    int i; for (i = 0; i < SIGDIFF; ++i) sigaction(SIGRTMIN + i, &sig_act, 0);

    REG_HANDLER(SIGINT, handle_sigint);
    REG_HANDLER(SIGUSR1, handle_requests);
    REG_HANDLER(SIGCHLD, handle_sigchld);
}

int run_child() {
    DECLARE_SIGACT
    REG_HANDLER(SIGUSR1, handle_sigusr);
    unsigned int sleep_time = (unsigned int)(rand() % 10);
    sleep(sleep_time);
    /**/ printf("%i: sending SIGUSR1 to parent\n", getpid());
    kill(getppid(), SIGUSR1);
    pause();
    /**/ printf("%i: stopping with status %i\n", getpid(), sleep_time);
    return sleep_time;
}

// MAIN ////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv) {
    srand(time(0));
    pid_t pid;
    parent = getpid();

    set_mask();
    register_parent_signals();

    int i; for (i = 0; i < N; ++i) {
        pid = fork();
        alive_children++;
        if (pid) {
            children[i] = pid;
            printf("Created child with PID %i\n", pid);
        } else {
            return run_child();
        }
    }

    /**/ printf("Executing parent loop\n");
    while (alive_children > 0) if (exit_flag) terminate_children();
    return 0;
}