#ifndef _SKIP_LIST_H_
#define _SKIP_LIST_H_

#define MAX_LEVEL           64

#define INIT_SET_PARAMETERS 32, 50

typedef intptr_t val_t;
typedef intptr_t level_t;

typedef struct node
{
    val_t val;
    level_t level;
    __mram_ptr struct node *forward[1];
} node_t;

typedef struct intset
{
    __mram_ptr node_t *head;
    __mram_ptr node_t *tail;
    level_t level;
    int prob;
    int max_level;
} intset_t;

__mram_ptr intset_t *
set_new(level_t max_level, int prob);

int 
set_contains(stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val);

int 
set_add(stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val, int transactional, uint64_t *seed);

int 
set_remove(stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val);

#endif /* _SKIP_LIST_H_ */