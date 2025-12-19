# Hello SAW - Simple verification examples

ROOT := ../..
include $(ROOT)/config.mk

BITCODE := max.bc uninterp.bc loop_invariant.bc
SOURCES := max.c uninterp.c loop_invariant.c
SAW_SCRIPTS := max.saw uninterp.saw loop_invariant.saw

.PHONY: all clean verify verify-max verify-uninterp verify-loop

all: $(BITCODE)

%.bc: %.c
	$(CLANG) $(CFLAGS) $< -o $@

verify: verify-max verify-uninterp verify-loop

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

clean:
	rm -f $(BITCODE)
