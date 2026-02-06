# Project Style and Technical Specification

This document outlines the technical requirements, coding standards, and tool configurations for this project.

## 1. Overview
This project is in "modern" C.
## 2. Technical Requirements
- **Standard**: ISO C23 (C2X).
- **Compiler Support**: Modern Clang (e.g., Clang 15+) or GCC (e.g., GCC 13+) with support for `[[...]]` attribute syntax.
- **Build System**: CMake 3.15+.

## 3. Toolchain & Quality Assurance

### 3.1 Build System
- **Generator**: CMake.
- **Warnings**: Enforced with `-Wall -Wextra -Werror -Wpedantic`.

### 3.2 Clang-Tidy
- Integrated into the CMake build process via `CMAKE_C_CLANG_TIDY`.
- All warnings are treated as errors.
- **Enabled Modules**:
  - `bugprone-*`: Catch common logic errors.
  - `performance-*`: Identify potential performance bottlenecks.
  - `readability-*`: Improve code maintainability.
  - `clang-analyzer-*`: Static analysis for memory and logic bugs.
- C++ specific checks (`modernize-*`, `cppcoreguidelines-*`) are explicitly disabled.

### 3.3 Sanitizers
- **AddressSanitizer (ASan)**: Detects memory errors (out-of-bounds, use-after-free, leaks). Enabled with `-DUSE_ASAN=ON`.
- **ThreadSanitizer (TSan)**: Detects data races. Enabled with `-DUSE_TSAN=ON`.
- *Note*: ASan and TSan are mutually exclusive.

## 4. Resource Management (RAII in C)
- Automatic resource cleanup is achieved using the `[[gnu::cleanup(function)]]` attribute.
- This pattern ensures that resources (memory, file handles, locks) are released automatically when the variable goes out of scope.
- **Cleanup functions** must accept a pointer to the variable being cleaned up.

## 5. C23 Feature Usage
- **Attributes**: cleanup, `[[maybe_unused]]`.
- **Keywords**: `bool`, `true`, `false`, `static_assert` (single argument).
- **Standard Library**: Uses `nullptr` and `constexpr` where supported; provides fallbacks for environments with partial C23 support.

## 6. Data Structures
- **Intrusive Structures**: We strongly prefer intrusive data structures (e.g., intrusive linked lists) over traditional pointer-to-data structures. Intrusive structures improve cache locality, reduce separate heap allocations, and simplify ownership management by embedding the management metadata directly within the data structure.
- **Generic Handling**: Use `offsetof` and the `container_of` pattern to navigate from metadata nodes back to the parent structure.

## 6. Project Structure
- `src/`: Source files (`.c`).
- `include/`: Public headers (`.h`).
- `CMakeLists.txt`: Build configuration.
- `.clang-tidy`: Linter configuration.
- `README.md`: Usage documentation.

## 7. Formatting
Code formatting follows modern C conventions. Run `clang-format` before committing.
