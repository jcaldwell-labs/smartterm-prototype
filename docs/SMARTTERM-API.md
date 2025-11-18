# SmartTerm API Documentation

**Version:** 1.0.0
**License:** MIT

A comprehensive C library for building terminal UIs with scrolling output buffers, interactive input handling, status bars, context-aware coloring, and thread-safe operations.

---

## Table of Contents

1. [Overview](#overview)
2. [Quick Start Guide](#quick-start-guide)
3. [Core Concepts](#core-concepts)
4. [Complete API Reference](#complete-api-reference)
5. [Code Examples](#code-examples)
6. [Integration Guide](#integration-guide)
7. [Best Practices](#best-practices)
8. [Troubleshooting](#troubleshooting)

---

## Overview

SmartTerm is a C library that simplifies building sophisticated terminal user interfaces. It provides:

- **Scrolling Output Buffer**: A top region with unlimited scrollback capability
- **Input Area**: Bottom region with readline support, history, and completion
- **Status Bar**: Configurable separator bar for displaying application state
- **Context-Aware Coloring**: Automatic color highlighting based on message type (error, warning, success, etc.)
- **Thread-Safe Operations**: Safe to use from multiple threads (with proper configuration)
- **Rich Features**: Search, export, theming, key handling, and more

### Key Features

- **Multiple Output Contexts**: Normal, Error, Warning, Success, Info, Debug, Command, Comment, Special, Search
- **Built-in Themes**: Default, dark, light, solarized, and nord
- **Flexible Export**: Save content as plain text, ANSI colors, Markdown, or HTML
- **History Management**: Readline history with persistent file storage
- **Tab Completion**: Custom completion callbacks
- **Search Functionality**: Plain text and regex search with navigation
- **Terminal Resize Handling**: Automatic detection and adaptation to terminal size changes

---

## Quick Start Guide

### 1. Basic Initialization

```c
#include "smartterm.h"
#include <stdio.h>

int main(void) {
    // Initialize with default configuration
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (ctx == NULL) {
        fprintf(stderr, "Failed to initialize SmartTerm\n");
        return 1;
    }

    // Use SmartTerm...

    // Cleanup
    smartterm_cleanup(ctx);
    return 0;
}
```

### 2. Write Output

```c
// Write a normal message
smartterm_write(ctx, "Hello, World!", CTX_NORMAL);

// Write an error (red)
smartterm_write(ctx, "An error occurred", CTX_ERROR);

// Write success (green)
smartterm_write(ctx, "Operation completed successfully", CTX_SUCCESS);
```

### 3. Read User Input

```c
char *input = smartterm_read_line(ctx, "Enter command: ");
if (input != NULL) {
    printf("User entered: %s\n", input);
    free(input);
}
```

### 4. Update Status Bar

```c
smartterm_status_set(ctx, "Status: Ready", "Lines: 42");
```

### 5. Interactive Loop Example

```c
int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (!ctx) return 1;

    smartterm_status_set(ctx, "SmartTerm Shell", "Ready");

    while (1) {
        char *input = smartterm_read_line(ctx, "$ ");

        if (input == NULL) {
            smartterm_write(ctx, "Goodbye!", CTX_NORMAL);
            break;
        }

        if (strlen(input) > 0) {
            smartterm_write_fmt(ctx, CTX_COMMAND, "$ %s", input);
            // Process input...
            smartterm_write(ctx, "Command executed", CTX_SUCCESS);
        }

        smartterm_status_update(ctx, NULL, "Lines: %d",
                               smartterm_get_line_count(ctx));

        free(input);
    }

    smartterm_cleanup(ctx);
    return 0;
}
```

---

## Core Concepts

### Types and Enumerations

#### smartterm_context_t

Context types determine the color and style of output:

| Context | Color | Use Case |
|---------|-------|----------|
| `CTX_NORMAL` | Default | Regular messages |
| `CTX_ERROR` | Red | Error messages |
| `CTX_WARNING` | Yellow | Warnings |
| `CTX_SUCCESS` | Green | Success messages |
| `CTX_INFO` | Cyan | Information |
| `CTX_DEBUG` | Magenta | Debug output |
| `CTX_COMMAND` | Yellow | User command echo |
| `CTX_COMMENT` | Green | Comments |
| `CTX_SPECIAL` | Cyan | Special actions |
| `CTX_SEARCH` | Magenta | Search highlighting |
| `CTX_USER_START` | Custom | User-defined (100+) |

#### smartterm_error_t

Return codes for error handling:

```c
SMARTTERM_OK       // Success (0)
SMARTTERM_ERROR    // Generic error (-1)
SMARTTERM_NOMEM    // Out of memory (-2)
SMARTTERM_INVALID  // Invalid argument (-3)
SMARTTERM_NOTINIT  // Not initialized (-4)
SMARTTERM_IOERROR  // I/O error (-5)
```

#### smartterm_config_t

Configuration structure for initialization:

```c
typedef struct {
    int max_lines;              // Max output lines (default: 1000)
    int output_height;          // Output window height (0 = auto)
    bool status_bar_enabled;    // Show status bar (default: true)
    const char *prompt;         // Default prompt (default: "> ")
    bool history_enabled;       // Enable readline history (default: true)
    const char *history_file;   // History file path
    int history_size;           // Max history entries (default: 1000)
    smartterm_theme *theme;     // Custom theme
    bool multiline_enabled;     // Enable multi-line input (default: false)
    bool thread_safe;           // Enable thread safety (default: true)
} smartterm_config_t;
```

---

## Complete API Reference

### Initialization and Cleanup

#### smartterm_init()
```c
smartterm_ctx* smartterm_init(const smartterm_config_t *config);
```
**Description**: Initialize SmartTerm with configuration.

**Parameters**:
- `config`: Configuration options (NULL for defaults)

**Returns**: Context handle on success, NULL on failure

**Notes**:
- Must be called before any other smartterm functions
- Call `smartterm_cleanup()` when done

**Example**:
```c
smartterm_config_t cfg = smartterm_default_config();
cfg.max_lines = 5000;
cfg.history_file = "~/.smartterm_history";

smartterm_ctx *ctx = smartterm_init(&cfg);
if (!ctx) {
    perror("Failed to initialize");
    return 1;
}
```

#### smartterm_cleanup()
```c
void smartterm_cleanup(smartterm_ctx *ctx);
```
**Description**: Cleanup and free SmartTerm resources.

**Parameters**:
- `ctx`: Context handle

**Notes**:
- Safe to call multiple times
- Restores terminal to original state
- Should be called before program exit

**Example**:
```c
smartterm_cleanup(ctx);
```

#### smartterm_default_config()
```c
smartterm_config_t smartterm_default_config(void);
```
**Description**: Get default configuration.

**Returns**: Configuration structure with sensible defaults

**Example**:
```c
smartterm_config_t cfg = smartterm_default_config();
cfg.max_lines = 2000;  // Modify as needed
smartterm_ctx *ctx = smartterm_init(&cfg);
```

---

### Output Functions

#### smartterm_write()
```c
int smartterm_write(smartterm_ctx *ctx, const char *text,
                    smartterm_context_t context);
```
**Description**: Write a line to output buffer.

**Parameters**:
- `ctx`: Context handle
- `text`: Text to write (null-terminated)
- `context`: Context type for coloring

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Notes**:
- Thread-safe
- Automatically triggers render

**Example**:
```c
smartterm_write(ctx, "Processing file...", CTX_INFO);
smartterm_write(ctx, "File not found", CTX_ERROR);
```

#### smartterm_write_fmt()
```c
int smartterm_write_fmt(smartterm_ctx *ctx, smartterm_context_t context,
                        const char *format, ...);
```
**Description**: Write formatted output (printf-style).

**Parameters**:
- `ctx`: Context handle
- `context`: Context type for coloring
- `format`: Printf-style format string
- `...`: Variable arguments

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
int count = 42;
float progress = 95.5;
smartterm_write_fmt(ctx, CTX_SUCCESS,
                   "Processed %d files (%.1f%% complete)",
                   count, progress);
```

#### smartterm_write_meta()
```c
int smartterm_write_meta(smartterm_ctx *ctx, const char *text,
                         const smartterm_line_meta_t *meta);
```
**Description**: Write with metadata (timestamps, tags, etc.).

**Parameters**:
- `ctx`: Context handle
- `text`: Text to write
- `meta`: Metadata for line (context, timestamp, tag)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_line_meta_t meta = {
    .context = CTX_DEBUG,
    .timestamp = time(NULL),
    .tag = "network"
};
smartterm_write_meta(ctx, "Connection established", &meta);
```

#### smartterm_clear()
```c
int smartterm_clear(smartterm_ctx *ctx);
```
**Description**: Clear output buffer.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_clear(ctx);
smartterm_write(ctx, "Buffer cleared", CTX_NORMAL);
```

#### smartterm_render()
```c
int smartterm_render(smartterm_ctx *ctx);
```
**Description**: Force re-render of display.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Notes**:
- Usually not needed (auto-renders on write)
- Useful after terminal resize

**Example**:
```c
// After handling SIGWINCH
signal(SIGWINCH, on_resize);

void on_resize(int sig) {
    smartterm_render(ctx);
}
```

---

### Input Functions

#### smartterm_read_line()
```c
char* smartterm_read_line(smartterm_ctx *ctx, const char *prompt);
```
**Description**: Read a line of input from user.

**Parameters**:
- `ctx`: Context handle
- `prompt`: Prompt string (NULL = use default)

**Returns**: Allocated string with input, or NULL on EOF/error

**Notes**:
- Caller must `free()` returned string
- Not thread-safe (call from main thread only)
- Adds to history if enabled

**Example**:
```c
char *line = smartterm_read_line(ctx, "Enter name: ");
if (line) {
    printf("Got: %s\n", line);
    free(line);
} else {
    printf("EOF or error\n");
}
```

#### smartterm_read_multiline()
```c
char* smartterm_read_multiline(smartterm_ctx *ctx, const char *prompt);
```
**Description**: Read multi-line input from user.

**Parameters**:
- `ctx`: Context handle
- `prompt`: Prompt string

**Returns**: Allocated string with input, or NULL on EOF/error

**Notes**:
- Caller must `free()` returned string
- Submit with double-newline or Ctrl+D
- Requires `multiline_enabled` in config

**Example**:
```c
smartterm_config_t cfg = smartterm_default_config();
cfg.multiline_enabled = true;
smartterm_ctx *ctx = smartterm_init(&cfg);

char *code = smartterm_read_multiline(ctx, ">> ");
if (code) {
    // Process multi-line input
    free(code);
}
```

#### smartterm_set_prompt()
```c
int smartterm_set_prompt(smartterm_ctx *ctx, const char *prompt);
```
**Description**: Set default prompt.

**Parameters**:
- `ctx`: Context handle
- `prompt`: New prompt string

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_set_prompt(ctx, "user@host$ ");
```

---

### Status Bar

#### smartterm_status_set()
```c
int smartterm_status_set(smartterm_ctx *ctx, const char *left,
                         const char *right);
```
**Description**: Set status bar text.

**Parameters**:
- `ctx`: Context handle
- `left`: Left-aligned text (NULL = no change)
- `right`: Right-aligned text (NULL = no change)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_status_set(ctx, "Connected", "1000 lines");
smartterm_status_set(ctx, NULL, "Updated");  // Keep left, change right
```

#### smartterm_status_update()
```c
int smartterm_status_update(smartterm_ctx *ctx, const char *left_fmt,
                            const char *right_fmt, ...);
```
**Description**: Update status bar with formatted text.

**Parameters**:
- `ctx`: Context handle
- `left_fmt`: Printf-style format for left side (NULL = no change)
- `right_fmt`: Printf-style format for right side (NULL = no change)
- `...`: Variable arguments

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
int processed = 100, total = 1000;
smartterm_status_update(ctx, "Processing", "Progress: %d/%d",
                       processed, total);
```

#### smartterm_status_set_visible()
```c
int smartterm_status_set_visible(smartterm_ctx *ctx, bool visible);
```
**Description**: Show/hide status bar.

**Parameters**:
- `ctx`: Context handle
- `visible`: true to show, false to hide

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_status_set_visible(ctx, false);  // Hide status bar
// ... do something ...
smartterm_status_set_visible(ctx, true);   // Show it again
```

---

### Scrollback and Navigation

#### smartterm_scroll()
```c
int smartterm_scroll(smartterm_ctx *ctx, int lines);
```
**Description**: Scroll output buffer.

**Parameters**:
- `ctx`: Context handle
- `lines`: Number of lines to scroll (positive = up, negative = down)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Notes**:
- Automatically updates scroll indicator in status bar

**Example**:
```c
smartterm_scroll(ctx, 10);   // Scroll up 10 lines
smartterm_scroll(ctx, -5);   // Scroll down 5 lines
```

#### smartterm_scroll_top()
```c
int smartterm_scroll_top(smartterm_ctx *ctx);
```
**Description**: Scroll to top of buffer.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_scroll_top(ctx);
```

#### smartterm_scroll_bottom()
```c
int smartterm_scroll_bottom(smartterm_ctx *ctx);
```
**Description**: Scroll to bottom of buffer (most recent).

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_scroll_bottom(ctx);
```

#### smartterm_get_scroll_pos()
```c
int smartterm_get_scroll_pos(smartterm_ctx *ctx);
```
**Description**: Get current scroll position.

**Parameters**:
- `ctx`: Context handle

**Returns**: Line index of top visible line

**Example**:
```c
int pos = smartterm_get_scroll_pos(ctx);
printf("Viewing from line %d\n", pos);
```

#### smartterm_set_auto_scroll()
```c
int smartterm_set_auto_scroll(smartterm_ctx *ctx, bool enabled);
```
**Description**: Enable/disable auto-scroll on new output.

**Parameters**:
- `ctx`: Context handle
- `enabled`: true to auto-scroll, false to stay at current position

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
// Pause auto-scroll for manual inspection
smartterm_set_auto_scroll(ctx, false);

// Resume auto-scroll
smartterm_set_auto_scroll(ctx, true);
```

---

### Search

#### smartterm_search()
```c
int smartterm_search(smartterm_ctx *ctx, const char *pattern, bool use_regex,
                     smartterm_search_result_t **results, int *count);
```
**Description**: Search in output buffer.

**Parameters**:
- `ctx`: Context handle
- `pattern`: Search pattern (plain text or regex)
- `use_regex`: true for regex, false for plain text
- `results`: Output array of search results (allocated by function)
- `count`: Output number of results

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Notes**:
- Caller must free results with `smartterm_free_search_results()`

**Example**:
```c
smartterm_search_result_t *results = NULL;
int count = 0;

if (smartterm_search(ctx, "error", false, &results, &count) == SMARTTERM_OK) {
    printf("Found %d matches\n", count);
    for (int i = 0; i < count; i++) {
        printf("Line %d, column %d, length %d\n",
               results[i].line_index,
               results[i].column,
               results[i].length);
    }
    smartterm_free_search_results(results);
}
```

#### smartterm_free_search_results()
```c
void smartterm_free_search_results(smartterm_search_result_t *results);
```
**Description**: Free search results array.

**Parameters**:
- `results`: Results array from `smartterm_search()`

**Example**:
```c
smartterm_free_search_results(results);
```

#### smartterm_search_next()
```c
int smartterm_search_next(smartterm_ctx *ctx);
```
**Description**: Jump to next search match.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code if no more matches

**Example**:
```c
if (smartterm_search_next(ctx) == SMARTTERM_OK) {
    smartterm_status_set(ctx, NULL, "Next match found");
}
```

#### smartterm_search_prev()
```c
int smartterm_search_prev(smartterm_ctx *ctx);
```
**Description**: Jump to previous search match.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code if no more matches

**Example**:
```c
smartterm_search_prev(ctx);
```

#### smartterm_search_clear()
```c
int smartterm_search_clear(smartterm_ctx *ctx);
```
**Description**: Clear search highlights.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_search_clear(ctx);
```

---

### Export

#### smartterm_export()
```c
int smartterm_export(smartterm_ctx *ctx, const char *filename,
                     smartterm_export_format_t format, int start_line,
                     int end_line, bool include_meta);
```
**Description**: Export output buffer to file.

**Parameters**:
- `ctx`: Context handle
- `filename`: Output file path
- `format`: Export format (PLAIN, ANSI, MARKDOWN, HTML)
- `start_line`: First line to export (0 = first)
- `end_line`: Last line to export (-1 = last)
- `include_meta`: Include metadata (timestamps, context)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Export Formats**:
- `EXPORT_PLAIN`: Plain text
- `EXPORT_ANSI`: With ANSI color codes
- `EXPORT_MARKDOWN`: Markdown format
- `EXPORT_HTML`: HTML format

**Example**:
```c
// Export all lines as plain text
smartterm_export(ctx, "output.txt", EXPORT_PLAIN, 0, -1, false);

// Export lines 10-50 with metadata as HTML
smartterm_export(ctx, "report.html", EXPORT_HTML, 10, 50, true);

// Export as Markdown with colors
smartterm_export(ctx, "log.md", EXPORT_MARKDOWN, 0, -1, false);
```

#### smartterm_export_string()
```c
char* smartterm_export_string(smartterm_ctx *ctx,
                              smartterm_export_format_t format,
                              int start_line, int end_line, bool include_meta);
```
**Description**: Export to string.

**Parameters**:
- `ctx`: Context handle
- `format`: Export format
- `start_line`: First line to export (0 = first)
- `end_line`: Last line to export (-1 = last)
- `include_meta`: Include metadata

**Returns**: Allocated string with exported content, or NULL on error

**Notes**:
- Caller must `free()` returned string

**Example**:
```c
char *content = smartterm_export_string(ctx, EXPORT_PLAIN, 0, -1, false);
if (content) {
    printf("Exported content:\n%s\n", content);
    free(content);
}
```

---

### Tab Completion

#### smartterm_completer_fn (callback type)
```c
typedef char** (*smartterm_completer_fn)(const char *text, int start,
                                         int end, void *data);
```
**Description**: Completion callback function type.

**Parameters**:
- `text`: Text to complete
- `start`: Start position in line buffer
- `end`: End position in line buffer
- `data`: User data passed to `smartterm_set_completer()`

**Returns**: NULL-terminated array of completion candidates (caller frees)

#### smartterm_set_completer()
```c
int smartterm_set_completer(smartterm_ctx *ctx, smartterm_completer_fn completer,
                            void *data);
```
**Description**: Set tab completion callback.

**Parameters**:
- `ctx`: Context handle
- `completer`: Completion callback function
- `data`: User data passed to callback

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
// Command list for completion
static const char *commands[] = {
    "help", "exit", "status", "clear", NULL
};

char** command_completer(const char *text, int start, int end, void *data) {
    // Simple completer that suggests all commands starting with text
    int count = 0;
    for (int i = 0; commands[i]; i++) {
        if (strncmp(commands[i], text, strlen(text)) == 0) {
            count++;
        }
    }

    if (count == 0) return NULL;

    char **results = malloc((count + 1) * sizeof(char*));
    int idx = 0;
    for (int i = 0; commands[i]; i++) {
        if (strncmp(commands[i], text, strlen(text)) == 0) {
            results[idx++] = strdup(commands[i]);
        }
    }
    results[idx] = NULL;
    return results;
}

// Set the completer
smartterm_set_completer(ctx, command_completer, NULL);
```

---

### Themes

#### smartterm_theme_create()
```c
smartterm_theme* smartterm_theme_create(const char *name);
```
**Description**: Create new custom theme.

**Parameters**:
- `name`: Theme name

**Returns**: Theme handle on success, NULL on failure

**Example**:
```c
smartterm_theme *custom = smartterm_theme_create("my-theme");
if (custom) {
    smartterm_theme_set_color(custom, CTX_ERROR, 1, 0);  // Red foreground
}
```

#### smartterm_theme_free()
```c
void smartterm_theme_free(smartterm_theme *theme);
```
**Description**: Free custom theme.

**Parameters**:
- `theme`: Theme handle

**Notes**:
- Do not use after freeing
- Do not free built-in themes

**Example**:
```c
smartterm_theme_free(custom);
```

#### smartterm_theme_set_color()
```c
int smartterm_theme_set_color(smartterm_theme *theme,
                              smartterm_context_t context,
                              int fg, int bg);
```
**Description**: Set color for context.

**Parameters**:
- `theme`: Theme handle
- `context`: Context type
- `fg`: Foreground color (0-255)
- `bg`: Background color (0-255)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Color Values** (0-7 are standard):
- 0: Black
- 1: Red
- 2: Green
- 3: Yellow
- 4: Blue
- 5: Magenta
- 6: Cyan
- 7: White

**Example**:
```c
// Red error text on black background
smartterm_theme_set_color(theme, CTX_ERROR, 1, 0);

// Green success on dark background
smartterm_theme_set_color(theme, CTX_SUCCESS, 2, 0);
```

#### smartterm_theme_set_symbol()
```c
int smartterm_theme_set_symbol(smartterm_theme *theme,
                               smartterm_symbol_t symbol,
                               const char *value);
```
**Description**: Set symbol.

**Parameters**:
- `theme`: Theme handle
- `symbol`: Symbol type
- `value`: Symbol string

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Symbol Types**:
- `SYM_PROMPT`: Input prompt
- `SYM_MULTILINE_PROMPT`: Multi-line prompt continuation
- `SYM_STATUS_SEP`: Status bar separator
- `SYM_SCROLL_INDICATOR`: Scrollback indicator
- `SYM_SEARCH_MATCH`: Search match marker

**Example**:
```c
smartterm_theme_set_symbol(theme, SYM_PROMPT, ">> ");
smartterm_theme_set_symbol(theme, SYM_STATUS_SEP, "─");
smartterm_theme_set_symbol(theme, SYM_SCROLL_INDICATOR, "↕");
```

#### smartterm_theme_set_attribute()
```c
int smartterm_theme_set_attribute(smartterm_theme *theme,
                                  smartterm_context_t context,
                                  int attribute);
```
**Description**: Set text attribute for context.

**Parameters**:
- `theme`: Theme handle
- `context`: Context type
- `attribute`: ncurses attribute (A_BOLD, A_DIM, etc.)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
// Make error text bold and red
smartterm_theme_set_color(theme, CTX_ERROR, 1, 0);
smartterm_theme_set_attribute(theme, CTX_ERROR, A_BOLD);
```

#### smartterm_theme_get_builtin()
```c
const smartterm_theme* smartterm_theme_get_builtin(const char *name);
```
**Description**: Get built-in theme.

**Parameters**:
- `name`: Theme name ("default", "dark", "light", "solarized", "nord")

**Returns**: Theme handle on success, NULL if not found

**Notes**:
- Do not free built-in themes
- Can be used directly with `smartterm_set_theme()`

**Example**:
```c
const smartterm_theme *nord = smartterm_theme_get_builtin("nord");
if (nord) {
    smartterm_set_theme(ctx, nord);
}
```

#### smartterm_set_theme()
```c
int smartterm_set_theme(smartterm_ctx *ctx, const smartterm_theme *theme);
```
**Description**: Set active theme.

**Parameters**:
- `ctx`: Context handle
- `theme`: Theme handle (NULL = default)

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
// Use built-in solarized theme
const smartterm_theme *solarized = smartterm_theme_get_builtin("solarized");
smartterm_set_theme(ctx, solarized);

// Or use default
smartterm_set_theme(ctx, NULL);
```

---

### Key Handlers

#### smartterm_key_handler_fn (callback type)
```c
typedef void (*smartterm_key_handler_fn)(smartterm_ctx *ctx, int key,
                                         void *data);
```
**Description**: Key handler callback function type.

**Parameters**:
- `ctx`: Context handle
- `key`: Key code (from ncurses)
- `data`: User data passed to `smartterm_register_key_handler()`

#### smartterm_register_key_handler()
```c
int smartterm_register_key_handler(smartterm_ctx *ctx, int key,
                                   smartterm_key_handler_fn handler,
                                   void *data);
```
**Description**: Register key handler.

**Parameters**:
- `ctx`: Context handle
- `key`: Key code to handle (from ncurses KEY_* constants)
- `handler`: Handler callback
- `data`: User data passed to callback

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Common Key Codes** (ncurses):
- `KEY_UP`, `KEY_DOWN`, `KEY_LEFT`, `KEY_RIGHT`
- `KEY_HOME`, `KEY_END`, `KEY_PPAGE`, `KEY_NPAGE`
- `KEY_DC`, `KEY_BACKSPACE`

**Example**:
```c
#include <ncurses.h>

void handle_page_down(smartterm_ctx *ctx, int key, void *data) {
    smartterm_scroll(ctx, -10);  // Scroll down 10 lines
}

void handle_page_up(smartterm_ctx *ctx, int key, void *data) {
    smartterm_scroll(ctx, 10);   // Scroll up 10 lines
}

// Register handlers
smartterm_register_key_handler(ctx, KEY_NPAGE, handle_page_down, NULL);
smartterm_register_key_handler(ctx, KEY_PPAGE, handle_page_up, NULL);
```

#### smartterm_unregister_key_handler()
```c
int smartterm_unregister_key_handler(smartterm_ctx *ctx, int key);
```
**Description**: Unregister key handler.

**Parameters**:
- `ctx`: Context handle
- `key`: Key code

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
smartterm_unregister_key_handler(ctx, KEY_NPAGE);
```

---

### Error Handling

#### smartterm_error_string()
```c
const char* smartterm_error_string(int code);
```
**Description**: Get error string for error code.

**Parameters**:
- `code`: Error code

**Returns**: Human-readable error string

**Example**:
```c
int result = smartterm_write(ctx, "test", CTX_NORMAL);
if (result != SMARTTERM_OK) {
    fprintf(stderr, "Error: %s\n", smartterm_error_string(result));
}
```

#### smartterm_get_last_error()
```c
int smartterm_get_last_error(smartterm_ctx *ctx);
```
**Description**: Get last error code.

**Parameters**:
- `ctx`: Context handle

**Returns**: Last error code

**Example**:
```c
int err = smartterm_get_last_error(ctx);
if (err != SMARTTERM_OK) {
    printf("Last error: %s\n", smartterm_error_string(err));
}
```

---

### Utility Functions

#### smartterm_version()
```c
const char* smartterm_version(void);
```
**Description**: Get library version.

**Returns**: Version string (e.g., "1.0.0")

**Example**:
```c
printf("SmartTerm version: %s\n", smartterm_version());
```

#### smartterm_get_line_count()
```c
int smartterm_get_line_count(smartterm_ctx *ctx);
```
**Description**: Get output buffer line count.

**Parameters**:
- `ctx`: Context handle

**Returns**: Number of lines in buffer

**Example**:
```c
int lines = smartterm_get_line_count(ctx);
printf("Buffer has %d lines\n", lines);
```

#### smartterm_get_line()
```c
const char* smartterm_get_line(smartterm_ctx *ctx, int index);
```
**Description**: Get line from buffer.

**Parameters**:
- `ctx`: Context handle
- `index`: Line index (0 = oldest)

**Returns**: Line text, or NULL if invalid index

**Notes**:
- Returned string is valid until next write/clear operation
- Do not free

**Example**:
```c
for (int i = 0; i < smartterm_get_line_count(ctx); i++) {
    const char *line = smartterm_get_line(ctx, i);
    if (line) {
        printf("[%d] %s\n", i, line);
    }
}
```

#### smartterm_get_line_meta()
```c
int smartterm_get_line_meta(smartterm_ctx *ctx, int index,
                            smartterm_line_meta_t *meta);
```
**Description**: Get line metadata.

**Parameters**:
- `ctx`: Context handle
- `index`: Line index
- `meta`: Output metadata structure

**Returns**: `SMARTTERM_OK` on success, error code if invalid index

**Example**:
```c
smartterm_line_meta_t meta;
if (smartterm_get_line_meta(ctx, 0, &meta) == SMARTTERM_OK) {
    printf("Line context: %d, timestamp: %ld, tag: %s\n",
           meta.context, meta.timestamp, meta.tag ? meta.tag : "none");
}
```

#### smartterm_get_terminal_size()
```c
int smartterm_get_terminal_size(smartterm_ctx *ctx, int *rows, int *cols);
```
**Description**: Get terminal size.

**Parameters**:
- `ctx`: Context handle
- `rows`: Output rows
- `cols`: Output columns

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Example**:
```c
int rows, cols;
if (smartterm_get_terminal_size(ctx, &rows, &cols) == SMARTTERM_OK) {
    printf("Terminal: %d rows x %d cols\n", rows, cols);
}
```

#### smartterm_handle_resize()
```c
int smartterm_handle_resize(smartterm_ctx *ctx);
```
**Description**: Handle terminal resize.

**Parameters**:
- `ctx`: Context handle

**Returns**: `SMARTTERM_OK` on success, error code on failure

**Notes**:
- Usually called from SIGWINCH handler

**Example**:
```c
#include <signal.h>

static smartterm_ctx *global_ctx = NULL;

void handle_resize(int sig) {
    if (global_ctx) {
        smartterm_handle_resize(global_ctx);
    }
}

int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    global_ctx = ctx;

    signal(SIGWINCH, handle_resize);
    // ... rest of program ...
}
```

---

## Code Examples

### Example 1: Simple Interactive Shell

```c
#include "smartterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize SmartTerm\n");
        return 1;
    }

    smartterm_write(ctx, "SmartTerm Shell v1.0", CTX_INFO);
    smartterm_write(ctx, "Type 'help' for commands, 'exit' to quit", CTX_INFO);
    smartterm_status_set(ctx, "Shell Ready", "Press Tab for completion");

    int line_count = 0;

    while (1) {
        char *input = smartterm_read_line(ctx, "$ ");

        if (!input) {
            smartterm_write(ctx, "Goodbye!", CTX_SUCCESS);
            break;
        }

        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        // Echo the command
        smartterm_write_fmt(ctx, CTX_COMMAND, "$ %s", input);

        // Handle commands
        if (strcmp(input, "exit") == 0) {
            smartterm_write(ctx, "Exiting...", CTX_SUCCESS);
            free(input);
            break;
        } else if (strcmp(input, "help") == 0) {
            smartterm_write(ctx, "Available commands:", CTX_NORMAL);
            smartterm_write(ctx, "  exit    - Exit shell", CTX_NORMAL);
            smartterm_write(ctx, "  clear   - Clear screen", CTX_NORMAL);
            smartterm_write(ctx, "  help    - Show help", CTX_NORMAL);
        } else if (strcmp(input, "clear") == 0) {
            smartterm_clear(ctx);
        } else {
            smartterm_write_fmt(ctx, CTX_ERROR, "Unknown command: %s", input);
        }

        line_count++;
        smartterm_status_update(ctx, NULL, "Lines: %d", line_count);

        free(input);
    }

    smartterm_cleanup(ctx);
    return 0;
}
```

### Example 2: Log Viewer with Search

```c
#include "smartterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void write_sample_logs(smartterm_ctx *ctx) {
    smartterm_write(ctx, "Starting log viewer...", CTX_INFO);

    for (int i = 0; i < 100; i++) {
        if (i % 10 == 0) {
            smartterm_write_fmt(ctx, CTX_WARNING, "Warning: Check iteration %d", i);
        } else if (i % 15 == 0) {
            smartterm_write_fmt(ctx, CTX_ERROR, "Error occurred at iteration %d", i);
        } else {
            smartterm_write_fmt(ctx, CTX_NORMAL, "Processing item %d", i);
        }
    }
}

int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (!ctx) return 1;

    write_sample_logs(ctx);
    smartterm_status_set(ctx, "Log Viewer", "Ready");

    while (1) {
        char *input = smartterm_read_line(ctx, "search> ");

        if (!input) break;
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        smartterm_search_result_t *results = NULL;
        int count = 0;

        if (smartterm_search(ctx, input, false, &results, &count) == SMARTTERM_OK) {
            smartterm_write_fmt(ctx, CTX_INFO, "Found %d matches", count);

            for (int i = 0; i < count && i < 5; i++) {
                const char *line = smartterm_get_line(ctx, results[i].line_index);
                if (line) {
                    smartterm_write_fmt(ctx, CTX_SEARCH,
                                      "Match %d at line %d: %s",
                                      i + 1, results[i].line_index, line);
                }
            }

            smartterm_free_search_results(results);
        } else {
            smartterm_write(ctx, "Search failed", CTX_ERROR);
        }

        free(input);
    }

    smartterm_cleanup(ctx);
    return 0;
}
```

### Example 3: Themed Application

```c
#include "smartterm.h"
#include <stdio.h>
#include <stdlib.h>

smartterm_theme* create_custom_theme(void) {
    smartterm_theme *theme = smartterm_theme_create("custom");
    if (!theme) return NULL;

    // Set colors
    smartterm_theme_set_color(theme, CTX_ERROR, 1, 0);       // Red
    smartterm_theme_set_color(theme, CTX_SUCCESS, 2, 0);     // Green
    smartterm_theme_set_color(theme, CTX_WARNING, 3, 0);     // Yellow
    smartterm_theme_set_color(theme, CTX_INFO, 6, 0);        // Cyan

    // Set attributes
    smartterm_theme_set_attribute(theme, CTX_ERROR, A_BOLD);
    smartterm_theme_set_attribute(theme, CTX_SUCCESS, A_BOLD);

    // Set symbols
    smartterm_theme_set_symbol(theme, SYM_PROMPT, ">>> ");
    smartterm_theme_set_symbol(theme, SYM_STATUS_SEP, "━");

    return theme;
}

int main(void) {
    smartterm_config_t cfg = smartterm_default_config();
    cfg.theme = create_custom_theme();

    smartterm_ctx *ctx = smartterm_init(&cfg);
    if (!ctx) return 1;

    smartterm_write(ctx, "Welcome to Custom Theme App", CTX_SUCCESS);
    smartterm_write(ctx, "This is an error message", CTX_ERROR);
    smartterm_write(ctx, "This is a warning", CTX_WARNING);
    smartterm_write(ctx, "This is info", CTX_INFO);

    smartterm_read_line(ctx, NULL);  // Wait for input

    smartterm_cleanup(ctx);
    if (cfg.theme) smartterm_theme_free(cfg.theme);
    return 0;
}
```

### Example 4: Export and Analysis

```c
#include "smartterm.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (!ctx) return 1;

    // Generate some content
    for (int i = 0; i < 50; i++) {
        if (i % 5 == 0) {
            smartterm_write_fmt(ctx, CTX_WARNING, "Item %d - Warning", i);
        } else {
            smartterm_write_fmt(ctx, CTX_SUCCESS, "Item %d - Success", i);
        }
    }

    // Export in different formats
    smartterm_export(ctx, "output.txt", EXPORT_PLAIN, 0, -1, false);
    smartterm_export(ctx, "output.html", EXPORT_HTML, 0, -1, true);
    smartterm_export(ctx, "output.md", EXPORT_MARKDOWN, 0, -1, false);

    // Get as string and process
    char *content = smartterm_export_string(ctx, EXPORT_PLAIN, 0, 10, false);
    if (content) {
        printf("First 10 lines:\n%s\n", content);
        free(content);
    }

    smartterm_cleanup(ctx);
    return 0;
}
```

### Example 5: Multi-threaded Output

```c
#include "smartterm.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

smartterm_ctx *global_ctx = NULL;

void* worker_thread(void *arg) {
    int id = (intptr_t)arg;

    for (int i = 0; i < 10; i++) {
        smartterm_write_fmt(global_ctx, CTX_NORMAL,
                          "Thread %d: Working on task %d", id, i);
        usleep(100000);  // 100ms
    }

    return NULL;
}

int main(void) {
    smartterm_config_t cfg = smartterm_default_config();
    cfg.thread_safe = true;  // Enable thread safety

    global_ctx = smartterm_init(&cfg);
    if (!global_ctx) return 1;

    smartterm_write(global_ctx, "Starting multi-threaded test", CTX_INFO);

    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, worker_thread, (void*)(intptr_t)i);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    smartterm_write(global_ctx, "All threads completed", CTX_SUCCESS);
    smartterm_read_line(global_ctx, NULL);

    smartterm_cleanup(global_ctx);
    return 0;
}
```

---

## Integration Guide

### Step 1: Add SmartTerm to Your Project

Copy the SmartTerm library files to your project:
```
your-project/
├── src/
├── include/
│   └── smartterm.h
└── lib/
    ├── libsmartterm.a
    └── libsmartterm.so
```

### Step 2: Include Header

```c
#include "smartterm.h"
```

### Step 3: Link Library

**CMake**:
```cmake
find_library(SMARTTERM_LIB smartterm PATHS ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(your_target ${SMARTTERM_LIB})
```

**Makefile**:
```makefile
CFLAGS = -I./include
LDFLAGS = -L./lib -lsmartterm
```

**GCC/Clang**:
```bash
gcc myprogram.c -I./include -L./lib -lsmartterm -o myprogram
```

### Step 4: Initialize and Use

```c
int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (!ctx) return 1;

    // Use SmartTerm...

    smartterm_cleanup(ctx);
    return 0;
}
```

### Step 5: Handle Terminal Signals

For production code, handle terminal resize:
```c
#include <signal.h>

static smartterm_ctx *global_ctx = NULL;

void handle_sigwinch(int sig) {
    if (global_ctx) {
        smartterm_handle_resize(global_ctx);
    }
}

int main(void) {
    global_ctx = smartterm_init(NULL);
    signal(SIGWINCH, handle_sigwinch);
    // ... rest of program ...
}
```

---

## Best Practices

### 1. Memory Management

Always free allocated memory:
```c
// Free input lines
char *input = smartterm_read_line(ctx, "prompt> ");
if (input) {
    free(input);
}

// Free search results
smartterm_free_search_results(results);

// Free themes (custom only, not built-in)
if (custom_theme) {
    smartterm_theme_free(custom_theme);
}

// Free exported strings
char *exported = smartterm_export_string(ctx, EXPORT_PLAIN, 0, -1, false);
if (exported) {
    free(exported);
}
```

### 2. Error Checking

Always check return codes:
```c
int result = smartterm_write(ctx, "text", CTX_NORMAL);
if (result != SMARTTERM_OK) {
    fprintf(stderr, "Write failed: %s\n", smartterm_error_string(result));
    return 1;
}
```

### 3. Configuration

Use appropriate configuration for your use case:
```c
smartterm_config_t cfg = smartterm_default_config();

// For interactive shell with history
cfg.history_enabled = true;
cfg.history_file = "~/.myapp_history";
cfg.history_size = 2000;

// For multi-threaded app
cfg.thread_safe = true;

// For apps with lots of output
cfg.max_lines = 10000;

smartterm_ctx *ctx = smartterm_init(&cfg);
```

### 4. Use Appropriate Contexts

Choose the right context for different message types:
```c
smartterm_write(ctx, "Normal operation", CTX_NORMAL);
smartterm_write(ctx, "Something went wrong", CTX_ERROR);
smartterm_write(ctx, "Be careful", CTX_WARNING);
smartterm_write(ctx, "Operation succeeded", CTX_SUCCESS);
smartterm_write(ctx, "FYI", CTX_INFO);
smartterm_write(ctx, "Debug info", CTX_DEBUG);
```

### 5. Status Bar Updates

Keep status bar updated for long operations:
```c
for (int i = 0; i < total; i++) {
    // Do work

    if (i % 100 == 0) {
        smartterm_status_update(ctx, "Processing",
                              "Progress: %d/%d", i, total);
    }
}
```

### 6. Completion Callbacks

Implement efficient completion:
```c
char** my_completer(const char *text, int start, int end, void *data) {
    // Only complete if text is non-empty
    if (!text || !*text) return NULL;

    // Allocate result array
    char **results = malloc(MAX_COMPLETIONS * sizeof(char*));
    int count = 0;

    // Find matches
    for (int i = 0; candidates[i] && count < MAX_COMPLETIONS - 1; i++) {
        if (strncmp(candidates[i], text, strlen(text)) == 0) {
            results[count++] = strdup(candidates[i]);
        }
    }

    results[count] = NULL;
    return results;
}
```

### 7. Thread Safety

If using multiple threads:
```c
// Enable thread safety in config
smartterm_config_t cfg = smartterm_default_config();
cfg.thread_safe = true;

// Now it's safe to call smartterm_write() from multiple threads
pthread_t t1, t2;
pthread_create(&t1, NULL, thread_func, ctx);
pthread_create(&t2, NULL, thread_func, ctx);
```

### 8. Cleanup on Exit

Always cleanup before exiting:
```c
int main(void) {
    smartterm_ctx *ctx = smartterm_init(NULL);
    if (!ctx) return 1;

    // ... use SmartTerm ...

    smartterm_cleanup(ctx);  // Restore terminal state
    return 0;
}
```

---

## Troubleshooting

### Display Issues

**Q: Colors are not showing**
- A: Ensure your terminal supports ANSI colors
- Check `TERM` environment variable: `echo $TERM`
- Try: `export TERM=xterm-256color`

**Q: Status bar is overlapping output**
- A: Adjust output height in config:
  ```c
  cfg.output_height = rows - 2;  // Leave room for status and input
  ```

**Q: Terminal looks corrupted after program exits**
- A: Ensure `smartterm_cleanup()` is called
- Explicitly restore terminal with `smartterm_handle_resize(ctx)`

### Input Issues

**Q: Tab completion not working**
- A: Ensure completer is registered before input:
  ```c
  smartterm_set_completer(ctx, my_completer, NULL);
  char *input = smartterm_read_line(ctx, "> ");
  ```

**Q: History file not being created**
- A: Check file path and permissions:
  ```c
  cfg.history_file = "/tmp/.myapp_history";  // Use writable path
  cfg.history_enabled = true;
  ```

**Q: Multiline input not working**
- A: Enable in config:
  ```c
  cfg.multiline_enabled = true;
  ```

### Performance Issues

**Q: Slow output with many lines**
- A: Reduce max_lines or use scrollback more efficiently:
  ```c
  cfg.max_lines = 5000;  // Default is 1000
  ```

**Q: Memory usage growing**
- A: Check that search results are freed:
  ```c
  smartterm_free_search_results(results);
  ```

### Threading Issues

**Q: Crashes with multiple threads**
- A: Enable thread safety:
  ```c
  cfg.thread_safe = true;
  ```

**Q: Deadlocks**
- A: Don't call `smartterm_read_line()` from worker threads
- Only call from main thread

### Search Issues

**Q: Regex search not working**
- A: Ensure pattern is valid regex:
  ```c
  const char *pattern = "^error:.*critical$";  // Anchors work
  smartterm_search(ctx, pattern, true, &results, &count);
  ```

**Q: Search results are empty**
- A: Check pattern and buffer content:
  ```c
  // Verify content exists
  int lines = smartterm_get_line_count(ctx);
  printf("Buffer has %d lines\n", lines);
  ```

### Theme Issues

**Q: Custom theme not applying**
- A: Set theme before first render:
  ```c
  smartterm_config_t cfg = smartterm_default_config();
  cfg.theme = smartterm_theme_get_builtin("nord");
  smartterm_ctx *ctx = smartterm_init(&cfg);
  ```

**Q: Colors not matching theme**
- A: Check terminal color support:
  ```bash
  echo $COLORTERM  # Should be "truecolor" or "256color"
  ```

### Export Issues

**Q: Export file not created**
- A: Check file path and permissions:
  ```c
  // Use absolute path
  smartterm_export(ctx, "/tmp/output.txt", EXPORT_PLAIN, 0, -1, false);
  ```

**Q: Export includes unwanted metadata**
- A: Set `include_meta` to false:
  ```c
  smartterm_export(ctx, "file.txt", EXPORT_PLAIN, 0, -1, false);
  ```

### Terminal Size Issues

**Q: Content not fitting in window**
- A: Handle resize properly:
  ```c
  signal(SIGWINCH, handle_resize);

  void handle_resize(int sig) {
      smartterm_handle_resize(ctx);
  }
  ```

**Q: Size detection failing**
- A: Check if running in proper terminal:
  ```c
  int rows, cols;
  if (smartterm_get_terminal_size(ctx, &rows, &cols) != SMARTTERM_OK) {
      fprintf(stderr, "Not running in terminal\n");
  }
  ```

---

## Getting Help

- Check `smartterm_error_string()` for descriptive error messages
- Review code examples in this documentation
- Ensure proper initialization and cleanup
- Enable debug output if available: `cfg.debug = true`

---

**SmartTerm API Documentation - Version 1.0.0**

Last Updated: 2025-11-18

License: MIT

For the latest documentation and updates, visit the SmartTerm repository.
