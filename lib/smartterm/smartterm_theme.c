/*
 * SmartTerm Library - Theme Implementation
 *
 * Color themes and styling.
 */

#include "smartterm_internal.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

/* Default theme singleton */
static smartterm_theme g_default_theme = {0};
static bool g_default_theme_initialized = false;

/*
 * Initialize default theme
 */
static void init_default_theme(void)
{
    if (g_default_theme_initialized) {
        return;
    }

    strcpy(g_default_theme.name, "default");
    g_default_theme.is_builtin = true;

    /* Set default colors (index 0 = fg, 1 = bg) */
    g_default_theme.colors[CTX_NORMAL][0] = COLOR_WHITE;
    g_default_theme.colors[CTX_NORMAL][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_ERROR][0] = COLOR_RED;
    g_default_theme.colors[CTX_ERROR][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_WARNING][0] = COLOR_YELLOW;
    g_default_theme.colors[CTX_WARNING][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_SUCCESS][0] = COLOR_GREEN;
    g_default_theme.colors[CTX_SUCCESS][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_INFO][0] = COLOR_CYAN;
    g_default_theme.colors[CTX_INFO][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_DEBUG][0] = COLOR_MAGENTA;
    g_default_theme.colors[CTX_DEBUG][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_COMMAND][0] = COLOR_YELLOW;
    g_default_theme.colors[CTX_COMMAND][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_COMMENT][0] = COLOR_GREEN;
    g_default_theme.colors[CTX_COMMENT][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_SPECIAL][0] = COLOR_CYAN;
    g_default_theme.colors[CTX_SPECIAL][1] = COLOR_BLACK;

    g_default_theme.colors[CTX_SEARCH][0] = COLOR_MAGENTA;
    g_default_theme.colors[CTX_SEARCH][1] = COLOR_BLACK;

    /* Set default attributes */
    for (int i = 0; i < CTX_USER_START; i++) {
        g_default_theme.attributes[i] = A_NORMAL;
    }
    g_default_theme.attributes[CTX_ERROR] = A_BOLD;

    /* Set default symbols */
    g_default_theme.symbols[SYM_PROMPT] = strdup_safe("> ");
    g_default_theme.symbols[SYM_MULTILINE_PROMPT] = strdup_safe("... ");
    g_default_theme.symbols[SYM_STATUS_SEP] = strdup_safe(" | ");
    g_default_theme.symbols[SYM_SCROLL_INDICATOR] = strdup_safe(" [SCROLL] ");
    g_default_theme.symbols[SYM_SEARCH_MATCH] = strdup_safe(">");

    g_default_theme_initialized = true;
}

/*
 * Get default theme
 */
const smartterm_theme* theme_get_default(void)
{
    init_default_theme();
    return &g_default_theme;
}

/*
 * Apply theme colors to ncurses
 */
int theme_apply_colors(smartterm_ctx* ctx)
{
    if (!ctx || !ctx->theme) {
        return SMARTTERM_INVALID;
    }

    if (!has_colors()) {
        return SMARTTERM_OK; /* No color support, skip */
    }

    /* Initialize color pairs for each context */
    for (int i = 0; i < CTX_USER_START; i++) {
        init_pair(i + 1, /* Color pairs start at 1 */
                  ctx->theme->colors[i][0], ctx->theme->colors[i][1]);
    }

    return SMARTTERM_OK;
}

/*
 * Create new theme
 */
smartterm_theme* smartterm_theme_create(const char* name)
{
    if (!name) {
        return NULL;
    }

    smartterm_theme* theme = calloc(1, sizeof(smartterm_theme));
    if (!theme) {
        return NULL;
    }

    strncpy(theme->name, name, MAX_THEME_NAME - 1);
    theme->name[MAX_THEME_NAME - 1] = '\0';
    theme->is_builtin = false;

    /* Copy default colors */
    init_default_theme();
    memcpy(theme->colors, g_default_theme.colors, sizeof(theme->colors));
    memcpy(theme->attributes, g_default_theme.attributes, sizeof(theme->attributes));

    /* Copy default symbols */
    for (int i = 0; i < SYM_COUNT; i++) {
        theme->symbols[i] = strdup_safe(g_default_theme.symbols[i]);
    }

    return theme;
}

/*
 * Free theme
 */
void smartterm_theme_free(smartterm_theme* theme)
{
    if (!theme || theme->is_builtin) {
        return;
    }

    for (int i = 0; i < SYM_COUNT; i++) {
        free(theme->symbols[i]);
    }

    free(theme);
}

/*
 * Set color for context
 */
int smartterm_theme_set_color(smartterm_theme* theme, smartterm_context_t context, int fg, int bg)
{
    if (!theme || context < 0 || context >= CTX_USER_START) {
        return SMARTTERM_INVALID;
    }

    theme->colors[context][0] = fg;
    theme->colors[context][1] = bg;

    return SMARTTERM_OK;
}

/*
 * Set symbol
 */
int smartterm_theme_set_symbol(smartterm_theme* theme, smartterm_symbol_t symbol, const char* value)
{
    if (!theme || !value || symbol < 0 || symbol >= SYM_COUNT) {
        return SMARTTERM_INVALID;
    }

    free(theme->symbols[symbol]);
    theme->symbols[symbol] = strdup_safe(value);

    return SMARTTERM_OK;
}

/*
 * Set attribute for context
 */
int smartterm_theme_set_attribute(smartterm_theme* theme, smartterm_context_t context,
                                  int attribute)
{
    if (!theme || context < 0 || context >= CTX_USER_START) {
        return SMARTTERM_INVALID;
    }

    theme->attributes[context] = attribute;

    return SMARTTERM_OK;
}

/*
 * Get built-in theme
 */
const smartterm_theme* smartterm_theme_get_builtin(const char* name)
{
    if (!name) {
        return NULL;
    }

    init_default_theme();

    if (strcmp(name, "default") == 0) {
        return &g_default_theme;
    }

    /* Additional built-in themes can be added here */

    return NULL;
}

/*
 * Set active theme
 */
int smartterm_set_theme(smartterm_ctx* ctx, const smartterm_theme* theme)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    /* Free old theme if owned */
    if (ctx->owns_theme && ctx->theme) {
        smartterm_theme_free((smartterm_theme*)ctx->theme);
    }

    if (theme) {
        ctx->theme = theme;
        ctx->owns_theme = false;
    } else {
        ctx->theme = theme_get_default();
        ctx->owns_theme = false;
    }

    /* Apply colors */
    theme_apply_colors(ctx);

    /* Re-render */
    return render_all(ctx);
}
