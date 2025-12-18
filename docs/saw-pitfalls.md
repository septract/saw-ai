# SAW/Cryptol Pitfalls

Surprising behaviors and gotchas discovered during verification work.

## 1. Cryptol `where` Clause Scoping

**Problem:** Lambda variables appear "not in scope" inside `where` clause definitions.

**Root Cause:** The `where` clause in Cryptol only scopes to the expression it's directly attached to. If the `where` is parsed as attaching to a sub-expression, outer lambda variables won't be visible.

**Failing Example:**
```saw
prove_print z3
    {{ \w s ->
       (t 1 s)
       ==
       (t 1 s)
       where
         t i x = (w@i) ^ x    // ERROR: w not in scope
    }};
```

**Why it fails:** The `where` attaches only to `(t 1 s)` on the RHS of `==`, not the whole expression. So `w` from the lambda isn't visible.

**Working Example:**
```saw
prove_print z3
    {{ \w s ->
       ((t 1 s) == (t 1 s)
         where
           t i x = (w@i) ^ x)   // OK: parens wrap entire expr
    }};
```

**Key Rule:** Wrap the entire expression that uses the `where`-defined names in parentheses:
```
(expr-using-t where t = ...)
```

**Galois Pattern (from tools/saw/examples/aes/aes.saw):**
```saw
{{ \w pt -> cipher w pt ==
    (stateToMsg (... t 1 (t 2 ...))
        where
        t i state = AddRoundKey (w@i) ...)   // trailing ) closes the (
    }};
```

Note the `(` immediately after `==` and the `)` after the `where` definition - these wrap the entire RHS including the `where` clause.

## 2. CRYPTOLPATH Required for Module Imports

**Problem:** Cryptol imports fail with "Could not find module".

**Solution:** Set `CRYPTOLPATH` environment variable to include the specs directory:
```bash
CRYPTOLPATH="../../../specs/cryptol-specs" saw script.saw
```

Or in Makefile:
```makefile
export CRYPTOLPATH := $(ROOT)/specs/cryptol-specs
```

## 3. Type Annotations for Array Indexing

**Problem:** `sequentToSATQuery: expected EqTrue, actual: Cryptol.Num` errors when using array indexing like `w@0`.

**Possible Cause:** Polymorphic index type isn't resolved.

**Solution:** Add explicit type annotation: `w@(0:[8])` or ensure the array type is explicit in the lambda: `\(w:KeySchedule) -> ...`

## 4. `llvm_unint` Does Not Exist in SAW 1.4

**Problem:** `Unbound variable: llvm_unint` when trying to keep functions uninterpreted during LLVM symbolic execution.

**Root Cause:** `llvm_unint` (and `jvm_unint`, `mir_unint`) are documented in SAW's CHANGES.md for a **future unreleased version**. They do not exist in SAW 1.4.

**Wrong approach (SAW 1.4):**
```saw
let my_spec = do {
    // ... setup ...
    llvm_unint ["SubBytes", "ShiftRows"];  // ERROR: doesn't exist
    llvm_execute_func [...];
    // ...
};
```

**Correct approach (SAW 1.4):** Use `w4_unint_z3` in the **proof script**, not the spec:
```saw
llvm_verify m "func" overrides false spec
    do {
        simplify simpset;
        w4_unint_z3 ["SubBytes", "ShiftRows"];
    };
```

With compositional verification (overrides), the C primitives are replaced with Cryptol specs. The `w4_unint_z3` then keeps those Cryptol specs uninterpreted during the SMT proof.

**Reference:** See [uninterpreted-functions-in-saw.md](uninterpreted-functions-in-saw.md) for details.
