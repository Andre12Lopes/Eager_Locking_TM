#ifndef _TINY_H_
#define _TINY_H_

#include <stdint.h>

#ifdef TX_IN_MRAM
#define TYPE __mram_ptr
#else
#define TYPE
#endif

#ifdef DATA_IN_MRAM
#define TYPE_ACC __mram_ptr
#else
#define TYPE_ACC
#endif

#ifndef R_SET_SIZE
# define R_SET_SIZE                 2                /* Initial size of read sets */
#endif /* ! RW_SET_SIZE */

#ifndef W_SET_SIZE
# define W_SET_SIZE                 2                /* Initial size of write sets */
#endif /* ! RW_SET_SIZE */

#ifndef LOCK_ARRAY_LOG_SIZE
#define LOCK_ARRAY_LOG_SIZE         10               /* Size of lock array: 2^10 = 1024 */
#endif

#define LOCK_ARRAY_SIZE             (1 << LOCK_ARRAY_LOG_SIZE)

typedef intptr_t stm_word_t;

typedef struct r_entry         /* Read set entry */
{                              
    volatile struct lock_entry *lock; /* Pointer to lock (for fast access) */
    unsigned int dropped;
} r_entry_t;

typedef struct r_set
{                               /* Read set */   /* Array of entries */
    r_entry_t entries[R_SET_SIZE];       /* Array of entries */
    unsigned int nb_entries;    /* Number of entries */
    unsigned int size;          /* Size of array */
} r_set_t;

typedef struct w_entry
{                                           /* Write set entry */

    volatile TYPE_ACC stm_word_t *addr;     /* Address written */
    stm_word_t value;                       /* New (write-back) or old (write-through) value */
    volatile struct lock_entry *lock;       /* Pointer to lock (for fast access) */
    TYPE struct w_entry *next;              /* WRITE_BACK_ETL || WRITE_THROUGH: Next address covered by same lock (if any) */
} w_entry_t;

typedef struct w_set
{                                   /* Write set */
    w_entry_t entries[W_SET_SIZE];  /* Array of entries */
    unsigned int nb_entries;        /* Number of entries */
    unsigned int size;              /* Size of array */
} w_set_t;


typedef struct _stm_tx
{
    volatile stm_word_t status; /* Transaction status */
    r_set_t r_set;              /* Read set */
    w_set_t w_set;              /* Write set */
    // Variables to measure performance
    unsigned int read_only;
    unsigned int TID;
    unsigned long long rng;
    unsigned long retries;
    perfcounter_t time;
    perfcounter_t start_time;
    perfcounter_t start_read;
    perfcounter_t start_write;
    perfcounter_t start_validation;
    uint64_t process_cycles;
    uint64_t read_cycles;
    uint64_t write_cycles;
    uint64_t validation_cycles;    
    uint64_t total_read_cycles;
    uint64_t total_write_cycles;
    uint64_t total_validation_cycles;
    uint64_t total_commit_validation_cycles;
    uint64_t commit_cycles;
    uint64_t total_cycles;
} stm_tx;

typedef struct lock_entry
{
    uint8_t mutex_r;
    uint8_t mutex_g;
    unsigned int readers;
} lock_entry_t;

typedef struct
{
    volatile struct lock_entry locks[LOCK_ARRAY_SIZE];
} global_t;

void stm_init(void);

void stm_start(TYPE stm_tx *tx);

stm_word_t stm_load(TYPE stm_tx *tx, volatile __mram_ptr stm_word_t *addr);

void stm_store(TYPE stm_tx *tx, volatile __mram_ptr stm_word_t *addr, stm_word_t value);

int stm_commit(TYPE stm_tx *tx);

#endif /* _TINY_H_ */
