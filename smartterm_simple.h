/*
 * SmartTerm Simple Library
 *
 * A minimal terminal UI library extracted from the SmartTerm POC.
 * Provides scrolling output, context-aware coloring, and status bar.
 *
 * Usage:
 *   1. Call smartterm_init() to initialize
 *   2. Add output with smartterm_add_output()
 *   3. Read input with smartterm_readline()
 *   4. Update status with smartterm_set_status()
 *   5. Call smartterm_cleanup() when done
 */

#ifndef SMARTTERM_SIMPLE_H
#define SMARTTERM_SIMPLE_H

// Context types for output coloring
typedef enum {
    SMARTTERM_CTX_NORMAL,    // Default white
    SMARTTERM_CTX_COMMAND,   // Yellow (!)
    SMARTTERM_CTX_COMMENT,   // Green (#)
    SMARTTERM_CTX_SPECIAL,   // Cyan (@)
    SMARTTERM_CTX_SEARCH     // Magenta (/)
} smartterm_context_t;

/*
 * Initialize the SmartTerm UI.
 * Must be called before any other smartterm functions.
 *
 * Returns: 0 on success, -1 on failure
 */
int smartterm_init(void);

/*
 * Cleanup and restore terminal.
 * Call this before exiting your program.
 */
void smartterm_cleanup(void);

/*
 * Add a line to the output buffer.
 * The line will be displayed with colors based on context.
 *
 * text: The text to display
 * ctx: Context type for coloring
 */
void smartterm_add_output(const char *text, smartterm_context_t ctx);

/*
 * Read a line of input from the user.
 * Uses readline for editing, history, and multi-line support.
 *
 * prompt: The prompt string to display
 *
 * Returns: Pointer to input string (caller must free), or NULL on EOF
 */
char* smartterm_readline(const char *prompt);

/*
 * Update the status bar.
 *
 * left: Text to display on the left side
 * right: Text to display on the right side
 */
void smartterm_set_status(const char *left, const char *right);

/*
 * Clear the output buffer.
 * Removes all lines from the scrolling output area.
 */
void smartterm_clear_output(void);

/*
 * Detect context from input string.
 * Helper function to identify context based on first character.
 *
 * input: The input string to analyze
 *
 * Returns: Detected context type
 */
smartterm_context_t smartterm_detect_context(const char *input);

/*
 * Get text without context prefix.
 * If input starts with context marker (!, #, @, /), returns pointer
 * to the text after the marker. Otherwise returns the original input.
 *
 * input: The input string
 *
 * Returns: Pointer to text (no need to free, points into input)
 */
const char* smartterm_strip_context(const char *input);

#endif // SMARTTERM_SIMPLE_H
