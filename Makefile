# SAW Crypto Verification Project
# Top-level Makefile

# Project root for config.mk
ROOT := $(CURDIR)
include $(ROOT)/config.mk

# Experiment directories
EXPERIMENTS := experiments/hello-saw experiments/ffs experiments/crypto-algorithms

.PHONY: all clean verify help $(EXPERIMENTS)

all: $(EXPERIMENTS)

$(EXPERIMENTS):
	@$(MAKE) -C $@

verify:
	@for dir in $(EXPERIMENTS); do \
		echo "=== Verifying $$dir ==="; \
		$(MAKE) -C $$dir verify; \
	done

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
	@echo "  experiments/crypto-algorithms  - SHA1 verification"
	@echo ""
	@echo "Tools:"
	@echo "  CLANG = $(CLANG)"
	@echo "  SAW   = $(SAW)"
