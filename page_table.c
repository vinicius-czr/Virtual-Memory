#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "page_table.h"

int page_table_init(page_table_t *pt, int num_frames, const char *backing_store_path) {
    memset(pt, 0, sizeof(*pt));

    for (int i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        pt->frame_of_page[i] = -1;
    }

    pt->backing_store = fopen(backing_store_path, "rb");
    if (pt->backing_store == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o backing store '%s'.\n",
                backing_store_path);
        return -1;
    }

    pt->physical_memory = malloc((size_t)num_frames * PAGE_SIZE);
    if (pt->physical_memory == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente para alocar %d frames.\n", num_frames);
        fclose(pt->backing_store);
        return -1;
    }

    pt->num_frames = num_frames;
    pt->next_free_frame = 0;
    pt->page_faults = 0;
    return 0;
}

void page_table_destroy(page_table_t *pt) {
    if (pt == NULL) {
        return;
    }
    free(pt->physical_memory);
    pt->physical_memory = NULL;
    if (pt->backing_store != NULL) {
        fclose(pt->backing_store);
        pt->backing_store = NULL;
    }
}

static int load_page_into_frame(page_table_t *pt, int page_number) {
    if (pt->next_free_frame >= pt->num_frames) {
        fprintf(stderr,
                "Erro interno: memoria fisica cheia sem algoritmo de substituicao "
                "configurado (isso sera resolvido no Dia 2).\n");
        return -1;
    }

    int frame = pt->next_free_frame++;

    if (fseek(pt->backing_store, (long)page_number * PAGE_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Erro: falha ao buscar a pagina %d no backing store.\n", page_number);
        return -1;
    }

    size_t nread = fread(pt->physical_memory + (size_t)frame * PAGE_SIZE, 1, PAGE_SIZE,
                          pt->backing_store);
    if (nread != PAGE_SIZE) {
        fprintf(stderr, "Erro: leitura incompleta da pagina %d no backing store.\n", page_number);
        return -1;
    }

    pt->frame_of_page[page_number] = frame;
    pt->page_faults++;
    return frame;
}

int translate_address(page_table_t *pt, int virtual_address,
                       int *out_physical_address, int *out_value) {
    int page_number = virtual_address / PAGE_SIZE;
    int offset = virtual_address % PAGE_SIZE;

    int frame = pt->frame_of_page[page_number];
    if (frame == -1) {
        frame = load_page_into_frame(pt, page_number);
        if (frame == -1) {
            return -1;
        }
    }

    int physical_address = frame * PAGE_SIZE + offset;
    *out_physical_address = physical_address;
    *out_value = (int)(signed char)pt->physical_memory[physical_address];
    return 0;
}