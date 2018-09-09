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

__declspec(align(RL_X86_CACHE_LINE_SIZE)) struct rl_aligned_cache_size {
    volatile size_t value;
};

struct rl_cdrain_queue {
    void *__restrict elements;
    
    /* Current head index */
    struct rl_aligned_cache_size head_index;
    
    /* Exclusive index of the last element added to the queue */
    struct rl_aligned_cache_size available_tail_index;
    
    /* Exclusive index of the last element pending addition to the queue */
    struct rl_aligned_cache_size allocated_tail_index;
};

__declspec(align(RL_X86_CACHE_LINE_SIZE)) struct rl_aligned_cache_cdrain_queue {
    struct rl_cdrain_queue queue;
};

/* 
 Not MT-Safe
 No synchronization is performed by this function, except for the synchronization included in malloc
 on the elements pointer for the queue
*/
int rl_cdrain_queue_init(struct rl_cdrain_queue *queue, size_t capacity, const size_t elem_sizeof);

/* This function is MT-Safe */
int rl_cdrain_queue_add(struct rl_cdrain_queue *queue, const void *elements, 
    const size_t nitems, const size_t elem_sizeof, const unsigned int flags);

/* 
 This function is not MT-Safe to call with the same queue/buffer object
 Otherwise, it is MT-Safe 
*/
size_t rl_cdrain_queue_drain(struct rl_cdrain_queue *__restrict queue, void *__restrict buffer, 
    const size_t max_items, const size_t elem_sizeof, const unsigned int flags);

/* 
 This function is reentrant. 
 This function synchronizes-with the numb
 If a drain operation is occuring in parallel with the same queue object,
 then the result returned is undefined.
*/
size_t rl_cdrain_queue_size(struct rl_cdrain_queue *queue);

size_t rl_cdrain_queue_capacity(struct rl_cdrain_queue *queue, const unsigned int flags);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* RL_CDRAIN_QUEUE_H */
