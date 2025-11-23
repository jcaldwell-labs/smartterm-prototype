/*
 * SmartTerm Example: Log Viewer
 *
 * Demonstrates real-time log monitoring with SmartTerm.
 * Simulates a log viewer with different log levels.
 */

#include <pthread.h>
#include <signal.h>
#include <smartterm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Global context for worker thread */
static smartterm_ctx* g_ctx = NULL;
static bool g_running = true;
static bool g_paused = false;

/* Log levels */
typedef enum { LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERROR } log_level_t;

/* Generate simulated log entry */
static void generate_log(smartterm_ctx* ctx)
{
    const char* components[] = {"WebServer", "Database", "Auth", "API", "Cache", "Queue"};

    const char* debug_msgs[] = {"Processing request", "Cache hit", "Query executed",
                                "Connection pool status: OK"};

    const char* info_msgs[] = {"Request completed successfully", "User logged in",
                               "Background job started", "Configuration reloaded"};

    const char* warning_msgs[] = {"Slow query detected", "High memory usage",
                                  "Rate limit approaching", "Deprecated API called"};

    const char* error_msgs[] = {"Connection timeout", "Invalid request parameter",
                                "Authentication failed", "Database error"};

    /* Random log level (weighted toward info) */
    int level_rand = rand() % 100;
    log_level_t level;
    const char* message;

    if (level_rand < 20) {
        level = LOG_DEBUG;
        message = debug_msgs[rand() % 4];
    } else if (level_rand < 70) {
        level = LOG_INFO;
        message = info_msgs[rand() % 4];
    } else if (level_rand < 90) {
        level = LOG_WARNING;
        message = warning_msgs[rand() % 4];
    } else {
        level = LOG_ERROR;
        message = error_msgs[rand() % 4];
    }

    const char* component = components[rand() % 6];

    /* Format log entry */
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[20];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    char buffer[256];
    const char* level_str;
    smartterm_context_t ctx_type;

    switch (level) {
    case LOG_DEBUG:
        level_str = "DEBUG";
        ctx_type = CTX_DEBUG;
        break;
    case LOG_INFO:
        level_str = "INFO ";
        ctx_type = CTX_INFO;
        break;
    case LOG_WARNING:
        level_str = "WARN ";
        ctx_type = CTX_WARNING;
        break;
    case LOG_ERROR:
        level_str = "ERROR";
        ctx_type = CTX_ERROR;
        break;
    default:
        level_str = "UNKNOWN";
        ctx_type = CTX_NORMAL;
    }

    snprintf(buffer, sizeof(buffer), "[%s] [%s] [%s] %s", time_str, level_str, component, message);

    smartterm_write(ctx, buffer, ctx_type);
}

/* Log generator thread */
static void* log_generator(void* arg)
{
    (void)arg;

    while (g_running) {
        usleep(100000 + (rand() % 400000)); /* 100-500ms delay */

        if (!g_running)
            break;
        if (g_paused)
            continue;

        generate_log(g_ctx);
    }

    return NULL;
}

/* Signal handler for Ctrl+C */
static void signal_handler(int sig)
{
    (void)sig;
    g_running = false;
}

int main(void)
{
    srand(time(NULL));
    signal(SIGINT, signal_handler);

    /* Initialize SmartTerm */
    smartterm_config_t config = smartterm_default_config();
    config.history_enabled = false; /* No history for log viewer */
    config.prompt = "cmd> ";
    config.max_lines = 5000; /* More lines for logs */

    smartterm_ctx* ctx = smartterm_init(&config);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize SmartTerm\n");
        return 1;
    }

    g_ctx = ctx;

    /* Welcome messages */
    smartterm_write(ctx, "=== SmartTerm Log Viewer ===", CTX_INFO);
    smartterm_write(ctx, "Monitoring application logs...", CTX_SUCCESS);
    smartterm_write(ctx, "Commands: /pause, /resume, /clear, /export, /search, /quit", CTX_COMMENT);
    smartterm_write(ctx, "", CTX_NORMAL);

    /* Set status bar */
    smartterm_status_set(ctx, "Log Viewer", "Monitoring");

    /* Start log generator thread */
    pthread_t generator_thread;
    pthread_create(&generator_thread, NULL, log_generator, NULL);

    /* Main loop */
    int total_logs = 0;

    while (g_running) {
        /* Non-blocking input check would go here
         * For simplicity, we'll just process on enter */
        char* input = smartterm_read_line(ctx, NULL);

        if (!input) {
            break;
        }

        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        /* Process commands */
        if (strcmp(input, "/quit") == 0 || strcmp(input, "/exit") == 0) {
            smartterm_write(ctx, "Stopping log viewer...", CTX_WARNING);
            g_running = false;
        } else if (strcmp(input, "/pause") == 0) {
            g_paused = true;
            smartterm_write(ctx, "Log monitoring paused", CTX_WARNING);
            smartterm_status_set(ctx, "Log Viewer", "PAUSED");
        } else if (strcmp(input, "/resume") == 0) {
            g_paused = false;
            smartterm_write(ctx, "Log monitoring resumed", CTX_SUCCESS);
            smartterm_status_set(ctx, "Log Viewer", "Monitoring");
        } else if (strcmp(input, "/clear") == 0) {
            smartterm_clear(ctx);
            smartterm_write(ctx, "--- Logs cleared ---", CTX_COMMENT);
            total_logs = 0;
        } else if (strcmp(input, "/export") == 0) {
            int result = smartterm_export(ctx, "logs_export.txt", EXPORT_PLAIN, 0, -1, true);
            if (result == SMARTTERM_OK) {
                smartterm_write(ctx, "Logs exported to logs_export.txt", CTX_SUCCESS);
            } else {
                smartterm_write(ctx, "Failed to export logs", CTX_ERROR);
            }
        } else if (strncmp(input, "/search ", 8) == 0) {
            const char* pattern = input + 8;
            smartterm_search_result_t* results;
            int count;

            int ret = smartterm_search(ctx, pattern, false, &results, &count);
            if (ret == SMARTTERM_OK) {
                smartterm_write_fmt(ctx, CTX_SUCCESS, "Found %d matches for: %s", count, pattern);
                smartterm_write(ctx, "Use /next and /prev to navigate", CTX_COMMENT);
                smartterm_free_search_results(results);
            } else {
                smartterm_write_fmt(ctx, CTX_ERROR, "Search failed for: %s", pattern);
            }
        } else if (strcmp(input, "/next") == 0) {
            if (smartterm_search_next(ctx) != SMARTTERM_OK) {
                smartterm_write(ctx, "No search results", CTX_WARNING);
            }
        } else if (strcmp(input, "/prev") == 0) {
            if (smartterm_search_prev(ctx) != SMARTTERM_OK) {
                smartterm_write(ctx, "No search results", CTX_WARNING);
            }
        } else {
            smartterm_write_fmt(ctx, CTX_ERROR, "Unknown command: %s", input);
        }

        total_logs = smartterm_get_line_count(ctx);

        /* Update status bar */
        if (!g_paused) {
            char status[64];
            snprintf(status, sizeof(status), "Logs: %d", total_logs);
            smartterm_status_set(ctx, "Log Viewer", status);
        }

        free(input);
    }

    /* Cleanup */
    g_running = false;
    pthread_join(generator_thread, NULL);
    smartterm_cleanup(ctx);

    printf("Log viewer exited.\n");
    return 0;
}
