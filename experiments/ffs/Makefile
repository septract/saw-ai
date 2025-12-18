# Find First Set - SAW verification example

ROOT := ../..
include $(ROOT)/config.mk

BITCODE := ffs.bc
SOURCES := ffs.c

.PHONY: all clean verify

all: $(BITCODE)

%.bc: %.c
	$(CLANG) $(CFLAGS) $< -o $@

verify: $(BITCODE)
	@echo "Verifying ffs.saw..."
	@$(SAW) ffs.saw

clean:
	rm -f $(BITCODE)
