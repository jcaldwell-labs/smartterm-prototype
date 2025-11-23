/*
 * SmartTerm Example: Chat Client
 *
 * Demonstrates an IRC-style chat interface with SmartTerm.
 * Simulates a chat client with multiple message types.
 */

#include <pthread.h>
#include <smartterm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

/* Global context for worker thread */
static smartterm_ctx* g_ctx = NULL;
static bool g_running = true;

/* Simulate incoming messages */
static void* message_simulator(void* arg)
{
    (void)arg;

    const char* users[] = {"Alice", "Bob", "Charlie", "Dave"};
    const char* messages[] = {"Hey everyone!",
                              "How's it going?",
                              "Anyone want to grab lunch?",
                              "Check out this new feature!",
                              "Meeting in 5 minutes",
                              "Great work on the project!",
                              "lol",
                              "brb",
                              "ttyl"};

    int num_users = sizeof(users) / sizeof(users[0]);
    int num_messages = sizeof(messages) / sizeof(messages[0]);

    while (g_running) {
        sleep(5 + (rand() % 10)); /* Random delay 5-15 seconds */

        if (!g_running)
            break;

        /* Random user and message */
        const char* user = users[rand() % num_users];
        const char* message = messages[rand() % num_messages];

        /* Write simulated message (thread-safe) */
        char buffer[256];
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char time_str[10];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

        snprintf(buffer, sizeof(buffer), "[%s] <%s> %s", time_str, user, message);

        smartterm_write(g_ctx, buffer, CTX_INFO);
    }

    return NULL;
}

/* Process chat command */
static void process_command(smartterm_ctx* ctx, const char* input)
{
    if (input[0] != '/') {
        /* Regular message */
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        char time_str[10];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);

        char buffer[256];
        snprintf(buffer, sizeof(buffer), "[%s] <You> %s", time_str, input);
        smartterm_write(ctx, buffer, CTX_SUCCESS);
        return;
    }

    /* Command */
    if (strcmp(input, "/help") == 0) {
        smartterm_write(ctx, "Chat Commands:", CTX_INFO);
        smartterm_write(ctx, "  /help   - Show this help", CTX_NORMAL);
        smartterm_write(ctx, "  /users  - List users", CTX_NORMAL);
        smartterm_write(ctx, "  /clear  - Clear screen", CTX_NORMAL);
        smartterm_write(ctx, "  /export - Export chat log", CTX_NORMAL);
        smartterm_write(ctx, "  /quit   - Exit chat", CTX_NORMAL);
        smartterm_write(ctx, "", CTX_NORMAL);
        smartterm_write(ctx, "Just type a message and press Enter to send.", CTX_NORMAL);
    } else if (strcmp(input, "/users") == 0) {
        smartterm_write(ctx, "Users in channel:", CTX_INFO);
        smartterm_write(ctx, "  - You", CTX_SUCCESS);
        smartterm_write(ctx, "  - Alice", CTX_NORMAL);
        smartterm_write(ctx, "  - Bob", CTX_NORMAL);
        smartterm_write(ctx, "  - Charlie", CTX_NORMAL);
        smartterm_write(ctx, "  - Dave", CTX_NORMAL);
    } else if (strcmp(input, "/clear") == 0) {
        smartterm_clear(ctx);
        smartterm_write(ctx, "--- Chat cleared ---", CTX_COMMENT);
    } else if (strcmp(input, "/export") == 0) {
        int result = smartterm_export(ctx, "chat_log.txt", EXPORT_PLAIN, 0, -1, true);
        if (result == SMARTTERM_OK) {
            smartterm_write(ctx, "Chat log exported to chat_log.txt", CTX_SUCCESS);
        } else {
            smartterm_write(ctx, "Failed to export chat log", CTX_ERROR);
        }
    } else if (strcmp(input, "/quit") == 0 || strcmp(input, "/exit") == 0) {
        smartterm_write(ctx, "Disconnecting from chat...", CTX_WARNING);
        g_running = false;
    } else {
        smartterm_write_fmt(ctx, CTX_ERROR, "Unknown command: %s", input);
        smartterm_write(ctx, "Type /help for available commands", CTX_COMMENT);
    }
}

int main(void)
{
    srand(time(NULL));

    /* Initialize SmartTerm */
    smartterm_config_t config = smartterm_default_config();
    config.history_enabled = true;
    config.prompt = "> ";

    smartterm_ctx* ctx = smartterm_init(&config);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize SmartTerm\n");
        return 1;
    }

    g_ctx = ctx;

    /* Welcome messages */
    smartterm_write(ctx, "=== SmartTerm Chat Client ===", CTX_INFO);
    smartterm_write(ctx, "Connected to #general", CTX_SUCCESS);
    smartterm_write(ctx, "Type /help for commands", CTX_COMMENT);
    smartterm_write(ctx, "", CTX_NORMAL);

    /* Set status bar */
    smartterm_status_set(ctx, "#general", "5 users online");

    /* Start message simulator thread */
    pthread_t simulator_thread;
    pthread_create(&simulator_thread, NULL, message_simulator, NULL);

    /* Main loop */
    int message_count = 0;

    while (g_running) {
        char* input = smartterm_read_line(ctx, NULL);

        if (!input) {
            /* EOF */
            break;
        }

        /* Skip empty input */
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        /* Process command or message */
        process_command(ctx, input);

        if (input[0] != '/') {
            message_count++;
        }

        /* Update status bar */
        char status[64];
        snprintf(status, sizeof(status), "Messages sent: %d", message_count);
        smartterm_status_set(ctx, "#general", status);

        free(input);
    }

    /* Cleanup */
    g_running = false;
    pthread_join(simulator_thread, NULL);
    smartterm_cleanup(ctx);

    printf("Chat client exited.\n");
    return 0;
}
