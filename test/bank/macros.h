#ifndef _MACROS_H_
#define _MACROS_H_

#define BUFFER_SIZE 100

#define START_DEBUG(tx, b, idx)                                                                                        \
    do                                                                                                                 \
    {                                                                                                                  \
        stm_start(tx);	                                                                                           \
        b[idx] = 'S';                                                                                                  \
    	idx = (idx + 1) % BUFFER_SIZE

#define LOAD_DEBUG(tx, val, b, idx, acc)                                                                               \
	    stm_load(tx, val);                                                                                           \
	    b[idx] = 'L';                                                                                                  \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
	    b[idx] = acc + '0';                                                                                            \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        b[idx] = 'R';                                                                                              \
	        idx = (idx + 1) % BUFFER_SIZE;                                                                             \
	        continue;                                                                                                  \
	    }

#define STORE_DEBUG(tx, val, v, b, idx, acc)                                                                           \
	    b[idx] = 'w';                                                                                                  \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
	    stm_store(tx, val, v);                                                                                         \
	    b[idx] = 'W';                                                                                                  \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
	    b[idx] = acc + '0';                                                                                            \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        b[idx] = 'R';                                                                                              \
	        idx = (idx + 1) % BUFFER_SIZE;                                                                             \
	        continue;                                                                                                  \
	    }

#define COMMIT_DEBUG(tx, b, idx)                                                                                       \
	    stm_commit(tx);                                                                                                \
	    b[idx] = 'C';                                                                                                  \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
	    if ((tx)->status != 4)                                                                                         \
	    {                                                                                                              \
	        break;                                                                                                     \
	    }                                                                                                              \
	    b[idx] = 'R';                                                                                                  \
	    idx = (idx + 1) % BUFFER_SIZE;                                                                                 \
    }                                                                                                                  \
    while (1)

#define START(tx)                                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        stm_start(tx);

#define LOAD(tx, val, ab, tid)                                                                                         \
	    stm_load(tx, val);                                                                                             \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        ab++;                                                                                                      \
	        /*for (volatile long i = 0; i < 1000 * tid; ++i) {}*/                                                      \
	        continue;                                                                                                  \
	    }

#define LOAD_RO(tx, val, ab)                                                                                           \
	    stm_load(tx, val);                                                                                             \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        ab++;                                                                                                      \
	        break;                                                                                                     \
	    }

#define STORE(tx, val, v, ab)                                                                                          \
	    stm_store(tx, val, v);                                                                                         \
	    if ((tx)->status == 4)                                                                                         \
	    {                                                                                                              \
	        ab++;                                                                                                      \
	        continue;                                                                                                  \
	    }

#define COMMIT(tx, ab)                                                                                                 \
	    stm_commit(tx);                                                                                                \
	    if ((tx)->status != 4)                                                                                         \
	    {                                                                                                              \
	        break;                                                                                                     \
	    }                                                                                                              \
	    ab++;                                                                                                          \
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