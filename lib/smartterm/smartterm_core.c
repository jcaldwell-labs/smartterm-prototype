/*
 * SmartTerm Library - Core Implementation
 *
 * Initialization, cleanup, and core functions.
 */

#include "smartterm_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Library version */
#define SMARTTERM_VERSION "1.0.0"

/*
 * Get default configuration
 */
smartterm_config_t smartterm_default_config(void) {
    smartterm_config_t config = {
        .max_lines = 1000,
        .output_height = 0,  /* Auto */
        .status_bar_enabled = true,
        .prompt = "> ",
        .history_enabled = true,
        .history_file = NULL,
        .history_size = 1000,
        .theme = NULL,  /* Default theme */
        .multiline_enabled = false,
        .thread_safe = true
    };
    return config;
}

/*
 * Initialize ncurses
 */
static int init_ncurses(smartterm_ctx *ctx) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    /* Initialize colors if supported */
    if (has_colors()) {
        start_color();
        use_default_colors();
    }

    /* Get terminal size */
    getmaxyx(stdscr, ctx->term_rows, ctx->term_cols);

    /* Calculate window heights */
    int status_height = ctx->config.status_bar_enabled ? 1 : 0;
    int input_height = 2;  /* Space for readline */
    int output_height = ctx->term_rows - status_height - input_height;

    if (ctx->config.output_height > 0 &&
        ctx->config.output_height < output_height) {
        output_height = ctx->config.output_height;
    }

    /* Create windows */
    ctx->output_win = newwin(output_height, ctx->term_cols, 0, 0);
    if (!ctx->output_win) {
        endwin();
        return SMARTTERM_ERROR;
    }
    scrollok(ctx->output_win, TRUE);

    if (ctx->config.status_bar_enabled) {
        ctx->status_win = newwin(1, ctx->term_cols, output_height, 0);
        if (!ctx->status_win) {
            delwin(ctx->output_win);
            endwin();
            return SMARTTERM_ERROR;
        }
    }

    refresh();
    ctx->ncurses_active = true;

    return SMARTTERM_OK;
}

/*
 * Initialize SmartTerm
 */
smartterm_ctx* smartterm_init(const smartterm_config_t *config) {
    smartterm_ctx *ctx = calloc(1, sizeof(smartterm_ctx));
    if (!ctx) {
        return NULL;
    }

    /* Set configuration */
    if (config) {
        ctx->config = *config;
    } else {
        ctx->config = smartterm_default_config();
    }

    /* Initialize output buffer */
    if (output_buffer_init(&ctx->buffer, ctx->config.max_lines,
                          ctx->config.thread_safe) != SMARTTERM_OK) {
        free(ctx);
        return NULL;
    }

    /* Initialize ncurses */
    if (init_ncurses(ctx) != SMARTTERM_OK) {
        output_buffer_cleanup(&ctx->buffer);
        free(ctx);
        return NULL;
    }

    /* Set theme */
    if (ctx->config.theme) {
        ctx->theme = ctx->config.theme;
        ctx->owns_theme = false;
    } else {
        ctx->theme = theme_get_default();
        ctx->owns_theme = false;
    }
    theme_apply_colors(ctx);

    /* Set prompt */
    if (ctx->config.prompt) {
        strncpy(ctx->prompt, ctx->config.prompt, MAX_PROMPT_LENGTH - 1);
        ctx->prompt[MAX_PROMPT_LENGTH - 1] = '\0';
    } else {
        strcpy(ctx->prompt, "> ");
    }

    /* Initialize status bar */
    ctx->status_visible = ctx->config.status_bar_enabled;
    strcpy(ctx->status_left, "SmartTerm");
    snprintf(ctx->status_right, MAX_STATUS_TEXT, "v%s", SMARTTERM_VERSION);

    /* Initialize search */
    ctx->search.pattern = NULL;
    ctx->search.results = NULL;
    ctx->search.result_count = 0;
    ctx->search.current_result = -1;

    /* Initialize key handlers */
    ctx->key_handler_capacity = 10;
    ctx->key_handlers = calloc(ctx->key_handler_capacity,
                               sizeof(key_handler_entry_t));
    ctx->key_handler_count = 0;

    ctx->initialized = true;
    ctx->last_error = SMARTTERM_OK;

    /* Initial render */
    render_all(ctx);

    return ctx;
}

/*
 * Cleanup SmartTerm
 */
void smartterm_cleanup(smartterm_ctx *ctx) {
    if (!ctx) {
        return;
    }

    if (!ctx->initialized) {
        free(ctx);
        return;
    }

    /* Cleanup search */
    free(ctx->search.pattern);
    free(ctx->search.results);

    /* Cleanup key handlers */
    free(ctx->key_handlers);

    /* Cleanup theme if owned */
    if (ctx->owns_theme && ctx->theme) {
        smartterm_theme_free((smartterm_theme*)ctx->theme);
    }

    /* Cleanup ncurses */
    if (ctx->ncurses_active) {
        if (ctx->output_win) {
            delwin(ctx->output_win);
        }
        if (ctx->status_win) {
            delwin(ctx->status_win);
        }
        endwin();
    }

    /* Cleanup output buffer */
    output_buffer_cleanup(&ctx->buffer);

    ctx->initialized = false;
    free(ctx);
}

/*
 * Clear output buffer
 */
int smartterm_clear(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    output_buffer_clear(&ctx->buffer);
    return render_output(ctx);
}

/*
 * Force re-render
 */
int smartterm_render(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    return render_all(ctx);
}

/*
 * Get library version
 */
const char* smartterm_version(void) {
    return SMARTTERM_VERSION;
}

/*
 * Get error string
 */
const char* smartterm_error_string(int code) {
    switch (code) {
        case SMARTTERM_OK:
            return "Success";
        case SMARTTERM_ERROR:
            return "Generic error";
        case SMARTTERM_NOMEM:
            return "Out of memory";
        case SMARTTERM_INVALID:
            return "Invalid argument";
        case SMARTTERM_NOTINIT:
            return "Not initialized";
        case SMARTTERM_IOERROR:
            return "I/O error";
        default:
            return "Unknown error";
    }
}

/*
 * Get last error
 */
int smartterm_get_last_error(smartterm_ctx *ctx) {
    if (!ctx) {
        return SMARTTERM_NOTINIT;
    }
    return ctx->last_error;
}

/*
 * Get line count
 */
int smartterm_get_line_count(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return 0;
    }
    return ctx->buffer.count;
}

/*
 * Get line from buffer
 */
const char* smartterm_get_line(smartterm_ctx *ctx, int index) {
    if (!ctx || !ctx->initialized) {
        return NULL;
    }
    return output_buffer_get_line(&ctx->buffer, index);
}

/*
 * Get line metadata
 */
int smartterm_get_line_meta(smartterm_ctx *ctx, int index,
                            smartterm_line_meta_t *meta) {
    if (!ctx || !ctx->initialized || !meta) {
        return SMARTTERM_INVALID;
    }
    return output_buffer_get_line_meta(&ctx->buffer, index, meta);
}

/*
 * Get terminal size
 */
int smartterm_get_terminal_size(smartterm_ctx *ctx, int *rows, int *cols) {
    if (!ctx || !ctx->initialized || !rows || !cols) {
        return SMARTTERM_INVALID;
    }
    *rows = ctx->term_rows;
    *cols = ctx->term_cols;
    return SMARTTERM_OK;
}

/*
 * Handle terminal resize
 */
int smartterm_handle_resize(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    /* Get new terminal size */
    endwin();
    refresh();
    getmaxyx(stdscr, ctx->term_rows, ctx->term_cols);

    /* Resize windows */
    int status_height = ctx->status_visible ? 1 : 0;
    int input_height = 2;
    int output_height = ctx->term_rows - status_height - input_height;

    wresize(ctx->output_win, output_height, ctx->term_cols);
    if (ctx->status_win) {
        mvwin(ctx->status_win, output_height, 0);
        wresize(ctx->status_win, 1, ctx->term_cols);
    }

    /* Re-render */
    return render_all(ctx);
}

/*
 * Utility: Get current timestamp
 */
long get_timestamp(void) {
    return time(NULL);
}

/*
 * Utility: Safe string duplication
 */
char* strdup_safe(const char *s) {
    if (!s) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char *copy = malloc(len);
    if (copy) {
        memcpy(copy, s, len);
    }
    return copy;
}
