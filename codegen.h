#ifndef CODEGEN_H
#define CODEGEN_H

/*
 * codegen.h — LLVM IR Code Generation Interface
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */

#include <stdio.h>
#include "ast.h"
#include "symtable.h"

typedef struct IRGen {
    FILE       *out;          /* Output file for LLVM IR */
    SymbolTable *st;
    int         tmp_counter;  /* SSA register counter */
    int         label_counter;
    char        current_func[128];
} IRGen;

IRGen *irgen_new  (FILE *out, SymbolTable *st);
void   irgen_free (IRGen *g);
void   irgen_emit (IRGen *g, ASTNode *root);

#endif
