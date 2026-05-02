#ifndef SYMTABLE_H
#define SYMTABLE_H

/*
 * symtable.h — Symbol Table for Semantic Analysis
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */

#include "ast.h"

#define MAX_SYMBOLS 512
#define MAX_SCOPES  64

typedef struct Symbol {
    char    *name;
    DataType type;
    int      is_func;
    int      param_count;
    DataType param_types[16];
    DataType return_type;
    int      scope_level;
    int      is_initialized;
} Symbol;

typedef struct SymbolTable {
    Symbol   symbols[MAX_SYMBOLS];
    int      count;
    int      scope_level;
    int      scope_stack[MAX_SCOPES];
    int      scope_top;
} SymbolTable;

SymbolTable *symtable_new      (void);
void         symtable_free     (SymbolTable *st);
void         symtable_enter_scope(SymbolTable *st);
void         symtable_exit_scope (SymbolTable *st);
int          symtable_insert   (SymbolTable *st, const char *name, DataType type, int scope);
Symbol      *symtable_lookup   (SymbolTable *st, const char *name);
void         symtable_print    (SymbolTable *st);

#endif /* SYMTABLE_H */
