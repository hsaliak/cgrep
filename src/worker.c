#include "worker.h"
#include "raii.h"
#include "discovery.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

void work_queue_init(work_queue_t *q) {
    q->head = q->tail = NULL;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
    q->done = false;
}

void work_queue_push(work_queue_t *q, const char *filename) {
    work_item_t *item = malloc(sizeof(work_item_t));
    item->filename = strdup(filename);
    item->node.next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (q->tail) {
        q->tail->next = &item->node;
        q->tail = &item->node;
    } else {
        q->head = q->tail = &item->node;
    }
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

char* work_queue_pop(work_queue_t *q) {
    pthread_mutex_lock(&q->mutex);
    while (q->head == NULL && !q->done) {
        pthread_cond_wait(&q->cond, &q->mutex);
    }

    if (q->head == NULL) {
        pthread_mutex_unlock(&q->mutex);
        return NULL;
    }

    list_node_t *node = q->head;
    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    pthread_mutex_unlock(&q->mutex);

    work_item_t *item = container_of(node, work_item_t, node);
    char *filename = item->filename;
    free(item);
    return filename;
}

void work_queue_set_done(work_queue_t *q) {
    pthread_mutex_lock(&q->mutex);
    q->done = true;
    pthread_cond_broadcast(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

void work_queue_destroy(work_queue_t *q) {
    pthread_mutex_destroy(&q->mutex);
    pthread_cond_destroy(&q->cond);
}

void* worker_thread(void *arg) {
    worker_args_t *args = (worker_args_t*)arg;
    work_queue_t *q = args->queue;

    while (true) {
        auto_free char *filename = work_queue_pop(q);
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
