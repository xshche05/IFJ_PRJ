//
// Created by Spagetik on 22.10.2023.
//

#include "stack.h"


stack_t *stack_init() {
    stack_t *stack = (stack_t *) malloc(sizeof(stack_t));
    stack->top = NULL;
    return stack;
}

void stack_push(stack_t *stack, void *data) {
    stack_item_t *item = (stack_item_t *) malloc(sizeof(stack_item_t));
    item->data = data;
    item->down = stack->top;
    stack->top = item;
}

void stack_pop(stack_t *stack) {
    if (stack->top == NULL) {
        return;
    }
    stack_item_t *item = stack->top;
    stack->top = item->down;
    free(item);
}

void *stack_top(stack_t *stack) {
    if (stack == NULL) {
        return NULL;
    }
    if (stack->top == NULL) {
        return NULL;
    }
    return stack->top->data;
}

void stack_destroy(stack_t *stack) {
    while (stack->top != NULL) {
        stack_pop(stack);
    }
    free(stack);
}

stack_interface_t Stack = {
        .init = stack_init,
        .push = stack_push,
        .pop = stack_pop,
        .top = stack_top,
        .destroy = stack_destroy
};