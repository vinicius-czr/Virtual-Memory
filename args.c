#include <stdio.h>
#include <string.h>

#include "args.h"

static int parse_algo(const char *token, replacement_algo_t *out) {
    if (strcmp(token, "fifo") == 0) {
        *out = ALGO_FIFO;
        return 0;
    }
    if (strcmp(token, "lru") == 0) {
        *out = ALGO_LRU;
        return 0;
    }
    return -1;
}

static void print_usage(const char *prog_name) {
    fprintf(stderr,
            "Uso: %s <arquivo_enderecos> <algoritmo_pagina> <algoritmo_tlb>\n"
            "  <algoritmo_pagina> e <algoritmo_tlb> devem ser: fifo | lru\n"
            "Exemplo: %s address.txt lru fifo\n",
            prog_name, prog_name);
}

int parse_args(int argc, char *argv[], cli_args_t *out) {
    if (argc != 4) {
        fprintf(stderr, "Erro: numero de argumentos invalido (esperado 3, recebido %d).\n",
                argc - 1);
        print_usage(argv[0]);
        return -1;
    }

    if (parse_algo(argv[2], &out->page_algo) != 0) {
        fprintf(stderr, "Erro: algoritmo de substituicao de pagina invalido: '%s'.\n", argv[2]);
        print_usage(argv[0]);
        return -1;
    }

    if (parse_algo(argv[3], &out->tlb_algo) != 0) {
        fprintf(stderr, "Erro: algoritmo de substituicao de TLB invalido: '%s'.\n", argv[3]);
        print_usage(argv[0]);
        return -1;
    }

    out->address_file = argv[1];
    return 0;
}