/*
 * SmartTerm Proof of Concept
 *
 * Demonstrates:
 * - Scrolling output region (top)
 * - Expanding readline input (bottom)
 * - Status bar (between them)
 * - Context awareness (>, !, #, @)
 *
 * Build: gcc -o smartterm_poc smartterm_poc.c -lncurses -lreadline
 * Run: ./smartterm_poc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_OUTPUT_LINES 1000

// Output buffer to prevent prompt duplication
typedef struct {
    char **lines;
    int count;
    int capacity;
    int scroll_offset;
} OutputBuffer;

// Context types
typedef enum {
    CTX_NORMAL,
    CTX_COMMAND,    // !
    CTX_COMMENT,    // #
    CTX_SPECIAL,    // @
    CTX_SEARCH      // /
} ContextType;

// Global state
static OutputBuffer output;
static WINDOW *output_win;
static WINDOW *status_win;
static int screen_height, screen_width;

// Initialize output buffer
void init_output_buffer() {
    output.capacity = MAX_OUTPUT_LINES;
    output.lines = malloc(sizeof(char*) * output.capacity);
    output.count = 0;
    output.scroll_offset = 0;
}

// Add line to output buffer (no prompt duplication)
void add_output_line(const char *text, ContextType ctx) {
    if (output.count >= output.capacity) {
        // Drop oldest line
        free(output.lines[0]);
        memmove(output.lines, output.lines + 1, sizeof(char*) * (output.capacity - 1));
        output.count--;
    }

    // Store with context prefix for coloring
    char prefix = (ctx == CTX_COMMAND) ? '!' :
                  (ctx == CTX_COMMENT) ? '#' :
                  (ctx == CTX_SPECIAL) ? '@' :
                  (ctx == CTX_SEARCH)  ? '/' : ' ';

    size_t len = strlen(text) + 3;
    output.lines[output.count] = malloc(len);
    snprintf(output.lines[output.count], len, "%c %s", prefix, text);
    output.count++;
}

// Detect context from input
ContextType detect_context(const char *input) {
    if (input[0] == '!') return CTX_COMMAND;
    if (input[0] == '#') return CTX_COMMENT;
    if (input[0] == '@') return CTX_SPECIAL;
    if (input[0] == '/') return CTX_SEARCH;
    return CTX_NORMAL;
}

// Render output buffer to window
void render_output() {
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

// Render status bar
void render_status(const char *left, const char *right) {
    werase(status_win);

    wattron(status_win, A_REVERSE);
    mvwprintw(status_win, 0, 0, "%-*s", screen_width, "");
    mvwprintw(status_win, 0, 2, "%s", left);
    mvwprintw(status_win, 0, screen_width - strlen(right) - 2, "%s", right);
    wattroff(status_win, A_REVERSE);

    wrefresh(status_win);
}

// Initialize ncurses
void init_ui() {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    // Initialize colors
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

    // Status bar (single line, above input)
    status_win = newwin(1, screen_width, screen_height - 3, 0);

    refresh();
}

// Cleanup ncurses
void cleanup_ui() {
    delwin(output_win);
    delwin(status_win);
    endwin();
}

// Custom readline wrapper for ncurses compatibility
char* readline_with_ncurses(const char *prompt) {
    // Temporarily suspend ncurses for readline
    def_prog_mode();
    endwin();

    // Use readline (handles multi-line via continuation)
    char *input = readline(prompt);

    // Resume ncurses
    reset_prog_mode();
    refresh();

    return input;
}

int main() {
    init_output_buffer();
    init_ui();

    // Welcome message
    add_output_line("SmartTerm Proof of Concept", CTX_NORMAL);
    add_output_line("Commands: help, clear, quit", CTX_NORMAL);
    add_output_line("Context: ! (command) # (comment) @ (special) / (search)", CTX_NORMAL);

    render_output();
    render_status("Ready", "v0.0.1 POC");

    int running = 1;
    int cmd_count = 0;

    while (running) {
        // Read input with readline (handles expanding input, history, editing)
        char *input = readline_with_ncurses("> ");

        if (!input) {
            running = 0;
            break;
        }

        // Skip empty input
        if (strlen(input) == 0) {
            free(input);
            continue;
        }

        // Add to history
        add_history(input);

        // Detect context
        ContextType ctx = detect_context(input);
        const char *text = (ctx != CTX_NORMAL) ? input + 1 : input; // Skip prefix

        // Handle commands
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            running = 0;
        } else if (strcmp(input, "help") == 0) {
            add_output_line("Available commands:", CTX_NORMAL);
            add_output_line("  help    - Show this help", CTX_NORMAL);
            add_output_line("  clear   - Clear screen", CTX_NORMAL);
            add_output_line("  quit    - Exit program", CTX_NORMAL);
            add_output_line("  !cmd    - System command context", CTX_NORMAL);
            add_output_line("  #text   - Comment context", CTX_NORMAL);
            add_output_line("  @action - Special action context", CTX_NORMAL);
        } else if (strcmp(input, "clear") == 0) {
            // Clear output buffer
            for (int i = 0; i < output.count; i++) {
                free(output.lines[i]);
            }
            output.count = 0;
        } else {
            // Echo input with context
            char echo_buf[512];
            snprintf(echo_buf, sizeof(echo_buf), "Echo [%s]: %s",
                    ctx == CTX_COMMAND ? "CMD" :
                    ctx == CTX_COMMENT ? "COMMENT" :
                    ctx == CTX_SPECIAL ? "SPECIAL" :
                    ctx == CTX_SEARCH ? "SEARCH" : "NORMAL",
                    text);
            add_output_line(echo_buf, ctx);
        }

        cmd_count++;

        // Update display
        render_output();

        char status_right[64];
        snprintf(status_right, sizeof(status_right), "Commands: %d", cmd_count);
        render_status("SmartTerm POC", status_right);

        free(input);
    }

    cleanup_ui();

    // Cleanup output buffer
    for (int i = 0; i < output.count; i++) {
        free(output.lines[i]);
    }
    free(output.lines);

    printf("SmartTerm POC exited cleanly.\n");
    return 0;
}
