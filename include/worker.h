#ifndef WORKER_H
#define WORKER_H

#include "matcher.h"
#include <pthread.h>

typedef struct {
    char **items;
    size_t head;
    size_t tail;
    size_t capacity;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool done;
    int pending_items;
} work_queue_t;

struct discovery_config;

typedef struct {
    work_queue_t *queue;
    const grep_config_t *grep_config;
    const struct discovery_config *discovery_config;
} worker_args_t;

void work_queue_init(work_queue_t *queue);
void work_queue_push(work_queue_t *queue, const char *filename);
char* work_queue_pop(work_queue_t *queue);
void work_queue_item_done(work_queue_t *queue);
void work_queue_set_done(work_queue_t *queue);
void work_queue_destroy(work_queue_t *queue);

/**
 * RAII helper for work_queue_t.
 */
static inline void work_queue_cleanup(work_queue_t *queue) {
    if (queue->items) { // Simple check to see if it was initialized
        work_queue_destroy(queue);
    }
}

#define auto_work_queue [[gnu::cleanup(work_queue_cleanup)]]

void* worker_thread(void *arg);

#endif // WORKER_H
