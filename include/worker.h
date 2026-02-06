#ifndef WORKER_H
#define WORKER_H

#include "matcher.h"
#include "list.h"
#include <pthread.h>

typedef struct {
    char *filename;
    list_node_t node;
} work_item_t;

typedef struct {
    list_node_t *head;
    list_node_t *tail;
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
