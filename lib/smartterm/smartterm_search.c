/*
 * SmartTerm Library - Search Implementation
 *
 * Search functionality for output buffer.
 */

#include "smartterm_internal.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/*
 * Search in output buffer (plain text)
 */
static int search_plain(smartterm_ctx *ctx, const char *pattern,
                        smartterm_search_result_t **results, int *count) {
    pthread_mutex_lock(&ctx->buffer.mutex);

    int match_count = 0;
    int match_capacity = 10;
    smartterm_search_result_t *matches = calloc(match_capacity,
                                                 sizeof(smartterm_search_result_t));

    if (!matches) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        return SMARTTERM_NOMEM;
    }

    /* Search each line */
    for (int i = 0; i < ctx->buffer.count; i++) {
        const char *line = ctx->buffer.lines[i].text;
        const char *pos = line;

        /* Find all occurrences in this line */
        while ((pos = strstr(pos, pattern)) != NULL) {
            /* Expand array if needed */
            if (match_count >= match_capacity) {
                match_capacity *= 2;
                smartterm_search_result_t *new_matches =
                    realloc(matches, match_capacity * sizeof(smartterm_search_result_t));
                if (!new_matches) {
                    free(matches);
                    pthread_mutex_unlock(&ctx->buffer.mutex);
                    return SMARTTERM_NOMEM;
                }
                matches = new_matches;
            }

            /* Add match */
            matches[match_count].line_index = i;
            matches[match_count].column = pos - line;
            matches[match_count].length = strlen(pattern);
            match_count++;

            pos += strlen(pattern);
        }
    }

    pthread_mutex_unlock(&ctx->buffer.mutex);

    *results = matches;
    *count = match_count;

    return SMARTTERM_OK;
}

/*
 * Search in output buffer (regex)
 */
static int search_regex(smartterm_ctx *ctx, const char *pattern,
                        smartterm_search_result_t **results, int *count) {
    regex_t regex;
    int ret = regcomp(&regex, pattern, REG_EXTENDED);
    if (ret != 0) {
        return SMARTTERM_INVALID;
    }

    pthread_mutex_lock(&ctx->buffer.mutex);

    int match_count = 0;
    int match_capacity = 10;
    smartterm_search_result_t *matches = calloc(match_capacity,
                                                 sizeof(smartterm_search_result_t));

    if (!matches) {
        pthread_mutex_unlock(&ctx->buffer.mutex);
        regfree(&regex);
        return SMARTTERM_NOMEM;
    }

    /* Search each line */
    for (int i = 0; i < ctx->buffer.count; i++) {
        const char *line = ctx->buffer.lines[i].text;
        const char *search_pos = line;
        regmatch_t match;

        /* Find all matches in this line (not just first) */
        while (regexec(&regex, search_pos, 1, &match, 0) == 0) {
            /* Expand array if needed */
            if (match_count >= match_capacity) {
                match_capacity *= 2;
                smartterm_search_result_t *new_matches =
                    realloc(matches, match_capacity * sizeof(smartterm_search_result_t));
                if (!new_matches) {
                    free(matches);
                    pthread_mutex_unlock(&ctx->buffer.mutex);
                    regfree(&regex);
                    return SMARTTERM_NOMEM;
                }
                matches = new_matches;
            }

            /* Add match (adjust column for offset from start of line) */
            matches[match_count].line_index = i;
            matches[match_count].column = (search_pos - line) + match.rm_so;
            matches[match_count].length = match.rm_eo - match.rm_so;
            match_count++;

            /* Move past this match to find next one */
            search_pos += match.rm_eo;
            if (*search_pos == '\0') break;  /* End of line */
        }
    }

    pthread_mutex_unlock(&ctx->buffer.mutex);
    regfree(&regex);

    *results = matches;
    *count = match_count;

    return SMARTTERM_OK;
}

/*
 * Search in output buffer
 */
int smartterm_search(smartterm_ctx *ctx, const char *pattern, bool use_regex,
                     smartterm_search_result_t **results, int *count) {
    if (!ctx || !ctx->initialized || !pattern || !results || !count) {
        return SMARTTERM_INVALID;
    }

    /* Free previous search results */
    free(ctx->search.pattern);
    free(ctx->search.results);

    /* Perform search */
    int ret;
    if (use_regex) {
        ret = search_regex(ctx, pattern, results, count);
    } else {
        ret = search_plain(ctx, pattern, results, count);
    }

    if (ret == SMARTTERM_OK) {
        /* Save search state */
        ctx->search.pattern = strdup_safe(pattern);
        ctx->search.use_regex = use_regex;
        ctx->search.results = *results;
        ctx->search.result_count = *count;
        ctx->search.current_result = (*count > 0) ? 0 : -1;
    }

    return ret;
}

/*
 * Free search results
 */
void smartterm_free_search_results(smartterm_search_result_t *results) {
    free(results);
}

/*
 * Jump to next search match
 */
int smartterm_search_next(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    if (ctx->search.result_count == 0 || ctx->search.current_result < 0) {
        return SMARTTERM_ERROR;
    }

    /* Move to next match */
    ctx->search.current_result = (ctx->search.current_result + 1) %
                                  ctx->search.result_count;

    /* Scroll to show the match */
    int line = ctx->search.results[ctx->search.current_result].line_index;
    ctx->buffer.scroll_offset = ctx->buffer.count - line - 1;
    if (ctx->buffer.scroll_offset < 0) {
        ctx->buffer.scroll_offset = 0;
    }

    return render_output(ctx);
}

/*
 * Jump to previous search match
 */
int smartterm_search_prev(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    if (ctx->search.result_count == 0 || ctx->search.current_result < 0) {
        return SMARTTERM_ERROR;
    }

    /* Move to previous match */
    ctx->search.current_result--;
    if (ctx->search.current_result < 0) {
        ctx->search.current_result = ctx->search.result_count - 1;
    }

    /* Scroll to show the match */
    int line = ctx->search.results[ctx->search.current_result].line_index;
    ctx->buffer.scroll_offset = ctx->buffer.count - line - 1;
    if (ctx->buffer.scroll_offset < 0) {
        ctx->buffer.scroll_offset = 0;
    }

    return render_output(ctx);
}

/*
 * Clear search highlights
 */
int smartterm_search_clear(smartterm_ctx *ctx) {
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    free(ctx->search.pattern);
    free(ctx->search.results);

    ctx->search.pattern = NULL;
    ctx->search.results = NULL;
    ctx->search.result_count = 0;
    ctx->search.current_result = -1;

    return render_output(ctx);
}
