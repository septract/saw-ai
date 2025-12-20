# SHA1 Verification

ROOT := ../../..
include $(ROOT)/config.mk

# Original source from B-Con repo
REPO := ../repo

# Bitcode targets
BITCODE := sha1.bc sha1_single_round.bc

# SAW verification scripts (in order of dependency)
SAW_SCRIPTS := sha1_verify_primitives.saw \
               sha1_verify_single_round.saw \
               sha1_concrete_test.saw

.PHONY: all clean verify verify-ci verify-primitives verify-rounds verify-concrete

all: $(BITCODE)

# Compile original SHA1 from repo
sha1.bc: $(REPO)/sha1.c
	$(CLANG) $(CFLAGS) $< -o $@

# Compile decomposed version (local, needs repo headers)
sha1_single_round.bc: sha1_single_round.c $(REPO)/sha1.h
	$(CLANG) $(CFLAGS) -I$(REPO) $< -o $@

# Run all verifications
verify: $(BITCODE)
	@echo "--- Verifying primitives (Ch, Parity, Maj) ---"
	@$(SAW) sha1_verify_primitives.saw
	@echo ""
	@echo "--- Verifying single round functions ---"
	@$(SAW) sha1_verify_single_round.saw
	@echo ""
	@echo "--- Running concrete tests ---"
	@$(SAW) sha1_concrete_test.saw

# CI-safe verification (skips platform-specific round tests)
verify-ci: $(BITCODE)
	@echo "--- Verifying primitives (Ch, Parity, Maj) ---"
	@$(SAW) sha1_verify_primitives.saw
	@echo ""
	@echo "--- Running concrete tests ---"
	@$(SAW) sha1_concrete_test.saw

# Individual verification targets
verify-primitives: sha1_single_round.bc
	$(SAW) sha1_verify_primitives.saw

verify-rounds: sha1_single_round.bc
	$(SAW) sha1_verify_single_round.saw

verify-concrete: sha1.bc
	$(SAW) sha1_concrete_test.saw

clean:
	rm -f $(BITCODE)
