/*
 * SmartTerm Example: REPL (Read-Eval-Print Loop)
 *
 * Demonstrates a simple calculator REPL using SmartTerm.
 */

#include <math.h>
#include <smartterm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Simple expression evaluator */
static double evaluate(const char* expr)
{
    /* Simple calculator: supports +, -, *, /, and parentheses */
    /* For demo purposes, just use sscanf for simple operations */
    double a, b;
    char op;

    if (sscanf(expr, "%lf %c %lf", &a, &op, &b) == 3) {
        switch (op) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            return (b != 0) ? a / b : NAN;
        default:
            return NAN;
        }
    }

    /* Try single number */
    if (sscanf(expr, "%lf", &a) == 1) {
        return a;
    }

    return NAN;
}

/* Tab completion for REPL commands */
static char** repl_completer(const char* text, int start, int end, void* data)
{
    (void)start;
    (void)end;
    (void)data;

    static const char* commands[] = {"help", "quit", "exit", "clear", "history", "export", NULL};

    /* Count matches */
    int match_count = 0;
    for (int i = 0; commands[i]; i++) {
        if (strncmp(text, commands[i], strlen(text)) == 0) {
            match_count++;
        }
    }

    if (match_count == 0) {
        return NULL;
    }

    /* Allocate result array */
    char** matches = calloc(match_count + 1, sizeof(char*));
    int match_idx = 0;

    for (int i = 0; commands[i]; i++) {
        if (strncmp(text, commands[i], strlen(text)) == 0) {
            matches[match_idx++] = strdup(commands[i]);
        }
    }

    return matches;
}

int main(void)
{
    /* Initialize SmartTerm */
    smartterm_config_t config = smartterm_default_config();
    config.history_enabled = true;
    config.prompt = "calc> ";

    smartterm_ctx* ctx = smartterm_init(&config);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize SmartTerm\n");
        return 1;
    }

    /* Set tab completion */
    smartterm_set_completer(ctx, repl_completer, NULL);

    /* Welcome message */
    smartterm_write(ctx, "SmartTerm Calculator REPL", CTX_INFO);
    smartterm_write(ctx, "Type expressions like: 5 + 3, 10 * 2, etc.", CTX_NORMAL);
    smartterm_write(ctx, "Commands: help, clear, quit", CTX_NORMAL);
    smartterm_write(ctx, "", CTX_NORMAL);

    /* Set status bar */
    smartterm_status_set(ctx, "Calculator REPL", "Ready");

    int expr_count = 0;
    bool running = true;

    while (running) {
        char* input = smartterm_read_line(ctx, NULL);

        if (!input) {
            /* EOF or error */
            break;
        }

        /* Skip empty input */
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        /* Handle commands */
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            smartterm_write(ctx, "Goodbye!", CTX_SUCCESS);
            running = false;
        } else if (strcmp(input, "help") == 0) {
            smartterm_write(ctx, "Calculator REPL Help:", CTX_INFO);
            smartterm_write(ctx, "  Enter expressions: 5 + 3, 10 * 2, 15 / 3", CTX_NORMAL);
            smartterm_write(ctx, "  Operators: + - * /", CTX_NORMAL);
            smartterm_write(ctx, "  Commands:", CTX_NORMAL);
            smartterm_write(ctx, "    help    - Show this help", CTX_NORMAL);
            smartterm_write(ctx, "    clear   - Clear screen", CTX_NORMAL);
            smartterm_write(ctx, "    history - Show command history", CTX_NORMAL);
            smartterm_write(ctx, "    export  - Export output to file", CTX_NORMAL);
            smartterm_write(ctx, "    quit    - Exit REPL", CTX_NORMAL);
        } else if (strcmp(input, "clear") == 0) {
            smartterm_clear(ctx);
        } else if (strcmp(input, "history") == 0) {
            smartterm_write_fmt(ctx, CTX_INFO, "Total expressions evaluated: %d", expr_count);
        } else if (strcmp(input, "export") == 0) {
            int result = smartterm_export(ctx, "repl_output.txt", EXPORT_PLAIN, 0, -1, true);
            if (result == SMARTTERM_OK) {
                smartterm_write(ctx, "Exported to repl_output.txt", CTX_SUCCESS);
            } else {
                smartterm_write(ctx, "Export failed", CTX_ERROR);
            }
        } else {
            /* Evaluate expression */
            double result = evaluate(input);

            if (isnan(result)) {
                smartterm_write_fmt(ctx, CTX_ERROR, "Error: Invalid expression: %s", input);
            } else {
                smartterm_write_fmt(ctx, CTX_SUCCESS, "= %.6g", result);
                expr_count++;
            }
        }

        /* Update status bar */
        char status[64];
        snprintf(status, sizeof(status), "Evaluated: %d", expr_count);
        smartterm_status_set(ctx, "Calculator REPL", status);

        free(input);
    }

    /* Cleanup */
    smartterm_cleanup(ctx);

    printf("Calculator REPL exited.\n");
    return 0;
}
