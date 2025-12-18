# Crypto-Algorithms Verification

Formal verification of [B-Con's crypto-algorithms](https://github.com/B-Con/crypto-algorithms) C implementations against Galois Cryptol reference specifications.

## Algorithms

### AES-128

Full symbolic verification of AES-128 encryption and decryption.

| Component | Status | Technique |
|-----------|--------|-----------|
| SubBytes, InvSubBytes | VERIFIED | Symbolic (128 bits) |
| ShiftRows, InvShiftRows | VERIFIED | Symbolic (128 bits) |
| MixColumns, InvMixColumns | VERIFIED | Symbolic (128 bits) |
| AddRoundKey | VERIFIED | Symbolic (128+128 bits) |
| aes_encrypt | VERIFIED | Compositional + uninterpreted |
| aes_decrypt | VERIFIED | Compositional + uninterpreted |
| Key expansion | PBT passing | 50 random keys |

```bash
make -C aes verify-encrypt-unint  # Full verification (~14 min)
```

### SHA-1

Partial verification of SHA-1 primitives and single rounds.

| Component | Status | Technique |
|-----------|--------|-----------|
| Ch, Parity, Maj | VERIFIED | Symbolic (96 bits) |
| Single rounds | VERIFIED | Compositional (224 bits) |
| Full compression | Not yet | - |

```bash
make -C sha1 verify-primitives  # Verify Ch/Parity/Maj
make -C sha1 verify-rounds      # Verify single rounds
```

## Source Code

Original C implementations are in `repo/` (cloned from B-Con, unmodified).
