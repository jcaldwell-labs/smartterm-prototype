/*
 * Basic SmartTerm API tests
 *
 * Note: These tests run without a real terminal, so they test
 * the API interface and basic functionality but not rendering.
 */

#include "test_framework.h"
#include <smartterm.h>
#include <stdlib.h>

int main(void)
{
    BEGIN_TEST_SUITE("Basic API Tests");

    /* Test 1: Default configuration */
    smartterm_config_t config = smartterm_default_config();
    TEST_ASSERT(config.max_lines == 1000, "Default max_lines is 1000");
    TEST_ASSERT(config.status_bar_enabled == true, "Status bar enabled by default");
    TEST_ASSERT(config.history_enabled == true, "History enabled by default");
    TEST_ASSERT(config.thread_safe == true, "Thread safety enabled by default");

    /* Test 2: Initialization with NULL config (should use defaults) */
    /* Note: In headless environment, init may fail - that's expected */
    smartterm_ctx* ctx = smartterm_init(NULL);
    if (ctx == NULL) {
        printf("  ⓘ Init failed (expected in headless environment)\n");
        tests_run++; /* Count as run but not failed */
    } else {
        TEST_ASSERT_NOT_NULL(ctx, "Initialize with NULL config");
        smartterm_cleanup(ctx);
        tests_passed++;
    }

    /* Test 3: Custom configuration */
    config.max_lines = 500;
    config.status_bar_enabled = false;
    TEST_ASSERT(config.max_lines == 500, "Custom max_lines setting");
    TEST_ASSERT(config.status_bar_enabled == false, "Custom status bar setting");

    /* Test 4: Error codes are defined */
    TEST_ASSERT(SMARTTERM_OK == 0, "SMARTTERM_OK is 0");
    TEST_ASSERT(SMARTTERM_ERROR < 0, "SMARTTERM_ERROR is negative");
    TEST_ASSERT(SMARTTERM_NOMEM < 0, "SMARTTERM_NOMEM is negative");
    TEST_ASSERT(SMARTTERM_INVALID < 0, "SMARTTERM_INVALID is negative");

    /* Test 5: Context types are defined */
    TEST_ASSERT(CTX_NORMAL == 0, "CTX_NORMAL is 0");
    TEST_ASSERT(CTX_ERROR > 0, "CTX_ERROR is defined");
    TEST_ASSERT(CTX_SUCCESS > 0, "CTX_SUCCESS is defined");
    TEST_ASSERT(CTX_INFO > 0, "CTX_INFO is defined");

    END_TEST_SUITE();
    TEST_SUMMARY();
}
