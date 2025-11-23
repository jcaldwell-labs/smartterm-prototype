/*
 * Simple Test Framework for SmartTerm
 */

#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Test counters */
extern int tests_run;
extern int tests_passed;
extern int tests_failed;

/* Test macros */
#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✓ %s\n", message);                                                           \
        } else {                                                                                   \
            tests_failed++;                                                                        \
            printf("  ✗ %s\n", message);                                                           \
            printf("    Failed at %s:%d\n", __FILE__, __LINE__);                                   \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_EQUAL(expected, actual, message)                                               \
    do {                                                                                           \
        tests_run++;                                                                               \
        if ((expected) == (actual)) {                                                              \
            tests_passed++;                                                                        \
            printf("  ✓ %s\n", message);                                                           \
        } else {                                                                                   \
            tests_failed++;                                                                        \
            printf("  ✗ %s (expected: %d, got: %d)\n", message, (int)(expected), (int)(actual));  \
            printf("    Failed at %s:%d\n", __FILE__, __LINE__);                                   \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_STR_EQUAL(expected, actual, message)                                           \
    do {                                                                                           \
        tests_run++;                                                                               \
        if ((expected) == NULL || (actual) == NULL) {                                              \
            tests_failed++;                                                                        \
            printf("  ✗ %s (NULL pointer: expected=%p, actual=%p)\n", message,                    \
                   (void*)(expected), (void*)(actual));                                           \
            printf("    Failed at %s:%d\n", __FILE__, __LINE__);                                   \
        } else if (strcmp((expected), (actual)) == 0) {                                            \
            tests_passed++;                                                                        \
            printf("  ✓ %s\n", message);                                                           \
        } else {                                                                                   \
            tests_failed++;                                                                        \
            printf("  ✗ %s (expected: '%s', got: '%s')\n", message, expected, actual);            \
            printf("    Failed at %s:%d\n", __FILE__, __LINE__);                                   \
        }                                                                                          \
    } while (0)

#define TEST_ASSERT_NULL(ptr, message) TEST_ASSERT((ptr) == NULL, message)

#define TEST_ASSERT_NOT_NULL(ptr, message) TEST_ASSERT((ptr) != NULL, message)

/* Test suite management */
#define BEGIN_TEST_SUITE(name) printf("\n=== %s ===\n", name);

#define END_TEST_SUITE()                                                                           \
    do {                                                                                           \
        printf("\n");                                                                              \
    } while (0)

#define TEST_SUMMARY()                                                                             \
    do {                                                                                           \
        printf("\n========================================\n");                                     \
        printf("Tests run: %d\n", tests_run);                                                      \
        printf("Tests passed: %d\n", tests_passed);                                                \
        printf("Tests failed: %d\n", tests_failed);                                                \
        printf("========================================\n");                                       \
        return (tests_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;                                 \
    } while (0)

#endif /* TEST_FRAMEWORK_H */
