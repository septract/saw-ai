# Uninterpreted Functions in SAW

A technique for managing proof complexity by treating certain functions as abstract symbols.

## Overview

When verifying complex programs with SAW, proof goals can become intractable due to the size of the terms involved. **Uninterpreted functions** provide a mechanism to abstract away complexity: instead of expanding a function's definition, SAW treats it as an opaque symbol where **the only known property is that equal inputs produce equal outputs**.

This is useful when:
1. A function has already been verified separately
2. A function's internal logic is irrelevant to the current proof
3. Proof terms become too large for SMT solvers to handle

## The Core Mechanism

### What Happens Internally

When SAW symbolically executes code, it builds a **term** (essentially a circuit or expression tree) representing the computation. By default, all function calls are inlined/expanded. With uninterpreted functions:

- The function call is represented as an **uninterpreted function symbol** in the logic
- The SMT solver knows nothing about the function except `f(x) == f(x)` for any x
- Common subterms containing the function can be **unified**, dramatically reducing proof complexity

### Example Intuition

Consider proving that `hash(hash(x)) == hash(hash(x))` where `hash` is SHA-256.

**Without uninterpreted functions:** SAW would expand SHA-256's definition twice on each side, creating massive terms that may timeout solvers.

**With `hash` uninterpreted:** The solver sees `f(f(x)) == f(f(x))` and trivially proves it by reflexivity.

## SAW Commands for Uninterpreted Functions

### 1. Solver Tactics with `_unint_` Variants

The primary interface. Each SMT solver has an `_unint_` variant:

```saw
// Using Z3 with uninterpreted functions
prove (w4_unint_z3 ["functionName"]) {{ spec x == impl x }};

// Using Yices
prove (w4_unint_yices ["functionName"]) {{ spec x == impl x }};

// Using CVC4/CVC5
prove (w4_unint_cvc4 ["functionName"]) {{ spec x == impl x }};
```

**Key point:** The string list contains **Cryptol function names** to keep uninterpreted.

### 2. `goal_eval_unint` - Evaluate Goals with Exceptions

Evaluates a proof goal to first-order form while **preserving** specified names:

```saw
prove do {
  goal_eval_unint ["sha256", "aes_encrypt"];
  w4;  // Then use any prover
} {{ property }};
```

Unlike regular `goal_eval`, this won't expand the listed functions.

### 3. `term_eval_unint` - Same for Arbitrary Terms (v1.4+)

Like `goal_eval_unint` but operates on `Term` values outside proof scripts:

```saw
let simplified = term_eval_unint ["complex_function"] myTerm;
```

### 4. `llvm_unint`, `jvm_unint`, `mir_unint` - During Symbolic Simulation

**WARNING: These commands do NOT exist in SAW 1.4.** They are planned for a future
release (see [CHANGES.md](https://github.com/GaloisInc/saw-script/blob/master/CHANGES.md)).

In the future version, these will keep Cryptol names opaque during symbolic execution:

```saw
// FUTURE SAW VERSION ONLY - not available in SAW 1.4
llvm_unint ["cryptol_spec_function"];
```

**For SAW 1.4:** Use `w4_unint_z3` in the proof script instead. With compositional
verification (overrides), the C code is replaced with Cryptol specs, and those
specs can be kept uninterpreted at proof time.

### 5. `offline_unint_smtlib2` - Export with Uninterpreted Functions

Exports the goal to SMT-LIB2 format with specified functions as uninterpreted:

```saw
offline_unint_smtlib2 ["hash_function"] "output.smt2" {{ goal }};
```

## Practical Example: HMAC Verification

From Galois's verification of Amazon s2n's HMAC:

```saw
// Hash functions are treated as uninterpreted
// because they're verified separately and would
// explode proof complexity if inlined

llvm_verify m "s2n_hmac_digest" [hash_spec]
  do {
    // ... setup code ...
  }
  do {
    unint_z3 [ "hash_init_c_state"
             , "hash_update_c_state"
             , "hash_digest_c_state"
             ];
  };
```

**Why this works:** The HMAC proof only cares that hash functions:
1. Are deterministic (same input gives same output)
2. Satisfy their specifications (verified elsewhere)

It doesn't need to reason about SHA-256's internal bit manipulations.

## Relationship to Compositional Verification

These are **complementary but different** techniques:

| Aspect | Compositional Verification | Uninterpreted Functions |
|--------|---------------------------|------------------------|
| **What** | Replace C function calls with Cryptol specs | Keep Cryptol functions abstract in SMT |
| **When** | During symbolic execution | During proof solving |
| **Effect** | Avoids re-simulating verified code | Avoids expanding definitions in terms |
| **Use case** | Layered C function verification | Managing term complexity |

**Often used together:** Compositional verification creates overrides for C functions, but the resulting proof goals may still contain complex Cryptol specs that benefit from uninterpreted treatment.

## Rewriting as a Related Technique

SAW's **simpsets** provide another approach to term simplification:

```saw
// Prove small lemmas first
let lemma1 = prove_print abc {{ f x == g x }};
let lemma2 = prove_print abc {{ h y == k y }};

// Combine into a simpset
let ss = addsimps [lemma1, lemma2] empty_ss;

// Use to simplify larger proofs
prove_print do { simplify ss; abc; } {{ complex_goal }};
```

**Difference:** Rewriting **replaces** terms with proven equivalents. Uninterpreted functions **abstract** terms without replacement.

## Important Caveats

### 1. Not All Solvers Support Uninterpreted Functions

| Solver | Uninterpreted Function Support |
|--------|-------------------------------|
| Z3, Yices, CVC4/5 | Yes |
| ABC (AIG-based) | No |
| Boolector | Limited |

ABC is a propositional solver and cannot handle uninterpreted functions at all.

### 2. Potential for Vacuous Proofs

If you over-abstract, you might prove something useless:

```saw
// BAD: If the code actually depends on internal hash behavior,
// this might prove a goal that's actually false for real SHA-256
prove (w4_unint_z3 ["sha256"]) {{ broken_code_using_sha256 == spec }};
```

**Mitigation:** Only use uninterpreted functions for things genuinely abstract to the proof.

### 3. Type Restrictions

`goal_eval_unint` has issues with some types:

```saw
// This works (Integer type)
let f: Integer -> Integer
prove (w4_unint_z3 ["f"]) {{ f 0 == f 0 }};

// This may fail (modular arithmetic types like Z 7)
let g: Z 7 -> Z 7
// May error: "could not create uninterpreted function argument of type (IntMod 7)"
```

**Workaround:** Use `w4_unint_z3` directly instead of `goal_eval_unint` + `w4` for problematic types.

### 4. Choosing What to Keep Uninterpreted

This is "one of the main intellectual challenges in completing a proof" (Galois).

Too little abstraction: Proofs timeout
Too much abstraction: Proofs become vacuous or unsolvable

**Heuristic:** Keep specs uninterpreted when they appear identically on both sides of an equivalence.

## Advanced: How Common Subterms Help

Consider verifying `impl_encrypt(impl_encrypt(x)) == spec_encrypt(spec_encrypt(x))`.

If you've proved `impl_encrypt == spec_encrypt` and have both as a spec `enc`:

```saw
// With enc uninterpreted:
// impl side becomes: enc(enc(x))
// spec side becomes: enc(enc(x))
// Solver: trivially equal by reflexivity!
```

This is the key insight: **when C code is overridden with a Cryptol spec, and that spec is kept uninterpreted, common subterms on both sides of equivalences can be unified**.

## Summary of Commands

| Command | Type | SAW 1.4 | Purpose |
|---------|------|---------|---------|
| `w4_unint_z3 [names]` | `ProofScript` | Yes | Prove with Z3, keeping names uninterpreted |
| `w4_unint_yices [names]` | `ProofScript` | Yes | Same with Yices |
| `goal_eval_unint [names]` | `ProofScript` | Yes | Evaluate goal, preserving names |
| `term_eval_unint [names]` | `Term -> Term` | Yes | Evaluate term, preserving names |
| `llvm_unint [names]` | `LLVMSetup` | **No** | Keep names opaque during LLVM simulation (future) |
| `offline_unint_smtlib2 [names] file` | `TopLevel` | Yes | Export to SMT-LIB2 with uninterpreted functions |

## References

- [Verifying s2n HMAC with SAW](https://www.galois.com/articles/verifying-s2n-hmac-with-saw) - Original HMAC verification blog series
- [Proving Program Equivalence with SAW](https://www.galois.com/articles/proving-program-equivalence-with-saw) - Using uninterpreted functions in practice
- [SAW Manual](https://saw.galois.com/manual.html) - Official documentation
- [Compositional Verification of Salsa20](https://saw.galois.com/intro/Salsa20.html) - Tutorial on compositional techniques
- [GaloisInc/saw-script](https://github.com/GaloisInc/saw-script) - SAW source and issue tracker
