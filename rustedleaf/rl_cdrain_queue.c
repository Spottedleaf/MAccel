/* Start rustedleaf copy-pastes */
#include <intrin.h>
/* End rustedleaf copy-pastes */

#include <Windows.h>

#include <stdint.h>
#include <stddef.h>

#include "..\atomic_bridge.h"

#include "rl_defs.h"
#include "rl_cdrain_queue.h"
#include "..\utils.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Start rustedleaf copy-pastes */

/* rl_math.h */

/*
 * Returns the ceil of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_ceil_log2_u32(const uint32_t val) {
    return (sizeof(val) * CHAR_BIT) - __lzcnt(val - 1);
}

/*
 * Returns the ceil of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_ceil_log2_u64(const uint64_t val) {
    return (sizeof(val) * CHAR_BIT) - (unsigned int ) __lzcnt64(val - 1);
}

/*
 * Returns the ceil of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_ceil_log2_size(const size_t val) {
    if (SIZE_MAX <= UINT32_MAX) {
        return rl_ceil_log2_u32((uint32_t) val);
    } else if (SIZE_MAX <= UINT64_MAX) {
        return rl_ceil_log2_u64((uint64_t) val);
    }
    abort();
}

 /*
 * Returns the floor of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_floor_log2_u32(const uint32_t val) {
    return (sizeof(val) * CHAR_BIT - 1) ^ __lzcnt(val);
}

/*
 * Returns the floor of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_floor_log2_u64(const uint64_t val) {
    return (sizeof(val) * CHAR_BIT - 1) ^ (unsigned int) __lzcnt64(val);
}

/*
 * Returns the floor of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_floor_log2_size(const size_t val) {
    if (SIZE_MAX <= UINT32_MAX) {
        return rl_floor_log2_u32((uint32_t) val);
    } else if (SIZE_MAX <= UINT64_MAX) {
        return rl_floor_log2_u64((uint64_t) val);
    }
    abort();
}

static inline size_t rl_round_ceil_log2_size(const size_t val) {
    static const size_t HIGH_BIT = (SIZE_MAX) ^ (SIZE_MAX >> 1);
    if (SIZE_MAX <= UINT32_MAX) {
        return HIGH_BIT >> ((sizeof(uint32_t) * CHAR_BIT) - rl_ceil_log2_u32((uint32_t)val));
    } else if (SIZE_MAX <= UINT64_MAX) {
        return HIGH_BIT >> ((sizeof(uint64_t) * CHAR_BIT) - rl_ceil_log2_u64((uint64_t)val));
    }
    abort();
}

static inline size_t rl_round_floor_log2_size(const size_t val) {
    static const size_t HIGH_BIT = (SIZE_MAX) ^ (SIZE_MAX >> 1);
    if (SIZE_MAX <= UINT32_MAX) {
        return HIGH_BIT >> ((sizeof(uint32_t) * CHAR_BIT - 1) ^ rl_floor_log2_u32((uint32_t)val));
    } else if (SIZE_MAX <= UINT64_MAX) {
        return HIGH_BIT >> ((sizeof(uint64_t) * CHAR_BIT - 1) ^ rl_floor_log2_u64((uint64_t)val));
    }
    abort();
}

static inline bool rl_mulsize_overflow(const size_t a, const size_t b) {
    return b && ((a * b) / b) != a;
}

/* rl_memutil.h */

static void *rl_smalloc(const size_t nitems, const size_t itemsize) {
    if (rl_mulsize_overflow(nitems, itemsize)) {
        printf("Overflow of multiplication for memory allocation: %zu, %zu\n", nitems, itemsize);
        exit(EXIT_FAILURE);
    }
    void *ret = malloc(nitems * itemsize);
    if (!ret) {
        printf("Unable to allocate %zu bytes of memory\n", (nitems * itemsize));
        exit(EXIT_FAILURE);
    }
    return ret;
}

/* End rustedleaf copy pastes */

static inline void sched_yield(void) {
    SwitchToThread();
}

static const size_t HIGH_BIT = SIZE_MAX ^ (SIZE_MAX >> 1);
static const size_t READING_BIT = (SIZE_MAX ^ (SIZE_MAX >> 1));
#define RL_CDRAIN_QUEUE_INDEX_LENGTH_MASK ((RL_CDRAIN_QUEUE_MAX_LENGTH << 1) - 1)

static size_t obtain_max_len(const size_t elem_sizeof) {
    /* The maximum length contains no index which multiplied by elem_sizeof is equal to or greater than SIZE_MAX */
    /* Or that is larger than RL_CDRAIN_QUEUE_MAX_LENGTH */
    
    size_t len = SIZE_MAX / elem_sizeof;
    if (len >= RL_CDRAIN_QUEUE_MAX_LENGTH) {
        return RL_CDRAIN_QUEUE_MAX_LENGTH;
    }
    
    return rl_round_floor_log2_size(len);
}

/* Inclusive head, Exclusive tail */
static inline size_t get_queue_length(const size_t head, const size_t tail, const size_t length_mask) {
    return (tail - head) & length_mask;
}

/* Inclusive head, Exclusive tail */
static inline size_t get_remaining_length(const size_t head, const size_t tail, const size_t length) {
    if (head <= tail) {
        return length - (tail - head);
    } else {
        return (head - tail);
    }
}

static void rl_wait_set(volatile size_t *value, const size_t wait, const size_t set) {
    size_t failures = 0;
    while (atomicb_load_explicit_size(value, memory_order_bridge_seq_cst) != wait) {
        if (++failures >= 128) {
            sched_yield();
        }
        rl_pause_intrin();
    }
    atomicb_store_explicit_size(value, set, memory_order_bridge_seq_cst);
}


int rl_cdrain_queue_add(struct rl_cdrain_queue *queue, const void *elements, 
    const size_t nitems, const size_t elem_sizeof, const unsigned int flags) {
    /* This is a maximum length which will not overflow */
    const size_t max_length = obtain_max_len(elem_sizeof);
    
    if (nitems >= max_length) {
        return RL_ENOMEM;
    }

    volatile size_t *head_index = &queue->head_index;
    volatile size_t *available_tail_index = &queue->available_tail_index;
    volatile size_t *allocated_tail_index = &queue->allocated_tail_index;
    
    size_t current_index = atomicb_load_explicit_size(allocated_tail_index, memory_order_bridge_acquire);
    size_t failures = 0;

    for (;;) {
        for (size_t i = 0; i < failures; ++i) {
            rl_pause_intrin();
        }
        /* Test if resizing */
        while (current_index == SIZE_MAX) {
            sched_yield();
            current_index = atomicb_load_explicit_size(allocated_tail_index, memory_order_bridge_acquire);
        }
        
        /* Not resizing */
        
        /* Decode length */
        const size_t length = rl_round_floor_log2_size(current_index);
        const size_t length_mask = length - 1;
        
        const size_t allocated_start = current_index ^ length; /* Inclusive */
        const size_t allocated_end = (current_index + nitems) & length_mask; /* Exclusive */
         
        size_t head_current_raw = atomicb_load_explicit_size(head_index, memory_order_bridge_acquire);
        
        /* Validate the indices */

        /*
         * May not be resizing
         * Must have equal encoded lengths (otherwise size check is invalid)
         */

        if (head_current_raw == SIZE_MAX) {
            rl_pause_intrin();
            current_index = atomicb_load_explicit_size(allocated_tail_index, memory_order_bridge_acquire);
            continue;
        }
        head_current_raw &= RL_CDRAIN_QUEUE_INDEX_LENGTH_MASK; /* Remove bitfields */

        const size_t length_head = rl_round_floor_log2_size(head_current_raw);

        if (length_head != length) {
            rl_pause_intrin();
            current_index = atomicb_load_explicit_size(allocated_tail_index, memory_order_bridge_acquire);
            continue;
        }

        /* Finished validating the head index */

        const size_t queue_start = head_current_raw & length_mask; /* Inclusive */

        /* Find remaining length to ensure there is enough space */
        const size_t remaining_length = get_remaining_length(queue_start, allocated_start, length);

        if (remaining_length > nitems) {
            /* space available */
            if (!atomicb_compare_exchange_strong_explicit_size(allocated_tail_index,
                    &current_index, allocated_end | length, memory_order_bridge_seq_cst, 
                    memory_order_bridge_seq_cst)) {
                ++failures;
                continue;
            }

            void *__restrict queued_elements = queue->elements;
            if (allocated_end >= allocated_start) {
                /* Sequential */
                arraycopy(queued_elements, allocated_start, elements, 0, nitems, elem_sizeof);
            } else {
                /* Possibly wrapped */
                const size_t end = length - allocated_start;
                arraycopy(queued_elements, allocated_start, elements, 0, end, elem_sizeof);
                arraycopy(queued_elements, 0, elements, end, nitems - end, elem_sizeof);
            }

            rl_wait_set(available_tail_index, current_index, allocated_end | length);
            return 0;
        }
        
        const size_t required_length = (length - remaining_length) + nitems; /* Exclusive end of queue once resized and copied */
        if ((flags & RL_CDRAIN_QUEUE_NORESIZE) || required_length >= max_length) {
            return RL_ENOMEM;
        }
        
        /* Attempt to lock allocated_tail_index */
        if (!atomicb_compare_exchange_strong_explicit_size(allocated_tail_index, &current_index, SIZE_MAX,
                memory_order_bridge_seq_cst, memory_order_bridge_seq_cst)) {
            ++failures;
            rl_pause_intrin();
            continue;
        }
        
        /* Add 1 to ensure that the resized queue will not be full */
        const size_t expected_length = rl_round_ceil_log2_size(required_length + 1);
        void *new_elements = rl_smalloc(expected_length, elem_sizeof);
        void *curr_elements = queue->elements;
        
        /* Copy previous elements over */
        /* Note that while new indices cannot be allocated, some writes to the queue may not be finished */
        /* wait for previous writes to complete */
        
        while (atomicb_load_explicit_size(available_tail_index, memory_order_bridge_acquire) != current_index) {
            rl_pause_intrin();
        }

        /* Copy the elements over */
        
        if (queue_start < allocated_start) {
            /* Sequential */
            arraycopy(new_elements, 0, curr_elements, queue_start, length - remaining_length, elem_sizeof);
        } else {
            /* Wrapped */
            const size_t end = length - queue_start;
            arraycopy(new_elements, 0, curr_elements, queue_start, end, elem_sizeof);
            arraycopy(new_elements, end, curr_elements, 0, allocated_start, elem_sizeof);
        }
        
        /* Copy our elements over */
        
        arraycopy(new_elements, length - remaining_length, elements, 0, nitems, elem_sizeof);
        
        /* Update head */
        
        for (size_t curr = atomicb_load_explicit_size(head_index, memory_order_bridge_acquire);;) {
            if (curr >= READING_BIT) {
                sched_yield();
                curr = atomicb_load_explicit_size(head_index, memory_order_bridge_acquire);
                continue;
            }
            /* Create new head index */
            /* Elements may have been read during resizing, re-adjust head */
            
            const size_t old_head = queue_start;
            const size_t updated_head = curr ^ length;
            
            const size_t new_head = get_queue_length(old_head, updated_head, length_mask) | expected_length;
            
            /* Try to lock head */
            if (!atomicb_compare_exchange_strong_explicit_size(head_index, &curr, SIZE_MAX,
                    memory_order_bridge_seq_cst, memory_order_bridge_seq_cst)) {
                rl_pause_intrin();
                continue;
            }
            
            queue->elements = new_elements;
            
            atomicb_store_explicit_size(available_tail_index, required_length | expected_length, memory_order_bridge_relaxed);
            
            atomicb_store_explicit_size(allocated_tail_index, required_length | expected_length, memory_order_bridge_release);

            atomicb_store_explicit_size(head_index, new_head, memory_order_bridge_seq_cst);
            /* Unlocked head */
            
            free(curr_elements);
            
            break;
        }
        
        return 0;
    }
}





size_t rl_cdrain_queue_drain(struct rl_cdrain_queue *__restrict queue, void *__restrict buffer, 
    const size_t max_items, const size_t elem_sizeof, const unsigned int flags) {
    size_t head;
    if (flags & RL_CDRAIN_QUEUE_NORESIZE) {
        head = atomicb_load_explicit_size(&queue->head_index, memory_order_bridge_seq_cst);
    } else {
        for (;;) {
            head = atomicb_fetch_or_explicit_size(&queue->head_index, READING_BIT, memory_order_bridge_seq_cst);
            if (head >= READING_BIT) {
                rl_pause_intrin();
                continue;
            }
            break;
        }
    }

    /* Since writes to tail are seq_cst, and sychronization already occured, tail is pretty up-to-date */
    /* Use acquire to synchronize with the state at tail's point in time */
    size_t tail = atomicb_load_explicit_size(&queue->available_tail_index, memory_order_bridge_acquire);

    /* By locking the head it is guaranteed the tail will have the same length as head */
    void *__restrict elements = queue->elements;

    const size_t length = rl_round_floor_log2_size(head);
    const size_t length_mask = length - 1;
    
    const size_t tail_encoded = tail;

    tail ^= length;
    head ^= length;

    size_t items_to_read = get_queue_length(head, tail, length_mask);

    if (items_to_read >= max_items) {
        items_to_read = max_items;
    }
    
    const size_t new_head = (head + items_to_read) & length_mask;
    
    if (buffer) {
        if (new_head >= head) {
            arraycopy(buffer, 0, elements, head, items_to_read, elem_sizeof);
        } else {
            const size_t end = length - head;
            arraycopy(buffer, 0, elements, head, end, elem_sizeof);
            arraycopy(buffer, end, elements, 0, items_to_read - end, elem_sizeof);
        }
    }

    if (flags & RL_CDRAIN_QUEUE_PEEK) {
        /* Only write if there's a need */
        if (!(flags & RL_CDRAIN_QUEUE_NORESIZE)) {
            atomicb_store_explicit_size(&queue->head_index, head | length, memory_order_bridge_seq_cst);
        }
    } else {
        atomicb_store_explicit_size(&queue->head_index, new_head | length, memory_order_bridge_seq_cst);
    }
    
    return items_to_read;
}

void rl_cdrain_queue_clear(struct rl_cdrain_queue *queue) {
    rl_cdrain_queue_drain(queue, NULL, SIZE_MAX, 0, 0); /* elem_sizeof only used if buffer != NULL */
}

static void get_queue_indices(struct rl_cdrain_queue *queue, const unsigned int flags, 
    size_t *headp, size_t *tailp, size_t *length) {
    volatile size_t *head_index = &queue->head_index;
    volatile size_t *available_tail_index = &queue->available_tail_index;
    for (;;) {
        size_t head = atomicb_load_explicit_size(head_index, memory_order_bridge_seq_cst);
        const size_t tail = atomicb_load_explicit_size(available_tail_index, memory_order_bridge_acquire);

        if (head == SIZE_MAX) {
            rl_pause_intrin();
            continue;
        }

        if (tail == SIZE_MAX) {
            sched_yield();
            continue;
        }
        
        head &= RL_CDRAIN_QUEUE_INDEX_LENGTH_MASK; /* Allow to be called during interrupt (ignores reading bit flag) */

        const size_t length_head = rl_round_floor_log2_size(head);
        const size_t length_tail = rl_round_floor_log2_size(tail);
        if (length_head != length_tail) {
            rl_pause_intrin();
            continue;
        }

        *length = length_head;
        *headp = head ^ length_head;
        *tailp = tail ^ length_head;

        return;
    }
}


size_t rl_cdrain_queue_size(struct rl_cdrain_queue *queue, const unsigned int flags) {
    size_t head, tail, length;
    get_queue_indices(queue, flags, &head, &tail, &length);

    return get_queue_length(head, tail, length - 1);
}

size_t rl_cdrain_queue_remaining_capacity(struct rl_cdrain_queue *queue, const unsigned int flags) {
    size_t head, tail, length;
    get_queue_indices(queue, flags, &head, &tail, &length);

    return get_remaining_length(head, tail, length);
}


size_t rl_cdrain_queue_capacity(struct rl_cdrain_queue *queue, const unsigned int flags) {
    if (flags & RL_CDRAIN_QUEUE_NORESIZE) {
        /* Use head as there is likely less contention, especially with this flag set */
        const size_t head = atomicb_load_explicit_size(&queue->head_index, memory_order_bridge_seq_cst);
        return rl_round_floor_log2_size(head & RL_CDRAIN_QUEUE_INDEX_LENGTH_MASK);
    }

    volatile size_t *allocated_tail_index = &queue->allocated_tail_index;

    for (;;) {
        const size_t tail = atomicb_load_explicit_size(allocated_tail_index, memory_order_bridge_seq_cst);

        if (tail == SIZE_MAX) {
            sched_yield();
            continue;
        }

        return rl_round_floor_log2_size(tail);
    }
}




int rl_cdrain_queue_init(struct rl_cdrain_queue *queue, size_t capacity, const size_t elem_sizeof) {
    if (capacity > RL_CDRAIN_QUEUE_MAX_LENGTH) {
        return RL_ENOMEM;
    } else if (capacity <= 32) {
        capacity = 32;
    } else {
        capacity = rl_round_ceil_log2_size(capacity);
    }

    if (capacity > obtain_max_len(elem_sizeof)) {
        return RL_ENOMEM;
    }

    void *__restrict elements = rl_smalloc(capacity, elem_sizeof);

    queue->elements = elements;

    atomicb_store_explicit_size(&queue->head_index, capacity, memory_order_bridge_relaxed);
    atomicb_store_explicit_size(&queue->available_tail_index, capacity, memory_order_bridge_relaxed);
    atomicb_store_explicit_size(&queue->allocated_tail_index, capacity, memory_order_bridge_relaxed);
    return 0;
}

void rl_cdrain_queue_free(struct rl_cdrain_queue *queue) {
    free(queue->elements);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
 