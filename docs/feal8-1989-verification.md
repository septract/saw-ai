# Verifying 1989 FEAL-8 Code with SAW

*This post, the Cryptol specifications, and all the SAW proofs were written by Claude Opus 4.5 (an AI) as an experiment in formal verification of cryptographic code. Development time: ~4 hours human+AI collaboration.*

## The Challenge: Verify 35-Year-Old C Code

[FEAL](https://en.wikipedia.org/wiki/FEAL) (Fast data Encipherment ALgorithm) is a 1987 block cipher that's famous for being broken—attacks on FEAL-4 led Biham and Shamir to develop **differential cryptanalysis**. We're not trying to prove it's secure (it isn't). We're proving that our C code correctly implements the algorithm.

Our target: a [1989 implementation](https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-2.zip) from Schneier's Applied Cryptography archive. It has all the hallmarks of late-80s C:

- **Unions for byte extraction** (`{ unsigned long All; ByteType Byte[4]; }`)
- **Global state** for the key schedule
- **Lazy-initialized lookup table** for the Rot2 function

This is not clean modern code. That's what makes it interesting.

## What We Proved

**Complete verification chain:**

```
C code (1989)  ══SAW══►  feal8_1989.cry  ══Cryptol══►  feal8.cry (HAC reference)
```

Every function verified against its spec. Every spec proven equivalent to the HAC reference.

| Stage | Function | Symbolic Bits | Notes |
|-------|----------|---------------|-------|
| 1 | Rot2 | 8 | Lookup table initialized |
| 2 | S0, S1 | 16 each | Uses Rot2 override |
| 3 | f | 96 | Round function (64-bit state + 32-bit subkey) |
| 4 | FK | 128 | Key schedule round function |
| 5 | SetKey | 64 | Full key schedule, all 2^64 keys |
| 6 | Encrypt/Decrypt | 64 | Concrete key, symbolic plaintext |
| 7 | Encrypt/Decrypt | 704 | Symbolic key schedule + plaintext |
| 8 | HAC equivalence | 128 | Proves 1989 spec == HAC spec for all key/message pairs |

**Total proof time: ~11 seconds.** Compositional verification makes it tractable.

## The Fun Part: We Found a Portability Bug

While verifying the `f` function, SAW complained about undefined memory. Here's the culprit:

```c
union {
    unsigned long All;
    ByteType Byte[4];
} RetVal;

RetVal.Byte[0] = S0(A.Byte[0], f1);
// ... set Byte[1..3] ...
return RetVal.All;  // Reads 8 bytes, only 4 were written!
```

In 1989, `unsigned long` was 32 bits. On modern 64-bit systems, it's 64 bits. **The high 32 bits are garbage.**

### Why It Worked Anyway

The garbage propagates through the computation but never escapes:
1. All union-based functions have garbage in high bits
2. Garbage XOR garbage = more garbage (still confined to high bits)
3. `DissH1` extracts output from `Byte[0..3]` only—the valid bits

We added `= {0}` initialization to fix it. The code now works correctly on 64-bit systems *and* passes formal verification.

## Key Files

- [feal8_1989_verify.saw](../experiments/feal/feal8_1989_verify.saw) - Complete SAW verification (8 stages)
- [feal8_1989.cry](../experiments/feal/feal8_1989.cry) - Cryptol spec matching C's little-endian byte order
- [feal8.cry](../experiments/feal/feal8.cry) - HAC reference spec (big-endian)
- [feal8_1989_portable.c](../experiments/feal/feal8_1989_portable.c) - Fixed C code with portability annotations

## Verification Highlights

**Uninterpreted functions make it fast.** We keep S0/S1 (the S-boxes) abstract during proofs. The solver only needs to verify structural equivalence, not reason through the actual S-box computations.

```saw
// This completes in milliseconds because S0/S1 are uninterpreted
prove_print (w4_unint_z3 ["S0", "S1"])
    {{ \key plain -> encrypt_equiv_to_HAC key plain }};
```

**Byte-order dance.** The 1989 code uses little-endian unions; the HAC spec uses big-endian. The subkeys are byte-swapped between representations, but byte-level I/O is identical. We proved this equivalence explicitly:

```cryptol
property encrypt_equiv_to_HAC key_bytes plain_bytes =
    encryptWithKey_1989 key_bytes plain_bytes ==
    fromBlock (encrypt (toBlock key_bytes) (toBlock plain_bytes))
```

## Lessons Learned

1. **SAW finds what testing misses.** The portability bug never caused test failures—the code produced correct output. SAW flagged it because formal verification can't reason about undefined behavior.

2. **Compositional verification is essential.** Directly verifying Encrypt without overrides would require reasoning through 8 rounds × 4 S-box calls = 32 S-box expansions. With overrides, it's fast.

3. **Old code is surprisingly verifiable.** Despite globals, unions, and lazy initialization, we achieved full symbolic verification. The key is building the right abstraction layers.

## Run It Yourself

```bash
cd experiments/feal
make verify-1989    # Full verification (~11 seconds)
```

## Sources

- [FEAL - Wikipedia](https://en.wikipedia.org/wiki/FEAL)
- [Handbook of Applied Cryptography, Section 7.5](https://cacr.uwaterloo.ca/hac/)
- [Schneier's Applied Cryptography Source Archive](https://www.schneier.com/books/applied-cryptography-source/)
