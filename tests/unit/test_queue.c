#include "unity.h"
#include "worker.h"
#include <pthread.h>
#include <unistd.h>

void setUp(void) {}
void tearDown(void) {}

void test_queue_basic_push_pop(void) {
    work_queue_t q;
    work_queue_init(&q);

    work_queue_push(&q, "test1.txt");
    work_queue_push(&q, "test2.txt");

    char *f1 = work_queue_pop(&q);
    TEST_ASSERT_EQUAL_STRING("test1.txt", f1);
    free(f1);

    char *f2 = work_queue_pop(&q);
    TEST_ASSERT_EQUAL_STRING("test2.txt", f2);
    free(f2);

    work_queue_destroy(&q);
}

void test_queue_done(void) {
    work_queue_t q;
    work_queue_init(&q);

    work_queue_set_done(&q);
    char *f = work_queue_pop(&q);
    TEST_ASSERT_TRUE(f == NULL);

    work_queue_destroy(&q);
}

typedef struct {
    work_queue_t *q;
    int count;
} producer_args_t;

void* producer(void *arg) {
    producer_args_t *args = (producer_args_t*)arg;
    for (int i = 0; i < args->count; i++) {
        work_queue_push(args->q, "item");
    }
    return NULL;
}

void test_queue_concurrent(void) {
    work_queue_t q;
    work_queue_init(&q);

    pthread_t p1, p2;
    producer_args_t args = {&q, 100};

    pthread_create(&p1, NULL, producer, &args);
    pthread_create(&p2, NULL, producer, &args);

    int total = 0;
    for (int i = 0; i < 200; i++) {
        char *f = work_queue_pop(&q);
        if (f) {
            total++;
            free(f);
        }
    }

    pthread_join(p1, NULL);
    pthread_join(p2, NULL);

    TEST_ASSERT_EQUAL_INT(200, total);
    work_queue_destroy(&q);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_queue_basic_push_pop);
    RUN_TEST(test_queue_done);
    RUN_TEST(test_queue_concurrent);
    return UNITY_END();
}
