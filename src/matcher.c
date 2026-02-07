#include "matcher.h"
#include "raii.h"
#include <stdio.h>
#include <string.h>
#include "output.h"

pcre2_code* matcher_compile(const char *pattern, bool case_insensitive, bool fixed_strings) {
    int errornumber;
    PCRE2_SIZE erroroffset;
    uint32_t options = 0;

    if (case_insensitive) {
        options |= PCRE2_CASELESS;
    }

    if (fixed_strings) {
        options |= PCRE2_LITERAL;
    } else {
        options |= PCRE2_MULTILINE;
    }

    pcre2_code *code = pcre2_compile(
        (PCRE2_SPTR)pattern,
        PCRE2_ZERO_TERMINATED,
        options,
        &errornumber,
        &erroroffset,
        NULL
    );

    if (code == NULL) {
        PCRE2_UCHAR buffer[256];
        pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed at offset %d: %s\n", (int)erroroffset, buffer);
        return NULL;
    }

    return code;
}

void matcher_process_buffer(const char *filename, const char *buffer, size_t length, const grep_config_t *config) {
    if (config->code == NULL) return;

    auto_pcre2_match_data pcre2_match_data *match_data = pcre2_match_data_create_from_pattern(config->code, NULL);
    if (match_data == NULL) return;

    if (config->invert_match) {
        /* 
         * Inverted match mode: iterate line-by-line and print lines that 
         * do NOT match the pattern.
         */
        const char *line_start = buffer;
        unsigned int line_number = 1;
        while (line_start < buffer + length) {
            // Find the end of the current line
            const char *line_end = line_start;
            while (line_end < buffer + length && *line_end != '\n') {
                line_end++;
            }

            // Perform match on the single line
            int return_code = pcre2_match(
                config->code,
                (PCRE2_SPTR)line_start,
                line_end - line_start,
                0,
                0,
                match_data,
                NULL
            );

            // If no match found, output the line
            if (return_code == PCRE2_ERROR_NOMATCH) {
                if (filename && config->line_numbering) {
                    output_printf("%s:%u:%.*s\n", filename, line_number, (int)(line_end - line_start), line_start);
                } else if (filename) {
                    output_printf("%s:%.*s\n", filename, (int)(line_end - line_start), line_start);
                } else if (config->line_numbering) {
                    output_printf("%u:%.*s\n", line_number, (int)(line_end - line_start), line_start);
                } else {
                    output_printf("%.*s\n", (int)(line_end - line_start), line_start);
                }
            } else if (return_code < 0) {
                fprintf(stderr, "Matching error %d\n", return_code);
            }

            // Move to the next line
            line_start = line_end;
            if (line_start < buffer + length && *line_start == '\n') {
                line_start++;
                line_number++;
            }
        }
    } else {
        /*
         * Standard match mode: find matches in the entire buffer and identify
         * the containing lines for output.
         */
        PCRE2_SIZE start_offset = 0;
        int return_code;
        unsigned int line_number = 1;
        const char *last_line_start = buffer;

        while (start_offset < length) {
            return_code = pcre2_match(
                config->code,
                (PCRE2_SPTR)buffer,
                length,
                start_offset,
                0,
                match_data,
                NULL
            );

            if (return_code < 0) {
                if (return_code != PCRE2_ERROR_NOMATCH) {
                    fprintf(stderr, "Matching error %d\n", return_code);
                }
                break;
            }

            PCRE2_SIZE *ovector = pcre2_get_ovector_pointer(match_data);
            
            // Find line start and end for the match
            const char *match_start = buffer + ovector[0];
            const char *line_start = match_start;
            while (line_start > buffer && *(line_start - 1) != '\n') {
                line_start--;
            }

            // Count lines up to match
            if (config->line_numbering) {
                for (const char *ptr_pos = last_line_start; ptr_pos < line_start; ptr_pos++) {
                    if (*ptr_pos == '\n') line_number++;
                }
                last_line_start = line_start;
            }

            const char *line_end = buffer + ovector[1];
            while (line_end < buffer + length && *line_end != '\n') {
                line_end++;
            }

            // Output matching line
            if (filename && config->line_numbering) {
                output_printf("%s:%u:%.*s\n", filename, line_number, (int)(line_end - line_start), line_start);
            } else if (filename) {
                output_printf("%s:%.*s\n", filename, (int)(line_end - line_start), line_start);
            } else if (config->line_numbering) {
                output_printf("%u:%.*s\n", line_number, (int)(line_end - line_start), line_start);
            } else {
                output_printf("%.*s\n", (int)(line_end - line_start), line_start);
            }

            // Move to the next line to avoid multiple matches on the same line (standard grep)
            start_offset = line_end - buffer;
            if (start_offset < length && buffer[start_offset] == '\n') {
                start_offset++;
            }
        }
    }
}
