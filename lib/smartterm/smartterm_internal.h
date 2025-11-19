/*
 * SmartTerm Library - Internal Header
 *
 * Internal structures and functions for SmartTerm library.
 * Not part of public API.
 */

#ifndef SMARTTERM_INTERNAL_H
#define SMARTTERM_INTERNAL_H

#include "../../include/smartterm.h"
#include <ncurses.h>
#include <pthread.h>
#include <time.h>

/* Maximum sizes */
#define MAX_LINE_LENGTH 4096
#define MAX_STATUS_TEXT 256
#define MAX_PROMPT_LENGTH 64
#define MAX_THEME_NAME 32

/* Output line structure */
typedef struct {
    char *text;
    smartterm_line_meta_t meta;
} output_line_t;

/* Output buffer structure */
typedef struct {
    output_line_t *lines;
    int count;
    int capacity;
    int scroll_offset;
    bool auto_scroll;
    pthread_mutex_t mutex;
} output_buffer_t;

/* Theme structure */
struct smartterm_theme {
    char name[MAX_THEME_NAME];
    int colors[CTX_USER_START][2];  /* [context][fg, bg] */
    int attributes[CTX_USER_START]; /* Text attributes */
    char *symbols[SYM_COUNT];        /* Symbol strings */
    bool is_builtin;                 /* Built-in theme flag */
};

/* Search state */
typedef struct {
    char *pattern;
    bool use_regex;
    smartterm_search_result_t *results;
    int result_count;
    int current_result;
} search_state_t;

/* Completion state */
typedef struct {
    smartterm_completer_fn completer;
    void *user_data;
} completion_state_t;

/* Key handler entry */
typedef struct {
    int key;
    smartterm_key_handler_fn handler;
    void *user_data;
} key_handler_entry_t;

/* SmartTerm context structure */
struct smartterm_ctx {
    /* Configuration */
    smartterm_config_t config;

    /* Output buffer */
    output_buffer_t buffer;

    /* ncurses windows */
    WINDOW *output_win;
    WINDOW *status_win;

    /* Status bar text */
    char status_left[MAX_STATUS_TEXT];
    char status_right[MAX_STATUS_TEXT];
    bool status_visible;

    /* Prompt */
    char prompt[MAX_PROMPT_LENGTH];

    /* Theme */
    const smartterm_theme *theme;
    bool owns_theme;

    /* Terminal size */
    int term_rows;
    int term_cols;

    /* Search state */
    search_state_t search;

    /* Completion */
    completion_state_t completion;

    /* Key handlers */
    key_handler_entry_t *key_handlers;
    int key_handler_count;
    int key_handler_capacity;

    /* Error state */
    int last_error;

    /* Flags */
    bool initialized;
    bool ncurses_active;
};

/*
 * Internal function prototypes
 */

/* Output buffer functions (smartterm_output.c) */
int output_buffer_init(output_buffer_t *buf, int capacity, bool thread_safe);
void output_buffer_cleanup(output_buffer_t *buf);
int output_buffer_add(output_buffer_t *buf, const char *text,
                      const smartterm_line_meta_t *meta);
void output_buffer_clear(output_buffer_t *buf);
const char* output_buffer_get_line(output_buffer_t *buf, int index);
int output_buffer_get_line_meta(output_buffer_t *buf, int index,
                                smartterm_line_meta_t *meta);

/* Rendering functions (smartterm_render.c) */
int render_output(smartterm_ctx *ctx);
int render_status(smartterm_ctx *ctx);
int render_all(smartterm_ctx *ctx);

/* Input functions (smartterm_input.c) */
char* input_read_line(smartterm_ctx *ctx, const char *prompt);
char* input_read_multiline(smartterm_ctx *ctx, const char *prompt);
int input_suspend_ncurses(smartterm_ctx *ctx);
int input_resume_ncurses(smartterm_ctx *ctx);

/* Theme functions (smartterm_theme.c) */
const smartterm_theme* theme_get_default(void);
int theme_apply_colors(smartterm_ctx *ctx);

/* Utility functions */
long get_timestamp(void);
char* strdup_safe(const char *s);

#endif /* SMARTTERM_INTERNAL_H */
