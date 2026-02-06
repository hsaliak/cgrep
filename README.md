# C Starter Template (C23)

A modern C starter template using C23, Clang-Tidy, and Sanitizers.

## Features
- **C23 Standard**: Uses the latest C standard features (`bool`, `nullptr`, `constexpr`, etc.).
- **RAII-like Cleanup**: Demonstrates use of `[[gnu::cleanup]]` for automatic resource management.
- **Clang-Tidy**: Integrated linting for C code.
- **Sanitizers**: Easy integration for AddressSanitizer (ASan) and ThreadSanitizer (TSan).

## Building

### Standard Build
```bash
mkdir build && cd build
cmake ..
make
```

### Build with AddressSanitizer
```bash
cmake -DUSE_ASAN=ON ..
make
```

### Build with ThreadSanitizer
```bash
cmake -DUSE_TSAN=ON ..
make
```

## Linting
Clang-Tidy runs automatically during compilation if found.
To run it manually:
```bash
clang-tidy src/main.c -- -Iinclude
```
