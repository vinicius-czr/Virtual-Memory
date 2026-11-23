# vcrc — Gerenciador de Memória Virtual (Implementação 6)

Infraestrutura de Software — CESAR School.
Implementação do *Designing a Virtual Memory Manager* (Silberschatz, 10ª ed.,
p. P-51), com as modificações do enunciado: 128 frames, FIFO/LRU para página
e para TLB, busca na TLB com uma thread por entrada.

> Status atual: **Dia 1 concluído** — tradução de endereço sem TLB, com 256
> frames (sem substituição ainda). Dia 2: reduzir para 128 frames + FIFO/LRU.

## Compilar

```bash
make        # gera o executável ./vm
make clean  # remove objetos e o executável
```

## Executar

```bash
./vm <arquivo_enderecos> <algoritmo_pagina> <algoritmo_tlb>
# algoritmo_pagina, algoritmo_tlb ∈ {fifo, lru}

# exemplo:
./vm addresses.txt lru fifo
```

O `BACKING_STORE.bin` precisa estar no diretório de onde o `./vm` é executado.

## Arquivos

| Arquivo | Responsabilidade |
|---|---|
| `args.c` / `args.h` | parsing e validação dos argumentos de linha de comando |
| `addr_reader.c` / `addr_reader.h` | leitura e validação do arquivo de endereços lógicos |
| `page_table.c` / `page_table.h` | tabela de páginas, leitura do backing store, tradução de endereço |
| `main.c` | orquestra o fluxo e imprime a saída |

## Decisões técnicas

1. **256 frames na primeira versão** → evita precisar de substituição
   enquanto validamos a tradução de endereço; será reduzido para 128 no
   Dia 2, junto com FIFO/LRU.
2. **Estatísticas de resumo no `stdout`** (Number of Translated Addresses,
   Page Faults, Page Fault Rate, TLB Hits, TLB Hit Rate), no mesmo formato
   do gabarito do professor.
3. **`getline` + `strtol` na leitura do arquivo de endereços** → valida
   formato (um inteiro por linha, 0–65535) e rejeita o arquivo inteiro com
   mensagem de erro coerente em caso de linha inválida, sem crashar.