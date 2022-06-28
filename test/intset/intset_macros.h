#ifndef _MACROS_H_
#define _MACROS_H_


#define START(tx)                                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        stm_start(tx);

#define LOAD(tx, addr, ab)                                                                                         	   \
	    stm_load(tx, (__mram_ptr stm_word_t *)addr);                                                                                             \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        (ab)++;                                                                                                      \
	        continue;                                                                                                  \
	    }

#define LOAD_RO(tx, addr, ab)                                                                                           \
	    stm_load(tx, (__mram_ptr stm_word_t *)addr);                                                                                             \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        (ab)++;                                                                                                      \
	        break;                                                                                                     \
	    }

#define STORE(tx, addr, v, ab)                                                                                          \
	    stm_store(tx, (__mram_ptr stm_word_t *)addr, (stm_word_t)v);                                                                                         \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        (ab)++;                                                                                                      \
	        continue;                                                                                                  \
	    }

#define COMMIT(tx, ab)                                                                                                 \
	    stm_commit(tx);                                                                                                \
	    if ((tx)->status != 4)                                                                                         \
	    {                                                                                                              \
	        break;                                                                                                     \
	    }                                                                                                              \
	    (ab)++;                                                                                                          \
    }                                                                                                                  \
    while (1)

#define RAND_R_FNC(seed)                                                                                               \
    ({                                                                                                                 \
        uint64_t next = (seed);                                                                                        \
        uint64_t result;                                                                                               \
        next *= 1103515245;                                                                                            \
        next += 12345;                                                                                                 \
        result = (uint64_t)(next >> 16) & (2048 - 1);                                                                  \
        next *= 1103515245;                                                                                            \
        next += 12345;                                                                                                 \
        result <<= 10;                                                                                                 \
        result ^= (uint64_t)(next >> 16) & (1024 - 1);                                                                 \
        next *= 1103515245;                                                                                            \
        next += 12345;                                                                                                 \
        result <<= 10;                                                                                                 \
        result ^= (uint64_t)(next >> 16) & (1024 - 1);                                                                 \
        (seed) = next;                                                                                                 \
        result; /* returns result */                                                                                   \
    })

#endif /* _MACROS_H_ */