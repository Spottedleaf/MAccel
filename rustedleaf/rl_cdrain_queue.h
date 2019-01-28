#ifndef RL_CDRAIN_QUEUE_H
#define RL_CDRAIN_QUEUE_H RL_CDRAIN_QUEUE_H

#include <stddef.h>
#include <stdint.h>

#include "..\atomic_bridge.h"
#include "rl_defs.h"


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Use 1 bit for the reading bit flag and 1 bit for queue length */
/* The max length may never be SIZE_MAX ^ (SIZE_MAX >> 1) */
static const size_t RL_CDRAIN_QUEUE_MAX_INDEX = SIZE_MAX >> 2;
static const size_t RL_CDRAIN_QUEUE_MAX_LENGTH = 1 + (SIZE_MAX >> 2);

enum rl_cdrain_queue_flags {
    RL_CDRAIN_QUEUE_NORESIZE = (1 << 0),
    RL_CDRAIN_QUEUE_PEEK     = (1 << 1),
};

__declspec(align(RL_X86_CACHE_LINE_SIZE)) struct rl_cdrain_queue {
    void *__restrict elements;

    /* Current head index */
    __declspec(align(RL_X86_CACHE_LINE_SIZE)) volatile size_t head_index;

    /* Exclusive index of the last element added to the queue */
    __declspec(align(RL_X86_CACHE_LINE_SIZE)) volatile size_t available_tail_index;

    /* Exclusive index of the last element pending addition to the queue */
    __declspec(align(RL_X86_CACHE_LINE_SIZE)) volatile size_t allocated_tail_index;
};

__declspec(align(RL_X86_CACHE_LINE_SIZE)) struct rl_aligned_cache_cdrain_queue {
    struct rl_cdrain_queue queue;
};


int rl_cdrain_queue_init(struct rl_cdrain_queue *queue, size_t capacity, const size_t elem_sizeof);

void rl_cdrain_queue_free(struct rl_cdrain_queue *queue);

int rl_cdrain_queue_add(struct rl_cdrain_queue *queue, const void *elements, 
    const size_t nitems, const size_t elem_sizeof, const unsigned int flags);

size_t rl_cdrain_queue_drain(struct rl_cdrain_queue *__restrict queue, void *__restrict buffer, 
    const size_t max_items, const size_t elem_sizeof, const unsigned int flags);

size_t rl_cdrain_queue_size(struct rl_cdrain_queue *queue, const unsigned int flags);

size_t rl_cdrain_queue_remaining_capacity(struct rl_cdrain_queue *queue, const unsigned int flags);

size_t rl_cdrain_queue_capacity(struct rl_cdrain_queue *queue, const unsigned int flags);

size_t rl_cdrain_queue_clear(struct rl_cdrain_queue *queue);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RL_CDRAIN_QUEUE_H */
