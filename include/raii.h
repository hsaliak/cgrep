#ifndef RAII_H
#define RAII_H

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

/**
 * @file raii.h
 * @brief RAII-like cleanup helpers for C23 using [[gnu::cleanup]]
 * Consolidates all resource management.
 */

static inline void cleanup_free(void *ptr) {
    void **ptr_val = (void **)ptr;
    if (*ptr_val) {
        free(*ptr_val);
        *ptr_val = NULL;
    }
}

static inline void cleanup_close(int *file_descriptor) {
    if (*file_descriptor >= 0) {
        close(*file_descriptor);
        *file_descriptor = -1;
    }
}

static inline void cleanup_file(FILE **file_ptr) {
    if (*file_ptr) {
        fclose(*file_ptr);
        *file_ptr = NULL;
    }
}

struct mmap_region {
    void *addr;
    size_t length;
};

static inline void cleanup_munmap(struct mmap_region *region) {
    if (region->addr && region->addr != MAP_FAILED) {
        munmap(region->addr, region->length);
        region->addr = MAP_FAILED;
    }
}

static inline void cleanup_mutex_unlock(pthread_mutex_t **mutex) {
    if (*mutex) {
        pthread_mutex_unlock(*mutex);
    }
}

static inline void cleanup_pcre2_code(pcre2_code **code) {
    if (*code) {
        pcre2_code_free(*code);
        *code = NULL;
    }
}

static inline void cleanup_pcre2_match_data(pcre2_match_data **match_data) {
    if (*match_data) {
        pcre2_match_data_free(*match_data);
        *match_data = NULL;
    }
}

static inline void cleanup_pcre2_compile_context(pcre2_compile_context **context) {
    if (*context) {
        pcre2_compile_context_free(*context);
        *context = NULL;
    }
}

static inline void cleanup_pcre2_match_context(pcre2_match_context **context) {
    if (*context) {
        pcre2_match_context_free(*context);
        *context = NULL;
    }
}

static inline void cleanup_str_array(char ***array_ptr) {
    if (*array_ptr) {
        char **array = *array_ptr;
        for (int i = 0; array[i] != NULL; i++) {
            free(array[i]);
        }
        free(array);
        *array_ptr = NULL;
    }
}

#define auto_free [[gnu::cleanup(cleanup_free)]]
#define auto_close [[gnu::cleanup(cleanup_close)]]
#define auto_file [[gnu::cleanup(cleanup_file)]]
#define auto_munmap [[gnu::cleanup(cleanup_munmap)]]
#define auto_pcre2_code [[gnu::cleanup(cleanup_pcre2_code)]]
#define auto_pcre2_match_data [[gnu::cleanup(cleanup_pcre2_match_data)]]
#define auto_pcre2_compile_ctx [[gnu::cleanup(cleanup_pcre2_compile_context)]]
#define auto_pcre2_match_ctx [[gnu::cleanup(cleanup_pcre2_match_context)]]
#define auto_str_array [[gnu::cleanup(cleanup_str_array)]]

#endif // RAII_H
