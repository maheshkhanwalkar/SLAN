#pragma once

#include "defs.h"
#include <stdint.h>

/* TODO: improve docs, since the current ones are somewhat
 * non-descriptive */

/**
 * Encode src into dest using Most Significant Byte (MSB)
 * @param src - source 32-bit int
 * @param dest - dest octet buffer
 */
void msb_encode32(uint32_t src, octet* dest);

/**
 * Decode src into dest using MSB
 * @param src - source octets
 * @param dest - destination int
 */
void msb_decode32(const octet* src, uint32_t* dest);

/**
 * Encode src into dest using MSB
 * @param src - source 64-bit int
 * @param dest - dest octet buffer
 */
void msb_encode64(uint64_t src, octet* dest);
