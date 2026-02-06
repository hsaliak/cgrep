#ifndef UNITY_H
#define UNITY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void setUp(void);
void tearDown(void);

#define TEST_ASSERT_TRUE(condition) \
    if (!(condition)) { \
        printf("%s:%d: FAIL: Expected true\n", __FILE__, __LINE__); \
        exit(1); \
    }

#define TEST_ASSERT_FALSE(condition) \
    if ((condition)) { \
        printf("%s:%d: FAIL: Expected false\n", __FILE__, __LINE__); \
        exit(1); \
    }

#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    if ((expected) != (actual)) { \
        printf("%s:%d: FAIL: Expected %d but was %d\n", __FILE__, __LINE__, (expected), (actual)); \
        exit(1); \
    }

#define TEST_ASSERT_EQUAL_STRING(expected, actual) \
    if (strcmp((expected), (actual)) != 0) { \
        printf("%s:%d: FAIL: Expected '%s' but was '%s'\n", __FILE__, __LINE__, (expected), (actual)); \
        exit(1); \
    }

#define RUN_TEST(test_func) \
    printf("Running %s...\n", #test_func); \
    setUp(); \
    test_func(); \
    tearDown(); \
    printf("OK\n");

#define UNITY_BEGIN()
#define UNITY_END() 0

#endif
