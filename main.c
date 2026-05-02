/*
 * main.c — DSL Compiler Driver
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 *
 * Pipeline:
 *   source.dsl → Lexer → Parser → AST → Semantic Analysis → LLVM IR → llc → binary
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symtable.h"
#include "semantic.h"
#include "codegen.h"

/* Declared by Bison/Flex */
extern FILE *yyin;
extern int   yyparse(void);
extern ASTNode *ast_root;

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s <source.dsl> [-o <output.ll>] [--ast] [--sym]\n"
        "  --ast   Print AST to stdout\n"
        "  --sym   Print symbol table to stdout\n", prog);
    exit(1);
}

int main(int argc, char *argv[]) {
    if (argc < 2) usage(argv[0]);

    const char *src_file  = argv[1];
    const char *out_file  = "output/output.ll";
    int show_ast = 0, show_sym = 0;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) { out_file = argv[++i]; }
        else if (strcmp(argv[i], "--ast") == 0)  show_ast = 1;
        else if (strcmp(argv[i], "--sym") == 0)  show_sym = 1;
    }

    /* ---- PHASE 1: Lexing + Parsing ---- */
    printf("[1/4] Lexing & Parsing: %s\n", src_file);
    yyin = fopen(src_file, "r");
    if (!yyin) { perror(src_file); return 1; }
    yyparse();
    fclose(yyin);

    if (!ast_root) {
        fprintf(stderr, "Error: no AST produced.\n"); return 1;
    }
    printf("      Parse OK.\n");

    if (show_ast) {
        printf("\n=== Abstract Syntax Tree ===\n");
        ast_print(ast_root, 0);
    }

    /* ---- PHASE 2: Semantic Analysis ---- */
    printf("[2/4] Semantic Analysis...\n");
    SymbolTable *st = symtable_new();
    int ok = semantic_analyse(ast_root, st);
    if (!ok) {
        fprintf(stderr, "      Semantic errors found. Aborting.\n");
        ast_free(ast_root); symtable_free(st); return 1;
    }
    printf("      Semantic analysis OK.\n");

    if (show_sym) symtable_print(st);

    /* ---- PHASE 3: LLVM IR Generation ---- */
    printf("[3/4] Generating LLVM IR → %s\n", out_file);
    FILE *ir_file = fopen(out_file, "w");
    if (!ir_file) { perror(out_file); return 1; }

    IRGen *irgen = irgen_new(ir_file, st);
    irgen_emit(irgen, ast_root);
    irgen_free(irgen);
    fclose(ir_file);
    printf("      IR written to %s\n", out_file);

    /* ---- PHASE 4: Invoke llc (if available) ---- */
    printf("[4/4] Invoking llc for native code generation...\n");
    char cmd[512];
    char asm_file[256];
    strncpy(asm_file, out_file, sizeof(asm_file) - 1);
    /* Replace .ll with .s */
    char *dot = strrchr(asm_file, '.');
    if (dot) strcpy(dot, ".s"); else strcat(asm_file, ".s");

    snprintf(cmd, sizeof(cmd), "llc %s -o %s 2>&1", out_file, asm_file);
    int ret = system(cmd);
    if (ret == 0)
        printf("      Assembly written to %s\n", asm_file);
    else
        printf("      llc not found or error — IR is ready at %s\n", out_file);

    /* Cleanup */
    ast_free(ast_root);
    symtable_free(st);

    printf("\nCompilation successful!\n");
    return 0;
}
