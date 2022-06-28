#include <mram.h>
#include <assert.h>

#include <defs.h>

#include "dpu_alloc.h"

__mram_ptr void *memory_ptr[NR_TASKLETS];


__mram_ptr void *mram_malloc(size_t size)
{
	assert(DPU_MRAM_HEAP_POINTER < (__mram_ptr void *)0x8FFFFF);

	int tid = me();
	__mram_ptr void *block = NULL;

	if (size <= 0)
	{
		return NULL;
	}

	if (memory_ptr[tid] + size >= (__mram_ptr void *)((((((tid + 1) * 2) + 8) << 20) | 0xFFFFF) + 1))
	{
		return NULL;
	}

	if (!memory_ptr[tid])
	{
		memory_ptr[tid] = (__mram_ptr void *)(((((tid * 2) + 8) << 20) | 0xFFFFF) + 1);
	}

	block = memory_ptr[tid];

	memory_ptr[tid] += size;

	return block; 
}


void mram_free(__mram_ptr void *ptr)
{

}