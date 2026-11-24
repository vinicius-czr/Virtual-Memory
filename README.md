# vcrc — Gerenciador de Memória Virtual (Implementação 7)

Infraestrutura de Software — CESAR School.
Implementação do *Designing a Virtual Memory Manager* (Silberschatz, 10ª ed.,
p. P-51), com as modificações do enunciado: 128 frames, substituição de
página FIFO/LRU, TLB (16 entradas) com substituição FIFO/LRU, busca na TLB
paralelizada com uma thread por entrada.

## Compilar

```bash
make        # gera o executável ./vm
make clean  # remove objetos e o executável
```

## Executar

```bash
./vm <arquivo_enderecos> <algoritmo_pagina> <algoritmo_tlb>
# algoritmo_pagina, algoritmo_tlb ∈ {fifo, lru}

# exemplo (igual ao enunciado):
./vm addresses.txt lru fifo
```

O `BACKING_STORE.bin` precisa estar no diretório de onde o `./vm` é executado.

## Arquivos

| Arquivo | Responsabilidade |
|---|---|
| `args.c` / `args.h` | parsing e validação dos argumentos de linha de comando |
| `addr_reader.c` / `addr_reader.h` | leitura e validação do arquivo de endereços lógicos |
| `page_table.c` / `page_table.h` | tabela de páginas (128 frames), substituição FIFO/LRU, leitura do backing store |
| `tlb.c` / `tlb.h` | TLB (16 entradas), substituição FIFO/LRU, busca paralela com 1 thread por entrada |
| `main.c` | orquestra o fluxo (TLB → tabela de páginas → saída) |

## Decisões técnicas

1. **FIFO/TLB e FIFO/página como buffer circular** → `fifo_next_victim`
   percorre os frames/entradas na ordem em que foram carregados, sem
   precisar de fila auxiliar.
2. **LRU com "relógio" lógico (`clock` incremental)** → cada acesso grava
   `last_used[frame] = clock++`; a vítima é o menor `last_used`. Simples de
   implementar corretamente e com custo aceitável para 128 frames / 16
   entradas (busca linear).
3. **TLB "toca" a tabela de páginas em HIT (`page_table_touch`)** → sem
   isso, um frame muito acessado via TLB nunca atualizaria sua recência na
   tabela de páginas e poderia ser escolhido como vítima indevidamente pelo
   LRU de página. Essa decisão foi o que permitiu bater exatamente
   `Page Faults = 539` do gabarito do professor mesmo com a TLB ativa.
4. **Busca na TLB com 1 thread por entrada (`pthread_create`/`pthread_join`
   por acesso)** → cada thread só lê a entrada que lhe foi designada e
   escreve em uma struct de resultado própria (sem mutex, sem condição de
   corrida), atendendo ao requisito do enunciado de paralelizar a busca.

## Testar contra o gabarito

```bash
make
cp tests/BACKING_STORE.bin .
./vm tests/addresses.txt lru fifo > saida.txt
diff saida.txt <(tr -d '\r' < tests/correct.txt)
# bate 100% (a única "diferença" é o final de linha CRLF do arquivo do professor)
```

## Status

Implementação completa: tradução de endereço, 128 frames com FIFO/LRU,
TLB com FIFO/LRU e busca por thread, tratamento de erro de argumentos e de
arquivo de endereços.
