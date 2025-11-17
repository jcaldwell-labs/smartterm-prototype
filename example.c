/*
 * SmartTerm Simple Library - Example Program
 *
 * Demonstrates usage of the smartterm_simple library.
 * This is a simple interactive shell that shows all library features.
 *
 * Build: make example
 * Run: make run
 */

#include "smartterm_simple.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // Initialize the library
    if (smartterm_init() != 0) {
        fprintf(stderr, "Failed to initialize SmartTerm\n");
        return 1;
    }

    // Display welcome message
    smartterm_add_output("SmartTerm Simple Library - Example", SMARTTERM_CTX_NORMAL);
    smartterm_add_output("Type 'help' for commands", SMARTTERM_CTX_NORMAL);
    smartterm_add_output("", SMARTTERM_CTX_NORMAL);

    // Set initial status
    smartterm_set_status("Ready", "v0.1.0");

    int running = 1;
    int cmd_count = 0;

    while (running) {
        // Read input from user
        char *input = smartterm_readline("> ");

        // Handle EOF (Ctrl+D)
        if (!input) {
            running = 0;
            break;
        }

        // Skip empty lines
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        // Detect context and get text without prefix
        smartterm_context_t ctx = smartterm_detect_context(input);
        const char *text = smartterm_strip_context(input);

        // Process commands
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            smartterm_add_output("Exiting...", SMARTTERM_CTX_NORMAL);
            running = 0;

        } else if (strcmp(input, "help") == 0) {
            smartterm_add_output("Available commands:", SMARTTERM_CTX_NORMAL);
            smartterm_add_output("  help    - Show this help message", SMARTTERM_CTX_NORMAL);
            smartterm_add_output("  clear   - Clear the output buffer", SMARTTERM_CTX_NORMAL);
            smartterm_add_output("  quit    - Exit the program", SMARTTERM_CTX_NORMAL);
            smartterm_add_output("", SMARTTERM_CTX_NORMAL);
            smartterm_add_output("Context prefixes:", SMARTTERM_CTX_NORMAL);
            smartterm_add_output("  !text   - Command context (yellow)", SMARTTERM_CTX_COMMAND);
            smartterm_add_output("  #text   - Comment context (green)", SMARTTERM_CTX_COMMENT);
            smartterm_add_output("  @text   - Special action context (cyan)", SMARTTERM_CTX_SPECIAL);
            smartterm_add_output("  /text   - Search context (magenta)", SMARTTERM_CTX_SEARCH);

        } else if (strcmp(input, "clear") == 0) {
            smartterm_clear_output();
            smartterm_add_output("Output cleared", SMARTTERM_CTX_NORMAL);

        } else {
            // Echo back the input with its context
            char response[512];
            const char *ctx_name =
                (ctx == SMARTTERM_CTX_COMMAND) ? "COMMAND" :
                (ctx == SMARTTERM_CTX_COMMENT) ? "COMMENT" :
                (ctx == SMARTTERM_CTX_SPECIAL) ? "SPECIAL" :
                (ctx == SMARTTERM_CTX_SEARCH) ? "SEARCH" : "NORMAL";

            snprintf(response, sizeof(response), "Echo [%s]: %s", ctx_name, text);
            smartterm_add_output(response, ctx);
        }

        cmd_count++;

        // Update status bar
        char status_right[64];
        snprintf(status_right, sizeof(status_right), "Lines: %d", cmd_count);
        smartterm_set_status("SmartTerm Example", status_right);

        free(input);
    }

    // Cleanup
    smartterm_cleanup();

    printf("SmartTerm example exited cleanly.\n");
    return 0;
}
