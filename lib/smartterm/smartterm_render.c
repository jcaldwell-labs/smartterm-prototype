/*
 * SmartTerm Library - Rendering Implementation
 *
 * Render output buffer and status bar to ncurses windows.
 */

#include "smartterm_internal.h"
#include <stdlib.h>
#include <string.h>

/*
 * Get color pair for context
 */
static int get_color_for_context(smartterm_ctx *ctx, smartterm_context_t context) {
    if (!ctx->theme) {
        return COLOR_PAIR(0);
    }

    if (context < 0 || context >= CTX_USER_START) {
        return COLOR_PAIR(0);
    }

    /* Color pairs are indexed from 1 */
    return COLOR_PAIR(context + 1);
}

/*
 * Get attribute for context
 */
static int get_attribute_for_context(smartterm_ctx *ctx,
                                      smartterm_context_t context) {
    if (!ctx->theme || context < 0 || context >= CTX_USER_START) {
        return A_NORMAL;
    }

    return ctx->theme->attributes[context];
}

/*
 * Render output buffer to window
 */
int render_output(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized || !ctx->output_win) {
        return SMARTTERM_NOTINIT;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);

    werase(ctx->output_win);
    box(ctx->output_win, 0, 0);

    int win_height, win_width;
    getmaxyx(ctx->output_win, win_height, win_width);

    /* Calculate visible lines (account for border) */
    int max_visible = win_height - 2;
    if (max_visible <= 0) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        wrefresh(ctx->output_win);
        return SMARTTERM_OK;
    }

    /* Calculate which lines to display */
    int start_line;
    if (ctx->buffer.scroll_offset == 0) {
        /* At bottom - show most recent lines */
        start_line = (ctx->buffer.count > max_visible) ?
                     (ctx->buffer.count - max_visible) : 0;
    } else {
        /* Scrolled up */
        start_line = ctx->buffer.count - ctx->buffer.scroll_offset - max_visible;
        if (start_line < 0) {
            start_line = 0;
        }
    }

    /* Render visible lines */
    int display_row = 1;  /* Start after border */
    for (int i = start_line; i < ctx->buffer.count && display_row <= max_visible; i++) {
        output_line_t *line = &ctx->buffer.lines[i];

        /* Apply color and attributes for context */
        int color = get_color_for_context(ctx, line->meta.context);
        int attr = get_attribute_for_context(ctx, line->meta.context);

        wattron(ctx->output_win, color | attr);

        /* Truncate line if too long for window */
        int max_width = win_width - 4;  /* Account for border and padding */
        if (max_width < 4) max_width = 4;  /* Minimum width */

        if (strlen(line->text) > (size_t)max_width) {
            /* Use dynamic allocation to avoid stack overflow on very wide terminals */
            char *truncated = malloc(max_width + 1);
            if (truncated) {
                strncpy(truncated, line->text, max_width - 3);
                truncated[max_width - 3] = '\0';
                strcat(truncated, "...");
                mvwprintw(ctx->output_win, display_row, 2, "%s", truncated);
                free(truncated);
            } else {
                /* Fallback: print what we can without truncation marker */
                mvwprintw(ctx->output_win, display_row, 2, "%.*s", max_width, line->text);
            }
        } else {
            mvwprintw(ctx->output_win, display_row, 2, "%s", line->text);
        }

        wattroff(ctx->output_win, color | attr);
        display_row++;
    }

    pthread_mutex_unlock(&ctx->buffer.mutex);

    wrefresh(ctx->output_win);
    return SMARTTERM_OK;
}

/*
 * Render status bar
 */
int render_status(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized || !ctx->status_win || !ctx->status_visible) {
        return SMARTTERM_OK;
    }

    werase(ctx->status_win);

    /* Render with reverse video for status bar effect */
    wattron(ctx->status_win, A_REVERSE);

    /* Fill entire line */
    for (int i = 0; i < ctx->term_cols; i++) {
        mvwaddch(ctx->status_win, 0, i, ' ');
    }

    /* Left-aligned text */
    mvwprintw(ctx->status_win, 0, 2, "%s", ctx->status_left);

    /* Right-aligned text */
    int right_len = strlen(ctx->status_right);
    int right_pos = ctx->term_cols - right_len - 2;
    if (right_pos > 0) {
        mvwprintw(ctx->status_win, 0, right_pos, "%s", ctx->status_right);
    }

    wattroff(ctx->status_win, A_REVERSE);

    wrefresh(ctx->status_win);
    return SMARTTERM_OK;
}

/*
 * Render all windows
 */
int render_all(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    int result = render_output(ctx);
    if (result == SMARTTERM_OK) {
        result = render_status(ctx);
    }

    return result;
}
