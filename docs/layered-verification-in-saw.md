# Layered Verification in SAW

This guide covers techniques for verifying complex functions that are too large for direct symbolic proof. These techniques are based on approaches used in production verification projects including AWS s2n HMAC, OpenSSL SHA3, and AWS LibCrypto.

> **Related Documentation**: For detailed examples with working code, see:
> - [compositional-verification-guide.md](compositional-verification-guide.md) - Practical guide with AES examples
> - [uninterpreted-functions-in-saw.md](uninterpreted-functions-in-saw.md) - Deep dive on uninterpreted functions
> - [saw-pitfalls.md](saw-pitfalls.md) - Common gotchas and errors

## The Problem

When verifying cryptographic code, direct symbolic proof often fails because:

1. **Term explosion**: Symbolic execution of loops generates terms that grow exponentially
2. **Solver timeout**: SMT solvers can't handle millions of clauses
3. **Memory exhaustion**: SAW or the solver runs out of memory

For example, SHA256's 64-round compression function generates a 3.5MB+ SAWCore term that no solver can handle directly.

## Solution: Layered Verification

Break the proof into layers, proving smaller pieces and composing them.

```
Layer 3: Full function      sha256_transform
            ↑ uses override
Layer 2: Sub-components     compress, messageSchedule
            ↑ uses override
Layer 1: Primitives         Ch, Maj, Sigma0, Sigma1, sigma0, sigma1
            ↑ direct proof
Layer 0: Axioms             Bitvector operations (trusted)
```

## Technique 1: Compositional Verification with Overrides

**Concept**: Prove a function correct, then use that proof as an "override" when verifying callers.

### Step 1: Prove the leaf function

```saw
// Prove single round (C signature: void sha256_round(uint32_t state[8], uint32_t ki, uint32_t wi))
let round_spec = do {
    // Arrays are passed by pointer in C
    state_ptr <- llvm_alloc (llvm_array 8 (llvm_int 32));
    state_in <- llvm_fresh_var "state_in" (llvm_array 8 (llvm_int 32));
    llvm_points_to state_ptr (llvm_term state_in);

    ki <- llvm_fresh_var "ki" (llvm_int 32);
    wi <- llvm_fresh_var "wi" (llvm_int 32);

    llvm_execute_func [state_ptr, llvm_term ki, llvm_term wi];

    // State is modified in place
    llvm_points_to state_ptr (llvm_term {{ round state_in ki wi }});
};

round_ov <- llvm_verify m "sha256_round" [] true round_spec z3;
```

### Step 2: Use override when proving caller

```saw
// Prove compress using round as override
let compress_spec = do {
    state_ptr <- llvm_alloc (llvm_array 8 (llvm_int 32));
    state_in <- llvm_fresh_var "state_in" (llvm_array 8 (llvm_int 32));
    llvm_points_to state_ptr (llvm_term state_in);

    ws_ptr <- llvm_alloc_readonly (llvm_array 64 (llvm_int 32));
    ws <- llvm_fresh_var "ws" (llvm_array 64 (llvm_int 32));
    llvm_points_to ws_ptr (llvm_term ws);

    llvm_execute_func [state_ptr, ws_ptr];

    llvm_points_to state_ptr (llvm_term {{ compress state_in ws }});
};

// Pass round_ov as override - SAW substitutes instead of re-proving
compress_ov <- llvm_verify m "sha256_compress" [round_ov] true compress_spec z3;
```

### Key Points

- The `true` parameter enables path satisfiability checking
- Overrides replace function calls with their specs during symbolic execution
- This prevents re-expanding already-proven code
- **Limitation**: Only works if C code has actual function calls (not macros)

## Technique 2: Uninterpreted Functions

**Concept**: Tell the solver to treat certain functions as black boxes (axiomatically equal to their specs).

### When to Use

- When primitives are inlined (macros) and can't be overridden
- When you want to prove structural equivalence without expanding definitions
- When the solver struggles with specific sub-expressions

### SAW Syntax

```saw
// Mark functions as uninterpreted
llvm_verify m "sha256_transform" [] false transform_spec
    (w4_unint_z3 ["Ch", "Maj", "Sigma0", "Sigma1"]);
```

### How It Works

1. SAW generates proof obligation: `llvm_output == cryptol_spec`
2. With uninterpreted `Ch`, solver sees: `Ch(x,y,z) == Ch(x,y,z)` (trivially true)
3. Solver only checks structure matches, not that Ch is computed correctly

### Caveat: Macro Expansion

If C uses macros:
```c
#define CH(x,y,z) (((x) & (y)) ^ (~(x) & (z)))
```

LLVM expands this inline. There's no `Ch` function to uninterpret. Solutions:

1. **Match LLVM structure in Cryptol**: Write Cryptol that produces the same term structure
2. **Uninterpret sub-expressions**: If `(x & y)` appears repeatedly, uninterpret that
3. **Use rewriting**: Simplify terms before solving

## Technique 3: Two-Stage Verification

**Concept**: Prove C matches a low-level spec, then separately prove low-level spec matches high-level spec.

```
Stage 1: C code ←→ Low-level Cryptol (structural match, tractable)
Stage 2: Low-level Cryptol ←→ High-level Cryptol (pure Cryptol proof)
```

### Why This Works

- Stage 1 is tractable because low-level Cryptol mirrors C structure
- Stage 2 is pure Cryptol, which SAW/Cryptol can reason about with rewriting

### Example: s2n HMAC Verification

```saw
// Stage 1: C matches implementation spec
let hmac_impl_spec = do {
    // ... setup ...
    llvm_return (llvm_term {{ hmac_impl key msg }});
};
llvm_verify m "s2n_hmac" [] false hmac_impl_spec z3;

// Stage 2: Prove impl == spec in Cryptol
impl_eq_spec <- prove_print z3
    {{ \key msg -> hmac_impl key msg == hmac_spec key msg }};
```

### The Low-Level Spec Pattern

Low-level specs should:

1. **Mirror C structure**: Same loop structure, same variable organization
2. **Match LLVM optimizations**: If LLVM transforms `~x & z` to `(x & z) ^ z`, do the same
3. **Use same types**: Match array sizes, word sizes exactly
4. **Preserve evaluation order**: Compute things in same order as C

```cryptol
// High-level (mathematical)
Ch x y z = (x && y) ^ (~x && z)

// Low-level (matches LLVM optimization)
Ch_impl x y z = (x && y) ^ (x && z) ^ z

// Prove equivalence separately
property Ch_correct x y z = Ch_impl x y z == Ch x y z
```

## Technique 4: Simpset Rewriting

**Concept**: Simplify terms before sending to solver using rewrite rules.

### Creating Simpsets

```saw
// Add proven lemmas to simpset
ch_lemma <- prove_print z3 {{ \x y z -> Ch_impl x y z == Ch x y z }};
let my_ss = addsimps [ch_lemma] (cryptol_ss ());

// Use simpset in proof
llvm_verify m "sha256_transform" [] false spec
    (do { simplify my_ss; z3; });
```

### Built-in Simpsets

```saw
cryptol_ss ()     // Standard Cryptol simplifications
basic_ss          // Basic logical simplifications
```

### Custom Rewrite Rules

```saw
// Rewrite rule: replace left side with right side
rule <- prove_print z3 {{ \x -> expensive_form x == simple_form x }};
```

## Technique 5: Goal Inspection and Debugging

When proofs fail or timeout, inspect what the solver sees.

### Print the Goal

```saw
llvm_verify m "func" [] false spec
    (do {
        print_goal;      // Show full goal
        assume_unsat;    // Skip actual solving
    });
```

### Evaluate Before Printing

```saw
llvm_verify m "func" [] false spec
    (do {
        goal_eval_unint ["Ch", "Maj"];  // Evaluate, keeping some uninterpreted
        print_goal;
        assume_unsat;
    });
```

### Compare Term Structures

For complex debugging, use `print_goal` to see what the solver receives:

```saw
llvm_verify m "sha256_transform" [] false transform_spec
    (do {
        print_goal;      // Shows the proof obligation before solving
        assume_unsat;    // Skip solving to just see the goal
    });
```

You can also test simpset rewriting in isolation:

```saw
// Verify your simpset rewrites terms correctly
prove_print
    do { simplify my_ss; w4_unint_z3 ["Ch", "Maj"]; }
    {{ transform w data == transform w data }};
```

## Technique 6: Incremental Bit Reduction

When full symbolic proof fails, reduce symbolic bits to find the threshold.

### Strategy

Start with fully concrete values and progressively add symbolic bits:

1. **Fully concrete**: Both state and data are concrete test vectors
2. **Partial symbolic**: State is symbolic (256 bits), data is concrete
3. **Full symbolic**: Both state and data are symbolic (768+ bits)

```saw
// Level 1: Concrete test - verifies spec correctness
let concrete_spec = do {
    state_ptr <- llvm_alloc (llvm_array 8 (llvm_int 32));
    llvm_points_to state_ptr (llvm_term {{ test_state }});
    // ... concrete data setup ...
    llvm_execute_func [state_ptr, data_ptr];
    llvm_points_to state_ptr (llvm_term {{ transform test_state test_data }});
};

// Level 2: Symbolic state (256 bits) - tests solver capacity
let partial_symbolic_spec = do {
    state_ptr <- llvm_alloc (llvm_array 8 (llvm_int 32));
    state <- llvm_fresh_var "state" (llvm_array 8 (llvm_int 32));
    llvm_points_to state_ptr (llvm_term state);
    // ... concrete data setup ...
    llvm_execute_func [state_ptr, data_ptr];
    llvm_points_to state_ptr (llvm_term {{ transform state test_data }});
};
```

### Interpretation

- If concrete works but symbolic fails: structure is correct, solver capacity exceeded
- If concrete fails: specification bug, fix before scaling up

## Putting It Together: SHA256 Strategy

Based on these techniques, here's a layered approach for SHA256:

### Layer 1: Validate Spec (Concrete)

```saw
// Test vectors - no solver stress
test1 <- prove_print z3 {{ sha256 "abc" == expected_hash1 }};
```

### Layer 2: Primitives (Small Symbolic)

```saw
// 96 bits each, should be instant
ch_correct <- llvm_verify m "ch" [] false ch_spec z3;
maj_correct <- llvm_verify m "maj" [] false maj_spec z3;
// ... etc
```

### Layer 3: Single Round (Medium Symbolic)

```saw
// 320 bits, should be <10 seconds
round_ov <- llvm_verify m "sha256_round" [] true round_spec z3;
```

### Layer 4: Full Transform (Large Symbolic)

Try in order until one works:

```saw
// Option A: Direct proof
llvm_verify m "sha256_transform" [] false transform_spec z3;

// Option B: With overrides (if C has functions)
llvm_verify m "sha256_transform" [round_ov, schedule_ov] false transform_spec z3;

// Option C: With uninterpreted functions
llvm_verify m "sha256_transform" [] false transform_spec
    (w4_unint_z3 ["Ch", "Maj", "Sigma0", "Sigma1", "sigma0", "sigma1"]);

// Option D: With simpset
llvm_verify m "sha256_transform" [] false transform_spec
    (do { simplify my_ss; z3; });
```

## References

- [Verifying s2n HMAC with SAW](https://galois.com/blog/2016/09/verifying-s2n-hmac-with-saw/)
- [Continuous Formal Verification of Amazon s2n](https://d1.awsstatic.com/Security/pdfs/Continuous_Formal_Verification_Of_Amazon_s2n.pdf)
- [SHA3 OpenSSL Verification](https://github.com/ericmercer/sha3-verification)
- [SAW Manual - Compositional Verification](https://saw.galois.com/manual.html)
- [Towards Verifying SHA256 in OpenSSL](https://dl.acm.org/doi/abs/10.1007/978-3-030-76384-8_5)
