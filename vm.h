#ifndef VM_H
#define VM_H

#define OFFSET_BITS        8
#define PAGE_SIZE          (1 << OFFSET_BITS)
#define PAGE_TABLE_ENTRIES 256
#define ADDRESS_SPACE_MAX  65536
#define TLB_ENTRIES        16

#define BACKING_STORE_FILE "BACKING_STORE.bin"

typedef enum {
    ALGO_FIFO,
    ALGO_LRU
} replacement_algo_t;

#endif 
