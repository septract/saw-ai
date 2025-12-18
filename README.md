A repo for experimenting with the SAW formal verification tool

## Setup

1. Clone with submodules:
   ```bash
   git clone --recursive <repo-url>
   # or if already cloned:
   git submodule update --init
   ```

2. Install SAW:
   ```bash
   ./scripts/install-saw.sh
   ```

3. Install LLVM 18 (macOS):
   ```bash
   brew install llvm@18
   ```

## Usage

```bash
make help     # Show targets
make all      # Build bitcode
make verify   # Run all verifications
```

See CLAUDE.md for detailed documentation.
