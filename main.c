#include <stdio.h>

#include "addr_reader.h"
#include "args.h"
#include "page_table.h"
#include "tlb.h"
#include "vm.h"

#define OUTPUT_FILE "correct.txt"

int main(int argc, char *argv[]) {
    cli_args_t cli;
    if (parse_args(argc, argv, &cli) != 0) {
        return 1;
    }

    address_list_t addresses;
    if (load_addresses(cli.address_file, &addresses) != 0) {
        return 1;
    }

    page_table_t pt;
    if (page_table_init(&pt, cli.page_algo, BACKING_STORE_FILE) != 0) {
        free_addresses(&addresses);
        return 1;
    }

    tlb_t tlb;
    tlb_init(&tlb, cli.tlb_algo);

    /* O enunciado exige que a saida seja escrita em um arquivo chamado
     * "correct.txt" (nao basta imprimir no stdout). */
    FILE *out = fopen(OUTPUT_FILE, "w");
    if (out == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar o arquivo de saida '%s'.\n", OUTPUT_FILE);
        page_table_destroy(&pt);
        free_addresses(&addresses);
        return 1;
    }

    for (size_t i = 0; i < addresses.count; i++) {
        int virtual_address = addresses.addresses[i];
        int page_number = virtual_address / PAGE_SIZE;
        int offset = virtual_address % PAGE_SIZE;

        int frame;
        int tlb_slot = tlb_lookup(&tlb, page_number, &frame);

        if (tlb_slot != -1) {
            page_table_touch(&pt, frame);
        } else {
            frame = page_table_resolve_frame(&pt, page_number);
            if (frame == -1) {
                fclose(out);
                page_table_destroy(&pt);
                free_addresses(&addresses);
                return 1;
            }
            tlb_slot = tlb_insert(&tlb, page_number, frame);
        }

        int physical_address = frame * PAGE_SIZE + offset;
        int value = (int)(signed char)pt.physical_memory[physical_address];

        fprintf(out, "Virtual address: %d TLB: %d Physical address: %d Value: %d\n",
                virtual_address, tlb_slot, physical_address, value);
    }

    fprintf(out, "Number of Translated Addresses = %zu\n", addresses.count);
    fprintf(out, "Page Faults = %ld\n", pt.page_faults);
    fprintf(out, "Page Fault Rate = %.3f\n",
            (double)pt.page_faults / (double)addresses.count);
    fprintf(out, "TLB Hits = %ld\n", tlb.hits);
    fprintf(out, "TLB Hit Rate = %.3f\n",
            (double)tlb.hits / (double)addresses.count);

    fclose(out);
    page_table_destroy(&pt);
    free_addresses(&addresses);
    return 0;
}