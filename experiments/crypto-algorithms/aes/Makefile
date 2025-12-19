# AES Verification
#
# Targets:
#   make all                  - Build bitcode files
#   make pbt                  - Property-based testing (~10 min, 380 random tests)
#   make verify               - Symbolic verification of primitives (~9 min)
#   make verify-keysetup      - Symbolic verification of key expansion (~30+ min)
#   make verify-encrypt-unint - Full encrypt/decrypt with concrete key (~14 min)
#   make verify-symbolic-key  - Full encrypt/decrypt with SYMBOLIC key schedule
#   make verify-all           - Full symbolic verification
#   make clean                - Remove generated files

ROOT := ../../..
include $(ROOT)/config.mk

# Original source from B-Con repo
REPO := ../repo

# Bitcode targets
BITCODE := aes.bc aes_pbt_harness.bc

# SAW scripts
SAW_SCRIPTS := aes_pbt.saw aes_verify.saw aes_verify_keysetup.saw aes_verify_encrypt_unint.saw

.PHONY: all clean verify verify-keysetup verify-encrypt-unint verify-symbolic-key verify-all pbt

all: $(BITCODE)

# Compile original AES from repo
aes.bc: $(REPO)/aes.c $(REPO)/aes.h
	$(CLANG) $(CFLAGS) $< -o $@

# Compile PBT harness (includes original AES via #include)
aes_pbt_harness.bc: aes_pbt_harness.c $(REPO)/aes.c $(REPO)/aes.h
	$(CLANG) $(CFLAGS) aes_pbt_harness.c -o $@

# Property-based testing (fast, randomized)
# Tests 380 random inputs across all AES-128 functions
pbt: $(BITCODE)
	$(SAW) aes_pbt.saw

# Symbolic verification of primitives (fast)
# Verifies SubBytes, ShiftRows, MixColumns, AddRoundKey
verify: $(BITCODE)
	$(SAW) aes_verify.saw

# Symbolic verification of key expansion (slow)
# Verifies aes_key_setup for ALL possible 128-bit keys
verify-keysetup: $(BITCODE)
	$(SAW) aes_verify_keysetup.saw

# Full verification with uninterpreted functions (concrete key)
# Uses cipher unroll lemmas + w4_unint_z3 for faster proofs (~14 min)
verify-encrypt-unint: $(BITCODE)
	$(SAW) aes_verify_encrypt_unint.saw

# Full verification with SYMBOLIC key schedule
# Proves correctness for ALL key schedules (combined with verify-keysetup = complete proof)
verify-symbolic-key: $(BITCODE)
	$(SAW) aes_verify_symbolic_key.saw

# Full symbolic verification (primitives + encrypt + key expansion)
verify-all: verify verify-symbolic-key verify-keysetup

clean:
	rm -f $(BITCODE)
