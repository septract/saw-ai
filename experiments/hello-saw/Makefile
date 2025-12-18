# Hello SAW - Simple verification example

BITCODE := max.bc
SOURCES := max.c
SAW_SCRIPTS := max.saw

.PHONY: all clean verify

all: $(BITCODE)

%.bc: %.c
	$(CLANG) $(CFLAGS) $< -o $@

verify: $(BITCODE)
	@echo "Verifying max.saw..."
	@$(SAW) max.saw

clean:
	rm -f $(BITCODE)
