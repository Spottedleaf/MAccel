#ifndef RL_QUEUE_H
#define RL_QUEUE_H RL_QUEUE_H

#include <stddef.h>
#include <stdint.h>

#include "rl_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Use 1 bit for the reading bit flag and 1 bit for queue length */
static const size_t RL_CDRAIN_QUEUE_MAX_INDEX = SIZE_MAX >> 2;
static const size_t RL_CDRAIN_QUEUE_MAX_LENGTH = 1 + (SIZE_MAX >> 2);

enum rl_cdrain_queue_flags {
    RL_CDRAIN_QUEUE_NORESIZE = 1
};

struct rl_cdrain_queue {
    void *__restrict elements;

    /* Current head index */
    alignas (RL_X86_CACHE_LINE_SIZE) std::atomic_size_t head_index;
    
    /* Inclusive index of the last element added to the queue. */
    alignas (RL_X86_CACHE_LINE_SIZE) std::atomic_size_t available_tail_index;
    
    /* Inclusive index of the next index to allocate, may be equal to the current head if the queue is empty. */
    alignas (RL_X86_CACHE_LINE_SIZE) std::atomic_size_t allocated_tail_index;
};

struct rl_aligned_cache_cdrain_queue {
    alignas (RL_X86_CACHE_LINE_SIZE) struct rl_cdrain_queue queue;
};

int rl_cdrain_queue_init(struct rl_cdrain_queue *queue, size_t capacity, size_t elem_sizeof);

int rl_cdrain_queue_add(struct rl_cdrain_queue *queue, const void *elements, size_t nitems, size_t elem_sizeof, unsigned int flags);

/* This function is not MT-Safe */
size_t rl_cdrain_queue_drain(struct rl_cdrain_queue *queue, void *buffer, size_t max_items, size_t elem_sizeof, unsigned int flags);

/* The value returned by this function is inaccurate if a drain operation is done concurrently */
size_t rl_cdrain_queue_size(struct rl_cdrain_queue *queue);


#ifdef __cplusplus
}
#endif

#endif
