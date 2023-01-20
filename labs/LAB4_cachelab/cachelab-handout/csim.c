/****************************************
 * Name: Haozhi Fan
 * Date: 2021/03/09
 * LAB4: Cache Lab
 ***************************************/

#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define SYSTEM_BITS 64

// cache line
typedef struct {
    unsigned valid, LRU_counter;
    unsigned long long tag;
} line, *line_ptr;

// search for empty slot in cache and insert data
int insert_data(line_ptr* lines, int E, unsigned long long tag) {
    for (int l = 0; l < E; l++) {
        if (!lines[l]->valid) {
            lines[l]->valid = 1;
            lines[l]->tag = tag;
            lines[l]->LRU_counter = 0;
            return 1;
        }
    }
    return 0;
}

// evict a line and replace with new data, given no empty space in cache
void replace_data(line_ptr* lines, int E, unsigned long long tag) {
    line_ptr eviction_line = lines[0];
    unsigned max_counter = eviction_line->LRU_counter;
    for (int l = 1; l < E; l++) {
        if (lines[l]->LRU_counter > max_counter) {
            max_counter = lines[l]->LRU_counter;
            eviction_line = lines[l];
        }
    }
    eviction_line->tag = tag;
    eviction_line->LRU_counter = 0;
}

// iterate through set to search for hit and increment LRU counter
int iterate_set(line_ptr* lines, int E, unsigned long long tag) {
    for (int l = 0; l < E; l++) {
        if (lines[l]->valid) {
            if (lines[l]->tag == tag) { 
                lines[l]->LRU_counter = 0;
                return 1;
            }
        }
    }
    return 0;
}

int simulate(char* fileName, int s, int E, int b, int* hit_count, 
                int* miss_count, int* eviction_count) {
    FILE * pFile = fopen(fileName, "r");
    char identifier;
    unsigned long long address;
    int size;
    int S = 1 << s;

    // declare cache as an 2D array of cache lines
    line_ptr cache[S][E];

    // initialize the cache with all lines having valid-bit = 0
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            cache[i][j] = (line_ptr) malloc(sizeof(line));
            cache[i][j]->valid = 0;
            cache[i][j]->LRU_counter = 0;
        }
    }

    // scan traces and do something
    while (fscanf(pFile, " %c %llx,%d", &identifier, &address, &size) > 0) {
        unsigned long long tag = address >> (b+s);
        int set_id = address << (SYSTEM_BITS-b-s) >> (SYSTEM_BITS-s);

        int hit;
        hit = iterate_set(cache[set_id], E, tag);

        // operation L, S and M have (partially) identical behavior due to
        // assumption for this simulator
        if (identifier == 'L' || identifier == 'S' || identifier == 'M') {
            if (hit) {                    // return cached data directly
                *hit_count += 1; 
            } 
            else { 
                *miss_count += 1;         // fetch data from lower level memory
                if (!insert_data(cache[set_id], E, tag)) {
                    replace_data(cache[set_id], E, tag);
                    *eviction_count += 1;   
                }
            }
        }

        // operation M is followed by another S operation, which must be a hit
        if (identifier == 'M') {
            hit = iterate_set(cache[set_id], E, tag);
            if (hit) { 
                *hit_count += 1; 
            }
            else { 
                *miss_count += 1; 
                 if (!insert_data(cache[set_id], E, tag)) {
                    replace_data(cache[set_id], E, tag);
                    *eviction_count += 1;   
                }
            }
        }

        // increment LRU_counter for all cached block at each iteration
        for (int i = 0; i < S; ++i) {
            for (int j = 0; j < E; ++j) {
                if (cache[i][j]->valid == 1)
                    cache[i][j]->LRU_counter++;
            }
        }
    }

    // free memory assigned for cache
    for (int i = 0; i < S; i++) {
        for (int j = 0; j < E; j++) {
            free(cache[i][j]);
        }
    }

    fclose(pFile);
    return 1;:q

}

int main(int argc, char** argv) {
    int opt, s, E, b;
    char* t;   
    // use getopt to get option and option arguments
    while ((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1) {
        switch(opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                t = optarg;
                break;
            default:
                printf("wrong argument");
                break;
        }
    }

    if (!t) {
        printf("use -t option to provide traces!\n");
        exit(-1);
    }

    int hit_count = 0, miss_count = 0, eviction_count = 0;
    simulate(t, s, E, b, &hit_count, &miss_count, &eviction_count);

    printSummary(hit_count, miss_count, eviction_count);
    return 0;
}
