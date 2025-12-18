# SAW Crypto Verification - Shared Configuration
# Include this file in all Makefiles: include $(ROOT)/config.mk

# Tools
CLANG := /opt/homebrew/opt/llvm@18/bin/clang
SAW := $(ROOT)/tools/saw/bin/saw

# Clang flags for SAW compatibility
CFLAGS := -emit-llvm -c -g -O0

# Cryptol specs path
CRYPTOLPATH := $(ROOT)/specs/cryptol-specs
export CRYPTOLPATH
