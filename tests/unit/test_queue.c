#include "unity.h"
#include "worker.h"
#include <pthread.h>
#include <unistd.h>

void setUp(void) {}
void tearDown(void) {}

void test_queue_basic_push_pop(void) {
    work_queue_t queue;
    work_queue_init(&queue);

    work_queue_push(&queue, "test1.txt");
    work_queue_push(&queue, "test2.txt");

    char *f1 = work_queue_pop(&queue);
    TEST_ASSERT_EQUAL_STRING("test1.txt", f1);
    free(f1);

    char *f2 = work_queue_pop(&queue);
    TEST_ASSERT_EQUAL_STRING("test2.txt", f2);
    free(f2);

    work_queue_destroy(&queue);
}

void test_queue_done(void) {
    work_queue_t queue;
    work_queue_init(&queue);

    work_queue_set_done(&queue);
    char *file = work_queue_pop(&queue);
    TEST_ASSERT_TRUE(file == NULL);

    work_queue_destroy(&queue);
}

typedef struct {
    work_queue_t *queue;
    int count;
} producer_args_t;

void* producer(void *arg) {
    producer_args_t *args = (producer_args_t*)arg;
    for (int i = 0; i < args->count; i++) {
        work_queue_push(args->queue, "item");
    }
    return NULL;
}

void test_queue_concurrent(void) {
    work_queue_t queue;
    work_queue_init(&queue);

    pthread_t thread1, thread2;
    producer_args_t args = {&queue, 100};

    pthread_create(&thread1, NULL, producer, &args);
    pthread_create(&thread2, NULL, producer, &args);

    int total = 0;
    for (int i = 0; i < 200; i++) {
        char *file = work_queue_pop(&queue);
        if (file) {
            total++;
            free(file);
        }
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    TEST_ASSERT_EQUAL_INT(200, total);
    work_queue_destroy(&queue);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_queue_basic_push_pop);
    RUN_TEST(test_queue_done);
    RUN_TEST(test_queue_concurrent);
    return UNITY_END();
}
