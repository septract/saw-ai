/*
 * Loop Invariant Examples - Simple to Complex
 *
 * Based on docs/saw-loop-invariants.md
 * Demonstrates SAW's __breakpoint__ mechanism for loop invariants.
 *
 * KEY INSIGHT: ALL live variables at the breakpoint MUST be passed to the
 * breakpoint function, otherwise SAW fails with:
 *   "Cannot find (unsafe) reg value rXX in TypedRegMap"
 *
 * Build: clang -emit-llvm -c -g -O0 loop_invariant_examples.c -o loop_invariant_examples.bc
 * Verify: saw loop_invariant_examples.saw
 */

#include <stddef.h>
#include <stdint.h>

/* ============================================================
 * Example 1: Simple breakpoint (no loop)
 *
 * Demonstrates basic breakpoint mechanics without loop complexity.
 * The breakpoint splits add2 into two logical functions:
 *   - add2: increments x, calls breakpoint
 *   - breakpoint: increments x again, returns
 * ============================================================ */

extern size_t __breakpoint__add2_inv(size_t*);

size_t add2(size_t x) {
    ++x;
    __breakpoint__add2_inv(&x);
    ++x;
    return x;
}

/* ============================================================
 * Example 2: Basic counting loop
 *
 * Classic loop invariant: c + (n - i) = n
 * At any point in the loop, the counter plus remaining iterations equals n.
 *
 * Variables: n (bound), c (counter), i (index)
 * All 3 must be passed to breakpoint.
 * ============================================================ */

extern size_t __breakpoint__count_inv(size_t*, size_t*, size_t*)
    __attribute__((noduplicate));

size_t count_n(size_t n) {
    size_t c = 0;
    for (size_t i = 0; __breakpoint__count_inv(&n, &c, &i), i < n; ++i) {
        ++c;
    }
    return c;
}

/* ============================================================
 * Example 3: Sum of array elements
 *
 * Demonstrates breakpoint with pointer to array data.
 * Invariant: sum + remaining elements = total sum
 *
 * Variables: arr (pointer), n (length), sum (accumulator), i (index)
 * ============================================================ */

extern uint32_t __breakpoint__sum_inv(uint32_t**, size_t*, uint32_t*, size_t*)
    __attribute__((noduplicate));

uint32_t sum_array(uint32_t* arr, size_t n) {
    uint32_t sum = 0;
    for (size_t i = 0; __breakpoint__sum_inv(&arr, &n, &sum, &i), i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

/* ============================================================
 * Example 4: Two accumulators
 *
 * Loop that updates two values each iteration.
 * Shows how multiple state variables work together.
 *
 * Variables: a, b (accumulators), n (bound), i (index)
 * Result: a increments by 1 each iteration, b increments by 2
 * ============================================================ */

extern size_t __breakpoint__dual_inv(size_t*, size_t*, size_t*, size_t*)
    __attribute__((noduplicate));

size_t dual_accumulate(size_t a, size_t n) {
    size_t b = 0;
    for (size_t i = 0; __breakpoint__dual_inv(&a, &b, &n, &i), i < n; ++i) {
        a += 1;
        b += 2;
    }
    return a + b;  // a + n + 2*n = a + 3*n
}

/* ============================================================
 * Example 5: Fixed iteration loop (unrolled-friendly)
 *
 * Loop with small fixed bound (4 iterations).
 * SAW can verify this without invariants, but we use breakpoints
 * to demonstrate the technique scales.
 *
 * Variables: x (accumulator), i (index)
 * ============================================================ */

extern uint32_t __breakpoint__fixed_inv(uint32_t*, size_t*)
    __attribute__((noduplicate));

uint32_t add_fixed_4(uint32_t x) {
    for (size_t i = 0; __breakpoint__fixed_inv(&x, &i), i < 4; ++i) {
        x += 10;
    }
    return x;  // x + 40
}

/* ============================================================
 * Example 6: Nested state transformation
 *
 * Each iteration applies: x = x * 2 + 1
 * After n iterations: x_n = x_0 * 2^n + (2^n - 1)
 *
 * This is harder to express as an invariant because it's exponential.
 * We verify for fixed n=3 iterations.
 * ============================================================ */

extern uint32_t __breakpoint__transform_inv(uint32_t*, size_t*)
    __attribute__((noduplicate));

uint32_t transform_3(uint32_t x) {
    for (size_t i = 0; __breakpoint__transform_inv(&x, &i), i < 3; ++i) {
        x = x * 2 + 1;
    }
    return x;  // x*8 + 7
}

/* ============================================================
 * Example 7: XOR reduction (crypto-relevant)
 *
 * XOR all elements of an array - common in crypto operations.
 * Invariant: result so far XOR remaining = final result
 *
 * Variables: arr (pointer), n (length), result (xor accumulator), i (index)
 * ============================================================ */

extern uint32_t __breakpoint__xor_inv(uint32_t**, size_t*, uint32_t*, size_t*)
    __attribute__((noduplicate));

uint32_t xor_reduce(uint32_t* arr, size_t n) {
    uint32_t result = 0;
    for (size_t i = 0; __breakpoint__xor_inv(&arr, &n, &result, &i), i < n; ++i) {
        result ^= arr[i];
    }
    return result;
}

/* ============================================================
 * Example 8: Fibonacci (REQUIRES breakpoint - unbounded n)
 *
 * Computes fib(n) iteratively. Cannot be unrolled because n is symbolic.
 *
 * Invariant: At iteration i, (a, b) = (fib(i), fib(i+1))
 * After n iterations: a = fib(n)
 *
 * We avoid temp variables by using simultaneous assignment pattern:
 * new_a = b, new_b = a + b
 * ============================================================ */

extern uint64_t __breakpoint__fib_inv(uint64_t*, uint64_t*, size_t*, size_t*)
    __attribute__((noduplicate));

uint64_t fib(size_t n) {
    uint64_t a = 0;
    uint64_t b = 1;
    for (size_t i = 0; __breakpoint__fib_inv(&a, &b, &n, &i), i < n; ++i) {
        // Compute new values using old a before updating
        b = a + b;  // new_b = old_a + old_b
        a = b - a;  // new_a = new_b - old_a = old_b
    }
    return a;
}

/* ============================================================
 * Example 9: Power/Exponentiation (REQUIRES breakpoint - unbounded n)
 *
 * Computes base^n via repeated multiplication.
 * Cannot be unrolled because n is symbolic.
 *
 * Invariant: result * base^(n-i) = base^n
 * At iteration i: result = base^i
 *
 * Variables: base, result, n, i
 * ============================================================ */

extern uint64_t __breakpoint__power_inv(uint64_t*, uint64_t*, size_t*, size_t*)
    __attribute__((noduplicate));

uint64_t power(uint64_t base, size_t n) {
    uint64_t result = 1;
    for (size_t i = 0; __breakpoint__power_inv(&base, &result, &n, &i), i < n; ++i) {
        result = result * base;
    }
    return result;
}

/* ============================================================
 * Example 10: GCD by subtraction (REQUIRES breakpoint - while loop)
 *
 * Euclidean algorithm variant using subtraction.
 * Loop bound is data-dependent - cannot be unrolled.
 *
 * Invariant: gcd(a, b) = gcd(a_original, b_original)
 * The GCD is preserved through each subtraction.
 *
 * Variables: a, b (converging values)
 * ============================================================ */

extern uint64_t __breakpoint__gcd_inv(uint64_t*, uint64_t*)
    __attribute__((noduplicate));

uint64_t gcd_subtract(uint64_t a, uint64_t b) {
    while (__breakpoint__gcd_inv(&a, &b), a != b) {
        if (a > b) {
            a = a - b;
        } else {
            b = b - a;
        }
    }
    return a;
}
