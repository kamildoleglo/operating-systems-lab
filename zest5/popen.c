#include <stdio.h>
#include <stdlib.h>


/*
 * FILE *popen(const char *command, const char *type);
 * int pclose(FILE *stream);
 */


int main(int argc, char **argv) {
    if(argc < 3) exit(EXIT_FAILURE);

    FILE *pipein_fp, *pipeout_fp;
    char readbuf[64];


    if (( pipein_fp = popen(argv[1], "r")) == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }


    if (( pipeout_fp = popen(argv[2], "w")) == NULL)
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }


    while(fgets(readbuf, 64, pipein_fp))
        fputs(readbuf, pipeout_fp);


    pclose(pipein_fp);
    pclose(pipeout_fp);

    return(0);
}
