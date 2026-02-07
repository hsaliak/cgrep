#ifndef MATCHER_H
#define MATCHER_H

#include <stdbool.h>
#include <stddef.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

typedef struct {
    pcre2_code *code;
    bool case_insensitive;
    bool line_numbering;
    bool fixed_strings;
    bool invert_match;
} grep_config_t;

/**
 * @brief Initialize the matcher with a pattern.
 */
pcre2_code* matcher_compile(const char *pattern, bool case_insensitive, bool fixed_strings);

/**
 * @brief Match a buffer and print results.
 */
void matcher_process_buffer(const char *filename, const char *buffer, size_t length, const grep_config_t *config);

#endif // MATCHER_H
