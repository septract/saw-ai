/*
 * Demonstration of uninterpreted functions in SAW
 *
 * This example shows how treating a function as "uninterpreted"
 * can simplify proofs when the same function appears on both sides
 * of an equivalence.
 */

#include <stdint.h>

// A "complex" function - imagine this is SHA-256 or similar
// The actual implementation doesn't matter for the demo
uint32_t complex_hash(uint32_t x) {
    // Some arbitrary complex computation
    uint32_t h = x;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = (h >> 16) ^ h;
    return h;
}

// Apply hash twice - "double hashing"
uint32_t double_hash(uint32_t x) {
    return complex_hash(complex_hash(x));
}

// Same operation, written slightly differently
uint32_t double_hash_v2(uint32_t x) {
    uint32_t first = complex_hash(x);
    uint32_t second = complex_hash(first);
    return second;
}

// A function that uses the hash in a specific pattern
uint32_t hash_xor_pattern(uint32_t a, uint32_t b) {
    return complex_hash(a) ^ complex_hash(b);
}

// Equivalent implementation
uint32_t hash_xor_pattern_v2(uint32_t a, uint32_t b) {
    uint32_t ha = complex_hash(a);
    uint32_t hb = complex_hash(b);
    return ha ^ hb;
}
