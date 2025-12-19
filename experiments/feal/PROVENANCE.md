# FEAL-8 Source Code Provenance

Three source files are used for verification:

## 1. feal8.c (Pate Williams, 1997)

**Location:** `src/` directory (gitignored, use `make download`)

- **Author:** Pate Williams
- **Date:** 1997
- **Source:** [Schneier's Applied Cryptography Source Code Archive](https://www.schneier.com/books/applied-cryptography-source/)
- **Direct download:** https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-WI-2.zip

Based on: "Handbook of Applied Cryptography" by Alfred J. Menezes et al., Section 7.5, pages 259-262

**Characteristics:**
- Portable bit-shift operations
- Explicit byte indexing
- Direct ROT2 computation

## 2. feal8_1989.c (Original 1989)

**Location:** `src/` directory (gitignored, use `make download`)

- **Author:** Unknown (from Applied Cryptography archive)
- **Date:** September 11, 1989
- **Source:** [Schneier's Applied Cryptography Source Code Archive](https://www.schneier.com/books/applied-cryptography-source/)
- **Direct download:** https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-2.zip

**Characteristics:**
- LOOKUP TABLE for Rot2 (256-entry precomputed table)
- Union-based byte manipulation (ENDIAN-DEPENDENT!)
- Global state for expanded key
- Structurally different from HAC description

> **WARNING:** Original code is NOT portable to LP64 systems (64-bit Linux/macOS).
> See `feal8_1989_portable.c` for the fixed version.

## 3. feal8_1989_portable.c (LP64-portable version)

**Location:** This directory (tracked in git)

- **Based on:** feal8_1989.c from FEAL8-2.zip
- **Modified:** December 2024
- **Modifications:** Zero-initialized unions to fix LP64 portability issue

The original 1989 code assumed ILP32 (`sizeof(long)==4`). On LP64 systems, `sizeof(long)==8`, causing unions like `{ unsigned long All; ByteType Byte[4]; }` to have uninitialized high bytes when `Byte[]` is written and `All` is read.

**Changes made:**
- `MakeH1()`: `RetVal = {0}`
- `f()`: `RetVal = {0}`
- `FK()`: `RetVal = {0}`
- `SetKey()`: `A = {0}`, `B = {0}`, `Q = {0}`

See [docs/feal8-1989-verification.md](../../docs/feal8-1989-verification.md) for full explanation.

## Notes

Both implementations produce identical ciphertext despite different internal representations. The 1989 version stores subkeys in host byte order via unions, while the 1997 version uses explicit bit shifts.

**Original FEAL algorithm by:** Akihiro Shimizu and Shoji Miyaguchi, NTT Japan (1987)

**License:** Not explicitly stated. Code has been widely redistributed as educational/reference implementations.

**Downloaded:** December 2024
