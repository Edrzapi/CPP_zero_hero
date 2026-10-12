/* ============================================================
 * cstack.c: Implementation of the C Stack Library
 * ============================================================
 * Compiled as C (note the .c extension). Manual memory
 * management with malloc/realloc/free and status codes.
 * ============================================================ */

#include "cstack.h"

#include <stdlib.h>

/* The real layout, invisible to users of the header. */
struct cstack {
    int* items;
    size_t size;
    size_t capacity;
};

cstack* cstack_create(size_t initial_capacity)
{
    cstack* stack = malloc(sizeof *stack);   /* valid C: void* converts implicitly */
    if (stack == NULL) {
        return NULL;
    }
    if (initial_capacity == 0) {
        initial_capacity = 1;
    }
    stack->items = malloc(initial_capacity * sizeof *stack->items);
    if (stack->items == NULL) {
        free(stack);
        return NULL;
    }
    stack->size = 0;
    stack->capacity = initial_capacity;
    return stack;
}

void cstack_destroy(cstack* stack)
{
    if (stack == NULL) {
        return;
    }
    free(stack->items);
    free(stack);
}

cstack_status cstack_push(cstack* stack, int value)
{
    if (stack == NULL) {
        return CSTACK_NULL_ARGUMENT;
    }
    if (stack->size == stack->capacity) {
        size_t new_capacity = stack->capacity * 2;
        int* grown = realloc(stack->items, new_capacity * sizeof *grown);
        if (grown == NULL) {
            return CSTACK_NO_MEMORY;   /* the old block is still valid */
        }
        stack->items = grown;
        stack->capacity = new_capacity;
    }
    stack->items[stack->size++] = value;
    return CSTACK_OK;
}

cstack_status cstack_pop(cstack* stack, int* out_value)
{
    if (stack == NULL || out_value == NULL) {
        return CSTACK_NULL_ARGUMENT;
    }
    if (stack->size == 0) {
        return CSTACK_EMPTY;
    }
    *out_value = stack->items[--stack->size];
    return CSTACK_OK;
}

size_t cstack_size(const cstack* stack)
{
    return stack == NULL ? 0 : stack->size;
}

const char* cstack_status_message(cstack_status status)
{
    switch (status) {
    case CSTACK_OK:            return "ok";
    case CSTACK_EMPTY:         return "stack is empty";
    case CSTACK_NO_MEMORY:     return "out of memory";
    case CSTACK_NULL_ARGUMENT: return "null argument";
    }
    return "unknown status";
}
