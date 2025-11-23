/*
 * SmartTerm Library - Status Bar Implementation
 *
 * Status bar management.
 */

#include "smartterm_internal.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/*
 * Set status bar text
 */
int smartterm_status_set(smartterm_ctx* ctx, const char* left, const char* right)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    if (left) {
        strncpy(ctx->status_left, left, MAX_STATUS_TEXT - 1);
        ctx->status_left[MAX_STATUS_TEXT - 1] = '\0';
    }

    if (right) {
        strncpy(ctx->status_right, right, MAX_STATUS_TEXT - 1);
        ctx->status_right[MAX_STATUS_TEXT - 1] = '\0';
    }

    return render_status(ctx);
}

/*
 * Update status bar with formatted text
 *
 * NOTE: Due to variadic argument limitations, only ONE of left_fmt or right_fmt
 * should be a format string (non-NULL). The variadic arguments apply to whichever
 * one is non-NULL. To update both sides with formatting, call this function twice.
 */
int smartterm_status_update(smartterm_ctx* ctx, const char* left_fmt, const char* right_fmt, ...)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    va_list args;
    va_start(args, right_fmt);

    /* Apply variadic args to whichever format string is provided */
    if (left_fmt && !right_fmt) {
        /* Format left side only */
        vsnprintf(ctx->status_left, MAX_STATUS_TEXT, left_fmt, args);
    } else if (right_fmt && !left_fmt) {
        /* Format right side only */
        vsnprintf(ctx->status_right, MAX_STATUS_TEXT, right_fmt, args);
    } else if (left_fmt && right_fmt) {
        /* Both provided - this is an error condition, but handle gracefully */
        /* Apply args to left, use right as-is (no formatting) */
        vsnprintf(ctx->status_left, MAX_STATUS_TEXT, left_fmt, args);
        strncpy(ctx->status_right, right_fmt, MAX_STATUS_TEXT - 1);
        ctx->status_right[MAX_STATUS_TEXT - 1] = '\0';
    }

    va_end(args);

    return render_status(ctx);
}

/*
 * Show/hide status bar
 */
int smartterm_status_set_visible(smartterm_ctx* ctx, bool visible)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    ctx->status_visible = visible;

    /* Trigger resize to adjust window layouts */
    return smartterm_handle_resize(ctx);
}
