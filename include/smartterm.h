/*
 * SmartTerm Library - Public API
 *
 * A C library for terminal UIs with:
 * - Scrolling output buffer (top region)
 * - Input area with readline support (bottom)
 * - Status bar (separator)
 * - Context-aware coloring
 * - Thread-safe operations
 *
 * Version: 1.0.0
 * License: MIT
 */

#ifndef SMARTTERM_H
#define SMARTTERM_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * ============================================================================
 * TYPES AND ENUMERATIONS
 * ============================================================================
 */

/* Opaque handle to SmartTerm context */
typedef struct smartterm_ctx smartterm_ctx;

/* Opaque handle to theme */
typedef struct smartterm_theme smartterm_theme;

/* Context types for output coloring */
typedef enum {
    CTX_NORMAL = 0,     /* Default text */
    CTX_ERROR,          /* Error messages (red) */
    CTX_WARNING,        /* Warnings (yellow) */
    CTX_SUCCESS,        /* Success messages (green) */
    CTX_INFO,           /* Info messages (cyan) */
    CTX_DEBUG,          /* Debug messages (magenta) */
    CTX_COMMAND,        /* Command input echo (yellow) */
    CTX_COMMENT,        /* Comments (green) */
    CTX_SPECIAL,        /* Special actions (cyan) */
    CTX_SEARCH,         /* Search context (magenta) */
    CTX_USER_START = 100 /* User-defined contexts start here */
} smartterm_context_t;

/* Return codes */
typedef enum {
    SMARTTERM_OK = 0,          /* Success */
    SMARTTERM_ERROR = -1,      /* Generic error */
    SMARTTERM_NOMEM = -2,      /* Out of memory */
    SMARTTERM_INVALID = -3,    /* Invalid argument */
    SMARTTERM_NOTINIT = -4,    /* Not initialized */
    SMARTTERM_IOERROR = -5     /* I/O error */
} smartterm_error_t;

/* Configuration options */
typedef struct {
    int max_lines;              /* Maximum output lines (default: 1000) */
    int output_height;          /* Output window height (0 = auto) */
    bool status_bar_enabled;    /* Show status bar (default: true) */
    const char *prompt;         /* Default prompt (default: "> ") */
    bool history_enabled;       /* Enable readline history (default: true) */
    const char *history_file;   /* History file path (NULL = no file) */
    int history_size;           /* Max history entries (default: 1000) */
    smartterm_theme *theme;     /* Theme (NULL = default) */
    bool multiline_enabled;     /* Enable multi-line input (default: false) */
    bool thread_safe;           /* Enable thread safety (default: true) */
} smartterm_config_t;

/* Output line metadata */
typedef struct {
    smartterm_context_t context;  /* Context type */
    long timestamp;               /* Unix timestamp */
    const char *tag;              /* Optional tag */
} smartterm_line_meta_t;

/* Search results */
typedef struct {
    int line_index;             /* Line number in buffer */
    int column;                 /* Column where match starts */
    int length;                 /* Length of match */
} smartterm_search_result_t;

/* Theme symbols */
typedef enum {
    SYM_PROMPT,                 /* Input prompt */
    SYM_MULTILINE_PROMPT,       /* Multi-line prompt continuation */
    SYM_STATUS_SEP,             /* Status bar separator */
    SYM_SCROLL_INDICATOR,       /* Scrollback indicator */
    SYM_SEARCH_MATCH,           /* Search match marker */
    SYM_COUNT                   /* Total number of symbols (keep last) */
} smartterm_symbol_t;

/* Export formats */
typedef enum {
    EXPORT_PLAIN,               /* Plain text */
    EXPORT_ANSI,                /* With ANSI color codes */
    EXPORT_MARKDOWN,            /* Markdown format */
    EXPORT_HTML                 /* HTML format */
} smartterm_export_format_t;

/*
 * ============================================================================
 * INITIALIZATION AND CLEANUP
 * ============================================================================
 */

/*
 * Initialize SmartTerm with configuration.
 *
 * config: Configuration options (NULL for defaults)
 * Returns: Context handle, or NULL on failure
 *
 * Note: Must be called before any other smartterm functions.
 *       Call smartterm_cleanup() when done.
 */
smartterm_ctx* smartterm_init(const smartterm_config_t *config);

/*
 * Cleanup and free SmartTerm resources.
 *
 * ctx: Context handle
 *
 * Note: Safe to call multiple times.
 *       Restores terminal to original state.
 */
void smartterm_cleanup(smartterm_ctx *ctx);

/*
 * Get default configuration.
 *
 * Returns: Configuration with default values
 */
smartterm_config_t smartterm_default_config(void);

/*
 * ============================================================================
 * OUTPUT FUNCTIONS
 * ============================================================================
 */

/*
 * Write a line to output buffer.
 *
 * ctx: Context handle
 * text: Text to write (null-terminated)
 * context: Context type for coloring
 * Returns: SMARTTERM_OK on success, error code on failure
 *
 * Note: Thread-safe. Automatically triggers render.
 */
int smartterm_write(smartterm_ctx *ctx, const char *text,
                    smartterm_context_t context);

/*
 * Write formatted output (printf-style).
 *
 * ctx: Context handle
 * context: Context type for coloring
 * format: Printf-style format string
 * ...: Variable arguments
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_write_fmt(smartterm_ctx *ctx, smartterm_context_t context,
                        const char *format, ...);

/*
 * Write with metadata.
 *
 * ctx: Context handle
 * text: Text to write
 * meta: Metadata for line
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_write_meta(smartterm_ctx *ctx, const char *text,
                         const smartterm_line_meta_t *meta);

/*
 * Clear output buffer.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_clear(smartterm_ctx *ctx);

/*
 * Force re-render of display.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code on failure
 *
 * Note: Usually not needed (auto-renders on write).
 *       Useful after terminal resize.
 */
int smartterm_render(smartterm_ctx *ctx);

/*
 * ============================================================================
 * INPUT FUNCTIONS
 * ============================================================================
 */

/*
 * Read a line of input from user.
 *
 * ctx: Context handle
 * prompt: Prompt string (NULL = use default)
 * Returns: Allocated string with input, or NULL on EOF/error
 *
 * Note: Caller must free() returned string.
 *       Not thread-safe (call from main thread only).
 *       Adds to history if enabled.
 */
char* smartterm_read_line(smartterm_ctx *ctx, const char *prompt);

/*
 * Read multi-line input from user.
 *
 * ctx: Context handle
 * prompt: Prompt string
 * Returns: Allocated string with input, or NULL on EOF/error
 *
 * Note: Caller must free() returned string.
 *       Submit with double-newline or Ctrl+D.
 *       Requires multiline_enabled in config.
 */
char* smartterm_read_multiline(smartterm_ctx *ctx, const char *prompt);

/*
 * Set default prompt.
 *
 * ctx: Context handle
 * prompt: New prompt string
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_set_prompt(smartterm_ctx *ctx, const char *prompt);

/*
 * ============================================================================
 * STATUS BAR
 * ============================================================================
 */

/*
 * Set status bar text.
 *
 * ctx: Context handle
 * left: Left-aligned text (NULL = no change)
 * right: Right-aligned text (NULL = no change)
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_status_set(smartterm_ctx *ctx, const char *left,
                         const char *right);

/*
 * Update status bar with formatted text.
 *
 * ctx: Context handle
 * left_fmt: Printf-style format for left side (NULL = no change)
 * right_fmt: Printf-style format for right side (NULL = no change)
 * ...: Variable arguments
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_status_update(smartterm_ctx *ctx, const char *left_fmt,
                            const char *right_fmt, ...);

/*
 * Show/hide status bar.
 *
 * ctx: Context handle
 * visible: true to show, false to hide
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_status_set_visible(smartterm_ctx *ctx, bool visible);

/*
 * ============================================================================
 * SCROLLBACK AND NAVIGATION
 * ============================================================================
 */

/*
 * Scroll output buffer.
 *
 * ctx: Context handle
 * lines: Number of lines to scroll (positive = up, negative = down)
 * Returns: SMARTTERM_OK on success, error code on failure
 *
 * Note: Automatically updates scroll indicator in status bar.
 */
int smartterm_scroll(smartterm_ctx *ctx, int lines);

/*
 * Scroll to top of buffer.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_scroll_top(smartterm_ctx *ctx);

/*
 * Scroll to bottom of buffer (most recent).
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_scroll_bottom(smartterm_ctx *ctx);

/*
 * Get current scroll position.
 *
 * ctx: Context handle
 * Returns: Line index of top visible line
 */
int smartterm_get_scroll_pos(smartterm_ctx *ctx);

/*
 * Enable/disable auto-scroll on new output.
 *
 * ctx: Context handle
 * enabled: true to auto-scroll, false to stay at current position
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_set_auto_scroll(smartterm_ctx *ctx, bool enabled);

/*
 * ============================================================================
 * SEARCH
 * ============================================================================
 */

/*
 * Search in output buffer.
 *
 * ctx: Context handle
 * pattern: Search pattern (plain text or regex)
 * use_regex: true for regex, false for plain text
 * results: Output array of search results (allocated by function)
 * count: Output number of results
 * Returns: SMARTTERM_OK on success, error code on failure
 *
 * Note: Caller must free results array with smartterm_free_search_results().
 */
int smartterm_search(smartterm_ctx *ctx, const char *pattern, bool use_regex,
                     smartterm_search_result_t **results, int *count);

/*
 * Free search results.
 *
 * results: Results array from smartterm_search()
 */
void smartterm_free_search_results(smartterm_search_result_t *results);

/*
 * Jump to next search match.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code if no more matches
 */
int smartterm_search_next(smartterm_ctx *ctx);

/*
 * Jump to previous search match.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code if no more matches
 */
int smartterm_search_prev(smartterm_ctx *ctx);

/*
 * Clear search highlights.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_search_clear(smartterm_ctx *ctx);

/*
 * ============================================================================
 * EXPORT
 * ============================================================================
 */

/*
 * Export output buffer to file.
 *
 * ctx: Context handle
 * filename: Output file path
 * format: Export format
 * start_line: First line to export (0 = first)
 * end_line: Last line to export (-1 = last)
 * include_meta: Include metadata (timestamps, context)
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_export(smartterm_ctx *ctx, const char *filename,
                     smartterm_export_format_t format, int start_line,
                     int end_line, bool include_meta);

/*
 * Export to string.
 *
 * ctx: Context handle
 * format: Export format
 * start_line: First line to export (0 = first)
 * end_line: Last line to export (-1 = last)
 * include_meta: Include metadata
 * Returns: Allocated string with exported content, or NULL on error
 *
 * Note: Caller must free() returned string.
 */
char* smartterm_export_string(smartterm_ctx *ctx,
                              smartterm_export_format_t format,
                              int start_line, int end_line, bool include_meta);

/*
 * ============================================================================
 * TAB COMPLETION
 * ============================================================================
 */

/*
 * Completion callback function type.
 *
 * text: Text to complete
 * start: Start position in line buffer
 * end: End position in line buffer
 * data: User data passed to smartterm_set_completer()
 * Returns: NULL-terminated array of completion candidates (caller frees)
 */
typedef char** (*smartterm_completer_fn)(const char *text, int start,
                                         int end, void *data);

/*
 * Set tab completion callback.
 *
 * ctx: Context handle
 * completer: Completion callback function
 * data: User data passed to callback
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_set_completer(smartterm_ctx *ctx, smartterm_completer_fn completer,
                            void *data);

/*
 * ============================================================================
 * THEMES
 * ============================================================================
 */

/*
 * Create new theme.
 *
 * name: Theme name
 * Returns: Theme handle, or NULL on failure
 */
smartterm_theme* smartterm_theme_create(const char *name);

/*
 * Free theme.
 *
 * theme: Theme handle
 */
void smartterm_theme_free(smartterm_theme *theme);

/*
 * Set color for context.
 *
 * theme: Theme handle
 * context: Context type
 * fg: Foreground color (0-255)
 * bg: Background color (0-255)
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_theme_set_color(smartterm_theme *theme,
                              smartterm_context_t context,
                              int fg, int bg);

/*
 * Set symbol.
 *
 * theme: Theme handle
 * symbol: Symbol type
 * value: Symbol string
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_theme_set_symbol(smartterm_theme *theme,
                               smartterm_symbol_t symbol,
                               const char *value);

/*
 * Set text attribute for context.
 *
 * theme: Theme handle
 * context: Context type
 * attribute: ncurses attribute (A_BOLD, A_DIM, etc.)
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_theme_set_attribute(smartterm_theme *theme,
                                  smartterm_context_t context,
                                  int attribute);

/*
 * Get built-in theme.
 *
 * name: Theme name ("default", "dark", "light", "solarized", "nord")
 * Returns: Theme handle, or NULL if not found
 *
 * Note: Do not free built-in themes.
 */
const smartterm_theme* smartterm_theme_get_builtin(const char *name);

/*
 * Set active theme.
 *
 * ctx: Context handle
 * theme: Theme handle (NULL = default)
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_set_theme(smartterm_ctx *ctx, const smartterm_theme *theme);

/*
 * ============================================================================
 * KEY HANDLERS
 * ============================================================================
 */

/*
 * Key handler callback function type.
 *
 * ctx: Context handle
 * key: Key code (from ncurses)
 * data: User data passed to smartterm_register_key_handler()
 */
typedef void (*smartterm_key_handler_fn)(smartterm_ctx *ctx, int key,
                                         void *data);

/*
 * Register key handler.
 *
 * ctx: Context handle
 * key: Key code to handle (from ncurses KEY_* constants)
 * handler: Handler callback
 * data: User data passed to callback
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_register_key_handler(smartterm_ctx *ctx, int key,
                                   smartterm_key_handler_fn handler,
                                   void *data);

/*
 * Unregister key handler.
 *
 * ctx: Context handle
 * key: Key code
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_unregister_key_handler(smartterm_ctx *ctx, int key);

/*
 * ============================================================================
 * ERROR HANDLING
 * ============================================================================
 */

/*
 * Get error string for error code.
 *
 * code: Error code
 * Returns: Human-readable error string
 */
const char* smartterm_error_string(int code);

/*
 * Get last error code.
 *
 * ctx: Context handle
 * Returns: Last error code
 */
int smartterm_get_last_error(smartterm_ctx *ctx);

/*
 * ============================================================================
 * UTILITY FUNCTIONS
 * ============================================================================
 */

/*
 * Get library version.
 *
 * Returns: Version string (e.g., "1.0.0")
 */
const char* smartterm_version(void);

/*
 * Get output buffer line count.
 *
 * ctx: Context handle
 * Returns: Number of lines in buffer
 */
int smartterm_get_line_count(smartterm_ctx *ctx);

/*
 * Get line from buffer.
 *
 * ctx: Context handle
 * index: Line index (0 = oldest)
 * Returns: Line text, or NULL if invalid index
 *
 * Note: Returned string is valid until next write/clear operation.
 *       Do not free.
 */
const char* smartterm_get_line(smartterm_ctx *ctx, int index);

/*
 * Get line metadata.
 *
 * ctx: Context handle
 * index: Line index
 * meta: Output metadata structure
 * Returns: SMARTTERM_OK on success, error code if invalid index
 */
int smartterm_get_line_meta(smartterm_ctx *ctx, int index,
                            smartterm_line_meta_t *meta);

/*
 * Get terminal size.
 *
 * ctx: Context handle
 * rows: Output rows
 * cols: Output columns
 * Returns: SMARTTERM_OK on success, error code on failure
 */
int smartterm_get_terminal_size(smartterm_ctx *ctx, int *rows, int *cols);

/*
 * Handle terminal resize.
 *
 * ctx: Context handle
 * Returns: SMARTTERM_OK on success, error code on failure
 *
 * Note: Usually called from SIGWINCH handler.
 */
int smartterm_handle_resize(smartterm_ctx *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SMARTTERM_H */
