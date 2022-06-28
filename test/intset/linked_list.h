#ifndef _LINKED_LIST_H_
#define _LINKED_LIST_H_

#define INIT_SET_PARAMETERS /* Nothing */

typedef unsigned int val_t;

typedef struct node
{
    val_t val;
    __mram_ptr struct node *next;
} node_t;

typedef struct intset
{
    __mram_ptr node_t *head;
} intset_t;

__mram_ptr intset_t *
set_new();

int 
set_contains(TYPE stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val);

int 
set_add(TYPE stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val, int transactional);

int 
set_remove(TYPE stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val);

#endif /* _LINKED_LIST_H_ */