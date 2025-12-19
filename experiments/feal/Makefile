# FEAL-8 SAW Verification
# Fast Data Encipherment Algorithm (8 rounds)

# Use Homebrew LLVM for SAW compatibility
CLANG = /opt/homebrew/opt/llvm@18/bin/clang
CLANG_FLAGS = -emit-llvm -c -g -O0

# Source files (in src/ directory - gitignored, download with make download)
SRC_DIR = src
FEAL_SRC = $(SRC_DIR)/feal8.c

# Bitcode files
FEAL_BC = feal8.bc

.PHONY: all clean bitcode verify test download help

all: bitcode

help:
	@echo "FEAL-8 SAW Verification"
	@echo ""
	@echo "Targets:"
	@echo "  download   - Download FEAL source code from Schneier archive"
	@echo "  all        - Build all bitcode files"
	@echo "  bitcode    - Compile C to LLVM bitcode"
	@echo "  test       - Run the C test program"
	@echo "  verify     - Run SAW verification (when scripts exist)"
	@echo "  clean      - Remove generated files"

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
bitcode: $(FEAL_BC)

$(FEAL_BC): $(FEAL_SRC)
	$(CLANG) $(CLANG_FLAGS) $< -o $@

$(FEAL_SRC):
	@echo "Source file not found. Run 'make download' first."
	@exit 1

# Build and run native test
test: feal8_test
	./feal8_test

feal8_test: $(FEAL_SRC)
	$(CC) -o $@ $<

# SAW verification (placeholder - scripts to be written)
verify: $(FEAL_BC)
	@echo "SAW verification scripts not yet implemented"
	@echo "TODO: Create feal8.cry Cryptol spec"
	@echo "TODO: Create feal8_verify.saw SAW script"

clean:
	rm -f $(FEAL_BC) feal8_test *.bc
