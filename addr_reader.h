#ifndef ADDR_READER_H
#define ADDR_READER_H

#include <stddef.h>

typedef struct {
    int *addresses;
    size_t count;
} address_list_t;

/* Le o arquivo em path, um endereco logico (inteiro, 0..65535) por linha.
 * Retorna 0 e preenche *out em caso de sucesso.
 * Retorna -1 em caso de erro (arquivo inexistente/sem permissao, ou linha
 * em formato diferente do esperado), ja tendo impresso uma mensagem de
 * erro coerente em stderr. */
int load_addresses(const char *path, address_list_t *out);

void free_addresses(address_list_t *list);

#endif 
