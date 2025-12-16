/*
 * SmartTerm Headless Demo
 *
 * Non-interactive demonstration that shows SmartTerm's features
 * and exports output in various formats.
 *
 * This is useful for:
 * - Generating documentation screenshots
 * - Automated testing
 * - CI/CD demonstrations
 * - README examples
 *
 * Build: see Makefile.lib
 * Run: ./build/bin/headless_demo [output_dir]
 */

#include <smartterm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Simulate user commands and demonstrate features */
void run_demo_sequence(smartterm_ctx *ctx)
{
    /* Welcome message */
    smartterm_write(ctx, "=== SmartTerm Headless Demo ===", CTX_INFO);
    smartterm_write(ctx, "This demo shows SmartTerm's features in non-interactive mode", CTX_NORMAL);
    smartterm_status_set(ctx, "SmartTerm Demo", "Running...");

    usleep(100000); /* Brief pause for effect */

    /* Demonstrate context types */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "1. Context-Aware Output:", CTX_INFO);
    smartterm_write(ctx, "Normal message - standard output", CTX_NORMAL);
    smartterm_write(ctx, "Info message - informational", CTX_INFO);
    smartterm_write(ctx, "Success message - operation completed", CTX_SUCCESS);
    smartterm_write(ctx, "Warning message - potential issue", CTX_WARNING);
    smartterm_write(ctx, "Error message - something failed", CTX_ERROR);
    smartterm_write(ctx, "Command output - system command result", CTX_COMMAND);
    smartterm_write(ctx, "Comment - user annotation", CTX_COMMENT);
    smartterm_write(ctx, "Special action - unique operation", CTX_SPECIAL);

    usleep(100000);

    /* Demonstrate formatted output */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "2. Formatted Output:", CTX_INFO);
    smartterm_write_fmt(ctx, CTX_NORMAL, "Formatted number: %d", 42);
    smartterm_write_fmt(ctx, CTX_SUCCESS, "Operation completed in %.2f seconds", 1.23);
    smartterm_write_fmt(ctx, CTX_INFO, "User: %s | Session: %s", "demo", "abc123");

    usleep(100000);

    /* Demonstrate command-like interaction */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "3. Simulated Command Execution:", CTX_INFO);
    smartterm_write(ctx, "> help", CTX_COMMAND);
    smartterm_write(ctx, "Available commands:", CTX_NORMAL);
    smartterm_write(ctx, "  help    - Show this help", CTX_NORMAL);
    smartterm_write(ctx, "  status  - Show system status", CTX_NORMAL);
    smartterm_write(ctx, "  calc    - Calculate expression", CTX_NORMAL);
    smartterm_write(ctx, "  quit    - Exit program", CTX_NORMAL);

    usleep(100000);

    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "> status", CTX_COMMAND);
    smartterm_write(ctx, "System Status:", CTX_INFO);
    smartterm_write_fmt(ctx, CTX_SUCCESS, "  CPU Usage:   %.1f%%", 23.4);
    smartterm_write_fmt(ctx, CTX_SUCCESS, "  Memory:      %d MB / %d MB", 4096, 8192);
    smartterm_write_fmt(ctx, CTX_SUCCESS, "  Uptime:      %d hours", 42);
    smartterm_status_set(ctx, "Demo", "Status Check");

    usleep(100000);

    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "> calc 2 + 2", CTX_COMMAND);
    smartterm_write_fmt(ctx, CTX_SUCCESS, "Result: %d", 4);

    usleep(100000);

    /* Demonstrate error handling */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "4. Error Handling:", CTX_INFO);
    smartterm_write(ctx, "> divide 10 0", CTX_COMMAND);
    smartterm_write(ctx, "ERROR: Division by zero", CTX_ERROR);
    smartterm_write(ctx, "# Division by zero caught and handled safely", CTX_COMMENT);

    usleep(100000);

    /* Demonstrate log-like output */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "5. Log-Style Output:", CTX_INFO);
    smartterm_write_fmt(ctx, CTX_INFO, "[%s] Application started", "2025-12-13 10:30:00");
    smartterm_write_fmt(ctx, CTX_SUCCESS, "[%s] Database connected", "2025-12-13 10:30:01");
    smartterm_write_fmt(ctx, CTX_INFO, "[%s] Loading configuration...", "2025-12-13 10:30:02");
    smartterm_write_fmt(ctx, CTX_SUCCESS, "[%s] Configuration loaded", "2025-12-13 10:30:02");
    smartterm_write_fmt(ctx, CTX_WARNING, "[%s] Cache miss - rebuilding index", "2025-12-13 10:30:03");
    smartterm_write_fmt(ctx, CTX_SUCCESS, "[%s] Server ready on port 8080", "2025-12-13 10:30:04");
    smartterm_status_set(ctx, "Demo", "Logging");

    usleep(100000);

    /* Demonstrate chat-like output */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "6. Chat-Style Interface:", CTX_INFO);
    smartterm_write(ctx, "Alice joined the channel", CTX_INFO);
    smartterm_write(ctx, "<Alice> Hello everyone!", CTX_NORMAL);
    smartterm_write(ctx, "Bob joined the channel", CTX_INFO);
    smartterm_write(ctx, "<Bob> Hey Alice!", CTX_NORMAL);
    smartterm_write(ctx, "<Alice> How's the SmartTerm library?", CTX_NORMAL);
    smartterm_write(ctx, "<Bob> It's great! Love the context coloring", CTX_NORMAL);
    smartterm_write(ctx, "* Alice sends a smile", CTX_SPECIAL);
    smartterm_status_set(ctx, "Chat", "2 users online");

    usleep(100000);

    /* Demonstrate multi-line content */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "7. Code-Like Output:", CTX_INFO);
    smartterm_write(ctx, "# Example function implementation", CTX_COMMENT);
    smartterm_write(ctx, "def calculate(x, y):", CTX_NORMAL);
    smartterm_write(ctx, "    result = x + y", CTX_NORMAL);
    smartterm_write(ctx, "    return result", CTX_NORMAL);
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "✓ Function defined successfully", CTX_SUCCESS);

    usleep(100000);

    /* Summary */
    smartterm_write(ctx, "", CTX_NORMAL);
    smartterm_write(ctx, "=== Demo Complete ===", CTX_SUCCESS);
    smartterm_write(ctx, "SmartTerm features demonstrated:", CTX_INFO);
    smartterm_write(ctx, "  ✓ Context-aware coloring (8 types)", CTX_SUCCESS);
    smartterm_write(ctx, "  ✓ Formatted output (printf-style)", CTX_SUCCESS);
    smartterm_write(ctx, "  ✓ Status bar updates", CTX_SUCCESS);
    smartterm_write(ctx, "  ✓ Clean scrolling buffer", CTX_SUCCESS);
    smartterm_write(ctx, "  ✓ No prompt duplication", CTX_SUCCESS);
    smartterm_write(ctx, "  ✓ Multiple UI patterns (REPL, chat, logs)", CTX_SUCCESS);

    smartterm_status_set(ctx, "Demo Complete", "Ready to export");
}

/* Export demo output in all formats */
void export_demo_output(smartterm_ctx *ctx, const char *output_dir)
{
    char filepath[512];

    printf("\nExporting demo output...\n");

    /* Export as plain text (no metadata) */
    snprintf(filepath, sizeof(filepath), "%s/demo_output.txt", output_dir);
    if (smartterm_export(ctx, filepath, EXPORT_PLAIN, 0, -1, false) == 0) {
        printf("  ✓ Plain text:  %s\n", filepath);
    } else {
        printf("  ✗ Failed to export plain text\n");
    }

    /* Export as ANSI (with colors, no metadata) */
    snprintf(filepath, sizeof(filepath), "%s/demo_output.ansi", output_dir);
    if (smartterm_export(ctx, filepath, EXPORT_ANSI, 0, -1, false) == 0) {
        printf("  ✓ ANSI format: %s\n", filepath);
    } else {
        printf("  ✗ Failed to export ANSI\n");
    }

    /* Export as Markdown (no metadata) */
    snprintf(filepath, sizeof(filepath), "%s/demo_output.md", output_dir);
    if (smartterm_export(ctx, filepath, EXPORT_MARKDOWN, 0, -1, false) == 0) {
        printf("  ✓ Markdown:    %s\n", filepath);
    } else {
        printf("  ✗ Failed to export Markdown\n");
    }

    /* Export as HTML (no metadata) */
    snprintf(filepath, sizeof(filepath), "%s/demo_output.html", output_dir);
    if (smartterm_export(ctx, filepath, EXPORT_HTML, 0, -1, false) == 0) {
        printf("  ✓ HTML:        %s\n", filepath);
    } else {
        printf("  ✗ Failed to export HTML\n");
    }

    printf("\nDemo output exported to: %s/\n", output_dir);
}

int main(int argc, char **argv)
{
    const char *output_dir = (argc > 1) ? argv[1] : "./demo_output";

    printf("SmartTerm Headless Demo\n");
    printf("=======================\n\n");
    printf("This demo runs SmartTerm in non-interactive mode\n");
    printf("to demonstrate features and export output.\n\n");

    /* Create output directory if needed */
    char mkdir_cmd[256];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", output_dir);
    if (system(mkdir_cmd) != 0) {
        fprintf(stderr, "Warning: Failed to create output directory\n");
    }

    /* Initialize SmartTerm with headless-friendly config */
    smartterm_config_t config = smartterm_default_config();
    config.max_lines = 1000;
    config.history_enabled = false;  /* No history in headless mode */
    config.status_bar_enabled = true;
    config.theme = NULL;  /* Use default theme */

    smartterm_ctx *ctx = smartterm_init(&config);
    if (!ctx) {
        fprintf(stderr, "ERROR: Failed to initialize SmartTerm\n");
        return 1;
    }

    /* Run the demo sequence */
    printf("Running demo sequence...\n");
    run_demo_sequence(ctx);
    printf("Demo sequence complete.\n");

    /* Export output in all formats */
    export_demo_output(ctx, output_dir);

    /* Cleanup */
    smartterm_cleanup(ctx);

    printf("\n✓ Demo complete!\n");
    printf("\nYou can view the exported files with:\n");
    printf("  cat %s/demo_output.txt       # Plain text\n", output_dir);
    printf("  cat %s/demo_output.ansi      # ANSI colored\n", output_dir);
    printf("  cat %s/demo_output.md        # Markdown\n", output_dir);
    printf("  open %s/demo_output.html     # HTML (in browser)\n", output_dir);

    return 0;
}
