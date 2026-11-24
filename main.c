#include <stdio.h>

#include "addr_reader.h"
#include "args.h"
#include "page_table.h"
#include "vm.h"

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

    for (size_t i = 0; i < addresses.count; i++) {
        int virtual_address = addresses.addresses[i];
        int physical_address;
        int value;

        if (translate_address(&pt, virtual_address, &physical_address, &value) != 0) {
            page_table_destroy(&pt);
            free_addresses(&addresses);
            return 1;
        }

        printf("Virtual address: %d Physical address: %d Value: %d\n",
               virtual_address, physical_address, value);
    }

    printf("Number of Translated Addresses = %zu\n", addresses.count);
    printf("Page Faults = %ld\n", pt.page_faults);
    printf("Page Fault Rate = %.3f\n",
           (double)pt.page_faults / (double)addresses.count);
    printf("TLB Hits = %d\n", 0);
    printf("TLB Hit Rate = %.3f\n", 0.0);

    page_table_destroy(&pt);
    free_addresses(&addresses);
    return 0;
}
