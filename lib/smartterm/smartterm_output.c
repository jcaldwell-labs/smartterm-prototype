/*
 * SmartTerm Library - Output Buffer Implementation
 *
 * Thread-safe output buffer with metadata.
 */

#include "smartterm_internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/*
 * Initialize output buffer
 */
int output_buffer_init(output_buffer_t *buf, int capacity, bool thread_safe) {
    buf->lines = calloc(capacity, sizeof(output_line_t));
    if (!buf->lines) {
        return SMARTTERM_NOMEM;
    }

    buf->capacity = capacity;
    buf->count = 0;
    buf->scroll_offset = 0;
    buf->auto_scroll = true;

    if (thread_safe) {
        if (pthread_mutex_init(&buf->mutex, NULL) != 0) {
            free(buf->lines);
            return SMARTTERM_ERROR;
        }
    }

    return SMARTTERM_OK;
}

/*
 * Cleanup output buffer
 */
void output_buffer_cleanup(output_buffer_t *buf) {
    if (!buf || !buf->lines) {
        return;
    }

    /* Free all lines */
    for (int i = 0; i < buf->count; i++) {
        free(buf->lines[i].text);
        free((void*)buf->lines[i].meta.tag);
    }

    free(buf->lines);
    pthread_mutex_destroy(&buf->mutex);
}

/*
 * Add line to output buffer
 */
int output_buffer_add(output_buffer_t *buf, const char *text,
                      const smartterm_line_meta_t *meta) {
    if (!buf || !text) {
        return SMARTTERM_INVALID;
    }

    pthread_mutex_lock(&buf->mutex);

    /* If buffer is full, drop oldest line */
    if (buf->count >= buf->capacity) {
        free(buf->lines[0].text);
        free((void*)buf->lines[0].meta.tag);

        /* Shift all lines down */
        memmove(&buf->lines[0], &buf->lines[1],
                sizeof(output_line_t) * (buf->capacity - 1));
        buf->count--;

        /* Adjust scroll offset */
        if (buf->scroll_offset > 0) {
            buf->scroll_offset--;
        }
    }

    /* Add new line */
    buf->lines[buf->count].text = strdup_safe(text);
    if (!buf->lines[buf->count].text) {
        pthread_mutex_unlock(&buf->mutex);
        return SMARTTERM_NOMEM;
    }

    /* Set metadata */
    if (meta) {
        buf->lines[buf->count].meta = *meta;
        if (meta->tag) {
            buf->lines[buf->count].meta.tag = strdup_safe(meta->tag);
            /* Check for allocation failure */
            if (!buf->lines[buf->count].meta.tag) {
                free(buf->lines[buf->count].text);
                pthread_mutex_unlock(&buf->mutex);
                return SMARTTERM_NOMEM;
            }
        } else {
            buf->lines[buf->count].meta.tag = NULL;
        }
    } else {
        buf->lines[buf->count].meta.context = CTX_NORMAL;
        buf->lines[buf->count].meta.timestamp = get_timestamp();
        buf->lines[buf->count].meta.tag = NULL;
    }

    buf->count++;

    /* Auto-scroll to bottom if enabled */
    if (buf->auto_scroll) {
        buf->scroll_offset = 0;
    }

    pthread_mutex_unlock(&buf->mutex);
    return SMARTTERM_OK;
}

/*
 * Clear output buffer
 */
void output_buffer_clear(output_buffer_t *buf) {
    if (!buf) {
        return;
    }

    pthread_mutex_lock(&buf->mutex);

    for (int i = 0; i < buf->count; i++) {
        free(buf->lines[i].text);
        free((void*)buf->lines[i].meta.tag);
    }

    buf->count = 0;
    buf->scroll_offset = 0;

    pthread_mutex_unlock(&buf->mutex);
}

/*
 * Get line from buffer
 */
const char* output_buffer_get_line(output_buffer_t *buf, int index) {
    if (!buf || index < 0 || index >= buf->count) {
        return NULL;
    }

    pthread_mutex_lock(&buf->mutex);
    const char *text = buf->lines[index].text;
    pthread_mutex_unlock(&buf->mutex);

    return text;
}

/*
 * Get line metadata
 */
int output_buffer_get_line_meta(output_buffer_t *buf, int index,
                                smartterm_line_meta_t *meta) {
    if (!buf || !meta || index < 0 || index >= buf->count) {
        return SMARTTERM_INVALID;
    }

    pthread_mutex_lock(&buf->mutex);
    *meta = buf->lines[index].meta;
    pthread_mutex_unlock(&buf->mutex);

    return SMARTTERM_OK;
}

/*
 * Write line to output
 */
int smartterm_write(smartterm_ctx *ctx, const char *text,
                    smartterm_context_t context) {
    if (!ctx || !ctx->initialized || !text) {
        return SMARTTERM_INVALID;
    }

    smartterm_line_meta_t meta = {
        .context = context,
        .timestamp = get_timestamp(),
        .tag = NULL
    };

    int result = output_buffer_add(&ctx->buffer, text, &meta);
    if (result == SMARTTERM_OK) {
        result = render_output(ctx);
    }

    ctx->last_error = result;
    return result;
}

/*
 * Write formatted output
 */
int smartterm_write_fmt(smartterm_ctx *ctx, smartterm_context_t context,
                        const char *format, ...) {
    if (!ctx || !ctx->initialized || !format) {
        return SMARTTERM_INVALID;
    }

    char buffer[MAX_LINE_LENGTH];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, MAX_LINE_LENGTH, format, args);
    va_end(args);

    return smartterm_write(ctx, buffer, context);
}

/*
 * Write with metadata
 */
int smartterm_write_meta(smartterm_ctx *ctx, const char *text,
                         const smartterm_line_meta_t *meta) {
    if (!ctx || !ctx->initialized || !text || !meta) {
        return SMARTTERM_INVALID;
    }

    int result = output_buffer_add(&ctx->buffer, text, meta);
    if (result == SMARTTERM_OK) {
        result = render_output(ctx);
    }

    ctx->last_error = result;
    return result;
}
