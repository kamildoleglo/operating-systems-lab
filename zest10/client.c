//
// Created by kamil on 06.06.18.
//


#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "application_helper.h"

int main(int argc, char** argv) {
    int sockfd = 0, n = 0;
    char recvBuff[1024];
    struct sockaddr_in6 serv_addr;

    if(argc != 2)
        FAILURE_EXIT("Usage: %s <server ip> \r\n", argv[0]);

    if((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0)
        FAILURE_EXIT("Could not create socket \r\n");

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_port = htons(5000);

    if(inet_pton(AF_INET6, argv[1], &serv_addr.sin6_addr) <= 0)
        FAILURE_EXIT("inet_pton error occured \r\n");

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        FAILURE_EXIT("Connection failed \r\n");


    while ((n = read(sockfd, recvBuff, sizeof(recvBuff) - 1)) > 0) {
        recvBuff[n] = 0;
        if (fputs(recvBuff, stdout) == EOF)
            printf("Error : Fputs error \r\n");
    }

    if (n < 0)
        printf("Read error \r\n");

    shutdown(sockfd, SHUT_RDWR);
    close(sockfd);

    return 0;
}