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
} work_queue_t;

typedef struct {
    work_queue_t *queue;
    const grep_config_t *grep_config;
} worker_args_t;

void work_queue_init(work_queue_t *q);
void work_queue_push(work_queue_t *q, const char *filename);
char* work_queue_pop(work_queue_t *q);
void work_queue_set_done(work_queue_t *q);
void work_queue_destroy(work_queue_t *q);

void* worker_thread(void *arg);

#endif // WORKER_H
