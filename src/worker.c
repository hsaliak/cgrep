#include "worker.h"
#include "raii.h"
#include "discovery.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

void work_queue_init(work_queue_t *queue) {
    queue->capacity = 1024;
    queue->items = malloc(queue->capacity * sizeof(char *));
    queue->head = 0;
    queue->tail = 0;
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
    
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

char* work_queue_pop(work_queue_t *queue) {
    pthread_mutex_lock(&queue->mutex);
    
    while (queue->head == queue->tail && !queue->done) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
    }

    if (queue->head == queue->tail) {
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

void* worker_thread(void *arg) {
    worker_args_t *args = (worker_args_t*)arg;
    work_queue_t *queue = args->queue;

    while (true) {
        auto_free char *filename = work_queue_pop(queue);
        if (filename == NULL) break;

        auto_close int fd = open(filename, O_RDONLY);
        if (fd < 0) continue;

        struct stat st;
        if (fstat(fd, &st) != 0 || st.st_size == 0) continue;

        auto_munmap struct mmap_region region = { .addr = MAP_FAILED, .length = (size_t)st.st_size };
        region.addr = mmap(NULL, region.length, PROT_READ, MAP_PRIVATE, fd, 0);
        if (region.addr == MAP_FAILED) continue;

        if (is_binary(region.addr, region.length)) {
            continue;
        }

        matcher_process_buffer(filename, region.addr, region.length, args->grep_config);
    }

    return NULL;
}
