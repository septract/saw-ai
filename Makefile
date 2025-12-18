# SAW Crypto Verification Project
# Top-level Makefile

# Tools
CLANG := /opt/homebrew/opt/llvm@18/bin/clang
SAW := $(CURDIR)/tools/saw/bin/saw

# Clang flags for SAW compatibility
CFLAGS := -emit-llvm -c -g -O0

# Export for sub-makefiles
export CLANG SAW CFLAGS

# Experiment directories
EXPERIMENTS := experiments/hello-saw experiments/crypto-algorithms

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
	@echo "  experiments/crypto-algorithms  - SHA1 verification"
	@echo ""
	@echo "Tools:"
	@echo "  CLANG = $(CLANG)"
	@echo "  SAW   = $(SAW)"
