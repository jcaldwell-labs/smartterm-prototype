/*
 * SmartTerm Library - Input Implementation
 *
 * Input handling with readline integration.
 */

#include "smartterm_internal.h"
#include <readline/history.h>
#include <readline/readline.h>
#include <stdlib.h>
#include <string.h>

/* Global completion state for readline callbacks */
static smartterm_ctx* g_completion_ctx = NULL;

/*
 * Suspend ncurses for readline
 */
int input_suspend_ncurses(smartterm_ctx* ctx)
{
    if (!ctx || !ctx->ncurses_active) {
        return SMARTTERM_NOTINIT;
    }

    def_prog_mode(); /* Save current mode */
    endwin();        /* Suspend ncurses */

    return SMARTTERM_OK;
}

/*
 * Resume ncurses after readline
 */
int input_resume_ncurses(smartterm_ctx* ctx)
{
    if (!ctx) {
        return SMARTTERM_NOTINIT;
    }

    reset_prog_mode(); /* Restore saved mode */
    refresh();         /* Refresh screen */

    /* Re-render windows */
    return render_all(ctx);
}

/*
 * Readline completion wrapper
 */
static char** readline_completion_wrapper(const char* text, int start, int end)
{
    if (!g_completion_ctx || !g_completion_ctx->completion.completer) {
        return NULL;
    }

    return g_completion_ctx->completion.completer(text, start, end,
                                                  g_completion_ctx->completion.user_data);
}

/*
 * Read single line of input
 */
char* input_read_line(smartterm_ctx* ctx, const char* prompt)
{
    if (!ctx || !ctx->initialized) {
        return NULL;
    }

    /* Set up completion if configured */
    if (ctx->completion.completer) {
        g_completion_ctx = ctx;
        rl_attempted_completion_function = readline_completion_wrapper;
    }

    /* Suspend ncurses */
    input_suspend_ncurses(ctx);

    /* Use readline */
    const char* actual_prompt = prompt ? prompt : ctx->prompt;
    char* input = readline(actual_prompt);

    /* Resume ncurses */
    input_resume_ncurses(ctx);

    /* Add to history if enabled and non-empty */
    if (input && ctx->config.history_enabled && strlen(input) > 0) {
        add_history(input);
    }

    return input;
}

/*
 * Read multi-line input (simplified version)
 */
char* input_read_multiline(smartterm_ctx* ctx, const char* prompt)
{
    if (!ctx || !ctx->initialized) {
        return NULL;
    }

    if (!ctx->config.multiline_enabled) {
        return input_read_line(ctx, prompt);
    }

    /* For now, fall back to single-line
     * Full multi-line implementation would require custom input handler
     */
    return input_read_line(ctx, prompt);
}

/*
 * Read line - public API
 */
char* smartterm_read_line(smartterm_ctx* ctx, const char* prompt)
{
    if (!ctx || !ctx->initialized) {
        return NULL;
    }

    return input_read_line(ctx, prompt);
}

/*
 * Read multi-line - public API
 */
char* smartterm_read_multiline(smartterm_ctx* ctx, const char* prompt)
{
    if (!ctx || !ctx->initialized) {
        return NULL;
    }

    return input_read_multiline(ctx, prompt);
}

/*
 * Set prompt
 */
int smartterm_set_prompt(smartterm_ctx* ctx, const char* prompt)
{
    if (!ctx || !ctx->initialized || !prompt) {
        return SMARTTERM_INVALID;
    }

    strncpy(ctx->prompt, prompt, MAX_PROMPT_LENGTH - 1);
    ctx->prompt[MAX_PROMPT_LENGTH - 1] = '\0';

    return SMARTTERM_OK;
}

/*
 * Set tab completer
 */
int smartterm_set_completer(smartterm_ctx* ctx, smartterm_completer_fn completer, void* data)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    ctx->completion.completer = completer;
    ctx->completion.user_data = data;

    return SMARTTERM_OK;
}
