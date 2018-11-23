//
// Created by kamil on 19.03.18.
//

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <memory.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/resource.h>
#include <time.h>

#define MALLOC_CHECK(ptr){ \
    if((ptr) == NULL){ \
        printf("Cannot allocate memory, exiting\r\n"); \
        exit(EXIT_FAILURE); \
    } \
}

#define FREAD_CHECK(read, size){ \
    if((read) < (size)){ \
        printf("Cannot read full record, trying to continue, read %zu out of %zu bytes \r\n", read, size); \
    } \
}

#define FWRITE_AND_WRITE_CHECK(written, size){ \
    if((written) < (size)){ \
        printf("Cannot write full record, trying to continue, written %zu out of %zu bytes \r\n", written, size); \
    } \
}

#define FOPEN_CHECK(ptr){ \
    if((ptr) == NULL){ \
        printf("Cannot open file, exiting\r\n"); \
    exit(EXIT_FAILURE); \
    } \
}

#define FSEEK_CHECK(seek_status){ \
    if((seek_status) != 0){ \
    printf("Cannot move within file, exiting\r\n"); \
    exit(EXIT_FAILURE); \
    } \
}

#define OPEN_CHECK(file_descriptor){ \
    if ((file_descriptor) < 0) { \
        printf("Cannot open file, exiting\r\n"); \
        exit(EXIT_FAILURE); \
    } \
}

#define LSEEK_CHECK(offset){ \
    if ((offset) < 0) { \
        printf("Cannot move within file, exiting\r\n"); \
        exit(EXIT_FAILURE); \
    } \
}

#define READ_CHECK(status){ \
    if ((status) < 0) { \
        printf("Cannot read file, exiting\r\n"); \
        exit(EXIT_FAILURE); \
    } \
}
#define RUSAGE_DIFF(r1, r2) \
    ((double) ((r1).tv_sec - (r2).tv_sec) + (double) ((r1).tv_nsec - (r2).tv_nsec)/(double)1E9)


#define TIMESPEC_DIFF(r1, r2) \
    ((double) ((r1).tv_sec - (r2).tv_sec) + (double) ((r1).tv_usec - (r2).tv_usec)/(double)1E6)


#define MEASURE(prefix, suffix, command, records, record_size){ \
    struct rusage usage_start, usage_end;\
    struct timespec real_start, real_end;\
    getrusage(RUSAGE_SELF, &usage_start);\
    clock_gettime(CLOCK_REALTIME, &real_start);\
    (command);\
    getrusage(RUSAGE_SELF, &usage_end);\
    clock_gettime(CLOCK_REALTIME, &real_end);\
    printf(prefix " %d blocks %zu bytes each using " suffix " functions\r\n", (records), (record_size));\
    printf("Real time = %lf \r\n", RUSAGE_DIFF(real_end, real_start));\
    printf("User time = %lf \r\n", TIMESPEC_DIFF(usage_end.ru_utime, usage_start.ru_utime));\
    printf("System time = %lf \r\n", TIMESPEC_DIFF(usage_end.ru_stime, usage_start.ru_stime)); \
}

#define ARGS_CHECK(argc, req) if((argc) < (req)) { \
    printf("Too few arguments, exiting \r\n"); \
    exit(EXIT_FAILURE); \
}


void generate(char *filename, int records, size_t record_size);
void sort(char *filename, int records, size_t record_size, bool sys);
void copy(char *from, char *to, int records, size_t buffer_size, bool sys);


int main(int argc, char **argv) {
    ARGS_CHECK(argc, 5);

    if (strcmp("generate", argv[1]) == 0) {

        generate(argv[2], atoi(argv[3]), (size_t) atoi(argv[4]));

    } else if (strcmp("sort", argv[1]) == 0) {

        if (argc > 5 && strcmp("sys", argv[5]) == 0) {
            MEASURE("Sort", "system", sort(argv[2], atoi(argv[3]), (size_t) atoi(argv[4]), true), atoi(argv[3]),
                    (size_t) atoi(argv[4]));
        } else {
            MEASURE("Sort", "library", sort(argv[2], atoi(argv[3]), (size_t) atoi(argv[4]), false), atoi(argv[3]),
                    (size_t) atoi(argv[4]));
        }

    } else if (strcmp("copy", argv[1]) == 0) {

        ARGS_CHECK(argc, 5);
        if (argc > 6 && strcmp("sys", argv[6]) == 0) {
            MEASURE("Copy", "system", copy(argv[2], argv[3], atoi(argv[4]), (size_t) atoi(argv[5]), true),
                    atoi(argv[4]), (size_t) atoi(argv[5]));
        } else {
            MEASURE("Copy", "library", copy(argv[2], argv[3], atoi(argv[4]), (size_t) atoi(argv[5]), false),
                    atoi(argv[4]), (size_t) atoi(argv[5]));
        }

    }

    return 0;
}

void generate(char *filename, int records, size_t record_size) {
    char command[128];
    sprintf(command, "head -c %zu < /dev/urandom > %s", records * record_size, filename);
    system(command);
}

void sort(char *filename, int records, size_t record_size, bool sys) {

    char *former_record = malloc(record_size);
    char *latter_record = malloc(record_size);
    MALLOC_CHECK(former_record);
    MALLOC_CHECK(latter_record);

    FILE *file_handler = NULL;
    int file_descriptor = 0;
    ssize_t former = 0, latter = 0;
    off_t offset = 0;

    if (sys) {
        file_descriptor = open(filename, O_RDWR);
        OPEN_CHECK(file_descriptor);
    } else {
        file_handler = fopen(filename, "r+");
        FOPEN_CHECK(file_handler);
    }

    for (size_t i = record_size; i < records * record_size; i += record_size) {
        size_t j = i;
        while (true) {
            if (j < record_size) break;

            //set position in file
            if (sys) {
                offset = lseek(file_descriptor, j - record_size, SEEK_SET);
                LSEEK_CHECK(offset);
                former = read(file_descriptor, former_record, record_size);
                latter = read(file_descriptor, latter_record, record_size);
                READ_CHECK(former);
                READ_CHECK(latter);
            } else {
                int seek_status = fseek(file_handler, j - record_size, 0);
                FSEEK_CHECK(seek_status);

                former = fread(former_record, record_size, 1, file_handler);
                latter = fread(latter_record, record_size, 1, file_handler);
                FREAD_CHECK(former, (size_t) 1);
                FREAD_CHECK(latter, (size_t) 1);
            }

            if ((unsigned int) former_record[0] <= (unsigned int) latter_record[0]) break;

            //swap
            if (sys) {
                offset = lseek(file_descriptor, j - record_size, SEEK_SET);
                LSEEK_CHECK(offset);
                former = write(file_descriptor, latter_record, record_size);
                latter = write(file_descriptor, former_record, record_size);
            } else {
                int seek_status = fseek(file_handler, j - record_size, 0);
                FSEEK_CHECK(seek_status);
                former = fwrite(latter_record, record_size, 1, file_handler);
                latter = fwrite(former_record, record_size, 1, file_handler);
            }

            FWRITE_AND_WRITE_CHECK(former, (size_t) 1);
            FWRITE_AND_WRITE_CHECK(latter, (size_t) 1);
            j -= record_size;
        }

    }

    free(latter_record);
    free(former_record);

    if (sys) {
        close(file_descriptor);
    } else {
        fclose(file_handler);
    }

}

void copy(char *from, char *to, int records, size_t buffer_size, bool sys) {

    char *buffer = malloc(buffer_size);
    MALLOC_CHECK(buffer);

    FILE *from_handler = NULL, *to_handler = NULL;
    int from_descriptor = 0, to_descriptor = 0;
    ssize_t b_read = 0, b_written = 0;

    if (sys) {
        from_descriptor = open(from, O_RDONLY);
        to_descriptor = open(to, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        OPEN_CHECK(from_descriptor);
        OPEN_CHECK(to_descriptor);
        do {
            b_read = read(from_descriptor, buffer, buffer_size);
            b_written = write(to_descriptor, buffer, buffer_size);
            READ_CHECK(b_read);
            FWRITE_AND_WRITE_CHECK(b_written, buffer_size);
            records--;
        } while (b_read > 0 && records > 0);

        close(from_descriptor);
        close(to_descriptor);
    } else {
        from_handler = fopen(from, "r");
        to_handler = fopen(to, "w");
        FOPEN_CHECK(from_handler);
        FOPEN_CHECK(to_handler);
        do {
            b_read = fread(buffer, buffer_size, 1, from_handler);
            b_written = fwrite(buffer, buffer_size, 1, to_handler);
            FREAD_CHECK(b_read, (size_t) 1);
            FWRITE_AND_WRITE_CHECK(b_written, (size_t) 1);
            records--;
        } while (b_read > 0 && records > 0);

        fclose(from_handler);
        fclose(to_handler);
    }

    free(buffer);
}
