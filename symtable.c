/*
 * symtable.c — Symbol Table implementation
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtable.h"

SymbolTable *symtable_new(void) {
    SymbolTable *st = calloc(1, sizeof(SymbolTable));
    st->scope_level = 0;
    st->scope_top   = 0;
    return st;
}

void symtable_free(SymbolTable *st) {
    for (int i = 0; i < st->count; i++) free(st->symbols[i].name);
    free(st);
}

void symtable_enter_scope(SymbolTable *st) {
    st->scope_level++;
    if (st->scope_top < MAX_SCOPES)
        st->scope_stack[st->scope_top++] = st->count;
}

void symtable_exit_scope(SymbolTable *st) {
    if (st->scope_top > 0) {
        int start = st->scope_stack[--st->scope_top];
        /* Free symbols in this scope */
        for (int i = start; i < st->count; i++) free(st->symbols[i].name);
        st->count = start;
    }
    if (st->scope_level > 0) st->scope_level--;
}

int symtable_insert(SymbolTable *st, const char *name, DataType type, int is_func) {
    /* Check duplicate in current scope */
    for (int i = 0; i < st->count; i++) {
        if (st->symbols[i].scope_level == st->scope_level &&
            strcmp(st->symbols[i].name, name) == 0) {
            fprintf(stderr, "Semantic error: '%s' already declared in this scope.\n", name);
            return 0;
        }
    }
    if (st->count >= MAX_SYMBOLS) {
        fprintf(stderr, "Symbol table overflow\n"); return 0;
    }
    Symbol *s = &st->symbols[st->count++];
    s->name        = strdup(name);
    s->type        = type;
    s->is_func     = is_func;
    s->scope_level = st->scope_level;
    s->is_initialized = 0;
    return 1;
}

Symbol *symtable_lookup(SymbolTable *st, const char *name) {
    /* Search from most-recent scope outward */
    for (int i = st->count - 1; i >= 0; i--) {
        if (strcmp(st->symbols[i].name, name) == 0)
            return &st->symbols[i];
    }
    return NULL;
}

void symtable_print(SymbolTable *st) {
    static const char *dtype[] = {"int","float","bool","string","void","unknown"};
    printf("\n=== Symbol Table ===\n");
    printf("%-20s %-10s %-8s %-6s\n", "Name", "Type", "IsFunc", "Scope");
    printf("%-20s %-10s %-8s %-6s\n", "----", "----", "------", "-----");
    for (int i = 0; i < st->count; i++) {
        Symbol *s = &st->symbols[i];
        printf("%-20s %-10s %-8s %-6d\n",
               s->name, dtype[s->type],
               s->is_func ? "yes" : "no",
               s->scope_level);
    }
    printf("====================\n\n");
}
