#ifndef SEMANTIC_H
#define SEMANTIC_H

/*
 * semantic.h — Semantic Analyser Interface
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */

#include "ast.h"
#include "symtable.h"

int semantic_analyse(ASTNode *root, SymbolTable *st);

#endif
