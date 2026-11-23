/* getline()/ssize_t sao extensoes POSIX, nao fazem parte do C11 puro;
 * precisamos declarar isso antes de qualquer #include. */
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "addr_reader.h"
#include "vm.h"

#define INITIAL_CAPACITY 1024

/* Valida que 'line' contem apenas um inteiro (sem sinal) em decimal,
 * ignorando espacos em branco nas pontas. Preenche *value em caso de
 * sucesso. */
static int parse_single_integer(const char *line, long *value) {
    char *endptr;

    while (isspace((unsigned char)*line)) {
        line++;
    }
    if (*line == '\0') {
        return -1; /* linha em branco: formato invalido */
    }

    errno = 0;
    long parsed = strtol(line, &endptr, 10);

    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }
    if (*endptr != '\0' || errno == ERANGE) {
        return -1;
    }

    *value = parsed;
    return 0;
}

int load_addresses(const char *path, address_list_t *out) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo de enderecos '%s' (%s).\n",
                path, strerror(errno));
        return -1;
    }

    size_t capacity = INITIAL_CAPACITY;
    int *addresses = malloc(capacity * sizeof(int));
    if (addresses == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente ao carregar '%s'.\n", path);
        fclose(fp);
        return -1;
    }

    size_t count = 0;
    size_t line_no = 0;
    char *line = NULL;
    size_t line_cap = 0;
    ssize_t nread;

    while ((nread = getline(&line, &line_cap, fp)) != -1) {
        line_no++;

        /* remove o '\n' / '\r' finais, se houver */
        while (nread > 0 && (line[nread - 1] == '\n' || line[nread - 1] == '\r')) {
            line[--nread] = '\0';
        }

        /* linhas totalmente em branco no fim do arquivo sao ignoradas */
        if (nread == 0) {
            continue;
        }

        long value;
        if (parse_single_integer(line, &value) != 0 ||
            value < 0 || value >= ADDRESS_SPACE_MAX) {
            fprintf(stderr,
                    "Erro: arquivo de enderecos '%s' em formato invalido na linha %zu: '%s'.\n"
                    "Esperado um unico endereco logico inteiro entre 0 e %d por linha.\n",
                    path, line_no, line, ADDRESS_SPACE_MAX - 1);
            free(line);
            free(addresses);
            fclose(fp);
            return -1;
        }

        if (count == capacity) {
            capacity *= 2;
            int *grown = realloc(addresses, capacity * sizeof(int));
            if (grown == NULL) {
                fprintf(stderr, "Erro: memoria insuficiente ao carregar '%s'.\n", path);
                free(line);
                free(addresses);
                fclose(fp);
                return -1;
            }
            addresses = grown;
        }

        addresses[count++] = (int)value;
    }

    free(line);
    fclose(fp);

    if (count == 0) {
        fprintf(stderr, "Erro: arquivo de enderecos '%s' esta vazio.\n", path);
        free(addresses);
        return -1;
    }

    out->addresses = addresses;
    out->count = count;
    return 0;
}

void free_addresses(address_list_t *list) {
    if (list == NULL) {
        return;
    }
    free(list->addresses);
    list->addresses = NULL;
    list->count = 0;
}