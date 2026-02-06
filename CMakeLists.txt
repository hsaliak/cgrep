cmake_minimum_required(VERSION 3.15)
project(CStarterProject LANGUAGES C)

# Standard: C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
find_package(PkgConfig REQUIRED)


# Clang-Tidy Integration
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
  set(CMAKE_C_CLANG_TIDY "${CLANG_TIDY_EXE}")
else()
  message(WARNING "clang-tidy not found")
endif()

# Sanitizer Options
option(ENABLE_ASAN "Enable AddressSanitizer" ON)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)

if(ENABLE_ASAN AND ENABLE_TSAN)
  message(FATAL_ERROR "ASan and TSan cannot be enabled at the same time")
endif()

if(ENABLE_ASAN)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

if(ENABLE_TSAN)
  add_compile_options(-fsanitize=thread)
  add_link_options(-fsanitize=thread)
endif()

# Compilation Flags
if(MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Werror -Wpedantic)
endif()

find_package(Threads REQUIRED)
if(APPLE)
    # Check common Homebrew locations for macOS
    set(PCRE2_PATHS "/opt/homebrew" "/usr/local")
    find_path(PCRE2_INCLUDE_DIR pcre2.h PATHS ${PCRE2_PATHS} PATH_SUFFIXES include)
    find_library(PCRE2_LIBRARY pcre2-8 PATHS ${PCRE2_PATHS} PATH_SUFFIXES lib)
    
    if(PCRE2_INCLUDE_DIR AND PCRE2_LIBRARY)
        set(PCRE2_FOUND TRUE)
        include_directories(include ${PCRE2_INCLUDE_DIR})
        set(PCRE2_LIBRARIES ${PCRE2_LIBRARY})
    else()
        message(FATAL_ERROR "PCRE2 not found. Please install it (e.g., brew install pcre2)")
    endif()
else()
    pkg_check_modules(PCRE2 REQUIRED libpcre2-8)
    include_directories(include ${PCRE2_INCLUDE_DIRS})
    set(PCRE2_LIBRARIES ${PCRE2_LIBRARIES})
endif()

set(SOURCES
    src/main.c
    src/discovery.c
    src/worker.c
    src/matcher.c
    src/output.c
)

add_executable(cgrep ${SOURCES})
target_link_libraries(cgrep PRIVATE Threads::Threads ${PCRE2_LIBRARIES})

# Testing
enable_testing()

add_executable(unit_tests 
    tests/unit/test_queue.c 
    tests/vendor/unity.c
    src/worker.c
    src/matcher.c
    src/discovery.c
    src/output.c
)
target_include_directories(unit_tests PRIVATE include tests/vendor)
target_link_libraries(unit_tests PRIVATE Threads::Threads ${PCRE2_LIBRARIES})
add_test(NAME UnitTests COMMAND unit_tests)

# Integration Tests
find_program(PYTHON_EXE NAMES python3 python)
if(PYTHON_EXE)
    add_test(NAME IntegrationTests 
             COMMAND ${PYTHON_EXE} ${CMAKE_SOURCE_DIR}/tests/integration/test_cli.py
             WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif()

# Custom target to run the full verification script
add_custom_target(verify
    COMMAND ${CMAKE_SOURCE_DIR}/scripts/verify.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running full verification suite (ASan + TSan)"
    USES_TERMINAL
)
