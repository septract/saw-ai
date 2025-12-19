# FEAL-8 SAW Verification

Formal verification of the FEAL-8 (Fast Data Encipherment Algorithm) block cipher.

## Background

FEAL was designed by Akihiro Shimizu and Shoji Miyaguchi at NTT Japan in 1987 as a faster alternative to DES. It is historically significant as the cipher that led to the discovery of **differential cryptanalysis** by Eli Biham and Adi Shamir.

### Technical Details

- **Block size**: 64 bits
- **Key size**: 64 bits
- **Structure**: Feistel network
- **Rounds**: 8 (FEAL-8), originally 4 (FEAL-4)

### Known Vulnerabilities

FEAL has been extensively broken:

- **FEAL-4**: Broken with differential cryptanalysis (Biham/Shamir, 1989)
- **FEAL-8**: Broken with ~10,000 chosen plaintext pairs (Gilbert/Chassé, 1990)
- **FEAL-N**: Broken for N ≤ 31 rounds (Biham/Shamir, 1991)
- Linear cryptanalysis was first demonstrated on FEAL (Matsui/Yamagishi, 1992)

## Source Code

The implementation is from Pate Williams (1997), based on the "Handbook of Applied Cryptography" by Menezes et al., Section 7.5, pages 259-262.

Original source: [Schneier's Applied Cryptography Source Code](https://www.schneier.com/books/applied-cryptography-source/)

## Files

```
Makefile         - Build system
README.md        - This file
.gitignore       - Excludes src/ and generated files
src/             - Third-party source (gitignored, use `make download`)
  feal8.c        - FEAL-8 C implementation (Pate Williams, 1997)
  PROVENANCE     - Source attribution and download info
feal8.bc         - LLVM bitcode (generated)
```

## Building

```bash
# Download FEAL source from Schneier archive
make download

# Build LLVM bitcode for SAW
make bitcode

# Run native test
make test

# Clean generated files
make clean
```

## Verification Goals

1. **Primitives**: Verify the Sd (substitution) and f (round) functions
2. **Key schedule**: Verify FEAL_key_schedule matches Cryptol spec
3. **Encryption/Decryption**: Verify FEAL_encryption and FEAL_decryption
4. **Correctness**: Prove decrypt(encrypt(m)) == m for all keys and messages
5. **(Advanced)**: Model differential characteristics used in attacks

## TODO

- [ ] Write Cryptol specification (feal8.cry)
- [ ] Concrete tests with known test vectors
- [ ] Symbolic verification of primitives (Sd, f, fK)
- [ ] Symbolic verification of key schedule
- [ ] Full encrypt/decrypt verification
- [ ] Document differential characteristics

## References

- Shimizu, A., Miyaguchi, S. (1987). "Fast Data Encipherment Algorithm FEAL"
- Biham, E., Shamir, A. (1991). "Differential Cryptanalysis of FEAL and N-Hash"
- Menezes, A. et al. (1996). "Handbook of Applied Cryptography", Section 7.5
- [The Amazing King - FEAL Differential Cryptanalysis](http://www.theamazingking.com/crypto-feal.php)
