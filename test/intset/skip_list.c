#if defined(SKIP_LIST)

#include <alloc.h>
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

#include <stm.h>

#include "skip_list.h"
#include "dpu_alloc.h"
#include "intset_macros.h"

#define VAL_MIN 0
#define VAL_MAX INT_MAX

static int random_level(__mram_ptr intset_t *set, uint64_t *seed)
{
    int l = 0;
    int r;

    r = RAND_R_FNC(*seed) % 100;
    while (l < set->max_level && r < set->prob)
    {
        r = RAND_R_FNC(*seed) % 100;
        l++;
    }

    return l;
}


static __mram_ptr node_t *new_node(val_t val, level_t level, int transactional)
{
    __mram_ptr node_t *node;

    // if (!transactional)
    // {
        node = (__mram_ptr node_t *)mram_malloc(sizeof(node_t) + level * sizeof(node_t *));
    // }
    // else
    // {
    //     node = (node_t *)TM_MALLOC(sizeof(node_t) + level * sizeof(node_t *));
    // }

    if (node == NULL)
    {
        exit(1);
    }

    node->val = val;
    node->level = level;

    return node;
}

__mram_ptr intset_t *set_new(level_t max_level, int prob)
{
    __mram_ptr intset_t *set;

    assert(max_level <= MAX_LEVEL);
    assert(prob >= 0 && prob <= 100);

    if ((set = (__mram_ptr intset_t *)mram_malloc(sizeof(intset_t))) == NULL)
    {
        exit(1);
    }

    set->max_level = max_level;
    set->prob = prob;
    set->level = 0;
    /* Set head and tail are immutable */
    set->tail = new_node(VAL_MAX, max_level, 0);
    set->head = new_node(VAL_MIN, max_level, 0);

    for (int i = 0; i <= max_level; i++)
    {
        set->head->forward[i] = set->tail;
        set->tail->forward[i] = NULL;
    }

    return set;
}

int set_contains(stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val)
{
    int result, i;
    __mram_ptr node_t *node, *next;
    val_t v;


    START(tx);
    v = VAL_MIN; /* Avoid compiler warning (should not be necessary) */
    node = set->head;
    for (i = TM_LOAD(&set->level); i >= 0; i--)
    {
        next = (node_t *)TM_LOAD(&node->forward[i]);
        while (1)
        {
            v = TM_LOAD(&next->val);
            if (v >= val)
            {
                break;
            }

            node = next;
            next = (node_t *)TM_LOAD(&node->forward[i]);
        }
    }

    result = (v == val);
    COMMIT(tx, *t_aborts);

    return result;
}

int set_add(stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val, int transactional, uint64_t *seed)
{
    int result, i;
    __mram_ptr node_t *update[MAX_LEVEL + 1];
    __mram_ptr node_t *node, *next;
    level_t level, l;
    val_t v;

    if (!transactional)
    {
        node = set->head;
        for (i = set->level; i >= 0; i--)
        {
            next = node->forward[i];
            while (next->val < val)
            {
                node = next;
                next = node->forward[i];
            }
            update[i] = node;
        }
        node = node->forward[0];

        if (node->val == val)
        {
            result = 0;
        }
        else
        {
            l = random_level(set, seed);
            if (l > set->level)
            {
                for (i = set->level + 1; i <= l; i++)
                {
                    update[i] = set->head;
                }

                set->level = l;
            }
            node = new_node(val, l, 0);

            for (i = 0; i <= l; i++)
            {
                node->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = node;
            }
            result = 1;
        }
    }
    else
    {
        // TM_START(1, RW);
        // v = VAL_MIN; /* Avoid compiler warning (should not be necessary) */
        // node = set->head;
        // level = TM_LOAD(&set->level);
        // for (i = level; i >= 0; i--)
        // {
        //     next = (node_t *)TM_LOAD(&node->forward[i]);
        //     while (1)
        //     {
        //         v = TM_LOAD(&next->val);
        //         if (v >= val)
        //             break;
        //         node = next;
        //         next = (node_t *)TM_LOAD(&node->forward[i]);
        //     }
        //     update[i] = node;
        // }

        // if (v == val)
        // {
        //     result = 0;
        // }
        // else
        // {
        //     l = random_level(set, td->seed);
        //     if (l > level)
        //     {
        //         for (i = level + 1; i <= l; i++)
        //             update[i] = set->head;
        //         TM_STORE(&set->level, l);
        //     }
        //     node = new_node(val, l, 1);
        //     for (i = 0; i <= l; i++)
        //     {
        //         node->forward[i] = (node_t *)TM_LOAD(&update[i]->forward[i]);
        //         TM_STORE(&update[i]->forward[i], node);
        //     }
        //     result = 1;
        // }
        // TM_COMMIT;
    }

    return result;
}

int set_remove(stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, val_t val)
{
    int result, i;
    // node_t *update[MAX_LEVEL + 1];
    // node_t *node, *next;
    // level_t level;
    // val_t v;

    // TM_START(2, RW);
    // v = VAL_MIN; /* Avoid compiler warning (should not be necessary) */
    // node = set->head;
    // level = TM_LOAD(&set->level);
    // for (i = level; i >= 0; i--)
    // {
    //     next = (node_t *)TM_LOAD(&node->forward[i]);
    //     while (1)
    //     {
    //         v = TM_LOAD(&next->val);
    //         if (v >= val)
    //             break;
    //         node = next;
    //         next = (node_t *)TM_LOAD(&node->forward[i]);
    //     }
    //     update[i] = node;
    // }
    // node = (node_t *)TM_LOAD(&node->forward[0]);

    // if (v != val)
    // {
    //     result = 0;
    // }
    // else
    // {
    //     for (i = 0; i <= level; i++)
    //     {
    //         if ((node_t *)TM_LOAD(&update[i]->forward[i]) == node)
    //             TM_STORE(&update[i]->forward[i], (node_t *)TM_LOAD(&node->forward[i]));
    //     }
    //     i = level;
    //     while (i > 0 && (node_t *)TM_LOAD(&set->head->forward[i]) == set->tail)
    //         i--;
    //     if (i != level)
    //         TM_STORE(&set->level, i);
    //     /* Free memory (delayed until commit) */
    //     TM_FREE2(node, sizeof(node_t) + node->level * sizeof(node_t *));
    //     result = 1;
    // }
    // TM_COMMIT;

    return result;
}

#endif