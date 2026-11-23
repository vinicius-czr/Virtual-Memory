#ifndef ARGS_H
#define ARGS_H

#include "vm.h"

typedef struct {
    const char *address_file;
    replacement_algo_t page_algo;
    replacement_algo_t tlb_algo;
} cli_args_t;

int parse_args(int argc, char *argv[], cli_args_t *out);

#endif 
