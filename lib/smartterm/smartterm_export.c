/*
 * SmartTerm Library - Export Implementation
 *
 * Export output buffer to various formats.
 */

#include "smartterm_internal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/*
 * Get ANSI color code for context
 */
static const char* get_ansi_color(smartterm_context_t context) {
    switch (context) {
        case CTX_ERROR:
            return "\033[1;31m";  /* Bold red */
        case CTX_WARNING:
            return "\033[1;33m";  /* Bold yellow */
        case CTX_SUCCESS:
            return "\033[1;32m";  /* Bold green */
        case CTX_INFO:
            return "\033[1;36m";  /* Bold cyan */
        case CTX_DEBUG:
            return "\033[1;35m";  /* Bold magenta */
        case CTX_COMMAND:
            return "\033[0;33m";  /* Yellow */
        case CTX_COMMENT:
            return "\033[0;32m";  /* Green */
        case CTX_SPECIAL:
            return "\033[0;36m";  /* Cyan */
        case CTX_SEARCH:
            return "\033[0;35m";  /* Magenta */
        default:
            return "\033[0m";     /* Reset */
    }
}

/*
 * Export to plain text format
 */
static char* export_plain(smartterm_ctx *ctx, int start_line, int end_line,
                          bool include_meta) {
    pthread_mutex_lock(&ctx->buffer.mutex);

    /* Calculate buffer size */
    size_t buffer_size = 0;
    for (int i = start_line; i <= end_line; i++) {
        buffer_size += strlen(ctx->buffer.lines[i].text) + 1;  /* Text + newline */
        if (include_meta) {
            /* Timestamp format: "[YYYY-MM-DD HH:MM:SS] " = 23 chars */
            buffer_size += 25;  /* Timestamp with some margin */
        }
    }
    buffer_size += 1;  /* Null terminator */

    char *output = malloc(buffer_size);
    if (!output) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        return NULL;
    }

    char *ptr = output;
    for (int i = start_line; i <= end_line; i++) {
        if (include_meta) {
            struct tm *tm_info = localtime(&ctx->buffer.lines[i].meta.timestamp);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            ptr += sprintf(ptr, "[%s] ", time_str);
        }

        ptr += sprintf(ptr, "%s\n", ctx->buffer.lines[i].text);
    }

    pthread_mutex_unlock(&ctx->buffer.mutex);
    return output;
}

/*
 * Export to ANSI format
 */
static char* export_ansi(smartterm_ctx *ctx, int start_line, int end_line,
                         bool include_meta) {
    pthread_mutex_lock(&ctx->buffer.mutex);

    /* Calculate buffer size (ANSI codes add ~10-20 chars per line) */
    size_t buffer_size = 0;
    for (int i = start_line; i <= end_line; i++) {
        buffer_size += strlen(ctx->buffer.lines[i].text) + 50;
        if (include_meta) {
            buffer_size += 50;
        }
    }
    buffer_size += 1;

    char *output = malloc(buffer_size);
    if (!output) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        return NULL;
    }

    char *ptr = output;
    for (int i = start_line; i <= end_line; i++) {
        if (include_meta) {
            struct tm *tm_info = localtime(&ctx->buffer.lines[i].meta.timestamp);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            ptr += sprintf(ptr, "\033[2m[%s]\033[0m ", time_str);
        }

        const char *color = get_ansi_color(ctx->buffer.lines[i].meta.context);
        ptr += sprintf(ptr, "%s%s\033[0m\n", color,
                      ctx->buffer.lines[i].text);
    }

    pthread_mutex_unlock(&ctx->buffer.mutex);
    return output;
}

/*
 * Export to markdown format
 */
static char* export_markdown(smartterm_ctx *ctx, int start_line, int end_line,
                             bool include_meta) {
    pthread_mutex_lock(&ctx->buffer.mutex);

    /* Calculate buffer size */
    size_t buffer_size = 500;  /* Header */
    for (int i = start_line; i <= end_line; i++) {
        buffer_size += strlen(ctx->buffer.lines[i].text) + 100;
    }

    char *output = malloc(buffer_size);
    if (!output) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        return NULL;
    }

    char *ptr = output;
    ptr += sprintf(ptr, "# SmartTerm Export\n\n");

    if (include_meta) {
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        char time_str[20];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        ptr += sprintf(ptr, "**Export Date**: %s\n\n", time_str);
        ptr += sprintf(ptr, "**Lines**: %d-%d\n\n", start_line, end_line);
    }

    ptr += sprintf(ptr, "## Output\n\n```\n");

    for (int i = start_line; i <= end_line; i++) {
        ptr += sprintf(ptr, "%s\n", ctx->buffer.lines[i].text);
    }

    ptr += sprintf(ptr, "```\n");

    pthread_mutex_unlock(&ctx->buffer.mutex);
    return output;
}

/*
 * Export to HTML format
 */
static char* export_html(smartterm_ctx *ctx, int start_line, int end_line,
                         bool include_meta) {
    pthread_mutex_lock(&ctx->buffer.mutex);

    /* Calculate buffer size */
    size_t buffer_size = 1000;  /* HTML wrapper */
    for (int i = start_line; i <= end_line; i++) {
        buffer_size += strlen(ctx->buffer.lines[i].text) + 200;
    }

    char *output = malloc(buffer_size);
    if (!output) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        return NULL;
    }

    char *ptr = output;
    ptr += sprintf(ptr, "<!DOCTYPE html>\n<html>\n<head>\n");
    ptr += sprintf(ptr, "<title>SmartTerm Export</title>\n");
    ptr += sprintf(ptr, "<style>\n");
    ptr += sprintf(ptr, "body { background: #000; color: #fff; font-family: monospace; }\n");
    ptr += sprintf(ptr, ".error { color: #f00; font-weight: bold; }\n");
    ptr += sprintf(ptr, ".warning { color: #ff0; font-weight: bold; }\n");
    ptr += sprintf(ptr, ".success { color: #0f0; font-weight: bold; }\n");
    ptr += sprintf(ptr, ".info { color: #0ff; font-weight: bold; }\n");
    ptr += sprintf(ptr, ".meta { color: #888; font-size: 0.9em; }\n");
    ptr += sprintf(ptr, "</style>\n</head>\n<body>\n<pre>\n");

    for (int i = start_line; i <= end_line; i++) {
        const char *css_class = "";
        switch (ctx->buffer.lines[i].meta.context) {
            case CTX_ERROR: css_class = "error"; break;
            case CTX_WARNING: css_class = "warning"; break;
            case CTX_SUCCESS: css_class = "success"; break;
            case CTX_INFO: css_class = "info"; break;
            default: break;
        }

        if (include_meta) {
            struct tm *tm_info = localtime(&ctx->buffer.lines[i].meta.timestamp);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
            ptr += sprintf(ptr, "<span class=\"meta\">[%s]</span> ", time_str);
        }

        if (css_class[0]) {
            ptr += sprintf(ptr, "<span class=\"%s\">%s</span>\n",
                          css_class, ctx->buffer.lines[i].text);
        } else {
            ptr += sprintf(ptr, "%s\n", ctx->buffer.lines[i].text);
        }
    }

    ptr += sprintf(ptr, "</pre>\n</body>\n</html>\n");

    pthread_mutex_unlock(&ctx->buffer.mutex);
    return output;
}

/*
 * Export output buffer to string
 */
char* smartterm_export_string(smartterm_ctx *ctx,
                              smartterm_export_format_t format,
                              int start_line, int end_line, bool include_meta) {
    if (!ctx || !ctx->initialized) {
        return NULL;
    }

    /* Normalize line range */
    if (start_line < 0) {
        start_line = 0;
    }
    if (end_line < 0 || end_line >= ctx->buffer.count) {
        end_line = ctx->buffer.count - 1;
    }
    if (start_line > end_line) {
        return NULL;
    }

    /* Export based on format */
    switch (format) {
        case EXPORT_PLAIN:
            return export_plain(ctx, start_line, end_line, include_meta);
        case EXPORT_ANSI:
            return export_ansi(ctx, start_line, end_line, include_meta);
        case EXPORT_MARKDOWN:
            return export_markdown(ctx, start_line, end_line, include_meta);
        case EXPORT_HTML:
            return export_html(ctx, start_line, end_line, include_meta);
        default:
            return NULL;
    }
}

/*
 * Export output buffer to file
 */
int smartterm_export(smartterm_ctx *ctx, const char *filename,
                     smartterm_export_format_t format, int start_line,
                     int end_line, bool include_meta) {
    if (!ctx || !ctx->initialized || !filename) {
        return SMARTTERM_INVALID;
    }

    /* Export to string */
    char *content = smartterm_export_string(ctx, format, start_line, end_line,
                                            include_meta);
    if (!content) {
        return SMARTTERM_ERROR;
    }

    /* Write to file */
    FILE *f = fopen(filename, "w");
    if (!f) {
        free(content);
        return SMARTTERM_IOERROR;
    }

    fprintf(f, "%s", content);
    fclose(f);
    free(content);

    return SMARTTERM_OK;
}
