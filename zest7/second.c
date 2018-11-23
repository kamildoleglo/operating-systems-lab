#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <assert.h>

int main(int argc, char **argv) {

    int oflags=O_RDWR;
    int i;

    char   *name   = "/test.123";
    int     fd     = shm_open(name, oflags, 0666 );

    printf("Shared memory descriptor: fd=%d \r\n", fd);

    assert (fd>0);

    struct stat sb;

    fstat(fd, &sb);
    off_t length = sb.st_size ;

    u_char *ptr = (u_char *) mmap(NULL, length, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    printf("Shared memory address: %p length: [0..%lu] \r\n", ptr, length-1);
    assert (ptr);

    printf("Memory contents: \r\n");
    for(i=0; i<100; i++)
        printf("%c", ptr[i]);

    ptr[0] = 'H';
    printf("Changing 'h' to 'H' \r\n");

    close(fd);
    exit(0);
}