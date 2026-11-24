#ifndef TLB_H
#define TLB_H

#include "vm.h"

#define TLB_INVALID -1

typedef struct {
    int page_number;
    int frame_number;
} tlb_entry_t;

typedef struct {
    tlb_entry_t entries[TLB_ENTRIES];

    replacement_algo_t algo;
    int fifo_next_victim;

    long clock;
    long last_used[TLB_ENTRIES];

    int entries_used;
    long hits;
} tlb_t;

void tlb_init(tlb_t *tlb, replacement_algo_t algo);

int tlb_lookup(tlb_t *tlb, int page_number, int *out_frame);

int tlb_insert(tlb_t *tlb, int page_number, int frame_number);

#endif 