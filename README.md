# SAW Formal Verification Experiments

> **WARNING: AI-GENERATED PROOFS - NOT FOR PRODUCTION USE**
>
> The verification scripts and proofs in this repository were generated with AI assistance (Claude). While formal verification tools like SAW provide mathematical guarantees *when used correctly*, these proofs:
>
> - Have **not been audited** by cryptography or formal methods experts
> - May contain subtle specification errors that invalidate the proofs
> - May verify properties that don't match the actual security requirements
> - Should be treated as **educational/experimental only**
>
> **Do not rely on these proofs for production cryptographic code.**

## What Is This?

This repository explores [SAW (Software Analysis Workbench)](https://saw.galois.com/) from Galois for formally verifying C implementations of cryptographic algorithms against reference specifications written in [Cryptol](https://cryptol.net/).

Formal verification aims to mathematically prove that code behaves according to a specification—not just test it on sample inputs. When successful, it provides much stronger guarantees than traditional testing.

## Experiments

| Experiment | Description | Status |
|------------|-------------|--------|
| `hello-saw/` | Simple max() function | Verified |
| `ffs/` | Find-first-set bit (4 implementations) | Verified (+ bug found in intentionally buggy version) |
| `crypto-algorithms/sha1/` | SHA1 hash primitives | Partially verified (Ch, Parity, Maj, single rounds) |
| `crypto-algorithms/aes/` | AES-128 encryption | **Verified** (primitives + PBT for full cipher) |

## Submodules

This repository includes two Git submodules:

- **[GaloisInc/cryptol-specs](https://github.com/GaloisInc/cryptol-specs)** (`specs/cryptol-specs/`) - Reference Cryptol specifications for cryptographic algorithms from Galois
- **[B-Con/crypto-algorithms](https://github.com/B-Con/crypto-algorithms)** (`experiments/crypto-algorithms/repo/`) - Public domain C implementations of common cryptographic algorithms

## Tools Used

- **[SAW](https://saw.galois.com/)** - Formal verification tool using symbolic execution and SMT solvers
- **[Cryptol](https://cryptol.net/)** - Domain-specific language for specifying cryptographic algorithms
- **[Z3](https://github.com/Z3Prover/z3)** - SMT solver (bundled with SAW)
- **LLVM 18** - For compiling C to bitcode that SAW can analyze

## Quick Start

```bash
# Clone with submodules
git clone --recursive <repo-url>
# Or if already cloned:
git submodule update --init

# Install SAW (downloads to tools/saw/, no sudo required)
./scripts/install-saw.sh

# Install LLVM 18 (macOS)
brew install llvm@18

# Build and verify
make all      # Compile C to LLVM bitcode
make verify   # Run all SAW verifications
```

## How It Works

1. **C code** is compiled to LLVM bitcode
2. **Cryptol specs** define the expected behavior mathematically
3. **SAW scripts** connect the two, asking: "Does this C function behave identically to this Cryptol spec for all possible inputs?"
4. **SMT solvers** (Z3) attempt to prove equivalence or find counterexamples

## Project Structure

```
.
├── experiments/
│   ├── hello-saw/           # Introductory example
│   ├── ffs/                 # Find-first-set tutorial
│   └── crypto-algorithms/
│       ├── repo/            # [submodule] B-Con's crypto-algorithms
│       ├── sha1/            # SHA1 verification scripts
│       └── aes/             # AES verification scripts
├── specs/
│   └── cryptol-specs/       # [submodule] Galois reference Cryptol specs
└── scripts/
    ├── install-saw.sh       # SAW installer
    └── saw-env.sh           # Environment setup
```

## Learning Resources

- [SAW Tutorial](https://saw.galois.com/tutorial.html)
- [Cryptol Reference](https://galois.github.io/cryptol/)
- [Galois Blog Posts on SAW](https://galois.com/blog/)

## License

The verification scripts in this repository are provided as-is for educational purposes. The submodules retain their original licenses:
- `specs/cryptol-specs/` - See [GaloisInc/cryptol-specs](https://github.com/GaloisInc/cryptol-specs) for license
- `experiments/crypto-algorithms/repo/` - Public domain ([B-Con/crypto-algorithms](https://github.com/B-Con/crypto-algorithms))
