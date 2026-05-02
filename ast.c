/*
 * ast.c — AST node constructors, printer, and destructor
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* ------------------------------------------------------------------ */
/* Internal helpers                                                     */
/* ------------------------------------------------------------------ */

static const char *node_type_str(NodeType t) {
    switch (t) {
        case NODE_INT_LIT:    return "INT_LIT";
        case NODE_FLOAT_LIT:  return "FLOAT_LIT";
        case NODE_BOOL_LIT:   return "BOOL_LIT";
        case NODE_STRING_LIT: return "STRING_LIT";
        case NODE_ID:         return "IDENTIFIER";
        case NODE_VAR_DECL:   return "VAR_DECL";
        case NODE_FUNC_DECL:  return "FUNC_DECL";
        case NODE_PARAM:      return "PARAM";
        case NODE_PROGRAM:    return "PROGRAM";
        case NODE_BLOCK:      return "BLOCK";
        case NODE_ASSIGN:     return "ASSIGN";
        case NODE_IF:         return "IF";
        case NODE_WHILE:      return "WHILE";
        case NODE_FOR:        return "FOR";
        case NODE_RETURN:     return "RETURN";
        case NODE_PRINT:      return "PRINT";
        case NODE_INPUT:      return "INPUT";
        case NODE_FUNC_CALL:  return "FUNC_CALL";
        case NODE_BINOP:      return "BINOP";
        case NODE_UNOP:       return "UNOP";
        case NODE_STMT_LIST:  return "STMT_LIST";
        case NODE_PARAM_LIST: return "PARAM_LIST";
        case NODE_ARG_LIST:   return "ARG_LIST";
        default:              return "UNKNOWN";
    }
}

static const char *op_str(OpType op) {
    switch (op) {
        case OP_ADD: return "+";  case OP_SUB: return "-";
        case OP_MUL: return "*";  case OP_DIV: return "/";
        case OP_MOD: return "%";  case OP_EQ:  return "==";
        case OP_NEQ: return "!="; case OP_LT:  return "<";
        case OP_GT:  return ">";  case OP_LEQ: return "<=";
        case OP_GEQ: return ">="; case OP_AND: return "&&";
        case OP_OR:  return "||"; case OP_NOT: return "!";
        case OP_NEG: return "-";  default:      return "?";
    }
}

/* ------------------------------------------------------------------ */
/* Constructors                                                         */
/* ------------------------------------------------------------------ */

ASTNode *ast_new_node(NodeType type, int line) {
    ASTNode *n = calloc(1, sizeof(ASTNode));
    if (!n) { fprintf(stderr, "OOM\n"); exit(1); }
    n->type = type;
    n->data_type = TYPE_UNKNOWN;
    n->line = line;
    return n;
}

ASTNode *ast_int_lit(int val, int line) {
    ASTNode *n = ast_new_node(NODE_INT_LIT, line);
    n->value.ival = val;
    n->data_type  = TYPE_INT;
    return n;
}

ASTNode *ast_float_lit(double val, int line) {
    ASTNode *n = ast_new_node(NODE_FLOAT_LIT, line);
    n->value.fval = val;
    n->data_type  = TYPE_FLOAT;
    return n;
}

ASTNode *ast_bool_lit(int val, int line) {
    ASTNode *n = ast_new_node(NODE_BOOL_LIT, line);
    n->value.ival = val;
    n->data_type  = TYPE_BOOL;
    return n;
}

ASTNode *ast_string_lit(const char *val, int line) {
    ASTNode *n = ast_new_node(NODE_STRING_LIT, line);
    n->value.sval = strdup(val);
    n->data_type  = TYPE_STRING;
    return n;
}

ASTNode *ast_id(const char *name, int line) {
    ASTNode *n = ast_new_node(NODE_ID, line);
    n->value.sval = strdup(name);
    return n;
}

ASTNode *ast_binop(OpType op, ASTNode *left, ASTNode *right, int line) {
    ASTNode *n = ast_new_node(NODE_BINOP, line);
    n->value.op = op;
    n->children[0] = left;
    n->children[1] = right;
    n->num_children = 2;
    return n;
}

ASTNode *ast_unop(OpType op, ASTNode *operand, int line) {
    ASTNode *n = ast_new_node(NODE_UNOP, line);
    n->value.op = op;
    n->children[0] = operand;
    n->num_children = 1;
    return n;
}

ASTNode *ast_add_child(ASTNode *parent, ASTNode *child) {
    if (!parent || !child) return parent;
    if (parent->num_children < 4) {
        parent->children[parent->num_children++] = child;
    }
    return parent;
}

/* ------------------------------------------------------------------ */
/* Printer                                                              */
/* ------------------------------------------------------------------ */

void ast_print(ASTNode *node, int depth) {
    if (!node) return;
    for (int i = 0; i < depth * 2; i++) putchar(' ');

    printf("[%s]", node_type_str(node->type));
    switch (node->type) {
        case NODE_INT_LIT:    printf(" %d", node->value.ival); break;
        case NODE_FLOAT_LIT:  printf(" %g", node->value.fval); break;
        case NODE_BOOL_LIT:   printf(" %s", node->value.ival ? "true" : "false"); break;
        case NODE_STRING_LIT: printf(" %s", node->value.sval); break;
        case NODE_ID:         printf(" %s", node->value.sval); break;
        case NODE_BINOP:      printf(" %s", op_str(node->value.op)); break;
        case NODE_UNOP:       printf(" %s", op_str(node->value.op)); break;
        default: break;
    }
    printf("  (line %d)\n", node->line);

    for (int i = 0; i < node->num_children; i++)
        ast_print(node->children[i], depth + 1);

    /* Traverse sibling list */
    ast_print(node->next, depth);
}

/* ------------------------------------------------------------------ */
/* Destructor                                                           */
/* ------------------------------------------------------------------ */

void ast_free(ASTNode *node) {
    if (!node) return;
    for (int i = 0; i < node->num_children; i++) ast_free(node->children[i]);
    ast_free(node->next);
    if (node->type == NODE_ID || node->type == NODE_STRING_LIT ||
        node->type == NODE_VAR_DECL || node->type == NODE_FUNC_DECL ||
        node->type == NODE_PARAM)
        free(node->value.sval);
    free(node);
}
