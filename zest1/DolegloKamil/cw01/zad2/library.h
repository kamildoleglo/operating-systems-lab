#ifndef ZEST1_LIBRARY_H
#define ZEST1_LIBRARY_H

char **create_static_array_with_blocks(int size, int block_size);

char **create_dynamic_array_with_blocks(int size, int block_size);

char **create_dynamic_array(int size);

void create_dynamic_block(int position, int block_size, char **array);

void delete_dynamic_block(int position, char **array);

void delete_static_block(int position, int block_size, char **array);

char *find_closest_block(int block_int, char **array, int size);

void delete_dynamic_array(char **array, int size);

void delete_static_array(char **array);


#endif