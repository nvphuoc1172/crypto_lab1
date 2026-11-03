# Repository Guidelines

## Project Structure & Module Organization

- `Lab01.pdf` defines the assignment requirements and submission format.
- `prng_sample/src/` contains application code: `main_prng.cpp` demonstrates the PRNG; `u512_sample.h` sketches the 512-bit integer type; `dh_sample.h` declares Diffie–Hellman functions. The latter two are incomplete interfaces.
- `prng_sample/lib/fips202/` contains SHAKE/SHA3 and the PRNG wrapper. Keep assignment-specific arithmetic in `src/`.
- `prng_sample/Makefile` builds the sample; `bin/` and `bin/build/` hold executable and object outputs.
- `.vscode/` contains local editor/debug settings. `prng_sample.zip` is the supplied archive; edit the extracted sources.

## Build, Test, and Development Commands

Run from `prng_sample/` in a Linux environment with GNU Make and `g++`:

```sh
make prng       # Build bin/prng
make run-prng   # Build and run the PRNG demonstration
make clean      # Remove bin/, including generated objects
```

On Windows, use WSL or adapt the platform code first: the current PRNG depends on `sys/random.h` and `getrandom`. Use C++14 or earlier, as required by the assignment; the Makefile currently does not select a language standard explicitly. No full Diffie–Hellman build target exists yet.

## Coding Style & Naming Conventions

Follow the application code’s four-space indentation and braces on separate lines. Use `snake_case` for functions and variables, descriptive `.cpp`/`.h` filenames, and header guards. Preserve existing interfaces such as `uint512_t` and `modular_exponentiation`. No formatter or linter configuration is provided; review compiler warnings enabled by the Makefile.

## Testing Guidelines

There is no test framework, test target, or coverage threshold. For new arithmetic and protocol code, add focused checks for carries, borrows, boundary values, modular exponentiation, known primes/composites, and matching Diffie–Hellman secrets. Document how to compile and run any added tests. Use deterministic, fully initialized seeds for reproducible checks.

## Commit & Pull Request Guidelines

No Git metadata is present in this checkout, so existing commit conventions cannot be verified. Use concise imperative subjects, such as `Implement modular exponentiation`. Keep changes focused. PR descriptions should state the affected assignment requirement, implementation approach, validation commands/results, and platform limitations.

## Assignment Constraints

Support primes of at least 512 bits, implement prime generation and primality testing yourself, and ensure both `p` and `(p-1)/2` are prime. Generate private keys in `[2, p-2]`. Treat fixed seeds as learning/test fixtures rather than production entropy.
