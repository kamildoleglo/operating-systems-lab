//
// Created by kamil on 06.06.18.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <signal.h>

#include "application_helper.h"

int listenfd = 0;

void quit(int _){
    printf("Shutting down \r\n");
    shutdown(listenfd, SHUT_RDWR);
    close(listenfd);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, quit);
    int connfd = 0;
	struct sockaddr_in6 serv_addr;
/*
    struct sockaddr_in6 {
        sa_family_t     sin6_family;   // AF_INET6
        in_port_t       sin6_port;     // port number
        uint32_t        sin6_flowinfo; // IPv6 flow information
        struct in6_addr sin6_addr;     // IPv6 address
        uint32_t        sin6_scope_id; // Scope ID (new in 2.4)
    };
*/
	char sendBuff[1025];
	time_t ticks;

	listenfd = socket(AF_INET6, SOCK_STREAM, 0);
	memset(sendBuff, '0', sizeof(sendBuff));

	serv_addr.sin6_family = AF_INET6;
	serv_addr.sin6_addr = in6addr_loopback;
	serv_addr.sin6_port = htons(5000);


    if(bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0 )
	    FAILURE_EXIT("Bind failed \r\n");

	listen(listenfd, 10);

    while(1) {
        connfd = accept(listenfd, (struct sockaddr*) NULL, NULL);

        ticks = time(NULL);
        snprintf(sendBuff, sizeof(sendBuff), "%.24s\r\n", ctime(&ticks));
        write(connfd, sendBuff, strlen(sendBuff));
        shutdown(connfd, SHUT_RDWR);
        close(connfd);
    }

    return 0;
}
