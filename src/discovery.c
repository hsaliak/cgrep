#include "discovery.h"
#include "worker.h"
#include "raii.h"
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fnmatch.h>
#include <limits.h>

bool should_process_file(const char *filename, const discovery_config_t *config) {
    const char *basename = strrchr(filename, '/');
    basename = (basename == NULL) ? filename : basename + 1;

    // Exclude filters
    for (size_t i = 0; i < config->exclude_count; i++) {
        if (fnmatch(config->exclude_patterns[i], basename, 0) == 0) {
            return false;
        }
    }

    // Include filters
    if (config->include_count > 0) {
        bool matched = false;
        for (size_t i = 0; i < config->include_count; i++) {
            if (fnmatch(config->include_patterns[i], basename, 0) == 0) {
                matched = true;
                break;
            }
        }
        if (!matched) return false;
    }

    return true;
}

bool is_binary(const char *buffer, size_t length) {
    size_t check_len = (length < 1024) ? length : 1024;
    for (size_t i = 0; i < check_len; i++) {
        if (buffer[i] == '\0') return true;
    }
    return false;
}

void discover_files(const char *path, const discovery_config_t *config, work_queue_t *queue) {
    struct stat path_stat;
    if (lstat(path, &path_stat) != 0) return;

    if (S_ISDIR(path_stat.st_mode)) {
        DIR *dir = opendir(path);
        if (!dir) return;

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            char full_path[PATH_MAX];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
            work_queue_push(queue, full_path);
        }
        closedir(dir);
    } else if (S_ISREG(path_stat.st_mode)) {
        if (should_process_file(path, config)) {
            work_queue_push(queue, path);
        }
    }
}
