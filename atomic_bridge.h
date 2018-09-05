#ifndef ATOMIC_BRIDGE_H
#define ATOMIC_BRIDGE_H ATOMIC_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

enum memory_order_bridge {
    memory_order_bridge_relaxed,
    memory_order_bridge_consume,
    memory_order_bridge_acquire,
    memory_order_bridge_release,
    memory_order_bridge_acq_rel,
    memory_order_bridge_seq_cst
};

/* uint8_t */

void atomicb_store_explicit_u8(volatile uint8_t *value, uint8_t desired, enum memory_order_bridge order);
uint8_t atomicb_load_explicit_u8(volatile uint8_t *value, enum memory_order_bridge order);

uint8_t atomicb_exchange_explicit_u8(volatile uint8_t *value, uint8_t desired, enum memory_order_bridge order);
bool atomicb_compare_exchange_strong_explicit_u8(volatile uint8_t *value, uint8_t *expected,
    uint8_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);
bool atomicb_compare_exchange_weak_explicit_u8(volatile uint8_t *value, uint8_t *expected,
    uint8_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);

uint8_t atomicb_fetch_add_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order);
uint8_t atomicb_fetch_sub_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order);
uint8_t atomicb_fetch_or_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order);
uint8_t atomicb_fetch_xor_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order);
uint8_t atomicb_fetch_and_explicit_u8(volatile uint8_t *value, uint8_t param, enum memory_order_bridge order);

/* uint16_t */

void atomicb_store_explicit_u16(volatile uint16_t *value, uint16_t desired, enum memory_order_bridge order);
uint16_t atomicb_load_explicit_u16(volatile uint16_t *value, enum memory_order_bridge order);

uint16_t atomicb_exchange_explicit_u16(volatile uint16_t *value, uint16_t desired, enum memory_order_bridge order);
bool atomicb_compare_exchange_strong_explicit_u16(volatile uint16_t *value, uint16_t *expected,
    uint16_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);
bool atomicb_compare_exchange_weak_explicit_u16(volatile uint16_t *value, uint16_t *expected,
    uint16_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);

uint16_t atomicb_fetch_add_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order);
uint16_t atomicb_fetch_sub_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order);
uint16_t atomicb_fetch_or_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order);
uint16_t atomicb_fetch_xor_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order);
uint16_t atomicb_fetch_and_explicit_u16(volatile uint16_t *value, uint16_t param, enum memory_order_bridge order);

/* uint32_t */

void atomicb_store_explicit_u32(volatile uint32_t *value, uint32_t desired, enum memory_order_bridge order);
uint32_t atomicb_load_explicit_u32(volatile uint32_t *value, enum memory_order_bridge order);

uint32_t atomicb_exchange_explicit_u32(volatile uint32_t *value, uint32_t desired, enum memory_order_bridge order);
bool atomicb_compare_exchange_strong_explicit_u32(volatile uint32_t *value, uint32_t *expected,
    uint32_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);
bool atomicb_compare_exchange_weak_explicit_u32(volatile uint32_t *value, uint32_t *expected,
    uint32_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);

uint32_t atomicb_fetch_add_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order);
uint32_t atomicb_fetch_sub_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order);
uint32_t atomicb_fetch_or_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order);
uint32_t atomicb_fetch_xor_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order);
uint32_t atomicb_fetch_and_explicit_u32(volatile uint32_t *value, uint32_t param, enum memory_order_bridge order);

/* uint64_t */

void atomicb_store_explicit_u64(volatile uint64_t *value, uint64_t desired, enum memory_order_bridge order);
uint64_t atomicb_load_explicit_u64(volatile uint64_t *value, enum memory_order_bridge order);

uint64_t atomicb_exchange_explicit_u64(volatile uint64_t *value, uint64_t desired, enum memory_order_bridge order);
bool atomicb_compare_exchange_strong_explicit_u64(volatile uint64_t *value, uint64_t *expected,
    uint64_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);
bool atomicb_compare_exchange_weak_explicit_u64(volatile uint64_t *value, uint64_t *expected,
    uint64_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);

uint64_t atomicb_fetch_add_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order);
uint64_t atomicb_fetch_sub_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order);
uint64_t atomicb_fetch_or_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order);
uint64_t atomicb_fetch_xor_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order);
uint64_t atomicb_fetch_and_explicit_u64(volatile uint64_t *value, uint64_t param, enum memory_order_bridge order);


/* size_t */

void atomicb_store_explicit_size(volatile size_t *value, size_t desired, enum memory_order_bridge order);
size_t atomicb_load_explicit_size(volatile size_t *value, enum memory_order_bridge order);

size_t atomicb_exchange_explicit_size(volatile size_t *value, size_t desired, enum memory_order_bridge order);
bool atomicb_compare_exchange_strong_explicit_size(volatile size_t *value, size_t *expected,
    size_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);
bool atomicb_compare_exchange_weak_explicit_size(volatile size_t *value, size_t *expected,
    size_t desired, enum memory_order_bridge succ, enum memory_order_bridge fail);

size_t atomicb_fetch_add_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order);
size_t atomicb_fetch_sub_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order);
size_t atomicb_fetch_or_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order);
size_t atomicb_fetch_xor_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order);
size_t atomicb_fetch_and_explicit_size(volatile size_t *value, size_t param, enum memory_order_bridge order);

#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* ATOMIC_BRIDGE_H */