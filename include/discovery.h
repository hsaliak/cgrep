#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char **include_patterns;
    size_t include_count;
    char **exclude_patterns;
    size_t exclude_count;
    bool ignore_binary;
    bool recursive;
} discovery_config_t;

#include "worker.h"

/**
 * @brief Discover files and add them to the work queue.
 */
void discover_files(const char *path, const discovery_config_t *config, work_queue_t *queue);

/**
 * @brief Check if a file is binary.
 */
bool is_binary(const char *buffer, size_t length);

/**
 * @brief Check if a filename matches include/exclude filters.
 */
bool should_process_file(const char *filename, const discovery_config_t *config);

#endif // DISCOVERY_H
