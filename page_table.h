#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include <stdio.h>

#include "vm.h"

typedef struct {
    int frame_of_page[PAGE_TABLE_ENTRIES]; 
    unsigned char *physical_memory;        
    int num_frames;
    int next_free_frame;
    FILE *backing_store;
    long page_faults;
} page_table_t;

int page_table_init(page_table_t *pt, int num_frames, const char *backing_store_path);

void page_table_destroy(page_table_t *pt);

int translate_address(page_table_t *pt, int virtual_address,
                       int *out_physical_address, int *out_value);

#endif
