#include <assert.h>
#include <barrier.h>
#include <perfcounter.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <defs.h>
#include <stm.h>

#include "intset_macros.h"

#if defined(LINKED_LIST)
#include "linked_list.h"
#elif defined(SKIP_LIST)
#include "skip_list.h"
#elif defined(HASH_SET)
#include "hash_set.h"
#endif

#define UPDATE_PERCENTAGE   0
#define SET_INITIAL_SIZE    10
// #define SET_INITIAL_SIZE    256
#define RAND_RANGE          100
// #define RAND_RANGE          512

#define N_TRANSACTIONS      100

#ifdef TX_IN_MRAM
#define TYPE __mram_ptr
#else
#define TYPE
#endif

BARRIER_INIT(barrier, NR_TASKLETS);

__host uint64_t nb_cycles;
__host uint64_t nb_process_cycles;
__host uint64_t nb_process_read_cycles;
__host uint64_t nb_process_write_cycles;
__host uint64_t nb_process_validation_cycles;
__host uint64_t nb_commit_cycles;
__host uint64_t nb_commit_validation_cycles;
__host uint64_t nb_wasted_cycles;
__host uint64_t n_aborts;
__host uint64_t n_trans;
__host uint64_t n_tasklets;

__mram_ptr intset_t *set;

#ifdef TX_IN_MRAM
stm_tx __mram_noinit tx_mram[NR_TASKLETS];
#endif

void test(TYPE stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, uint64_t *seed, int *last);
void print_linked_list(__mram_ptr intset_t *set);
void print_hash_set(__mram_ptr intset_t *set);

int main()
{
#ifndef TX_IN_MRAM
    stm_tx tx;
#endif
    uint64_t t_aborts = 0;
    int val, tid;
    uint64_t seed;
    int i = 0;
    int last = -1;
    perfcounter_t initial_time;

    seed = me();
    tid = me();

#ifdef TX_IN_MRAM
    tx_mram[tid].TID = tid;
    tx_mram[tid].rng = tid + 1;
    tx_mram[tid].process_cycles = 0;
    tx_mram[tid].read_cycles = 0;
    tx_mram[tid].write_cycles = 0;
    tx_mram[tid].validation_cycles = 0;
    tx_mram[tid].total_read_cycles = 0;
    tx_mram[tid].total_write_cycles = 0;
    tx_mram[tid].total_validation_cycles = 0;
    tx_mram[tid].total_commit_validation_cycles = 0;
    tx_mram[tid].commit_cycles = 0;
    tx_mram[tid].total_cycles = 0;
    tx_mram[tid].start_time = 0;
    tx_mram[tid].start_read = 0;
    tx_mram[tid].start_write = 0;
    tx_mram[tid].start_validation = 0;
    tx_mram[tid].retries = 0;
#else
    tx.TID = tid;
    tx.rng = tid + 1;
    tx.process_cycles = 0;
    tx.read_cycles = 0;
    tx.write_cycles = 0;
    tx.validation_cycles = 0;
    tx.total_read_cycles = 0;
    tx.total_write_cycles = 0;
    tx.total_validation_cycles = 0;
    tx.total_commit_validation_cycles = 0;
    tx.commit_cycles = 0;
    tx.total_cycles = 0;
    tx.start_time = 0;
    tx.start_read = 0;
    tx.start_write = 0;
    tx.start_validation = 0;
    tx.retries = 0;
#endif

    if (tid == 0)
    {
        set = set_new(INIT_SET_PARAMETERS);

        while (i < SET_INITIAL_SIZE) 
        {
            val = (RAND_R_FNC(seed) % RAND_RANGE) + 1;
            if (set_add(NULL, NULL, set, val, 0))
            {
                i++;
            }
        }

        // print_hash_set(set);

        n_trans = N_TRANSACTIONS * NR_TASKLETS;
        n_tasklets = NR_TASKLETS;
        n_aborts = 0;

        initial_time = perfcounter_config(COUNT_CYCLES, false);
    }

    barrier_wait(&barrier);

    for (int i = 0; i < N_TRANSACTIONS; ++i)
    {
#ifdef TX_IN_MRAM
        test(&(tx_mram[tid]), &t_aborts, set, &seed, &last);
#else
        test(&tx, &t_aborts, set, &seed, &last);
#endif
    }

    barrier_wait(&barrier);

    if (me() == 0)
    {
        nb_cycles = perfcounter_get() - initial_time;

        nb_process_cycles = 0;
        nb_commit_cycles = 0;
        nb_wasted_cycles = 0;
        nb_process_read_cycles = 0;
        nb_process_write_cycles = 0;
        nb_process_validation_cycles = 0;
        nb_commit_validation_cycles = 0;
    }

    for (int i = 0; i < NR_TASKLETS; ++i)
    {
        if (me() == i)
        {
            n_aborts += t_aborts;

#ifdef TX_IN_MRAM
            nb_process_cycles += ((double) tx_mram[tid].process_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_process_read_cycles += ((double) tx_mram[tid].total_read_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_process_write_cycles += ((double) tx_mram[tid].total_write_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_process_validation_cycles += ((double) tx_mram[tid].total_validation_cycles / (N_TRANSACTIONS * NR_TASKLETS));

            nb_commit_cycles += ((double) tx_mram[tid].commit_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_commit_validation_cycles += ((double) tx_mram[tid].total_commit_validation_cycles / (N_TRANSACTIONS * NR_TASKLETS));

            nb_wasted_cycles += ((double) (tx_mram[tid].total_cycles - (tx_mram[tid].process_cycles + tx_mram[tid].commit_cycles)) / (N_TRANSACTIONS * NR_TASKLETS));
#else
            nb_process_cycles += ((double) tx.process_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_process_read_cycles += ((double) tx.total_read_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_process_write_cycles += ((double) tx.total_write_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_process_validation_cycles += ((double) tx.total_validation_cycles / (N_TRANSACTIONS * NR_TASKLETS));

            nb_commit_cycles += ((double) tx.commit_cycles / (N_TRANSACTIONS * NR_TASKLETS));
            nb_commit_validation_cycles += ((double) tx.total_commit_validation_cycles / (N_TRANSACTIONS * NR_TASKLETS));

            nb_wasted_cycles += ((double) (tx.total_cycles - (tx.process_cycles + tx.commit_cycles)) / (N_TRANSACTIONS * NR_TASKLETS));
#endif
        }

        barrier_wait(&barrier);
    }

    if (me() == 0)
    {
        // print_linked_list(set);
        // print_hash_set(set);
    }

    return 0;
}


void test(TYPE stm_tx *tx, uint64_t *t_aborts, __mram_ptr intset_t *set, uint64_t *seed, int *last)
{
    int val, op;

    op = RAND_R_FNC(*seed) % 100;
    if (op < UPDATE_PERCENTAGE)
    {
        if (*last < 0)
        {
            /* Add random value */
            val = (RAND_R_FNC(*seed) % RAND_RANGE) + 1;
            if (set_add(tx, t_aborts, set, val, 1))
            {
                *last = val;
            }
        }
        else
        {
            /* Remove last value */
            set_remove(tx, t_aborts, set, *last);
            *last = -1;
        }
    }
    else
    {
        /* Look for random value */
        val = (RAND_R_FNC(*seed) % RAND_RANGE) + 1;
        set_contains(tx, t_aborts, set, val);
    }
}


void print_linked_list(__mram_ptr intset_t *set)
{
    // for (__mram_ptr node_t *n = set->head->next; n->next != NULL; n = n->next)
    // {
    //     printf("%p -> %u\n", n, n->val);
    // }
}

void print_hash_set(__mram_ptr intset_t *set)
{
    // __mram_ptr bucket_t *b;

    // for (int i = 0; i < 1024; ++i)
    // {
    //     b = (__mram_ptr bucket_t *)set->buckets[i];

    //     if (b)
    //     {
    //         printf("\n");
    //     }
        
    //     for (; b != NULL ; b = b->next)
    //     {
    //         printf("%u -", b->val);
    //     }
    // }

    // printf("\n");
}