
#include <stdint.h>
#include <stddef.h>

#include <atomic>

#include "atomic_bridge.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static inline std::memory_order convert_order(enum memory_order_bridge from) {
    return (std::memory_order) from;
}

/* uint8_t */

inline void atomicb_store_explicit_u8(volatile uint8_t *value, uint8_t desired, enum memory_order_bridge order) {
    std::atomic_store_explicit((std::atomic_uint8_t *)value, desired, convert_order(order));
}

inline uint8_t atomicb_load_explicit_u8(volatile uint8_t *value, enum memory_order_bridge order) {
    return std::atomic_load_explicit((std::atomic_uint8_t *)value, convert_order(order));
}

inline uint8_t atomicb_exchange_explicit_u8(volatile uint8_t *value, uint8_t desired, enum memory_order_bridge order) {
    return std::atomic_exchange_explicit((std::atomic_uint8_t *)value, desired, convert_order(order));
}

inline bool atomicb_compare_exchange_strong_explicit_u8(volatile uint8_t *value, uint8_t *expected,
    uint8_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_strong_explicit((std::atomic_uint8_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline bool atomicb_compare_exchange_weak_explicit_u8(volatile uint8_t *value, uint8_t *expected,
    uint8_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_weak_explicit((std::atomic_uint8_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline uint8_t atomicb_fetch_add_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_add_explicit((std::atomic_uint8_t *)value, param, convert_order(order));
}

inline uint8_t atomicb_fetch_sub_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_sub_explicit((std::atomic_uint8_t *)value, param, convert_order(order));
}

inline uint8_t atomicb_fetch_or_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_or_explicit((std::atomic_uint8_t *)value, param, convert_order(order));
}

inline uint8_t atomicb_fetch_xor_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_xor_explicit((std::atomic_uint8_t *)value, param, convert_order(order));
}

inline uint8_t atomicb_fetch_and_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_and_explicit((std::atomic_uint8_t *)value, param, convert_order(order));
}

/* uint16_t */

inline void atomicb_store_explicit_u16(volatile uint16_t *value, uint16_t desired, enum memory_order_bridge order) {
    std::atomic_store_explicit((std::atomic_uint16_t *)value, desired, convert_order(order));
}

inline uint16_t atomicb_load_explicit_u16(volatile uint16_t *value, enum memory_order_bridge order) {
    return std::atomic_load_explicit((std::atomic_uint16_t *)value, convert_order(order));
}

inline uint16_t atomicb_exchange_explicit_u16(volatile uint16_t *value, uint16_t desired, enum memory_order_bridge order) {
    return std::atomic_exchange_explicit((std::atomic_uint16_t *)value, desired, convert_order(order));
}

inline bool atomicb_compare_exchange_strong_explicit_u16(volatile uint16_t *value, uint16_t *expected,
    uint16_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_strong_explicit((std::atomic_uint16_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline bool atomicb_compare_exchange_weak_explicit_u16(volatile uint16_t *value, uint16_t *expected,
    uint16_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_weak_explicit((std::atomic_uint16_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline uint16_t atomicb_fetch_add_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_add_explicit((std::atomic_uint16_t *)value, param, convert_order(order));
}

inline uint16_t atomicb_fetch_sub_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_sub_explicit((std::atomic_uint16_t *)value, param, convert_order(order));
}

inline uint16_t atomicb_fetch_or_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_or_explicit((std::atomic_uint16_t *)value, param, convert_order(order));
}

inline uint16_t atomicb_fetch_xor_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_xor_explicit((std::atomic_uint16_t *)value, param, convert_order(order));
}

inline uint16_t atomicb_fetch_and_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_and_explicit((std::atomic_uint16_t *)value, param, convert_order(order));
}

/* uint32_t */

inline void atomicb_store_explicit_u32(volatile uint32_t *value, uint32_t desired, enum memory_order_bridge order) {
    std::atomic_store_explicit((std::atomic_uint32_t *)value, desired, convert_order(order));
}

inline uint32_t atomicb_load_explicit_u32(volatile uint32_t *value, enum memory_order_bridge order) {
    return std::atomic_load_explicit((std::atomic_uint32_t *)value, convert_order(order));
}

inline uint32_t atomicb_exchange_explicit_u32(volatile uint32_t *value, uint32_t desired, enum memory_order_bridge order) {
    return std::atomic_exchange_explicit((std::atomic_uint32_t *)value, desired, convert_order(order));
}

inline bool atomicb_compare_exchange_strong_explicit_u32(volatile uint32_t *value, uint32_t *expected,
    uint32_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_strong_explicit((std::atomic_uint32_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline bool atomicb_compare_exchange_weak_explicit_u32(volatile uint32_t *value, uint32_t *expected,
    uint32_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_weak_explicit((std::atomic_uint32_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline uint32_t atomicb_fetch_add_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_add_explicit((std::atomic_uint32_t *)value, param, convert_order(order));
}

inline uint32_t atomicb_fetch_sub_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_sub_explicit((std::atomic_uint32_t *)value, param, convert_order(order));
}

inline uint32_t atomicb_fetch_or_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_or_explicit((std::atomic_uint32_t *)value, param, convert_order(order));
}

inline uint32_t atomicb_fetch_xor_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_xor_explicit((std::atomic_uint32_t *)value, param, convert_order(order));
}

inline uint32_t atomicb_fetch_and_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_and_explicit((std::atomic_uint32_t *)value, param, convert_order(order));
}

/* uint64_t */

inline void atomicb_store_explicit_u64(volatile uint64_t *value, uint64_t desired, enum memory_order_bridge order) {
    std::atomic_store_explicit((std::atomic_uint64_t *)value, desired, convert_order(order));
}

inline uint64_t atomicb_load_explicit_u64(volatile uint64_t *value, enum memory_order_bridge order) {
    return std::atomic_load_explicit((std::atomic_uint64_t *)value, convert_order(order));
}

inline uint64_t atomicb_exchange_explicit_u64(volatile uint64_t *value, uint64_t desired, enum memory_order_bridge order) {
    return std::atomic_exchange_explicit((std::atomic_uint64_t *)value, desired, convert_order(order));
}

inline bool atomicb_compare_exchange_strong_explicit_u64(volatile uint64_t *value, uint64_t *expected,
    uint64_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_strong_explicit((std::atomic_uint64_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline bool atomicb_compare_exchange_weak_explicit_u64(volatile uint64_t *value, uint64_t *expected,
    uint64_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_weak_explicit((std::atomic_uint64_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline uint64_t atomicb_fetch_add_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_add_explicit((std::atomic_uint64_t *)value, param, convert_order(order));
}

inline uint64_t atomicb_fetch_sub_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_sub_explicit((std::atomic_uint64_t *)value, param, convert_order(order));
}

inline uint64_t atomicb_fetch_or_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_or_explicit((std::atomic_uint64_t *)value, param, convert_order(order));
}

inline uint64_t atomicb_fetch_xor_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_xor_explicit((std::atomic_uint64_t *)value, param, convert_order(order));
}

inline uint64_t atomicb_fetch_and_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_and_explicit((std::atomic_uint64_t *)value, param, convert_order(order));
}

/* size_t */

inline void atomicb_store_explicit_size(volatile size_t *value, size_t desired, enum memory_order_bridge order) {
    std::atomic_store_explicit((std::atomic_size_t *)value, desired, convert_order(order));
}

inline size_t atomicb_load_explicit_size(volatile size_t *value, enum memory_order_bridge order) {
    return std::atomic_load_explicit((std::atomic_size_t *)value, convert_order(order));
}

inline size_t atomicb_exchange_explicit_size(volatile size_t *value, size_t desired, enum memory_order_bridge order) {
    return std::atomic_exchange_explicit((std::atomic_size_t *)value, desired, convert_order(order));
}

inline bool atomicb_compare_exchange_strong_explicit_size(volatile size_t *value, size_t *expected,
    size_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_strong_explicit((std::atomic_size_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline bool atomicb_compare_exchange_weak_explicit_size(volatile size_t *value, size_t *expected,
    size_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail) {
    return std::atomic_compare_exchange_weak_explicit((std::atomic_size_t *)value, expected, desired,
        convert_order(succ), convert_order(fail));
}

inline size_t atomicb_fetch_add_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_add_explicit((std::atomic_size_t *)value, param, convert_order(order));
}

inline size_t atomicb_fetch_sub_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_sub_explicit((std::atomic_size_t *)value, param, convert_order(order));
}

inline size_t atomicb_fetch_or_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_or_explicit((std::atomic_size_t *)value, param, convert_order(order));
}

inline size_t atomicb_fetch_xor_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_xor_explicit((std::atomic_size_t *)value, param, convert_order(order));
}

inline size_t atomicb_fetch_and_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order) {
    return std::atomic_fetch_and_explicit((std::atomic_size_t *)value, param, convert_order(order));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */