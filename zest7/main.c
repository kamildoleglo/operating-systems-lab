#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include <assert.h>

int main(int argc, char **argv) {

    int oflags=O_RDWR;
    int opt;
    off_t   length = 2 * 1024;
    char   *name   = "/test.123";

    while ((opt = getopt(argc, argv, "cd")) != -1) {
        switch (opt) {
            case 'c':
                oflags = O_RDWR | O_CREAT;
                break;
            case 'd':
                shm_unlink(name);
                printf("Shared memory segment removed \r\n");
                exit(EXIT_SUCCESS);
            default:
                fprintf(stderr, "Usage: %s -[c|d]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    int fd = shm_open(name, oflags, 0666 );

    ftruncate(fd, length);

    printf("Shared memory descriptor: fd=%d \r\n", fd);

    assert (fd>0);

    u_char *ptr = (u_char *) mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    printf("Shared memory address: %p length: [0..%lu] \r\n", ptr, length-1);
    printf("Shared memory path: /dev/shm%s \r\n", name );
    assert (ptr);


    char *msg = "hello world!\n\0";
    printf("Writing %s to shared memory \r\n", msg);
    strcpy((char*)ptr,msg);


    close(fd);
    exit(0);
}