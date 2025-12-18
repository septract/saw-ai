# Claude Code Project Notes

## CRITICAL: Source Code Integrity

**NEVER modify the original source code being verified.** The entire point of formal verification is to prove properties about the *actual* code, not a modified version.

- The only permitted exception: adding comments when explicitly instructed
- Actual executable code (functions, logic, data structures) MUST remain unchanged
- If verification fails, fix the SAW script or Cryptol spec, not the C code
- If a bug is found, document it - don't silently fix it

## SAW Formal Verification Setup

This project uses SAW (Software Analysis Workbench) from Galois for formal verification of cryptographic code.

### Installation

Run the install script to download SAW locally (no sudo required):

```bash
./scripts/install-saw.sh
```

This installs SAW v1.4 with bundled solvers to `tools/saw/`.

### LLVM Version Compatibility

SAW 1.4 requires LLVM 14-18. Apple's system clang (17+) may produce incompatible bitcode. Use Homebrew LLVM:

```bash
brew install llvm@18
/opt/homebrew/opt/llvm@18/bin/clang -emit-llvm -c -g -O0 source.c -o source.bc
```

### Usage

Before using SAW tools, source the environment:

```bash
source scripts/saw-env.sh
```

This adds the following tools to your PATH:
- `saw` - SAW verification tool
- `cryptol` - Cryptol specification language
- `z3` - Z3 SMT solver

### Directory Structure

```
Makefile              # Top-level build (delegates to experiments)
scripts/
  install-saw.sh      # Downloads and installs SAW
  saw-env.sh          # Environment setup script
tools/
  saw/                # SAW installation (gitignored)
specs/
  cryptol-specs/      # Galois reference Cryptol specifications (cloned from GitHub)
experiments/
  hello-saw/          # Simple example (max function)
    Makefile
    max.c, max.saw
  ffs/                # Find First Set - tutorial example
    Makefile
    ffs.c, ffs.saw
  crypto-algorithms/  # Crypto library verification
    Makefile          # Delegates to algorithm subdirs
    repo/             # Cloned B-Con source (DO NOT MODIFY)
    sha1/             # SHA1 verification
      Makefile
      sha1.bc                    # Bitcode from original repo
      sha1_single_round.c        # Decomposed for compositional verification
      sha1_single_round.bc
      sha1_concrete_test.saw     # Concrete tests
      sha1_verify_primitives.saw # Verify Ch/Parity/Maj
      sha1_verify_single_round.saw # Verify single round functions
    aes/              # AES-128 verification
      Makefile
      aes_pbt_harness.c          # C wrappers for PBT
      aes_pbt.cry                # Cryptol specs for testing
      aes_pbt.saw                # Property-based testing
      aes_verify.saw             # Symbolic verification (primitives)
      aes_verify_keysetup.saw    # Symbolic verification (key expansion)
```

### Build System

The project uses recursive Makefiles. From the project root:

```bash
make help     # Show available targets
make all      # Build all bitcode files
make verify   # Run all SAW verifications
make clean    # Remove generated .bc files
```

Or run specific verifications:

```bash
# SHA1
cd experiments/crypto-algorithms/sha1
make verify-primitives  # Just verify Ch/Parity/Maj
make verify-rounds      # Just verify single rounds
make verify-concrete    # Just run concrete tests

# AES
cd experiments/crypto-algorithms/aes
make pbt                # Property-based testing (~10 min)
make verify             # Symbolic verify primitives (~9 min)
make verify-keysetup    # Symbolic verify key expansion (~30+ min)
```

### Current Experiments

**hello-saw/** - Simple max() function example

**ffs/** - Find First Set bit (from SAW tutorial)
- Four implementations: reference loop, binary search, DeBruijn, and buggy
- Demonstrates `llvm_extract` + `prove`/`sat` workflow
- Status: All VERIFIED
  - ffs_imp == ffs_ref: VERIFIED (32 bits symbolic)
  - ffs_musl == ffs_ref: VERIFIED (32 bits symbolic)
  - ffs_bug counterexample: FOUND (x = 0x101010)

**crypto-algorithms/sha1/** - Verifying B-Con SHA1 implementation
- Source: https://github.com/B-Con/crypto-algorithms
- Reference spec: specs/cryptol-specs/Primitive/Keyless/Hash/SHA1/
- Status:
  - Concrete tests: PASSING (C matches Cryptol on test vectors)
  - Primitives (Ch, Parity, Maj): VERIFIED (96 bits symbolic each)
  - Single rounds: VERIFIED (224 bits symbolic, compositional)

**crypto-algorithms/aes/** - Verifying B-Con AES-128 implementation
- Source: https://github.com/B-Con/crypto-algorithms
- Reference spec: specs/cryptol-specs/Primitive/Symmetric/Cipher/Block/AES/
- Files:
  - `aes_pbt_harness.c` - C wrappers for PBT (scalar interfaces)
  - `aes_pbt.cry` - Cryptol specs for testing
  - `aes_pbt.saw` - Property-based testing (380 random tests, ~10 min)
  - `aes_verify.saw` - Symbolic verification of primitives (~9 min)
  - `aes_verify_keysetup.saw` - Symbolic verification of key expansion (~30+ min)
- Status:
  - **Primitives VERIFIED** (symbolic, 128-256 bits):
    - SubBytes, InvSubBytes (~2 min each)
    - ShiftRows, InvShiftRows (<1 sec each)
    - MixColumns (~20 sec), InvMixColumns (~4 min)
    - AddRoundKey (<1 sec)
  - **Key expansion**: PBT passing (50 tests), symbolic available via `make verify-keysetup`
  - **Full encrypt/decrypt**: PBT passing (30 tests on random inputs)
- Make targets:
  ```bash
  make pbt              # Property-based testing (~10 min)
  make verify           # Symbolic verify primitives (~9 min)
  make verify-keysetup  # Symbolic verify key expansion (~30+ min)
  make verify-all       # Everything (~40 min)
  ```

### Verification Strategy

1. **Property-based testing first** - Fast random testing, catches spec bugs quickly
2. **Symbolic verification** - Exhaustive proofs for tractable functions
3. **Compositional verification** - Verify small functions, use as overrides for larger ones
