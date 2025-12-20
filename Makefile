# SAW Crypto Verification Project
# Top-level Makefile

# Project root for config.mk
ROOT := $(CURDIR)
include $(ROOT)/config.mk

# Experiment directories
EXPERIMENTS := experiments/hello-saw experiments/ffs experiments/crypto-algorithms experiments/feal

.PHONY: all clean verify verify-ci help $(EXPERIMENTS)

all: $(EXPERIMENTS)

$(EXPERIMENTS):
	@$(MAKE) -C $@

verify:
	@for dir in $(EXPERIMENTS); do \
		echo "=== Verifying $$dir ==="; \
		$(MAKE) -C $$dir verify; \
	done

# CI-safe verification (skips slow/platform-specific tests)
verify-ci:
	@echo "=== Verifying experiments/hello-saw ==="
	@$(MAKE) -C experiments/hello-saw verify
	@echo "=== Verifying experiments/ffs ==="
	@$(MAKE) -C experiments/ffs verify
	@echo "=== Verifying experiments/crypto-algorithms (CI mode) ==="
	@$(MAKE) -C experiments/crypto-algorithms verify-ci
	@echo "=== Verifying experiments/feal ==="
	@$(MAKE) -C experiments/feal verify-ci

clean:
	@for dir in $(EXPERIMENTS); do \
		$(MAKE) -C $$dir clean; \
	done

help:
	@echo "SAW Crypto Verification Project"
	@echo ""
	@echo "Targets:"
	@echo "  all     - Build all experiments (compile to bitcode)"
	@echo "  verify  - Run all SAW verification scripts"
	@echo "  clean   - Remove generated files"
	@echo ""
	@echo "Experiments:"
	@echo "  experiments/hello-saw          - Simple max() example"
	@echo "  experiments/ffs                - Find first set bit"
	@echo "  experiments/crypto-algorithms  - SHA1/AES verification"
	@echo "  experiments/feal               - FEAL-8 verification"
	@echo ""
	@echo "Tools:"
	@echo "  CLANG = $(CLANG)"
	@echo "  SAW   = $(SAW)"
