# Crypto Algorithms Verification
# Delegates to per-algorithm subdirectories

ROOT := ../..
include $(ROOT)/config.mk

ALGORITHMS := sha1 aes

.PHONY: all clean verify $(ALGORITHMS)

all: $(ALGORITHMS)

$(ALGORITHMS):
	@$(MAKE) -C $@

verify:
	@for alg in $(ALGORITHMS); do \
		echo "=== Verifying $$alg ==="; \
		$(MAKE) -C $$alg verify; \
	done

clean:
	@for alg in $(ALGORITHMS); do \
		$(MAKE) -C $$alg clean; \
	done
