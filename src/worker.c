#include "worker.h"
#include "raii.h"
#include "discovery.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void work_queue_init(work_queue_t *queue) {
    queue->capacity = 1024;
    queue->items = malloc(queue->capacity * sizeof(char *));
    queue->head = 0;
    queue->tail = 0;
    queue->pending_items = 0;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond, NULL);
    queue->done = false;
}

void work_queue_push(work_queue_t *queue, const char *filename) {
    pthread_mutex_lock(&queue->mutex);
    
    if (queue->tail == queue->capacity) {
        queue->capacity *= 2;
        queue->items = realloc(queue->items, queue->capacity * sizeof(char *));
    }
    
    queue->items[queue->tail++] = strdup(filename);
    queue->pending_items++;
    
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

char* work_queue_pop(work_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->head == queue->tail && !queue->done) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->done && queue->head == queue->tail) {
        pthread_mutex_unlock(&queue->mutex);
        return NULL;
    }

    char *filename = queue->items[queue->head++];
    
    // Auto-Reset optimization
    if (queue->head == queue->tail) {
        queue->head = 0;
        queue->tail = 0;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    return filename;
}

void work_queue_item_done(work_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->pending_items--;
    if (queue->pending_items == 0) {
        queue->done = true;
        pthread_cond_broadcast(&queue->cond);
    }
    pthread_mutex_unlock(&queue->mutex);
}

void work_queue_set_done(work_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    queue->done = true;
    pthread_cond_broadcast(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

void work_queue_destroy(work_queue_t *queue) {
    // Free any remaining strings in the queue
    for (size_t i = queue->head; i < queue->tail; i++) {
        free(queue->items[i]);
    }
    free(queue->items);
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);
}

static void search_fd(int fd, const char *filename, worker_args_t *args) {
    size_t capacity = 64 * 1024; // 64KB initial buffer
    size_t length = 0;
    auto_free char *buffer = malloc(capacity);
    if (!buffer) return;

    ssize_t n;
    while ((n = read(fd, buffer + length, capacity - length)) > 0) {
        length += (size_t)n;
        if (length == capacity) {
            capacity *= 2;
            char *new_buffer = realloc(buffer, capacity);
            if (!new_buffer) {
                return;
            }
            buffer = new_buffer;
        }
    }

    if (length > 0) {
        if (!args->discovery_config->ignore_binary || !is_binary(buffer, length)) {
            matcher_process_buffer(filename, buffer, length, args->grep_config);
        }
    }
}

static void search_file(const char *filename, worker_args_t *args) {
    auto_close int fd = open(filename, O_RDONLY);
    if (fd < 0) return;

    struct stat st;
    if (fstat(fd, &st) != 0) return;

    // For regular files with non-zero size, use mmap
    if (S_ISREG(st.st_mode) && st.st_size > 0) {
        auto_munmap struct mmap_region region = { .addr = MAP_FAILED, .length = (size_t)st.st_size };
        region.addr = mmap(NULL, region.length, PROT_READ, MAP_PRIVATE, fd, 0);
        if (region.addr != MAP_FAILED) {
            if (!args->discovery_config->ignore_binary || !is_binary(region.addr, region.length)) {
                matcher_process_buffer(filename, region.addr, region.length, args->grep_config);
            }
            return;
        }
    }

    // Fallback to reading for special files (FIFOs, etc.) or if mmap fails
    search_fd(fd, filename, args);
}

void* worker_thread(void *arg) {
    worker_args_t *args = (worker_args_t*)arg;
    work_queue_t *queue = args->queue;

    while (true) {
        auto_free char *path = work_queue_pop(queue);
        if (path == NULL) break;

        if (strcmp(path, "-") == 0) {
            search_fd(STDIN_FILENO, "(standard input)", args);
            work_queue_item_done(queue);
            continue;
        }

        struct stat st;
        if (lstat(path, &st) != 0) {
            work_queue_item_done(queue);
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            if (args->discovery_config->recursive) {
                discover_files(path, args->discovery_config, queue);
            }
        } else {
            // Process regular files or other readable types (like FIFOs)
            if (should_process_file(path, args->discovery_config)) {
                search_file(path, args);
            }
        }

        work_queue_item_done(queue);
    }

    return NULL;
}
