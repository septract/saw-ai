# FEAL-8 SAW Verification
# Fast Data Encipherment Algorithm (8 rounds)

# Use Homebrew LLVM for SAW compatibility
CLANG = /opt/homebrew/opt/llvm@18/bin/clang
CLANG_FLAGS = -emit-llvm -c -g -O0

# Source files (in src/ directory - gitignored, download with make download)
SRC_DIR = src
FEAL_SRC = $(SRC_DIR)/feal8.c
FEAL_1989_SRC = $(SRC_DIR)/feal8_1989.c

# Bitcode files
FEAL_BC = feal8.bc
FEAL_1989_BC = feal8_1989.bc

.PHONY: all clean bitcode verify test test-1989 download help

all: bitcode

help:
	@echo "FEAL-8 SAW Verification"
	@echo ""
	@echo "Targets:"
	@echo "  download     - Download FEAL source code from Schneier archive"
	@echo "  all          - Build all bitcode files"
	@echo "  bitcode      - Compile C to LLVM bitcode (both versions)"
	@echo "  test         - Run Pate Williams (1997) test"
	@echo "  test-1989    - Run original 1989 implementation test"
	@echo "  test-cryptol - Run Cryptol specification tests"
	@echo "  verify       - Run SAW verification"
	@echo "  clean        - Remove generated files"
	@echo ""
	@echo "Implementations:"
	@echo "  feal8.c      - Pate Williams (1997), HAC-based, portable"
	@echo "  feal8_1989.c - Original 1989, lookup table, union-based"

# Download source code from Schneier's archive
download:
	@mkdir -p $(SRC_DIR)
	@if [ ! -f $(FEAL_SRC) ]; then \
		echo "Downloading FEAL-8 source from Schneier archive..."; \
		curl -sL "https://www.schneier.com/wp-content/uploads/2015/03/FEAL8-WI-2.zip" -o /tmp/feal8.zip; \
		unzip -o /tmp/feal8.zip -d $(SRC_DIR); \
		mv $(SRC_DIR)/feal-8.c $(FEAL_SRC); \
		rm /tmp/feal8.zip; \
		echo "Downloaded to $(FEAL_SRC)"; \
		echo "See $(SRC_DIR)/PROVENANCE for source information"; \
	else \
		echo "$(FEAL_SRC) already exists"; \
	fi

# Compile to LLVM bitcode for SAW
bitcode: $(FEAL_BC) $(FEAL_1989_BC)

$(FEAL_BC): $(FEAL_SRC)
	$(CLANG) $(CLANG_FLAGS) $< -o $@

$(FEAL_1989_BC): $(FEAL_1989_SRC)
	$(CLANG) $(CLANG_FLAGS) $< -o $@

$(FEAL_SRC):
	@echo "Source file not found. Run 'make download' first."
	@exit 1

# Build and run native tests
test: feal8_test
	./feal8_test

test-1989: feal8_1989_test
	./feal8_1989_test

feal8_test: $(FEAL_SRC)
	$(CC) -o $@ $<

feal8_1989_test: $(FEAL_1989_SRC)
	$(CC) -o $@ $<

# Cryptol specification tests
CRYPTOL = $(shell pwd)/../../tools/saw/bin/cryptol

test-cryptol: feal8.cry
	@echo "Running Cryptol tests..."
	$(CRYPTOL) feal8.cry -c ":prove s0_example" -c ":prove s1_example"
	$(CRYPTOL) feal8.cry -c ":prove testVector" -c ":prove keyScheduleTest"
	$(CRYPTOL) feal8.cry -c ":check correctness"
	@echo "All Cryptol tests passed!"

# SAW verification
SAW = $(shell pwd)/../../tools/saw/bin/saw

verify: $(FEAL_BC) feal8.cry feal8_verify.saw
	@echo "Running SAW verification (Williams 1997 implementation)..."
	CRYPTOLPATH="../../specs/cryptol-specs" $(SAW) feal8_verify.saw

clean:
	rm -f $(FEAL_BC) $(FEAL_1989_BC) feal8_test feal8_1989_test *.bc
