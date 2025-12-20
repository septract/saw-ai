# Crypto Algorithms Verification
# Delegates to per-algorithm subdirectories

ROOT := ../..
include $(ROOT)/config.mk

ALGORITHMS := sha1 aes

.PHONY: all clean verify verify-ci $(ALGORITHMS)

all: $(ALGORITHMS)

$(ALGORITHMS):
	@$(MAKE) -C $@

verify:
	@for alg in $(ALGORITHMS); do \
		echo "=== Verifying $$alg ==="; \
		$(MAKE) -C $$alg verify; \
	done

# CI-safe verification
verify-ci:
	@echo "=== Verifying sha1 (CI mode) ==="
	@$(MAKE) -C sha1 verify-ci
	@echo "=== Verifying aes (CI mode) ==="
	@$(MAKE) -C aes verify-ci

clean:
	@for alg in $(ALGORITHMS); do \
		$(MAKE) -C $$alg clean; \
	done
