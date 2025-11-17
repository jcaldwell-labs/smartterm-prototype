/*
 * SmartTerm Simple Library - Implementation
 *
 * Extracted from smartterm_poc.c and refactored as a reusable library.
 */

#include "smartterm_simple.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_OUTPUT_LINES 1000

// Internal output buffer structure
typedef struct {
    char **lines;
    int count;
    int capacity;
    int scroll_offset;
} OutputBuffer;

// Internal global state
static OutputBuffer output;
static WINDOW *output_win = NULL;
static WINDOW *status_win = NULL;
static int screen_height = 0;
static int screen_width = 0;
static int initialized = 0;

// Internal function: Initialize output buffer
static void init_output_buffer(void) {
    output.capacity = MAX_OUTPUT_LINES;
    output.lines = malloc(sizeof(char*) * output.capacity);
    output.count = 0;
    output.scroll_offset = 0;
}

// Internal function: Free output buffer
static void free_output_buffer(void) {
    for (int i = 0; i < output.count; i++) {
        free(output.lines[i]);
    }
    free(output.lines);
    output.lines = NULL;
    output.count = 0;
}

// Internal function: Render output buffer to window
static void render_output(void) {
    if (!output_win) return;

    werase(output_win);
    box(output_win, 0, 0);

    int max_display_lines = screen_height - 5; // Leave room for status and input
    int start_line = (output.count > max_display_lines) ?
                     (output.count - max_display_lines) : 0;

    // Display lines from bottom up (most recent at bottom)
    for (int i = 0; i < output.count - start_line; i++) {
        int line_idx = start_line + i;
        char *line = output.lines[line_idx];

        // Color based on context prefix
        int color = COLOR_PAIR(1); // default
        if (line[0] == '!') color = COLOR_PAIR(2); // command (yellow)
        else if (line[0] == '#') color = COLOR_PAIR(3); // comment (green)
        else if (line[0] == '@') color = COLOR_PAIR(4); // special (cyan)
        else if (line[0] == '/') color = COLOR_PAIR(5); // search (magenta)

        wattron(output_win, color);
        mvwprintw(output_win, i + 1, 2, "%s", line + 2); // Skip prefix
        wattroff(output_win, color);
    }

    wrefresh(output_win);
}

// Public API Implementation

int smartterm_init(void) {
    if (initialized) {
        return 0; // Already initialized
    }

    // Initialize ncurses
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Initialize colors
    if (!has_colors()) {
        endwin();
        fprintf(stderr, "Error: Terminal does not support colors\n");
        return -1;
    }

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);   // Normal
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);  // Command
    init_pair(3, COLOR_GREEN, COLOR_BLACK);   // Comment
    init_pair(4, COLOR_CYAN, COLOR_BLACK);    // Special
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK); // Search

    getmaxyx(stdscr, screen_height, screen_width);

    // Create windows
    // Output takes most of screen (top)
    output_win = newwin(screen_height - 3, screen_width, 0, 0);
    scrollok(output_win, TRUE);

    // Status bar (single line, above input area)
    status_win = newwin(1, screen_width, screen_height - 3, 0);

    refresh();

    // Initialize output buffer
    init_output_buffer();

    initialized = 1;
    return 0;
}

void smartterm_cleanup(void) {
    if (!initialized) {
        return;
    }

    // Cleanup windows
    if (output_win) {
        delwin(output_win);
        output_win = NULL;
    }
    if (status_win) {
        delwin(status_win);
        status_win = NULL;
    }

    endwin();

    // Free output buffer
    free_output_buffer();

    initialized = 0;
}

void smartterm_add_output(const char *text, smartterm_context_t ctx) {
    if (!initialized || !text) return;

    if (output.count >= output.capacity) {
        // Drop oldest line
        free(output.lines[0]);
        memmove(output.lines, output.lines + 1, sizeof(char*) * (output.capacity - 1));
        output.count--;
    }

    // Store with context prefix for coloring
    char prefix = (ctx == SMARTTERM_CTX_COMMAND) ? '!' :
                  (ctx == SMARTTERM_CTX_COMMENT) ? '#' :
                  (ctx == SMARTTERM_CTX_SPECIAL) ? '@' :
                  (ctx == SMARTTERM_CTX_SEARCH)  ? '/' : ' ';

    size_t len = strlen(text) + 3;
    output.lines[output.count] = malloc(len);
    snprintf(output.lines[output.count], len, "%c %s", prefix, text);
    output.count++;

    // Render updated output
    render_output();
}

char* smartterm_readline(const char *prompt) {
    if (!initialized) return NULL;

    // Temporarily suspend ncurses for readline
    def_prog_mode();
    endwin();

    // Use readline (handles editing, history)
    char *input = readline(prompt);

    // Resume ncurses
    reset_prog_mode();
    refresh();

    // Re-render windows after resume
    render_output();
    touchwin(status_win);
    wrefresh(status_win);

    // Add to history if not empty
    if (input && strlen(input) > 0) {
        add_history(input);
    }

    return input;
}

void smartterm_set_status(const char *left, const char *right) {
    if (!initialized || !status_win) return;

    werase(status_win);

    wattron(status_win, A_REVERSE);
    mvwprintw(status_win, 0, 0, "%-*s", screen_width, "");

    if (left) {
        mvwprintw(status_win, 0, 2, "%s", left);
    }

    if (right) {
        mvwprintw(status_win, 0, screen_width - strlen(right) - 2, "%s", right);
    }

    wattroff(status_win, A_REVERSE);

    wrefresh(status_win);
}

void smartterm_clear_output(void) {
    if (!initialized) return;

    // Free all lines
    for (int i = 0; i < output.count; i++) {
        free(output.lines[i]);
    }
    output.count = 0;

    // Re-render
    render_output();
}

smartterm_context_t smartterm_detect_context(const char *input) {
    if (!input || input[0] == '\0') {
        return SMARTTERM_CTX_NORMAL;
    }

    switch (input[0]) {
        case '!': return SMARTTERM_CTX_COMMAND;
        case '#': return SMARTTERM_CTX_COMMENT;
        case '@': return SMARTTERM_CTX_SPECIAL;
        case '/': return SMARTTERM_CTX_SEARCH;
        default:  return SMARTTERM_CTX_NORMAL;
    }
}

const char* smartterm_strip_context(const char *input) {
    if (!input) return input;

    smartterm_context_t ctx = smartterm_detect_context(input);
    if (ctx != SMARTTERM_CTX_NORMAL) {
        return input + 1; // Skip the context prefix character
    }
    return input;
}
