/*
 * semantic.c — Semantic Analysis (type checking, scope, undeclared vars)
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

static int error_count = 0;

static void sem_error(int line, const char *msg) {
    fprintf(stderr, "Semantic error [line %d]: %s\n", line, msg);
    error_count++;
}

static DataType node_to_dtype(NodeType nt) {
    switch (nt) {
        case NODE_TYPE_INT:    return TYPE_INT;
        case NODE_TYPE_FLOAT:  return TYPE_FLOAT;
        case NODE_TYPE_BOOL:   return TYPE_BOOL;
        case NODE_TYPE_STRING: return TYPE_STRING;
        default:               return TYPE_UNKNOWN;
    }
}

/* Forward declaration */
static DataType analyse_node(ASTNode *node, SymbolTable *st);

static DataType analyse_node(ASTNode *node, SymbolTable *st) {
    if (!node) return TYPE_VOID;

    switch (node->type) {

    /* ---- Literals ---- */
    case NODE_INT_LIT:    node->data_type = TYPE_INT;    return TYPE_INT;
    case NODE_FLOAT_LIT:  node->data_type = TYPE_FLOAT;  return TYPE_FLOAT;
    case NODE_BOOL_LIT:   node->data_type = TYPE_BOOL;   return TYPE_BOOL;
    case NODE_STRING_LIT: node->data_type = TYPE_STRING; return TYPE_STRING;

    /* ---- Identifier ---- */
    case NODE_ID: {
        Symbol *sym = symtable_lookup(st, node->value.sval);
        if (!sym) {
            char buf[256];
            snprintf(buf, sizeof(buf), "undeclared variable '%s'", node->value.sval);
            sem_error(node->line, buf);
            return TYPE_UNKNOWN;
        }
        node->data_type = sym->type;
        return sym->type;
    }

    /* ---- Variable declaration ---- */
    case NODE_VAR_DECL: {
        DataType declared_type = node_to_dtype(node->children[0]->type);
        symtable_insert(st, node->value.sval, declared_type, 0);
        if (node->num_children == 2) {
            DataType init_type = analyse_node(node->children[1], st);
            if (init_type != declared_type && init_type != TYPE_UNKNOWN) {
                char buf[256];
                snprintf(buf, sizeof(buf),
                    "type mismatch in initialisation of '%s'", node->value.sval);
                sem_error(node->line, buf);
            }
            Symbol *sym = symtable_lookup(st, node->value.sval);
            if (sym) sym->is_initialized = 1;
        }
        return TYPE_VOID;
    }

    /* ---- Assignment ---- */
    case NODE_ASSIGN: {
        DataType lhs = analyse_node(node->children[0], st);
        DataType rhs = analyse_node(node->children[1], st);
        if (lhs != rhs && lhs != TYPE_UNKNOWN && rhs != TYPE_UNKNOWN) {
            sem_error(node->line, "type mismatch in assignment");
        }
        /* Mark initialised */
        if (node->children[0]->type == NODE_ID) {
            Symbol *sym = symtable_lookup(st, node->children[0]->value.sval);
            if (sym) sym->is_initialized = 1;
        }
        node->data_type = lhs;
        return lhs;
    }

    /* ---- Binary operations ---- */
    case NODE_BINOP: {
        DataType l = analyse_node(node->children[0], st);
        DataType r = analyse_node(node->children[1], st);
        OpType op = node->value.op;
        /* Relational → bool */
        if (op == OP_EQ || op == OP_NEQ || op == OP_LT || op == OP_GT ||
            op == OP_LEQ || op == OP_GEQ || op == OP_AND || op == OP_OR) {
            node->data_type = TYPE_BOOL;
            return TYPE_BOOL;
        }
        /* Arithmetic — promote int+float → float */
        if (l == TYPE_FLOAT || r == TYPE_FLOAT) {
            node->data_type = TYPE_FLOAT; return TYPE_FLOAT;
        }
        node->data_type = l;
        return l;
    }

    /* ---- Unary operation ---- */
    case NODE_UNOP: {
        DataType t = analyse_node(node->children[0], st);
        if (node->value.op == OP_NOT) { node->data_type = TYPE_BOOL; return TYPE_BOOL; }
        node->data_type = t;
        return t;
    }

    /* ---- Block / program ---- */
    case NODE_BLOCK:
        symtable_enter_scope(st);
        if (node->children[0]) analyse_node(node->children[0], st);
        symtable_exit_scope(st);
        return TYPE_VOID;

    case NODE_PROGRAM:
        symtable_enter_scope(st);
        if (node->children[0]) analyse_node(node->children[0], st);
        symtable_exit_scope(st);
        return TYPE_VOID;

    /* ---- Function declaration ---- */
    case NODE_FUNC_DECL: {
        DataType ret = node_to_dtype(node->children[1]->type);
        symtable_insert(st, node->value.sval, ret, 1);
        symtable_enter_scope(st);
        if (node->children[0]) analyse_node(node->children[0], st);
        if (node->children[2]) analyse_node(node->children[2], st);
        symtable_exit_scope(st);
        return TYPE_VOID;
    }

    /* ---- Parameter ---- */
    case NODE_PARAM: {
        DataType pt = node_to_dtype(node->children[0]->type);
        symtable_insert(st, node->value.sval, pt, 0);
        Symbol *sym = symtable_lookup(st, node->value.sval);
        if (sym) sym->is_initialized = 1;
        if (node->next) analyse_node(node->next, st);
        return TYPE_VOID;
    }

    /* ---- If statement ---- */
    case NODE_IF: {
        DataType cond = analyse_node(node->children[0], st);
        if (cond != TYPE_BOOL && cond != TYPE_UNKNOWN)
            sem_error(node->line, "if condition must be boolean");
        analyse_node(node->children[1], st);
        if (node->num_children == 3) analyse_node(node->children[2], st);
        return TYPE_VOID;
    }

    /* ---- While statement ---- */
    case NODE_WHILE: {
        DataType cond = analyse_node(node->children[0], st);
        if (cond != TYPE_BOOL && cond != TYPE_UNKNOWN)
            sem_error(node->line, "while condition must be boolean");
        analyse_node(node->children[1], st);
        return TYPE_VOID;
    }

    /* ---- For statement ---- */
    case NODE_FOR:
        analyse_node(node->children[0], st);
        analyse_node(node->children[1], st);
        analyse_node(node->children[2], st);
        analyse_node(node->children[3], st);
        return TYPE_VOID;

    /* ---- Print / Input / Return ---- */
    case NODE_PRINT:
        analyse_node(node->children[0], st);
        return TYPE_VOID;
    case NODE_INPUT:
        return TYPE_VOID;
    case NODE_RETURN:
        analyse_node(node->children[0], st);
        return TYPE_VOID;

    /* ---- Function call ---- */
    case NODE_FUNC_CALL: {
        Symbol *sym = symtable_lookup(st, node->value.sval);
        if (!sym) {
            char buf[256];
            snprintf(buf, sizeof(buf), "undeclared function '%s'", node->value.sval);
            sem_error(node->line, buf);
            return TYPE_UNKNOWN;
        }
        if (node->children[0]) analyse_node(node->children[0], st);
        node->data_type = sym->type;
        return sym->type;
    }

    /* ---- Statement list (linked via ->next) ---- */
    case NODE_STMT_LIST:
    default:
        for (int i = 0; i < node->num_children; i++)
            analyse_node(node->children[i], st);
        /* Traverse linked siblings */
        if (node->next) analyse_node(node->next, st);
        return TYPE_VOID;
    }
}

int semantic_analyse(ASTNode *root, SymbolTable *st) {
    error_count = 0;
    analyse_node(root, st);
    /* Also walk linked statements at top level */
    return (error_count == 0) ? 1 : 0;
}
