/*
 * SmartTerm Library - Key Handler Implementation
 *
 * Custom key handler registration and management.
 */

#include "smartterm_internal.h"
#include <stdlib.h>

/*
 * Register key handler
 */
int smartterm_register_key_handler(smartterm_ctx* ctx, int key, smartterm_key_handler_fn handler,
                                   void* data)
{
    if (!ctx || !ctx->initialized || !handler) {
        return SMARTTERM_INVALID;
    }

    /* Check if we need to expand array */
    if (ctx->key_handler_count >= ctx->key_handler_capacity) {
        int new_capacity = ctx->key_handler_capacity * 2;
        key_handler_entry_t* new_handlers =
            realloc(ctx->key_handlers, new_capacity * sizeof(key_handler_entry_t));
        if (!new_handlers) {
            return SMARTTERM_NOMEM;
        }
        ctx->key_handlers = new_handlers;
        ctx->key_handler_capacity = new_capacity;
    }

    /* Check if key already registered (update handler) */
    for (int i = 0; i < ctx->key_handler_count; i++) {
        if (ctx->key_handlers[i].key == key) {
            ctx->key_handlers[i].handler = handler;
            ctx->key_handlers[i].user_data = data;
            return SMARTTERM_OK;
        }
    }

    /* Add new handler */
    ctx->key_handlers[ctx->key_handler_count].key = key;
    ctx->key_handlers[ctx->key_handler_count].handler = handler;
    ctx->key_handlers[ctx->key_handler_count].user_data = data;
    ctx->key_handler_count++;

    return SMARTTERM_OK;
}

/*
 * Unregister key handler
 */
int smartterm_unregister_key_handler(smartterm_ctx* ctx, int key)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    /* Find and remove handler */
    for (int i = 0; i < ctx->key_handler_count; i++) {
        if (ctx->key_handlers[i].key == key) {
            /* Shift remaining handlers down */
            for (int j = i; j < ctx->key_handler_count - 1; j++) {
                ctx->key_handlers[j] = ctx->key_handlers[j + 1];
            }
            ctx->key_handler_count--;
            return SMARTTERM_OK;
        }
    }

    return SMARTTERM_ERROR; /* Key not found */
}

/*
 * Dispatch key event to registered handler (internal use)
 */
int smartterm_dispatch_key(smartterm_ctx* ctx, int key)
{
    if (!ctx || !ctx->initialized) {
        return SMARTTERM_NOTINIT;
    }

    /* Find handler for key */
    for (int i = 0; i < ctx->key_handler_count; i++) {
        if (ctx->key_handlers[i].key == key) {
            ctx->key_handlers[i].handler(ctx, key, ctx->key_handlers[i].user_data);
            return SMARTTERM_OK;
        }
    }

    return SMARTTERM_ERROR; /* No handler */
}
