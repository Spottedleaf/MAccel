/* Start rustedleaf copy-pastes */
#ifdef __linux__
#include <byteswap.h>
#endif /* __linux__ */

#ifdef _WIN32
#include <intrin.h>
#endif /* _WIN32 */

/* End rustedleaf copy-pastes */

#include <Windows.h>

#include <stdint.h>
#include <stddef.h>

#include "..\atomic_bridge.h"

#include "rl_defs.h"
#include "rl_cdrain_queue.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Start rustedleaf copy-pastes */

/* rl_math.h */

/*
 * Returns the ceil of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_ceil_log2_u32(const uint32_t val) {
    #ifdef __GNUC__
    return (sizeof(val) * CHAR_BIT) - __builtin_clzl(val - 1);
    #elif defined _WIN32
    return (sizeof(val) * CHAR_BIT) - __lzcnt(val - 1);
    #else
    abort();
    #endif
}

/*
 * Returns the ceil of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_ceil_log2_u64(const uint64_t val) {
    #ifdef __GNUC__
    return (sizeof(val) * CHAR_BIT) - __builtin_clzll(val - 1);
    #elif defined _WIN32
    return (sizeof(val) * CHAR_BIT) - (unsigned int ) __lzcnt64(val - 1);
    #else
    abort();
    #endif
}

/*
 * Returns the ceil of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_ceil_log2_size(size_t val) {
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
    #ifdef __GNUC__
    return (sizeof(val) * CHAR_BIT - 1) ^ __builtin_clzl(val);
    #elif defined _WIN32
    return (sizeof(val) * CHAR_BIT - 1) ^ __lzcnt(val);
    #else
    abort();
    #endif
}

/*
 * Returns the floor of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_floor_log2_u64(const uint64_t val) {
    #ifdef __GNUC__
    return (sizeof(val) * CHAR_BIT - 1) ^ __builtin_clzll(val);
    #elif defined _WIN32
    return (sizeof(val) * CHAR_BIT - 1) ^ (unsigned int) __lzcnt64(val);
    #else
    abort();
    #endif
}

/*
 * Returns the floor of the log2 of the specified value. Undefined result if val == 0
 */
static inline unsigned int rl_floor_log2_size(size_t val) {
    if (SIZE_MAX <= UINT32_MAX) {
        return rl_floor_log2_u32((uint32_t) val);
    } else if (SIZE_MAX <= UINT64_MAX) {
        return rl_floor_log2_u64((uint64_t) val);
    }
    abort();
}

static inline bool rl_mulsize_overflow(const size_t a, const size_t b) {
    return !b && ((a * b) / b) != a;
}

/* rl_memutil.h */

static void *rl_smalloc(const size_t nitems, const size_t itemsize) {
    if (rl_mulsize_overflow(nitems, itemsize)) {
        printf("Overflow of multiplication for memory allocation: %zi, %zi\n", nitems, itemsize);
        exit(EXIT_FAILURE);
    }
    void *ret = malloc(nitems * itemsize);
    if (!ret) {
        printf("Unable to allocate %zi bytes of memory\n", (nitems * itemsize));
        exit(EXIT_FAILURE);
    }
    return ret;
}


static inline void *rl_ptr_offset(const void *ptr, const size_t off) {
    const uint8_t *ptr_u8 = (const uint8_t *) ptr;
    return ptr_u8 + off;
}

/* End rustedleaf copy pastes */

static inline void sched_yield(void) {
    SwitchToThread();
}

static const size_t HIGH_BIT = SIZE_MAX ^ (SIZE_MAX >> 1);
static const size_t READING_BIT = (SIZE_MAX ^ (SIZE_MAX >> 1));
#define RL_CDRAIN_QUEUE_INDEX_MASK ((RL_CDRAIN_QUEUE_MAX_LENGTH << 1) - 1)

static size_t obtain_max_len(const size_t elem_sizeof) {
    /* The maximum length contains no index which multiplied by elem_sizeof is equal to or greater than SIZE_MAX */
    /* Or that is larger than RL_CDRAIN_QUEUE_MAX_LENGTH */
    
    size_t len = SIZE_MAX / elem_sizeof;
    if (len >= RL_CDRAIN_QUEUE_MAX_LENGTH) {
        return RL_CDRAIN_QUEUE_MAX_LENGTH;
    }
    
    return ((size_t)1) << rl_floor_log2_size(len);
}

static void rl_wait_set(volatile size_t *value, const size_t wait, const size_t set) {
    for (;;) {
        size_t temp = wait;
        /* This will synchronize with all previous additions */
        if (atomicb_compare_exchange_weak_explicit_size(value, &temp, set, 
                memory_order_bridge_seq_cst, memory_order_bridge_seq_cst)) {
            break;
        }
        rl_pause_intrin();
        break;
    }
}

int rl_cdrain_queue_add(struct rl_cdrain_queue *queue, const void *elements, const size_t nitems, const size_t elem_sizeof, const unsigned int flags) {
    /* This is a maximum length which will not overflow */
    const size_t max_length = obtain_max_len(elem_sizeof);
    
    if (nitems > max_length) {
        return RL_ENOMEM;
    }

    volatile size_t *head_index = &queue->head_index.value;
    volatile size_t *available_tail_index = &queue->available_tail_index.value;
    volatile size_t *allocated_tail_index = &queue->allocated_tail_index.value;
    
    size_t current_index = atomicb_load_explicit_size(allocated_tail_index, memory_order_bridge_acquire);
main_loop_start:
    for (;;) {
        /* Test if resizing */
        if (current_index == SIZE_MAX) {
            if (flags & RL_CDRAIN_QUEUE_NORESIZE) {
                return RL_ENOMEM;
            }
            do {
                sched_yield();
            } while (current_index == SIZE_MAX);
        }
        
        /* Not resizing */
        
        /* Decode length */
        const size_t length = ((size_t)1) << rl_floor_log2_size(current_index);
        const size_t length_mask = length - 1;
        
        const size_t allocated_start = current_index ^ length; /* Inclusive */
        const size_t queue_end = (allocated_start - 1) & length_mask; /* Inclusive */
        const size_t allocated_end = (current_index + nitems) & length_mask; /* Exclusive */
        
        size_t head_current_raw = atomicb_load_explicit_size(head_index, memory_order_bridge_acquire);
        
        /* Validate the head index */
        if (head_current_raw == SIZE_MAX) {
            rl_pause_intrin();
            continue;
        }
        head_current_raw &= RL_CDRAIN_QUEUE_INDEX_MASK; /* Remove bitfields */

        const size_t length_head = ((size_t)1) << rl_floor_log2_size(head_current_raw);

        if (length_head != length) {
            rl_pause_intrin();
            continue;
        }
        const size_t queue_start = head_current_raw & length_mask; /* Inclusive */

        /* Find remaining length to ensure there would be enough space */
        size_t remaining_length;
        if (queue_start <= queue_end) {
            //remaining_length = length - (queue_end - queue_start);
            remaining_length = length - queue_end + queue_start;
        } else {
            /* Find the space between the indices */
            remaining_length = queue_start - queue_end - 1;
        }

        if (remaining_length >= nitems) {
            /* space available */
            const size_t last_index = (allocated_end - 1) & length_mask; /* Inclusive */
            if (!atomicb_compare_exchange_strong_explicit_size(allocated_tail_index, &current_index, allocated_end | length,
                memory_order_bridge_seq_cst, memory_order_bridge_seq_cst)) {
                continue;
            }

            void *__restrict queued_elements = queue->elements;
            if (last_index >= allocated_start) {
                /* Sequential */
                memcpy(rl_ptr_offset(queued_elements, allocated_start * elem_sizeof), elements, nitems);
            } else {
                /* Wrapped */
                const size_t end = length - allocated_start;
                memcpy(rl_ptr_offset(queued_elements, allocated_start * elem_sizeof), elements, end);

                memcpy(queued_elements, rl_ptr_offset(elements, end), nitems - end);
            }

            rl_wait_set(available_tail_index, queue_end | length, last_index | length);

            return 0;
        }
        
        const size_t required_length = (length - remaining_length) + nitems;
        if ((flags & RL_CDRAIN_QUEUE_NORESIZE) || required_length >= max_length) {
            return RL_ENOMEM;
        }
        
        /* Attempt to lock allocated_tail_index */
        if (!atomicb_compare_exchange_strong_explicit_size(allocated_tail_index, &current_index, SIZE_MAX,
                memory_order_bridge_seq_cst, memory_order_bridge_seq_cst)) {
            rl_pause_intrin();
            continue;
        }
        
        /* Add 1 to ensure that the resized queue will not be full */
        const size_t expected_length = ((size_t)1) << rl_ceil_log2_size(required_length + 1);
        void *new_elements = rl_smalloc(expected_length, elem_sizeof);
        void *curr_elements = queue->elements;
        
        /* Copy previous elements over */
        /* Note that while new indices cannot be allocated, some writes to the queue may not be finished */
        /* wait for previous writes to complete */
        
        while (atomicb_load_explicit_size(available_tail_index, memory_order_bridge_seq_cst) != (queue_end | length)) {
            sched_yield();
        }

        /* Copy the elements over */
        
        if (queue_start < queue_end) {
            /* sequentially ordered */
            memcpy(new_elements, rl_ptr_offset(curr_elements, queue_start), length - remaining_length);
        } else {
            /* wrapped */
            
            /* copy from head->max index */
            memcpy(new_elements, rl_ptr_offset(curr_elements, queue_start), length - queue_start);
            
            /* copy from 0->tail */
            memcpy(rl_ptr_offset(new_elements, length - queue_start), curr_elements, queue_end + 1);
        }
        
        /* Copy our elements over */
        
        memcpy(rl_ptr_offset(new_elements, length - remaining_length), elements, nitems * elem_sizeof);
        
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
            
            size_t new_head;
            
            if (updated_head >= old_head) {
                new_head = (updated_head - old_head) | expected_length;
            } else {
                new_head = ((length - old_head) + (updated_head + 1)) | expected_length;
            }
            
            /* Try to lock head */
            if (!atomicb_compare_exchange_strong_explicit_size(head_index, &curr, SIZE_MAX,
                    memory_order_bridge_seq_cst, memory_order_bridge_seq_cst)) {
                rl_pause_intrin();
                continue;
            }
            
            queue->elements = new_elements;
            
            atomicb_store_explicit_size(available_tail_index, (required_length - 1) | expected_length, memory_order_bridge_relaxed);
            
            atomicb_store_explicit_size(head_index, new_head, memory_order_bridge_seq_cst);
            /* Unlocked head */
            
            free(curr_elements);
            
            break;
        }
        
        return 0;
    }
}

size_t rl_cdrain_queue_size(struct rl_cdrain_queue *queue) {
    volatile size_t *head_index = &queue->head_index.value;
    volatile size_t *available_tail_index = &queue->available_tail_index.value;
    for (;;) {
        size_t head = atomicb_load_explicit_size(head_index, memory_order_bridge_seq_cst);

        if (head == SIZE_MAX) {
            rl_pause_intrin();
            continue;
        }

        head &= RL_CDRAIN_QUEUE_INDEX_MASK; /* Allow to be called during interrupt (ignores reading bit flag) */

        size_t tail = atomicb_load_explicit_size(available_tail_index, memory_order_bridge_seq_cst);
        
        const size_t length_head = ((size_t)1) << rl_floor_log2_size(head);
        const size_t length_tail = ((size_t)1) << rl_floor_log2_size(tail);
        if (length_head != length_tail) {
            rl_pause_intrin();
            continue;
        }
        
        const size_t head_encoded = head;
        const size_t length_mask = length_head - 1;
        head &= length_mask;
        tail ^= length_head;
        
        if (tail >= head) {
            return tail - head;
        } else {
            return tail - head_encoded + 1;
        }
    }
}

size_t rl_cdrain_queue_drain(struct rl_cdrain_queue *queue, void *buffer, const size_t max_items, const size_t elem_sizeof, const unsigned int flags) {
    size_t head_encoded;
    for (;;) {
        head_encoded = atomicb_fetch_or_explicit_size(&queue->head_index.value, READING_BIT, memory_order_bridge_seq_cst);
        if (head_encoded == SIZE_MAX) {
            rl_pause_intrin();
            continue;
        }
        break;
    }
    /* By locking the head it is guaranteed the tail will have the same length as head */
    void *__restrict elements = queue->elements;

    const size_t length = ((size_t)1) << rl_floor_log2_size(head_encoded);
    const size_t length_mask = length - 1;
    const size_t head_decoded = head_encoded ^ length;
    
    const size_t tail = atomicb_load_explicit_size(&queue->available_tail_index.value, memory_order_bridge_relaxed) ^ length;

    size_t items;
    size_t items_to_read;

    if (tail >= head_decoded) {
        items = tail - head_decoded;
    } else {
        /* head_encoded = head_decoded + length */
        // items = length - (head_decoded - tail - 1);
        items = tail - head_encoded + 1;
    }

    if (items > max_items) {
        items_to_read = max_items;
    } else {
        items_to_read = items;
    }
    
    const size_t new_head = (head_decoded + items_to_read) & length_mask;
    
    if (buffer) {
        if (tail >= new_head) {
            memcpy(buffer, rl_ptr_offset(elements, head_decoded), items_to_read);
        } else {
            const size_t end = length - head_decoded;
            memcpy(buffer, rl_ptr_offset(elements, head_decoded), end);
            memcpy(rl_ptr_offset(buffer, end), elements, items_to_read - end);
        }
    }

    if (flags & RL_CDRAIN_QUEUE_PEEK) {
        atomicb_store_explicit_size(&queue->head_index.value, head_encoded, memory_order_bridge_seq_cst);
    } else {
        atomicb_store_explicit_size(&queue->head_index.value, new_head | length, memory_order_bridge_seq_cst);
    }
    
    return items_to_read;
}

int rl_cdrain_queue_init(struct rl_cdrain_queue *queue, size_t capacity, const size_t elem_sizeof) {
    if (capacity > RL_CDRAIN_QUEUE_MAX_LENGTH) {
        return RL_ENOMEM;
    }
    
    if (capacity < 32) {
        capacity = 32;
    } else {
        capacity = ((size_t)1) << rl_ceil_log2_size(capacity);
    }
    
    void *__restrict elements = rl_smalloc(capacity, elem_sizeof);
    
    queue->elements = elements;
    
    atomicb_store_explicit_size(&queue->head_index.value, capacity | (capacity - 1), memory_order_bridge_relaxed);
    atomicb_store_explicit_size(&queue->available_tail_index.value, capacity | (capacity - 1), memory_order_bridge_relaxed);
    atomicb_store_explicit_size(&queue->allocated_tail_index.value, capacity, memory_order_bridge_relaxed);
    return 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
 