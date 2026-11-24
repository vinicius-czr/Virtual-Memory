#include <string.h>
#include <pthread.h>

#include "tlb.h"

void tlb_init(tlb_t *tlb, replacement_algo_t algo) {
    memset(tlb, 0, sizeof(*tlb));
    for (int i = 0; i < TLB_ENTRIES; i++) {
        tlb->entries[i].page_number = TLB_INVALID;
        tlb->entries[i].frame_number = TLB_INVALID;
        tlb->last_used[i] = -1;
    }
    tlb->algo = algo;
    tlb->fifo_next_victim = 0;
    tlb->clock = 0;
    tlb->entries_used = 0;
    tlb->hits = 0;
}

typedef struct {
    const tlb_entry_t *entry;
    int target_page;
    int found;
} tlb_search_task_t;

static void *tlb_search_worker(void *arg) {
    tlb_search_task_t *task = (tlb_search_task_t *)arg;
    task->found = (task->entry->page_number == task->target_page) ? 1 : 0;
    return NULL;
}

int tlb_lookup(tlb_t *tlb, int page_number, int *out_frame) {
    pthread_t threads[TLB_ENTRIES];
    tlb_search_task_t tasks[TLB_ENTRIES];

    for (int i = 0; i < TLB_ENTRIES; i++) {
        tasks[i].entry = &tlb->entries[i];
        tasks[i].target_page = page_number;
        tasks[i].found = 0;
        if (pthread_create(&threads[i], NULL, tlb_search_worker, &tasks[i]) != 0) {
            tlb_search_worker(&tasks[i]);
            threads[i] = 0;
        }
    }

    int hit_index = -1;
    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (threads[i] != 0) {
            pthread_join(threads[i], NULL);
        }
        if (tasks[i].found && hit_index == -1) {
            hit_index = i;
        }
    }

    if (hit_index == -1) {
        return -1;
    }

    tlb->clock++;
    tlb->last_used[hit_index] = tlb->clock;
    tlb->hits++;
    *out_frame = tlb->entries[hit_index].frame_number;
    return hit_index;
}

static int tlb_pick_victim(tlb_t *tlb) {
    if (tlb->algo == ALGO_FIFO) {
        int idx = tlb->fifo_next_victim;
        tlb->fifo_next_victim = (tlb->fifo_next_victim + 1) % TLB_ENTRIES;
        return idx;
    }
    int victim = 0;
    for (int i = 1; i < TLB_ENTRIES; i++) {
        if (tlb->last_used[i] < tlb->last_used[victim]) {
            victim = i;
        }
    }
    return victim;
}

int tlb_insert(tlb_t *tlb, int page_number, int frame_number) {
    int idx;
    if (tlb->entries_used < TLB_ENTRIES) {
        idx = tlb->entries_used++;
    } else {
        idx = tlb_pick_victim(tlb);
    }

    tlb->entries[idx].page_number = page_number;
    tlb->entries[idx].frame_number = frame_number;
    tlb->clock++;
    tlb->last_used[idx] = tlb->clock;
    return idx;
}