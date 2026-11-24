#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdio.h>

#include "vm.h"

#define NUM_FRAMES 128

typedef struct {
    int frame_of_page[PAGE_TABLE_ENTRIES];
    int page_of_frame[NUM_FRAMES];
    unsigned char *physical_memory;
    FILE *backing_store;

    replacement_algo_t algo;
    int fifo_next_victim;
    long clock;
    long last_used[NUM_FRAMES];

    int frames_used;
    long page_faults;
} page_table_t;

int page_table_init(page_table_t *pt, replacement_algo_t algo, const char *backing_store_path);
void page_table_destroy(page_table_t *pt);

int page_table_resolve_frame(page_table_t *pt, int page_number);

void page_table_touch(page_table_t *pt, int frame);

#endif