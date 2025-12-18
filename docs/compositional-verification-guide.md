# Compositional Verification Guide

A practical guide to verifying complex functions in SAW using compositional techniques. Based on lessons learned from verifying AES-128 encryption/decryption.

## The Problem

When verifying complex functions like `aes_encrypt`, direct symbolic verification often fails:

1. **Term explosion** - Symbolic execution produces massive terms
2. **Solver timeout** - SMT solvers can't handle the complexity
3. **Memory exhaustion** - Terms don't fit in memory

For AES-128, direct verification of `aes_encrypt` with symbolic plaintext timed out after hours.

## The Solution: Compositional Verification

Break the problem into manageable pieces:

1. **Verify primitives separately** - Each small function gets its own proof
2. **Use overrides** - Replace verified functions with their specs during larger proofs
3. **Keep functions uninterpreted** - Don't expand function definitions in the solver
4. **Use lemmas and simpsets** - Prove structural equivalences and use them for rewriting

## Technique 1: Overrides (Compositional Verification)

### Concept

When verifying a function that calls other functions, you can "override" those calls with their Cryptol specifications instead of re-executing the C code.

### Pattern

```saw
// Step 1: Verify primitive and capture as override
let SubBytes_spec = do {
    state_ptr <- llvm_alloc state_type;
    state_in <- llvm_fresh_var "state_in" state_type;
    llvm_points_to state_ptr (llvm_term state_in);
    llvm_execute_func [state_ptr];
    llvm_points_to state_ptr (llvm_term {{ SubBytes state_in }});
};
SubBytes_ov <- llvm_verify m "SubBytes" [] false SubBytes_spec z3;

// Step 2: Use override when verifying larger function
llvm_verify m "aes_encrypt" [SubBytes_ov, ShiftRows_ov, ...] false encrypt_spec z3;
```

### What Happens

- When SAW symbolically executes `aes_encrypt` and encounters a call to `SubBytes`
- Instead of executing SubBytes's C code, it substitutes `{{ SubBytes state_in }}`
- The result is a term with Cryptol function applications, not expanded C code

### Benefits

- Avoids re-verifying already-proven code
- Produces cleaner, more structured terms
- Reduces symbolic execution time

## Technique 2: Uninterpreted Functions

### Concept

Keep certain functions as opaque symbols during SMT solving. The solver only knows that `f(x) == f(x)` for any `x`, not what `f` actually computes.

### Pattern

```saw
// In the proof script (final argument to llvm_verify)
llvm_verify m "aes_encrypt" overrides false spec
    do {
        simplify simpset;
        w4_unint_z3 ["SubBytes", "ShiftRows", "MixColumns", "AddRoundKey"];
    };
```

### When It Works

Uninterpreted functions help when the same function appears identically on both sides of an equivalence:

```
C side:      AddRoundKey(k, MixColumns(ShiftRows(SubBytes(state))))
Cryptol side: AddRoundKey(k, MixColumns(ShiftRows(SubBytes(state))))
```

With these functions uninterpreted, the solver sees:
```
f(k, g(h(i(state)))) == f(k, g(h(i(state))))
```

This is trivially true by reflexivity!

### When It Doesn't Work

- When you need to reason about what the function actually computes
- When the function appears differently on each side
- When proving properties that depend on the function's internals

### Critical Insight: What NOT to Keep Uninterpreted

Sometimes you need functions to be evaluated, not kept abstract.

**Example from AES:** We kept `SubBytes`, `ShiftRows`, `MixColumns`, `AddRoundKey` uninterpreted, but NOT `msgToState` and `stateToMsg`.

Why? The C code extracts bytes from state via array indexing:
```c
out[0] = state[0][0]; out[1] = state[1][0]; ...
```

Cryptol uses `stateToMsg`:
```cryptol
stateToMsg st = join (join (transpose st))
```

These are semantically equivalent but structurally different. If `stateToMsg` is uninterpreted, the solver can't see they're equal. By letting it evaluate, the solver can match the byte patterns.

**Rule of thumb:** Keep "interesting" functions (SubBytes, crypto operations) uninterpreted. Let "boring" functions (data marshalling, type conversions) evaluate.

## Technique 3: Unroll Lemmas and Simpsets

### The Problem

The Cryptol `cipher` function uses recursion:
```cryptol
cipher : KeySchedule -> [128] -> [128]
cipher w pt = stateToMsg final_state
  where
    state0 = AddRoundKey (w@0) (msgToState pt)
    rounds = [1..Nr-1]
    states = [state0] # [round w s | s <- states | i <- rounds]
    final_state = AddRoundKey (w@Nr) (ShiftRows (SubBytes (last states)))
```

The C code is explicitly unrolled (10 rounds of function calls, no loop).

With overrides, C produces:
```
AddRoundKey(w@10, ShiftRows(SubBytes(AddRoundKey(w@9, MixColumns(...)))))
```

But the spec is just `cipher w pt`, which doesn't structurally match.

### The Solution: Unroll Lemmas

Prove that `cipher` equals its unrolled form:

```saw
unroll_cipher_128 <- prove_print
    (w4_unint_z3 ["AddRoundKey", "MixColumns", "SubBytes", "ShiftRows"])
    {{ \w pt -> cipher w pt ==
    (stateToMsg (AddRoundKey (w@10) (ShiftRows (SubBytes (t 9 (t 8 (t 7 (t 6 (t 5 (t 4 (t 3 (t 2 (t 1 (AddRoundKey (w@0) (msgToState pt))))))))))))))
        where
        t i state = AddRoundKey (w@i) (MixColumns (ShiftRows (SubBytes state))))
    }};
```

This proves with uninterpreted primitives because both sides have the same structure after unrolling.

### Using Simpsets

Add the lemma to a simpset and use it to rewrite goals:

```saw
let ss = cryptol_ss ();
let ss_with_unroll = addsimps [unroll_cipher_128] ss;

llvm_verify m "aes_encrypt" overrides false spec
    do {
        simplify ss_with_unroll;  // Rewrites cipher -> unrolled form
        w4_unint_z3 ["SubBytes", "ShiftRows", "MixColumns", "AddRoundKey"];
    };
```

After `simplify`, the goal becomes:
```
C_result == unrolled_cipher_form
```

Both sides now have the same abstract structure, so the solver proves by reflexivity.

## Complete Recipe for Complex Verification

### Step 1: Identify the Primitives

List all the "leaf" functions that your target function calls.

For AES: `SubBytes`, `InvSubBytes`, `ShiftRows`, `InvShiftRows`, `MixColumns`, `InvMixColumns`, `AddRoundKey`

### Step 2: Verify Each Primitive

Write specs and verify each primitive independently:

```saw
SubBytes_ov <- llvm_verify m "SubBytes" [] false SubBytes_spec z3;
ShiftRows_ov <- llvm_verify m "ShiftRows" [] false ShiftRows_spec z3;
// ... etc
```

### Step 3: Prove Unroll Lemmas (if needed)

If your Cryptol spec uses recursion but C code is unrolled:

```saw
unroll_lemma <- prove_print
    (w4_unint_z3 ["primitive1", "primitive2", ...])
    {{ spec_function == explicit_unrolled_form }};
```

### Step 4: Build Simpset

```saw
let ss = cryptol_ss ();
let ss_with_lemmas = addsimps [unroll_lemma] ss;
```

### Step 5: Verify Target Function

```saw
llvm_verify m "target_function"
    [SubBytes_ov, ShiftRows_ov, ...]  // All primitive overrides
    false
    target_spec
    do {
        simplify ss_with_lemmas;
        w4_unint_z3 ["SubBytes", "ShiftRows", ...];  // Keep primitives uninterpreted
    };
```

## Debugging Tips

### 1. Start with Concrete Inputs

If symbolic verification times out, try with concrete inputs first to verify your spec is correct:

```saw
let concrete_test_spec = do {
    // Use concrete values instead of llvm_fresh_var
    llvm_points_to input_ptr (llvm_term {{ 0x00112233... }});
    // ...
};
```

### 2. Check Override Application

SAW logs when overrides are applied:
```
[timestamp] Applied override! SubBytes
```

If you don't see these messages, overrides aren't matching. Check:
- Function name spelling
- Argument types
- Memory layout assumptions

### 3. Examine Failed Goals

When a proof fails, SAW shows:
- **Expected term** - What the spec says
- **Actual term** - What the C code produces

Look for structural differences. Common issues:
- Type mismatches (e.g., `[16][8]` vs `[128]`)
- Byte ordering differences
- Functions that should evaluate but are kept uninterpreted

### 4. Test Simpset Rewriting

Verify your simpset actually rewrites terms:

```saw
// Test in isolation
prove_print
    do { simplify ss_with_lemmas; w4_unint_z3 [...]; }
    {{ cipher w pt == cipher w pt }};
```

### 5. Incremental Verification

Don't try to verify everything at once:

1. Verify primitives first
2. Test with concrete key, concrete plaintext
3. Test with concrete key, symbolic plaintext
4. Finally try symbolic key if needed

## Performance Expectations

With compositional verification and uninterpreted functions:

| Phase | Time |
|-------|------|
| Verify primitives (7 functions) | ~10 min |
| Prove unroll lemmas | < 1 sec |
| Verify aes_encrypt (symbolic plaintext) | ~10 sec |
| Verify aes_decrypt (symbolic ciphertext) | ~10 sec |
| **Total** | **~14 min** |

Without these techniques: **Timeout (hours+)**

## Summary

1. **Decompose** - Break complex functions into verified primitives
2. **Override** - Use proven specs instead of re-executing C code
3. **Uninterpret** - Keep complex functions abstract in the solver
4. **Rewrite** - Use lemmas to transform specs to match C structure
5. **Be selective** - Know which functions to keep abstract vs. evaluate

## References

- [uninterpreted-functions-in-saw.md](uninterpreted-functions-in-saw.md) - Details on uninterpreted functions
- [saw-pitfalls.md](saw-pitfalls.md) - Common gotchas to avoid
- [Galois AES example](../../tools/saw/examples/aes/aes.saw) - Reference implementation
- [aes_verify_encrypt_unint.saw](../../experiments/crypto-algorithms/aes/aes_verify_encrypt_unint.saw) - Working AES verification
