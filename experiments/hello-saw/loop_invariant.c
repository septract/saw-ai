/*
 * Loop Invariant Verification Examples
 *
 * FINDING: ALL live variables at the breakpoint MUST be passed to the
 * breakpoint function. If any live variable is not passed, SAW fails with:
 *   "Cannot find (unsafe) reg value rXX in TypedRegMap"
 *
 * Examples progress from simple to SHA256-like structure.
 */

#include <stddef.h>
#include <stdint.h>

// ============================================================
// Example 1: add2 - Simple breakpoint (no loop)
// Variables: x (1 total, 1 passed)
// ============================================================

extern size_t __breakpoint__inv(size_t*);

size_t add2(size_t x) {
    ++x;
    __breakpoint__inv(&x);
    ++x;
    return x;
}

// ============================================================
// Example 2: count_n - Basic loop with invariant
// Variables: n, c, i (3 total, 3 passed)
// ============================================================

extern size_t __breakpoint__count_inv(size_t*, size_t*, size_t*)
    __attribute__((noduplicate));

size_t count_n(size_t n) {
    size_t c = 0;
    for (size_t i = 0; __breakpoint__count_inv(&n, &c, &i), i < n; ++i) {
        ++c;
    }
    return c;
}

// ============================================================
// Example 3: Simple accumulator with 4 variables
// Testing if the SSA error is related to loop body complexity
// Variables: a, b, n, i (4 total, 4 passed)
// ============================================================

extern size_t __breakpoint__acc_inv(size_t*, size_t*, size_t*, size_t*)
    __attribute__((noduplicate));

size_t accumulate(size_t a, size_t n) {
    size_t b = 0;
    for (size_t i = 0;
         __breakpoint__acc_inv(&a, &b, &n, &i), i < n;
         ++i) {
        // Simple body like count_n, but updating both a and b
        ++a;
        ++b;
    }
    return a + b;
}

