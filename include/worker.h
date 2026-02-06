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

/**
 * @brief Initialize a work queue.
 */
void work_queue_init(work_queue_t *queue);

/**
 * @brief Push a new path into the queue and increment the pending items count.
 * 
 * @param queue The work queue.
 * @param filename The path to add.
 */
void work_queue_push(work_queue_t *queue, const char *filename);

/**
 * @brief Pop a path from the queue.
 * 
 * Blocks if the queue is empty but not 'done'.
 * 
 * @note Every successful pop MUST be followed by a call to work_queue_item_done()
 *       after the item is processed (or if processing is skipped).
 * @return A heap-allocated string (caller must free), or NULL if the queue is finished.
 */
char* work_queue_pop(work_queue_t *queue);

/**
 * @brief Decrement the pending items count and signal 'done' if it reaches zero.
 * 
 * This function handles the lifecycle of the queue. Once all pushed items
 * (and any initial sentinels) are marked as done, workers will exit.
 */
void work_queue_item_done(work_queue_t *queue);

/**
 * @brief Explicitly set the 'done' flag to true.
 * 
 * Use this to force termination of the workers even if pending items remain.
 */
void work_queue_set_done(work_queue_t *queue);

/**
 * @brief Destroy the queue and free its internal resources.
 */
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
