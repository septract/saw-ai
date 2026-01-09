# Hello SAW - Simple verification examples

ROOT := ../..
include $(ROOT)/config.mk

BITCODE := max.bc uninterp.bc loop_invariant.bc loop_invariant_examples.bc
SOURCES := max.c uninterp.c loop_invariant.c loop_invariant_examples.c
SAW_SCRIPTS := max.saw uninterp.saw loop_invariant.saw loop_invariant_examples.saw

.PHONY: all clean verify verify-max verify-uninterp verify-loop verify-loop-examples

all: $(BITCODE)

%.bc: %.c
	$(CLANG) $(CFLAGS) $< -o $@

verify: verify-max verify-uninterp verify-loop verify-loop-examples

verify-max: max.bc
	@echo "Verifying max.saw..."
	@$(SAW) max.saw

verify-uninterp: uninterp.bc
	@echo ""
	@echo "Verifying uninterp.saw (uninterpreted functions demo)..."
	@$(SAW) uninterp.saw

verify-loop: loop_invariant.bc
	@echo ""
	@echo "Verifying loop_invariant.saw (loop invariant demo)..."
	@$(SAW) loop_invariant.saw

verify-loop-examples: loop_invariant_examples.bc
	@echo ""
	@echo "Verifying loop_invariant_examples.saw (simple to complex examples)..."
	@$(SAW) loop_invariant_examples.saw

clean:
	rm -f $(BITCODE)
