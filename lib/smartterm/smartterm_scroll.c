/*
 * SmartTerm Library - Scrollback Implementation
 *
 * Scrollback navigation and control.
 */

#include "smartterm_internal.h"
#include <stdio.h>

/*
 * Scroll output buffer
 */
int smartterm_scroll(smartterm_ctx* ctx, int lines)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);

    /* Calculate new scroll offset */
    int new_offset = ctx->buffer.scroll_offset + lines;

    /* Clamp to valid range */
    if (new_offset < 0) {
        new_offset = 0;
    }

    int max_offset = ctx->buffer.count;
    if (new_offset > max_offset) {
        new_offset = max_offset;
    }

    ctx->buffer.scroll_offset = new_offset;

    /* Disable auto-scroll if scrolled up */
    if (new_offset > 0) {
        ctx->buffer.auto_scroll = false;
    }

    pthread_mutex_unlock(&ctx->buffer.mutex);

    /* Update status bar with scroll indicator */
    if (ctx->buffer.scroll_offset > 0) {
        char status_right[MAX_STATUS_TEXT];
        snprintf(status_right, MAX_STATUS_TEXT, "[SCROLL: -%d]", ctx->buffer.scroll_offset);
        smartterm_status_set(ctx, NULL, status_right);
    } else {
        smartterm_status_set(ctx, NULL, "");
    }

    return render_output(ctx);
}

/*
 * Scroll to top
 */
int smartterm_scroll_top(smartterm_ctx* ctx)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);
    ctx->buffer.scroll_offset = ctx->buffer.count;
    ctx->buffer.auto_scroll = false;
    pthread_mutex_unlock(&ctx->buffer.mutex);

    return smartterm_scroll(ctx, 0); /* Trigger render and status update */
}

/*
 * Scroll to bottom
 */
int smartterm_scroll_bottom(smartterm_ctx* ctx)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);
    ctx->buffer.scroll_offset = 0;
    ctx->buffer.auto_scroll = true;
    pthread_mutex_unlock(&ctx->buffer.mutex);

    /* Clear scroll indicator */
    smartterm_status_set(ctx, NULL, "");

    return render_output(ctx);
}

/*
 * Get current scroll position
 */
int smartterm_get_scroll_pos(smartterm_ctx* ctx)
{
    if (!ctx || !ctx->initialized) {
        return 0;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);
    int pos = ctx->buffer.count - ctx->buffer.scroll_offset;
    pthread_mutex_unlock(&ctx->buffer.mutex);

    return pos;
}

/*
 * Enable/disable auto-scroll
 */
int smartterm_set_auto_scroll(smartterm_ctx* ctx, bool enabled)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);
    ctx->buffer.auto_scroll = enabled;

    /* If enabling and not at bottom, scroll to bottom */
    if (enabled && ctx->buffer.scroll_offset != 0) {
        ctx->buffer.scroll_offset = 0;
    }
    pthread_mutex_unlock(&ctx->buffer.mutex);

    return render_output(ctx);
}
