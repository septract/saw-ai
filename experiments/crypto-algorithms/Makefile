# Crypto Algorithms Verification
# Delegates to per-algorithm subdirectories

ALGORITHMS := sha1

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
