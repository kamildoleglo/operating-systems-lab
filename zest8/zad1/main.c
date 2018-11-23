#include <stdio.h>

#include <pthread.h>
#include <stdlib.h>
#include <zconf.h>
#include <math.h>
#include <string.h>

#include "application_helper.h"

unsigned char** Image, ** Result; //using unsigned char for size optimization
float** Filter;
int Width, Height, Px_max_val, Filter_matrix_order, Number_of_threads;

void load_image(char* filename){
    FILE* image_file = fopen(filename, "r");
    if(image_file == NULL) FAILURE_EXIT("Cannot open image file \r\n");

    char control_chars[2] = {0, 0};
    if(fscanf(image_file, "%s ", control_chars) != 1 || strcmp(control_chars, "P2") != 0)
        FAILURE_EXIT("Malformed image file \r\n");

    if(fscanf(image_file, "%d ", &Width) != 1 ||
        fscanf(image_file, "%d ", &Height) != 1 ||
        fscanf(image_file, "%d ", &Px_max_val) != 1)
        FAILURE_EXIT("Malformed image file\r\n");

    if(Width < 1 || Height < 1 || Px_max_val < 1)
        FAILURE_EXIT("Malformed image file \r\n");

    if(Px_max_val > UCHAR_MAX)
        FAILURE_EXIT("Max pixel value is too large \r\n");

    Image = malloc(Height * sizeof(unsigned char*));
    for(int i = 0; i < Height; i++)
        Image[i] = malloc(Width * sizeof(unsigned char));

    int val;
    for (int i = 0; i < Height; i++) {
        for (int j = 0; j < Width; j++) {
            fscanf(image_file, "%d ", &val);
            Image[i][j] = (unsigned char) val;
        }
    }
    fclose(image_file);

    // Allocate memory for result
    Result = malloc(Height * sizeof(unsigned char*));
    for(int i = 0; i < Height; i++)
        Result[i] = malloc(Width * sizeof(unsigned char));

}

void load_filter(char* filename){
    FILE* filter_file = fopen(filename, "r");
    if(filter_file == NULL) FAILURE_EXIT("Cannot open filter file \r\n");

    if(fscanf(filter_file, "%d ", &Filter_matrix_order) != 1 || Filter_matrix_order < 1)
        FAILURE_EXIT("Malformed filter file\r\n");

    Filter = malloc(Filter_matrix_order * sizeof(float*));
    for(int i = 0; i < Filter_matrix_order; i++)
        Filter[i] = malloc(Filter_matrix_order * sizeof(float));

    float val;
    for (int i = 0; i < Filter_matrix_order; i++) {
        for (int j = 0; j < Filter_matrix_order; j++) {
            fscanf(filter_file, "%f ", &val);
            Filter[i][j] = val;
        }
    }

    fclose(filter_file);
}

static inline float px_val(int x, int y){
    float result = 0;
    int c = (int) ceil(Filter_matrix_order/2);
    for(int i = 0; i < Filter_matrix_order; i++){
        for(int j = 0; j < Filter_matrix_order; j++){
            int i_y = min(Width - 1, max(0, x - c + i));
            int i_x = min(Height - 1, max(0, y - c + j));
            result += Image[i_x][i_y] * Filter[i][j];
        }
    }
    return result;
}

static inline void process_chunk(int from_x, int to_x, int from_y, int to_y){
    for(int i = from_y; i < to_y; i++){
        for(int j = from_x; j < to_x; j++){
            Result[i][j] = (unsigned char) roundf(px_val(j,i));
        }
    }
}

void* thread_job(void* arg){
    int thread_number = *((int*)arg);

    int from_y = Height * thread_number / Number_of_threads;
    int to_y = Height * (thread_number + 1) / Number_of_threads;

    process_chunk(0, Width, from_y, to_y);

    return NULL;
}

void save_image(char* filename){
    FILE *out_file = fopen(filename, "w");
    if (out_file == NULL)
        FAILURE_EXIT("Cannot create output file \r\n");

    fprintf(out_file, "P2\n%d %d\n%d\n", Width, Height, Px_max_val);
    for (int x = 0; x < Height; ++x) {
        for (int y = 0; y < Width; ++y)
            fprintf(out_file, "%d ", Result[x][y]);
        fprintf(out_file, "\n");
    }
    fclose(out_file);
}

void process_image(){
    pthread_t* thread = malloc(Number_of_threads * sizeof(pthread_t));

    for (int i = 0; i < Number_of_threads; i++) {
        int* arg = malloc(sizeof(int));
        *arg = i;
        pthread_create(&thread[i], NULL, thread_job, arg);
    }

    for (int i = 0; i < Number_of_threads; i++)
        pthread_join(thread[i], NULL);

}

void save_times(char* filename, struct timespec* before, struct timespec* after){
    FILE *times_file = fopen(filename, "a");
    if (times_file == NULL)
        FAILURE_EXIT("Cannot create or open times file \r\n");

    fprintf(times_file, "Filtering time using %d threads with filter matrix of order %d: \r\n",
            Number_of_threads, Filter_matrix_order);
    time_t real_time =
            (after->tv_sec - before->tv_sec) * 1000000000 +
            (after->tv_nsec - before->tv_nsec);
    fprintf(times_file, "%ld ns \r\n", real_time);

    fclose(times_file);
}

int main(int argc, char** argv){
    if(argc < 5) FAILURE_EXIT("Not enough arguments. Usage: ./main <number of threads> <image path> <filter path> <result path> \r\n");

    Number_of_threads = TO_INT(argv[1]);
    if(Number_of_threads < 1)
        FAILURE_EXIT("Bad number of threads \r\n");

    load_image(argv[2]);
    load_filter(argv[3]);

    struct timespec before, after;

    if(clock_gettime(CLOCK_REALTIME, &before) != 0)
        printf("Cannot start time measure \r\n");

    process_image();

    if(clock_gettime(CLOCK_REALTIME, &after) != 0)
        printf("Cannot end time measure \r\n");

    save_image(argv[4]);
    save_times("./Times.txt", &before, &after);

    return 0;
}

