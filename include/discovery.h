#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <stdbool.h>
#include <stddef.h>

typedef struct discovery_config {
    char **include_patterns;
    size_t include_count;
    char **exclude_patterns;
    size_t exclude_count;
    bool ignore_binary;
    bool recursive;
} discovery_config_t;

#include "worker.h"

/**
 * @brief Discover files and directories and add them to the work queue.
 * 
 * If the path is a directory, its immediate children are added to the queue.
 * If the path is a regular file, it is added if it matches the configuration filters.
 * In a recursive search, worker threads call this function to expand subdirectories.
 * 
 * @param path The path to start discovery from.
 * @param config The discovery configuration (filters, recursive flag).
 * @param queue The work queue to push discovered items to.
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
