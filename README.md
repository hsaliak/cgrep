# cgrep - Grep in C23

A subset of `grep` implemented in modern C23, with multi-threaded recursive search, `mmap` for I/O, and `pcre2` for regex matching.

## Building

The project uses CMake and provides convenience targets for different build configurations.

### Common Build Targets
Once you have initialized a build directory (e.g., `mkdir build && cd build && cmake ..`), you can run the following targets:

- **Release Build** (Optimized, no sanitizers):
  ```bash
  cmake --build . --target release
  ```
- **ASan Build** (AddressSanitizer enabled):
  ```bash
  cmake --build . --target asan
  ```
- **TSan Build** (ThreadSanitizer enabled):
  ```bash
  cmake --build . --target tsan
  ```

### Standard CMake Build
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

#### Build Options
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

The project includes unit tests (C) and integration tests (Python). You can run tests against specific build configurations using CMake targets.

### Test Targets
From your build directory:
- **Test Release Build**: `cmake --build . --target test-release`
- **Test ASan Build**: `cmake --build . --target test-asan`
- **Test TSan Build**: `cmake --build . --target test-tsan`
- **Test All**: `cmake --build . --target test-all` (Runs all of the above)

### Full Verification
You can also run the integrated verification script:
```bash
cmake --build . --target verify
```

## Linting
Clang-Tidy runs automatically during compilation if found.
To run it manually:
```bash
clang-tidy src/*.c -- -Iinclude
```
