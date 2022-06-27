#ifndef _STM_WTETL_H_
#define _STM_WTETL_H_

#include <assert.h>
#include <stdio.h>

static inline void 
stm_wtetl_add_to_rs(TYPE stm_tx *tx, volatile lock_entry_t *lock, volatile readers_entry_t *read)
{
    TYPE r_entry_t *r;

    /* No need to add to read set for read-only transaction */
    if (tx->read_only)
    {
        return;
    }

    /* Add address and version to read set */
    if (tx->r_set.nb_entries == tx->r_set.size)
    {
        printf("[Warning!] Reached RS allocation function, aborting\n");
        assert(0);
    }

    r = &tx->r_set.entries[tx->r_set.nb_entries++];
    r->lock = lock;
    r->read = read;
    r->dropped = 0;
}

/*
 * Check if address has been written previously.
 */
static inline TYPE w_entry_t *
stm_has_written(TYPE stm_tx *tx, volatile TYPE_ACC stm_word_t *addr)
{
    TYPE w_entry_t *w;

    // printf("==> stm_has_written(%p[%lu-%lu],%p)\n", tx, (unsigned long)tx->start, (unsigned long)tx->end, addr);

    /* Look for write */
    w = tx->w_set.entries;
    for (int i = tx->w_set.nb_entries; i > 0; i--, w++)
    {
        if (w->addr == addr)
        {
            return w;
        }
    }

    return NULL;
}

static inline TYPE w_entry_t *
stm_has_lock(TYPE stm_tx *tx, volatile lock_entry_t *lock)
{
    TYPE w_entry_t *w;

    // printf("==> stm_has_written(%p[%lu-%lu],%p)\n", tx, (unsigned long)tx->start, (unsigned long)tx->end, addr);

    /* Look for write */
    w = tx->w_set.entries;
    for (int i = tx->w_set.nb_entries; i > 0; i--, w++)
    {
        if (w->lock == lock)
        {
            return w;
        }
    }

    return NULL;
}

static inline TYPE r_entry_t *
stm_has_lock_read(TYPE stm_tx *tx, volatile lock_entry_t *lock)
{
    TYPE r_entry_t *r;

    // printf("==> stm_has_written(%p[%lu-%lu],%p)\n", tx, (unsigned long)tx->start, (unsigned long)tx->end, addr);

    /* Look for write */
    r = tx->r_set.entries;
    for (int i = tx->r_set.nb_entries; i > 0; i--, r++)
    {
        if (r->lock == lock && r->dropped == 0)
        {
            return r;
        }
    }

    return NULL;
}

static inline void 
stm_wtetl_rollback(TYPE stm_tx *tx)
{
    TYPE w_entry_t *w;
    TYPE r_entry_t *r;

    // printf("==> stm_wtetl_rollback(%p[%lu-%lu]) N entries = %u\n", tx, (unsigned long)tx->start,
                // (unsigned long)tx->end, tx->w_set.nb_entries);

    assert(IS_ACTIVE(tx->status));

    /* Decrease #readers */
    r = tx->r_set.entries;
    for (int i = tx->r_set.nb_entries; i > 0; i--, r++)
    {
        if (r->dropped == 0)
        {
            mutex_lock(&(r->lock->mutex_r));

            (r->read->readers)--;

            if (r->read->readers == 0)
            {
                mutex_unlock(&(r->lock->mutex_g));
            }

            mutex_unlock(&(r->lock->mutex_r));
        }
    }

    /* Undo writes and drop locks */
    w = tx->w_set.entries;
    for (int i = tx->w_set.nb_entries; i > 0; i--, w++)
    {
        ATOMIC_STORE_VALUE(w->addr, w->value);

        if (w->next != NULL)
        {
            continue;
        }

        mutex_unlock(&(w->lock->mutex_g));
    }

    SET_STATUS(tx->status, TX_ABORTED);

    tx->retries++;
    if (tx->retries > 3)
    { /* TUNABLE */
        backoff(tx, tx->retries);
    }

    /* Make sure that all lock releases become visible */
    // ATOMIC_B_WRITE;
}

static inline stm_word_t 
stm_wtetl_read(TYPE stm_tx *tx, volatile __mram_ptr stm_word_t *addr)
{
    volatile lock_entry_t *lock;
    volatile readers_entry_t *read;
    stm_word_t value;

    // printf("==> stm_wt_read(t=%p[%lu-%lu],a=%p)\n", tx, (unsigned long)tx->start, (unsigned long)tx->end, addr);

    // Get lock table entry
    lock = GET_LOCK(addr);
    read = GET_READ(addr);

    if (stm_has_lock(tx, lock))
    {
        value = ATOMIC_LOAD_VALUE_MRAM(addr);

        return value;
    }

    if (!mutex_trylock(&(lock->mutex_r)))
    {
        stm_wtetl_rollback(tx);
   
        return 0;
    }

    if ((++(read->readers)) == 1)
    {
        if (!mutex_trylock(&(lock->mutex_g)))
        {
            // This value is being written by other transaction
            read->readers--;

            mutex_unlock(&(lock->mutex_r));
            stm_wtetl_rollback(tx);
            // printf("ROLLBACK READ | TID = %d | ADDR = %p | LOCK = %p\n", me(), addr, lock);
   
            return 0;
        }
    }

    mutex_unlock(&(lock->mutex_r));

    stm_wtetl_add_to_rs(tx, lock, read);

    value = ATOMIC_LOAD_VALUE_MRAM(addr);
    return value;
}


static inline TYPE w_entry_t *
stm_wtetl_write(TYPE stm_tx *tx, volatile __mram_ptr stm_word_t *addr, stm_word_t value)
{
    volatile lock_entry_t *lock;
    volatile readers_entry_t *read;
    TYPE w_entry_t *w = NULL;
    TYPE w_entry_t *prev = NULL;
    TYPE r_entry_t *r;
    int res;

    // printf("==> stm_wt_write(t=%p[%lu-%lu],a=%p,d=%p-%lu,m=0x%lx)\n", tx, (unsigned long)tx->start,
                // (unsigned long)tx->end, addr, (void *)value, (unsigned long)value, (unsigned long)mask);

    assert(IS_ACTIVE(tx->status));

    /* Get reference to lock */
    lock = GET_LOCK(addr);
    read = GET_READ(addr);

    if (!mutex_trylock(&(lock->mutex_r)))
    {
        stm_wtetl_rollback(tx);
   
        return 0;
    }

    r = stm_has_lock_read(tx, lock);
    if (r != NULL && read->readers == 1)
    {
        read->readers = 0;
        r->dropped = 1;
        mutex_unlock(&(r->lock->mutex_g));
    }

    res = mutex_trylock(&(lock->mutex_g));

    // if (&(r->lock->mutex_g) != &(lock->mutex_g))
    // {
    //     printf("%p | %p | LOCK = %p | R = %p | TID = %d\n", &(r->lock->mutex_g), &(lock->mutex_g), lock, r, me());
    //     printf("--------------------------------\n");
    // }

    mutex_unlock(&(lock->mutex_r));

    if (!res)
    {
        w = stm_has_lock(tx, lock);
        if (w == NULL)
        {
            // Lock owned by other transaction
            stm_wtetl_rollback(tx);
            // printf("ROLLBACK WRITE | TID = %d | ADDR = %p | LOCK = %p\n", me(), addr, lock);
            
            return NULL;
        }
        else
        {
            prev = w;
            while (1)
            {
                if (addr == prev->addr)
                {
                    ATOMIC_STORE_VALUE(addr, value);
            
                    return w;
                }

                if (prev->next == NULL)
                {
                    /* Remember last entry in linked list (for adding new entry)
                     */
                    break;
                }

                prev = prev->next;
            }
        }
    }


    if (tx->w_set.nb_entries == tx->w_set.size)
    {
        printf("[Warning!] Reached RS allocation function, aborting\n");
        assert(0);
    }

    w = &tx->w_set.entries[tx->w_set.nb_entries];


    w->addr = addr;
    w->lock = lock;
    w->read = read;
    // Remember old value
    w->value = ATOMIC_LOAD_VALUE_MRAM(addr);

    ATOMIC_STORE_VALUE(addr, value);

    w->next = NULL;
    if (prev != NULL)
    {
        prev->next = w;
    }

    tx->w_set.nb_entries++;

    return w;
}


static inline int
stm_wtetl_commit(TYPE stm_tx *tx)
{
    TYPE w_entry_t *w;
    TYPE r_entry_t *r;
    perfcounter_t s_time;

    // printf("==> stm_wt_commit(%p[%lu-%lu])\n", tx, (unsigned long)tx->start, (unsigned long)tx->end);


    s_time = perfcounter_config(COUNT_CYCLES, false);

    tx->total_commit_validation_cycles += perfcounter_get() - s_time;

    /* Make sure that the updates become visible before releasing locks */
    // ATOMIC_B_WRITE;

    /* Decrease #readers */
    r = tx->r_set.entries;
    for (int i = tx->r_set.nb_entries; i > 0; i--, r++)
    {
        if (r->dropped == 0)
        {
            mutex_lock(&(r->lock->mutex_r));

            if ((--(r->read->readers)) == 0)
            {
                mutex_unlock(&(r->lock->mutex_g));
            }

            mutex_unlock(&(r->lock->mutex_r));
        }
    }

    /* Undo writes and drop locks */
    w = tx->w_set.entries;
    for (int i = tx->w_set.nb_entries; i > 0; i--, w++)
    {
        if (w->next == NULL)
        {
            mutex_unlock(&(w->lock->mutex_g));
        }
    }

    /* Make sure that all lock releases become visible */
    /* TODO: is ATOMIC_MB_WRITE required? */
    // ATOMIC_B_WRITE;

    return 1;
}

#endif /* _STM_WTETL_H_ */