#ifndef AST_H
#define AST_H

/* ============================================================
   AST Node Types for the DSL Compiler
   Author : WASIM SHARAFATH M Y | RA2311026050171
   ============================================================ */

typedef enum {
    /* Literals */
    NODE_INT_LIT,
    NODE_FLOAT_LIT,
    NODE_BOOL_LIT,
    NODE_STRING_LIT,

    /* Identifiers */
    NODE_ID,

    /* Declarations */
    NODE_VAR_DECL,
    NODE_FUNC_DECL,
    NODE_PARAM,

    /* Statements */
    NODE_PROGRAM,
    NODE_BLOCK,
    NODE_ASSIGN,
    NODE_IF,
    NODE_WHILE,
    NODE_FOR,
    NODE_RETURN,
    NODE_PRINT,
    NODE_INPUT,
    NODE_FUNC_CALL,

    /* Expressions */
    NODE_BINOP,
    NODE_UNOP,

    /* Type nodes */
    NODE_TYPE_INT,
    NODE_TYPE_FLOAT,
    NODE_TYPE_BOOL,
    NODE_TYPE_STRING,

    /* List helpers */
    NODE_STMT_LIST,
    NODE_PARAM_LIST,
    NODE_ARG_LIST
} NodeType;

typedef enum {
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_MOD,
    OP_EQ,  OP_NEQ, OP_LT,  OP_GT,  OP_LEQ, OP_GEQ,
    OP_AND, OP_OR,  OP_NOT,
    OP_NEG
} OpType;

typedef enum {
    TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_STRING, TYPE_VOID, TYPE_UNKNOWN
} DataType;

/* Forward declaration */
typedef struct ASTNode ASTNode;

struct ASTNode {
    NodeType    type;
    DataType    data_type;   /* resolved by semantic analysis */
    int         line;

    union {
        int    ival;
        double fval;
        char  *sval;
        OpType op;
    } value;

    /* Children — max 4 for simplicity */
    ASTNode *children[4];
    int      num_children;

    /* Linked-list sibling for statement/param/arg lists */
    ASTNode *next;
};

/* ---- Constructor helpers ---- */
ASTNode *ast_new_node   (NodeType type, int line);
ASTNode *ast_int_lit    (int val, int line);
ASTNode *ast_float_lit  (double val, int line);
ASTNode *ast_bool_lit   (int val, int line);
ASTNode *ast_string_lit (const char *val, int line);
ASTNode *ast_id         (const char *name, int line);
ASTNode *ast_binop      (OpType op, ASTNode *left, ASTNode *right, int line);
ASTNode *ast_unop       (OpType op, ASTNode *operand, int line);
ASTNode *ast_add_child  (ASTNode *parent, ASTNode *child);
void     ast_print      (ASTNode *node, int depth);
void     ast_free       (ASTNode *node);

#endif /* AST_H */
