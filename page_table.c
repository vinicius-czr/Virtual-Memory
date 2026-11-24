#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "page_table.h"

int page_table_init(page_table_t *pt, replacement_algo_t algo, const char *backing_store_path) {
    memset(pt, 0, sizeof(*pt));

    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        pt->frame_of_page[i] = -1;
    }
    for (int i = 0; i < NUM_FRAMES; i++) {
        pt->page_of_frame[i] = -1;
        pt->last_used[i] = -1;
    }

    pt->backing_store = fopen(backing_store_path, "rb");
    if (pt->backing_store == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o backing store '%s'.\n", backing_store_path);
        return -1;
    }

    pt->physical_memory = malloc((size_t)NUM_FRAMES * PAGE_SIZE);
    if (pt->physical_memory == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para alocar %d frames.\n", NUM_FRAMES);
        fclose(pt->backing_store);
        return -1;
    }

    pt->algo = algo;
    pt->fifo_next_victim = 0;
    pt->clock = 0;
    pt->frames_used = 0;
    pt->page_faults = 0;
    return 0;
}

void page_table_destroy(page_table_t *pt) {
    if (pt == NULL) return;
    free(pt->physical_memory);
    pt->physical_memory = NULL;
    if (pt->backing_store != NULL) {
        fclose(pt->backing_store);
        pt->backing_store = NULL;
    }
}

static int pick_victim_frame(page_table_t *pt) {
    if (pt->algo == ALGO_FIFO) {
        int frame = pt->fifo_next_victim;
        pt->fifo_next_victim = (pt->fifo_next_victim + 1) % NUM_FRAMES;
        return frame;
    }


    int victim = 0;
    for (int i = 1; i < NUM_FRAMES; i++) {
        if (pt->last_used[i] < pt->last_used[victim]) {
            victim = i;
        }
    }
    return victim;
}

static int load_page_into_frame(page_table_t *pt, int page_number) {
    int frame;

    if (pt->frames_used < NUM_FRAMES) {
        frame = pt->frames_used++;
    } else {
        frame = pick_victim_frame(pt);
        int old_page = pt->page_of_frame[frame];
        if (old_page != -1) {
            pt->frame_of_page[old_page] = -1;
        }
    }

    if (fseek(pt->backing_store, (long)page_number * PAGE_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: falha ao buscar a pagina %d no backing store.\n", page_number);
        return -1;
    }

    size_t nread = fread(pt->physical_memory + (size_t)frame * PAGE_SIZE, 1, PAGE_SIZE, pt->backing_store);
    if (nread != PAGE_SIZE) {
        fprintf(stderr, "Erro: leitura incompleta da pagina %d no backing store.\n", page_number);
        return -1;
    }

    pt->frame_of_page[page_number] = frame;
    pt->page_of_frame[frame] = page_number;
    pt->page_faults++;
    return frame;
}

int translate_address(page_table_t *pt, int virtual_address,
                       int *out_physical_address, int *out_value) {
    int page_number = virtual_address / PAGE_SIZE;
    int offset = virtual_address % PAGE_SIZE;

    pt->clock++;

    int frame = pt->frame_of_page[page_number];
    if (frame == -1) {
        frame = load_page_into_frame(pt, page_number);
        if (frame == -1) {
            return -1;
        }
    }

    pt->last_used[frame] = pt->clock;

    int physical_address = frame * PAGE_SIZE + offset;
    *out_physical_address = physical_address;
    *out_value = (int)(signed char)pt->physical_memory[physical_address];
    return 0;
}