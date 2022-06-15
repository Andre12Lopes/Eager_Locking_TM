#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <perfcounter.h>

#include "stm.h"
#include "stm_internal.h"

global_t _tinystm;

void 
stm_init(void)
{
    printf("Initialized STM\n");

    // if (_tinystm.initialized)
    // {
    //     return;
    // }
    
    // memset((void *)_tinystm.locks, 0, LOCK_ARRAY_SIZE * sizeof(stm_word_t));
    // // CLOCK = 0

    // _tinystm.initialized = 1;
}

void 
stm_start(TYPE stm_tx *tx)
{
    return int_stm_start(tx);
}

stm_word_t 
stm_load(TYPE stm_tx *tx, volatile __mram_ptr stm_word_t *addr)
{
    return int_stm_load(tx, addr);
}

void 
stm_store(TYPE stm_tx *tx, volatile __mram_ptr stm_word_t *addr, stm_word_t value)
{
    int_stm_store(tx, addr, value);
}

int 
stm_commit(TYPE stm_tx *tx)
{
    return int_stm_commit(tx);
}