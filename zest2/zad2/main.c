#define _XOPEN_SOURCE 500
#define _DEFAULT_SOURCE
#include <stdio.h>
#include <dirent.h>
#include <ftw.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define NOPENFD 100

#define ARGS_CHECK(argc, req) if((argc) < (req)) { \
    printf("Too few arguments, exiting \r\n"); \
    exit(EXIT_FAILURE); \
}


bool before(time_t ref, time_t actual){
    return difftime(actual, ref) < 0.001;
}

bool after(time_t ref, time_t actual){
    return difftime(ref, actual) < 0.001;
}

bool equal(time_t ref, time_t actual){
    return fabs(difftime(ref, actual)) < 0.001;
}

char *get_permissions(long int mode) {
    char* permissions = malloc(10);
    permissions = strcpy(permissions, "---------\0");

    //User permissions
    if (mode & S_IRUSR) permissions[0] = 'r';
    if (mode & S_IWUSR) permissions[1] = 'w';
    if (mode & S_IXUSR) permissions[2] = 'x';

    //Group permissions
    if (mode & S_IRGRP) permissions[3] = 'r';
    if (mode & S_IWGRP) permissions[4] = 'w';
    if (mode & S_IXGRP) permissions[5] = 'x';

    //Others permissions
    if (mode & S_IROTH) permissions[6] = 'r';
    if (mode & S_IWOTH) permissions[7] = 'w';
    if (mode & S_IXOTH) permissions[8] = 'x';

    return permissions;
}

time_t TIME;
bool (*COMPARE)(time_t ref, time_t actual);

//NFTW =====================================================================================
int fn(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf){
    if(typeflag != FTW_F) return 0;
    if(!COMPARE(TIME, sb->st_mtime)) return(0);

    char *time = ctime(&sb->st_mtime);
    char *pos = strchr(time, '\n');
    if(pos != NULL) *pos = '\0';


    char *abs_path = realpath(fpath, NULL);
    char *permissions = get_permissions(sb->st_mode);

    printf("%10s %6ld %24s %s \r\n", permissions, sb->st_size, time, abs_path);

    free(permissions);
    free(abs_path);
    return 0;
}


void tree_traverse_nftw(char *dirpath){
    int flag = nftw(dirpath, &fn, NOPENFD, FTW_PHYS);
    if(flag != 0) {
        printf("Error while traversing tree, exiting \r\n");
        exit(EXIT_FAILURE);
    }
}


//SYS =====================================================================================
void traverse(char *abs_dirpath){
    DIR *current = opendir(abs_dirpath);
    if (current == NULL) {
        printf("Cannot open directory: %s \r\n", abs_dirpath);
        return;
    }

    struct dirent *entry;
    while((entry = readdir(current)) != NULL){
        if (strcmp(entry->d_name, ".")  == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char *current_abs_path = malloc(strlen(abs_dirpath) + 2 + strlen(entry->d_name));
        strcpy(current_abs_path, abs_dirpath);
        strcpy(current_abs_path + strlen(abs_dirpath), "/");
        strcpy(current_abs_path + strlen(abs_dirpath) + 1, entry->d_name);
        strcpy(current_abs_path + strlen(abs_dirpath) + 1 + strlen(entry->d_name), "\0");

        if (entry->d_type == DT_DIR) {
            traverse(current_abs_path);
            free(current_abs_path);
            continue;
        } else if (entry->d_type == DT_REG) {

            struct stat *sb = malloc(sizeof(struct stat));

            int flag = stat(current_abs_path, sb);
            if (flag != 0) {
                printf("Cannot read file attributes \r\n");
                return;
            }
            if (!COMPARE(TIME, sb->st_mtime)) {
                free(current_abs_path);
                free(sb);
                continue;
            }

            char *time = ctime(&sb->st_mtime);
            char *pos = strchr(time, '\n');
            if (pos != NULL) *pos = '\0';

            char *permissions = get_permissions(sb->st_mode);
            printf("%10s %6ld %24s %s \r\n", permissions, sb->st_size, time, current_abs_path);

            free(current_abs_path);
            free(permissions);
            free(sb);
        }
    }
    closedir(current);
}

void tree_traverse_sys(char *dirpath){
    char *abs_path = realpath(dirpath, NULL);

    traverse(abs_path);
    free(abs_path);
}


int main(int argc, char **argv) {
    ARGS_CHECK(argc, 4);
    DIR* start;
    if ((start = opendir(argv[1])) == NULL) {
        printf("Cannot open directory \r\n");
        exit(EXIT_FAILURE);
    }
    closedir(start);

    if(strcmp("<", argv[2]) == 0){
        COMPARE = &before;
    } else if(strcmp("=", argv[2]) == 0){
        COMPARE = &equal;
    } else if(strcmp(">", argv[2]) == 0){
        COMPARE = &after;
    } else {
        printf("Wrong operator %s", argv[2]);
        exit(EXIT_FAILURE);
    }


    //Parse time in format dd-mm-YYYY HH:MM:SS

    char *datetime = calloc(20, sizeof(char));
    strcpy(datetime, argv[3]);
    strcat(datetime, " ");
    strcat(datetime, argv[4]);
    struct tm tm_time;
    char *not_processed = strptime(datetime, "%d-%m-%Y %H:%M:%S", &tm_time);
    if(*not_processed != '\0'){
        printf("Bad date format \r\n");
        exit(EXIT_FAILURE);
    }
    TIME = mktime(&tm_time);
    free(datetime);


    if(argc > 5 && strcmp(argv[5], "sys") == 0){
        tree_traverse_sys(argv[1]);
    }else{
        tree_traverse_nftw(argv[1]);
    }

    return 0;
}
