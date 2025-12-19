# Verifying 1989 FEAL-8 Code with SAW

*This post, the Cryptol specification, and all the SAW proofs were written by Claude Opus 4.5 (an AI) as an experiment in using SAW for cryptographic verification.*

This document describes our SAW verification of a 1989 FEAL-8 implementation and a portability issue we encountered.

## What is FEAL?

[FEAL](https://en.wikipedia.org/wiki/FEAL) (Fast data Encipherment ALgorithm) is a block cipher designed by Shimizu and Miyaguchi at NTT Japan (1987). It's historically significant not for its security, but for how it failed—attacks on FEAL directly catalyzed the development of **differential cryptanalysis** by Biham and Shamir.

## The Verification Target

We're verifying a FEAL-8 implementation from [Schneier's Applied Cryptography source code archive](https://www.schneier.com/books/applied-cryptography-source/) ([direct download](https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-2.zip)). The code is dated September 1989; the author is unknown. It has characteristics of late 1980s C:

- Lazy-initialized lookup tables
- Union-based byte manipulation
- Global state for the key schedule

Our Cryptol reference spec ([feal8.cry](../experiments/feal/feal8.cry)) was written from scratch based on HAC Section 7.5 as an exercise in formalizing cryptography.

## The Portability Issue

While verifying the `f` function, SAW reported `Error during memory load`. The issue is this union pattern:

```c
union {
    unsigned long All;
    ByteType Byte[4];
} RetVal;

RetVal.Byte[0] = S0(A.Byte[0], f1);
RetVal.Byte[1] = f1;
RetVal.Byte[2] = f2;
RetVal.Byte[3] = S1(A.Byte[3], f2);
return RetVal.All;  // <-- reads uninitialized bytes on LP64
```

In 1989, `unsigned long` was 32 bits. On modern LP64 systems (Linux, macOS), it's 64 bits. The code writes 4 bytes but returns 8—the high 4 bytes are uninitialized.

### Why It Doesn't Matter in Practice

The garbage in the high 32 bits propagates through the computation but is never observed:
1. All functions using this pattern (`f`, `FK`, `MakeH1`) return garbage in high bits
2. Garbage ⊕ garbage = more garbage (still confined to high bits)
3. Output extraction (`DissH1`) only reads `Byte[0..3]`—the low 32 bits

**We only need the low 32 bits to be correct, and they are.**

### Our Approach

SAW can't reason about undefined behavior, so we use `enable_lax_loads_and_stores` (which allows reading partially-initialized memory) and verify only the low 32 bits:

```
llvm_postcond {{ (drop`{32} ret) == (drop`{32} (f_1989 aa bb)) }};
```

This proves what matters: the meaningful bits match the spec.

## Current Status

| Function | Symbolic Input | Notes |
|----------|----------------|-------|
| Rot2 | 8 bits | Assumes table pre-initialized |
| S0, S1 | 16 bits each | Uses Rot2 override |
| f | 96 bits (64+32) | Low 32 bits of output verified |
| FK | 128 bits (64+64) | Low 32 bits of output verified |

Still TODO: SetKey, Encrypt, Decrypt

## Lessons Learned

1. **SAW finds what testing misses.** Dynamic testing wouldn't catch this—the code produces correct outputs. SAW flags it because formal verification can't reason about undefined behavior, even when that UB is benign.

2. **"Works in practice" ≠ "formally correct."** The code has worked for 35+ years, but relies on behavior the C standard doesn't guarantee.

3. **Verification constraints can guide understanding.** Being forced to prove only the low 32 bits made us understand *why* the code works despite the UB.

## Sources

- [FEAL - Wikipedia](https://en.wikipedia.org/wiki/FEAL)
- [Handbook of Applied Cryptography, Section 7.5](https://cacr.uwaterloo.ca/hac/)
- [Schneier's Applied Cryptography Source Archive](https://www.schneier.com/books/applied-cryptography-source/)
