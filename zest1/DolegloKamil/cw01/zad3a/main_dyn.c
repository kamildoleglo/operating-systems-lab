//
// Created by kamil on 12.03.18.
//

#define testSize 100000
#define testBlock 1500
#define nOfUpdates 100

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/times.h>
#include <stdbool.h>
#include <unistd.h>
#include <dlfcn.h>

void rand_str(char *dest, size_t length);

void run_tests(void);

void print_result(char *name, struct timespec *real_time_diff, struct tms *system_time_diff);

void find_closest(int find, char **array, int size);

int linear_update(int linear, int mode, char **array, int size, int block);

int alternating_update(int alternating, int mode, char **array, int size, int block);

void fill_array(char **array, int size, int block);

FILE *open_file(void);

void print_result_to_file(FILE *f, char *name, struct timespec *real_time_diff, struct tms *system_time_diff);

void close_file(FILE *f);

int create_dynamic_array_with_blocks_wrapper(int size, int block);

int create_static_array_with_blocks_wrapper(int size, int block);
char **(*create_static_array_with_blocks)(int, int);
char **(*create_dynamic_array_with_blocks)(int, int);
char **(*create_dynamic_array)(int);
void (*create_dynamic_block)(int, int, char **);
void (*delete_dynamic_block)(int, char **);
void (*delete_static_block)(int, int, char **);
char *(*find_closest_block)(int, char **, int);
void (*delete_dynamic_array)(char **, int);
void (*delete_static_array)(char **);

int main(int argc, char *argv[]) {

    void *handle = dlopen("liblibrary.so", RTLD_LAZY);
    if(!handle){printf("Cannot open library"); exit(EXIT_FAILURE);}


    create_static_array_with_blocks = dlsym(handle, "create_static_array_with_blocks");
	if (!create_static_array_with_blocks) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    create_dynamic_array_with_blocks = dlsym(handle, "create_dynamic_array_with_blocks");
	if (!create_dynamic_array_with_blocks) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    create_dynamic_array = dlsym(handle, "create_dynamic_array");
	if (!create_dynamic_array) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    create_dynamic_block = dlsym(handle, "create_dynamic_block");
	if (!create_dynamic_block) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    delete_dynamic_block = dlsym(handle, "delete_dynamic_block");
	if (!delete_dynamic_block) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    delete_static_block = dlsym(handle, "delete_static_block");
	if (!delete_static_block) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    find_closest_block = dlsym(handle, "find_closest_block");
	if (!find_closest_block) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    delete_dynamic_array = dlsym(handle, "delete_dynamic_array");
	if (!delete_dynamic_array) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }
    delete_static_array = dlsym(handle, "delete_static_array");
	if (!delete_static_array) { fprintf(stderr, "dlopen failure: %s\n", dlerror()); 
           exit (EXIT_FAILURE); }




    srand((unsigned int) time(NULL));

    bool mode = false; //false for static, true for dynamic
    int opt;
    int size = 0, block = 0;
    int find = 0, linear = 0, alternating = 0;

    while ((opt = getopt(argc, argv, "n:b:m:f:l:a:")) != -1) {
        switch (opt) {
            case 'n':
                size = atoi(optarg);
                break;
            case 'b':
                block = atoi(optarg);
                break;
            case 'm':
                mode = (bool) atoi(optarg);
                break;
            case 'f':
                find = atoi(optarg);
                break;
            case 'l':
                linear = atoi(optarg);
                break;
            case 'a':
                alternating = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Usage: %s <-n arraySize> "\
                        "<-b blockSize> <-m mode> [-f nToFind] [-l linearDeallocAndAlloc] "\
                        "[-a alternatingDeallocAndAlloc]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }


    char **array = NULL;


    if (mode) {
        array = create_dynamic_array_with_blocks(size, block);
    } else {
        array = create_static_array_with_blocks(size, block);
    }

    fill_array(array, size, block);

    if (find) find_closest(find, array, size);

    if (linear) linear_update(linear, mode, array, size, block);

    if (alternating) alternating_update(alternating, mode, array, size, block);

    if (mode) {
        delete_dynamic_array(array, size);
    } else {
        delete_static_array(array);
    }

    run_tests();
    dlclose(handle);
    exit(EXIT_SUCCESS);
}

void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (size_t) ((double) rand() / RAND_MAX * (sizeof charset - 1));
        *dest++ = charset[index];
    }
    *dest = '\0';
}

void run_tests(void) {

    FILE *f = open_file();
    typedef void (*funcPtr)();
    char **static_a = create_static_array_with_blocks(testSize, testBlock);
    char **dynamic_a = create_dynamic_array_with_blocks(testSize, testBlock);
    fill_array(static_a, testSize, testBlock);
    fill_array(dynamic_a, testSize, testBlock);

    funcPtr func_call[8] = {
            (funcPtr) create_dynamic_array_with_blocks_wrapper(testSize, testBlock),
            (funcPtr) create_static_array_with_blocks_wrapper(testSize, testBlock),
            (funcPtr) find_closest_block(0, dynamic_a, testSize),
            (funcPtr) find_closest_block(0, static_a, testSize),
            (funcPtr) linear_update(nOfUpdates, true, dynamic_a, testSize, testBlock),
            (funcPtr) linear_update(nOfUpdates, false, static_a, testSize, testBlock),
            (funcPtr) alternating_update(nOfUpdates, true, dynamic_a, testSize, testBlock),
            (funcPtr) alternating_update(nOfUpdates, false, static_a, testSize, testBlock)
    };

    char *func_names[8] = {
            "creating dynamic array",
            "creating static array",
            "finding closest block in dynamic array",
            "finding closest block in static array",
            "linear deleting and creating dynamic blocks",
            "linear deleting and creating static blocks",
            "alternating deleting and creating dynamic blocks",
            "alternating deleting and creating static blocks",
    };
    struct timespec real_time_begin, real_time_end, real_time_diff;
    struct tms system_time_begin, system_time_end, system_time_diff;

    for (int i = 0; i < 8; i++) {
        times(&system_time_begin);
        clock_gettime(CLOCK_REALTIME, &real_time_begin);

        (void) func_call[i];

        times(&system_time_end);
        int flag = clock_gettime(CLOCK_REALTIME, &real_time_end);
        if (flag == -1) printf("Something went wrong");

        real_time_diff.tv_sec = real_time_end.tv_sec - real_time_begin.tv_sec;
        real_time_diff.tv_nsec = labs(real_time_end.tv_nsec - real_time_begin.tv_nsec);
        system_time_diff.tms_utime = system_time_end.tms_utime - system_time_begin.tms_utime;
        system_time_diff.tms_stime = system_time_end.tms_stime - system_time_begin.tms_stime;
        print_result(func_names[i], &real_time_diff, &system_time_diff);
        print_result_to_file(f, func_names[i], &real_time_diff, &system_time_diff);

    }

    delete_dynamic_array(dynamic_a, testSize);
    delete_static_array(static_a);
    close_file(f);

}

int create_dynamic_array_with_blocks_wrapper(int size, int block) {
    char **a = create_dynamic_array_with_blocks(size, block);
    delete_dynamic_array(a, size);
    return 0;
}

int create_static_array_with_blocks_wrapper(int size, int block) {
    char **a = create_static_array_with_blocks(size, block);
    delete_static_array(a);
    return 0;
}

void print_result(char *name, struct timespec *real_time_diff, struct tms *system_time_diff) {
    printf("Real time for %s = %ld.%ld \r\n", name, real_time_diff->tv_sec , real_time_diff->tv_nsec );
    printf("User time for %s = %f \r\n", name, (double) system_time_diff->tms_utime / CLOCKS_PER_SEC );
    printf("Kernel time for %s = %f \r\n", name, (double) system_time_diff->tms_stime / CLOCKS_PER_SEC );
    printf("\r\n");
}

void print_result_to_file(FILE *f, char *name, struct timespec *real_time_diff, struct tms *system_time_diff) {
    fprintf(f, "Real time for %s = %ld.%ld \r\n", name, real_time_diff->tv_sec, real_time_diff->tv_nsec );
    fprintf(f, "User time for %s = %f \r\n", name, (double) system_time_diff->tms_utime / CLOCKS_PER_SEC);
    fprintf(f, "Kernel time for %s = %f \r\n", name, (double) system_time_diff->tms_stime / CLOCKS_PER_SEC);
    fprintf(f, "\r\n");
}

FILE *open_file(void) {
    FILE *f = fopen("results.txt", "w");
    if (f == NULL) {
        printf("Error opening file!\n");
        exit(1);
    }
    fprintf(f, "Results of tests for arrays of %d blocks %d chars  \r\n\r\n", testSize, testBlock);
    return f;
}

void close_file(FILE *f) {
    fclose(f);
}

void find_closest(int find, char **array, int size) {
    printf("Reference block = %s \r\n", array[find]);
    char *closest = find_closest_block(find, array, size);
    printf("Closest block found = %s \r\n\r\n", closest);
}

int linear_update(int linear, int mode, char **array, int size, int block) {
    for (int i = 0; i < linear && i < size; i++) {
        if (mode) delete_dynamic_block(i, array);
        else delete_static_block(i, block, array);
    }

    for (int i = 0; i < linear && i < size; i++) {
        if (mode) create_dynamic_block(i, block, array);

        rand_str(array[i], (size_t) block - 1);
    }
    return linear < size ? linear : size;
}

int alternating_update(int alternating, int mode, char **array, int size, int block) {
    for (int i = 0; i < alternating && i < size; i++) {
        if (mode) {
            delete_dynamic_block(i, array);
            create_dynamic_block(i, block, array);
        } else {
            delete_static_block(i, block, array);
        }

        rand_str(array[i], (size_t) block - 1);
    }
    return alternating < size ? alternating : size;
}

void fill_array(char **array, int size, int block) {
    for (int i = 0; i < size; i++) {
        rand_str(array[i], (size_t) block - 1);
    }
}
