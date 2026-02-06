# cgrep - Multi-threaded High Performance Grep in C23

A high-performance subset of `grep` implemented in modern C23, featuring multi-threaded recursive search, `mmap` for fast I/O, and `pcre2` for regex matching.

## Features
- **C23 Standard**: Leveraging modern features like `bool`, `nullptr`, and `[[gnu::cleanup]]` for RAII.
- **Parallel Processing**: Multi-threaded architecture with a lock-protected work queue and intrusive data structures.
- **Fast I/O**: Uses `mmap` to minimize copies during file scanning.
- **Filtering**: Supports glob-based `--include` and `--exclude` patterns.
- **Memory & Thread Safety**: Fully instrumented with ASan and TSan.

## Building

### Standard Build (ASan enabled by default)
```bash
mkdir build && cd build
cmake ..
make
```

### Build Options
- `-DENABLE_ASAN=ON/OFF`: Enable AddressSanitizer (Default: ON).
- `-DENABLE_TSAN=ON/OFF`: Enable ThreadSanitizer (Default: OFF). Note: ASan and TSan cannot be enabled simultaneously.

## Usage

```bash
./cgrep [OPTIONS] PATTERN [FILE/DIR]
```

### Examples
- **Basic Search**:
  ```bash
  ./cgrep "regex_pattern" file.txt
  ```
- **Recursive Search**:
  ```bash
  ./cgrep -r "pattern" /path/to/dir
  ```
- **Case Insensitive with Line Numbers**:
  ```bash
  ./cgrep -i -n "pattern" file.txt
  ```
- **Filtering Files**:
  ```bash
  ./cgrep -r --include "*.c" --exclude "build/*" "TODO" .
  ```

## Testing & Verification

The project includes a comprehensive test suite (Unit tests in C, Integration tests in Python).

### Run Tests
```bash
cd build
make test  # Or ctest
```

### Full Verification
To run all tests under both ASan and TSan to ensure total correctness:
```bash
make verify
```

## Linting
Clang-Tidy runs automatically during compilation if found.
To run it manually:
```bash
clang-tidy src/*.c -- -Iinclude
```
