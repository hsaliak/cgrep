#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Cleanup function for memory allocated via malloc.
 * The attribute gnu::cleanup passes a pointer to the variable.
 */
static void free_ptr(void *ptr) {
    void **p = (void **)ptr;
    if (p && *p) {
        printf("Cleaning up memory at %p\n", *p);
        free(*p);
    }
}

/**
 * Cleanup function for FILE streams.
 */
static void close_file(FILE **f) {
    if (f && *f) {
        printf("Closing file stream\n");
        fclose(*f);
    }
}

int main(void) {
    // [[gnu::cleanup]] for RAII-like behavior (C23 attribute syntax)
    // This will call free_ptr(&buffer) when buffer goes out of scope.
    [[gnu::cleanup(free_ptr)]] char *buffer = malloc(100);
    if (buffer == NULL) { // Use nullptr in C23
        return EXIT_FAILURE;
    }

    snprintf(buffer, 100, "Hello C23 with [[gnu::cleanup]]!");
    printf("%s\n", buffer);

    [[gnu::cleanup(close_file)]] FILE *temp = fopen("temp.txt", "w");
    if (temp != NULL) { // Use nullptr in C23
        fprintf(temp, "Testing RAII-like file handling\n");
    }

    // C23: bool is a keyword (stdbool.h included for older compilers)
    bool success = true;
    if (success) {
        printf("Success is true\n");
    }

    // C23: constexpr (use const if compiler doesn't support C23 constexpr yet)
    const int max_value = 42;
    printf("Max value: %d\n", max_value);

    // C23: static_assert with one argument (supported in many modern compilers)
    static_assert(sizeof(int) >= 4, "Integer size check");

    return EXIT_SUCCESS;
}
