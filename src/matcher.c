#include "matcher.h"
#include "raii.h"
#include <stdio.h>
#include <string.h>
#include "output.h"

pcre2_code* matcher_compile(const char *pattern, bool case_insensitive) {
    int errornumber;
    PCRE2_SIZE erroroffset;
    uint32_t options = PCRE2_MULTILINE;

    if (case_insensitive) {
        options |= PCRE2_CASELESS;
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

    PCRE2_SIZE start_offset = 0;
    int rc;
    unsigned int line_number = 1;
    const char *last_line_start = buffer;

    while (start_offset < length) {
        rc = pcre2_match(
            config->code,
            (PCRE2_SPTR)buffer,
            length,
            start_offset,
            0,
            match_data,
            NULL
        );

        if (rc < 0) {
            if (rc != PCRE2_ERROR_NOMATCH) {
                fprintf(stderr, "Matching error %d\n", rc);
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
            for (const char *p = last_line_start; p < line_start; p++) {
                if (*p == '\n') line_number++;
            }
            last_line_start = line_start;
        }

        const char *line_end = buffer + ovector[1];
        while (line_end < buffer + length && *line_end != '\n') {
            line_end++;
        }

        // Output match (Thread-safe printing will be handled in output.c eventually)
        // For now, let's just use printf, but we'll need a mutex or a better way later.
        // We'll wrap this in a thread-safe output function.
        
        if (filename && config->line_numbering) {
            output_printf("%s:%u:%.*s\n", filename, line_number, (int)(line_end - line_start), line_start);
        } else if (filename) {
            output_printf("%s:%.*s\n", filename, (int)(line_end - line_start), line_start);
        } else if (config->line_numbering) {
            output_printf("%u:%.*s\n", line_number, (int)(line_end - line_start), line_start);
        } else {
            output_printf("%.*s\n", (int)(line_end - line_start), line_start);
        }

        // Move to next line to avoid multiple matches on same line if desired, 
        // or just move past the current match. Standard grep shows the line once if it matches.
        // But if multiple matches are on the same line, standard grep only prints the line once 
        // unless -o is used. We are doing a subset. Let's print the line once per match or 
        // move to next line.
        start_offset = line_end - buffer;
        if (start_offset < length && buffer[start_offset] == '\n') {
            start_offset++;
        }
    }
}
