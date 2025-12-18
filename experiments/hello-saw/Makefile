# Hello SAW - Simple verification examples

ROOT := ../..
include $(ROOT)/config.mk

BITCODE := max.bc uninterp.bc
SOURCES := max.c uninterp.c
SAW_SCRIPTS := max.saw uninterp.saw

.PHONY: all clean verify verify-max verify-uninterp

all: $(BITCODE)

%.bc: %.c
	$(CLANG) $(CFLAGS) $< -o $@

verify: verify-max verify-uninterp

verify-max: max.bc
	@echo "Verifying max.saw..."
	@$(SAW) max.saw

verify-uninterp: uninterp.bc
	@echo ""
	@echo "Verifying uninterp.saw (uninterpreted functions demo)..."
	@$(SAW) uninterp.saw

clean:
	rm -f $(BITCODE)
