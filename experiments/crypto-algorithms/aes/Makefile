# AES Verification
#
# Targets:
#   make all             - Build bitcode files
#   make pbt             - Property-based testing (~10 min, 380 random tests)
#   make verify          - Symbolic verification of primitives (~9 min)
#   make verify-keysetup - Symbolic verification of key expansion (~30+ min)
#   make verify-all      - Full symbolic verification (~40 min)
#   make clean           - Remove generated files

ROOT := ../../..
include $(ROOT)/config.mk

# Original source from B-Con repo
REPO := ../repo

# Bitcode targets
BITCODE := aes.bc aes_pbt_harness.bc

# SAW scripts
SAW_SCRIPTS := aes_pbt.saw aes_verify.saw aes_verify_keysetup.saw

.PHONY: all clean verify verify-keysetup verify-all pbt

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

# Full symbolic verification (primitives + key expansion)
verify-all: verify verify-keysetup

clean:
	rm -f $(BITCODE)
