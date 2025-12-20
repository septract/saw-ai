# FEAL-8 SAW Verification

Formal verification of the FEAL-8 block cipher - specifically, an **LP64-patched** version of a 1989 C implementation with unions, globals, and a lazy-initialized lookup table.

**Status: FULLY VERIFIED** (all functions, all keys, all messages)

> **Note:** The original 1989 code contains undefined behavior on 64-bit systems (LP64).
> We verify a patched version with zero-initialized unions. See [PROVENANCE.md](PROVENANCE.md).

## The Challenge

[FEAL](https://en.wikipedia.org/wiki/FEAL) (Fast data Encipherment ALgorithm) is famous for being broken—attacks on it led to differential cryptanalysis. We're not proving it's secure (it isn't). We're proving a 35-year-old C implementation correctly implements the algorithm.

Our target: code from [Schneier's Applied Cryptography archive](https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-2.zip) (September 1989). It has:
- Union-based byte extraction
- Global state for key schedule
- Lazy-initialized Rot2 lookup table
- A portability bug (fixed with `= {0}` initialization)

## What We Proved

```
C code (1989)  ══SAW══►  feal8_1989.cry  ══Cryptol══►  feal8.cry (HAC reference)
```

| Function | Symbolic Bits | Notes |
|----------|---------------|-------|
| Rot2 (init) | 8 | Verifies C loop builds correct lookup table |
| Rot2, S0, S1 | 8-16 | Lookup table + S-boxes |
| f, FK | 96-128 | Round function, key schedule function |
| SetKey | 64 | All 2^64 keys verified |
| Encrypt/Decrypt | 704 | Symbolic key schedule + plaintext |
| HAC equivalence | 128 | Full algorithm correctness |

**Total proof time: ~11 seconds** thanks to compositional verification.

## Quick Start

```bash
make verify-1989    # Full verification
make test-1989      # Run C test harness
```

## Files

| File | Description |
|------|-------------|
| [feal8_1989_portable.c](feal8_1989_portable.c) | C source with LP64 fixes |
| [feal8.cry](feal8.cry) | HAC reference spec (big-endian) |
| [feal8_1989.cry](feal8_1989.cry) | 1989-specific spec (little-endian) |
| [feal8_1989_verify.saw](feal8_1989_verify.saw) | SAW verification (8 stages) |
| [PROVENANCE.md](PROVENANCE.md) | Source attribution |

## Documentation

See [docs/feal8-1989-verification.md](../../docs/feal8-1989-verification.md) for the full story, including the portability bug we found and how compositional verification makes this tractable.

## Technical Details

- **Block size**: 64 bits
- **Key size**: 64 bits
- **Structure**: 8-round Feistel network
- **Known broken**: Differential cryptanalysis (Biham/Shamir), linear cryptanalysis (Matsui)

## References

- Shimizu & Miyaguchi (1987). "Fast Data Encipherment Algorithm FEAL"
- Biham & Shamir (1991). "Differential Cryptanalysis of FEAL and N-Hash"
- [Handbook of Applied Cryptography, Section 7.5](https://cacr.uwaterloo.ca/hac/)
