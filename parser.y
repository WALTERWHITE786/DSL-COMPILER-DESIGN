%{
/*
 * parser.y — Bison grammar for the DSL
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

extern int  yylex(void);
extern int  line_number;
void        yyerror(const char *msg);

ASTNode *ast_root = NULL;  /* Exported to main */
%}

%union {
    int     ival;
    double  fval;
    char   *sval;
    struct ASTNode *node;
}

/* ---- Tokens ---- */
%token PROGRAM END VAR FUNC IF ELSE WHILE FOR PRINT INPUT RETURN
%token INT_TYPE FLOAT_TYPE BOOL_TYPE STRING_TYPE
%token ASSIGN EQ NEQ LT GT LEQ GEQ AND OR NOT
%token PLUS MINUS MULT DIV MOD
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COMMA COLON ARROW

%token <ival> INT_LIT BOOL_LIT
%token <fval> FLOAT_LIT
%token <sval> ID STRING_LIT

/* ---- Non-terminal types ---- */
%type <node> program stmt_list stmt
%type <node> var_decl func_decl param_list param_item
%type <node> if_stmt while_stmt for_stmt return_stmt print_stmt input_stmt
%type <node> assignment func_call arg_list
%type <node> expr term factor primary
%type <node> type_spec block

/* ---- Precedence (lowest → highest) ---- */
%right    ASSIGN
%left     OR
%left     AND
%right    NOT
%left     EQ NEQ
%left     LT GT LEQ GEQ
%left     PLUS MINUS
%left     MULT DIV MOD
%right    UMINUS

%start program

%%

/* ================================================================ */
program
    : PROGRAM ID LBRACE stmt_list RBRACE END
        {
            ASTNode *n = ast_new_node(NODE_PROGRAM, line_number);
            n->value.sval = $2;
            n->children[0] = $4;
            n->num_children = 1;
            ast_root = n;
            $$ = n;
        }
    ;

/* ---- Statement list ---- */
stmt_list
    : stmt_list stmt  { $$ = $1; ASTNode *t = $1; while(t->next) t=t->next; t->next = $2; }
    | stmt            { $$ = $1; }
    ;

stmt
    : var_decl        { $$ = $1; }
    | func_decl       { $$ = $1; }
    | assignment SEMICOLON { $$ = $1; }
    | if_stmt         { $$ = $1; }
    | while_stmt      { $$ = $1; }
    | for_stmt        { $$ = $1; }
    | return_stmt SEMICOLON { $$ = $1; }
    | print_stmt SEMICOLON  { $$ = $1; }
    | input_stmt SEMICOLON  { $$ = $1; }
    | func_call SEMICOLON   { $$ = $1; }
    ;

/* ---- Variable declaration ---- */
var_decl
    : VAR ID COLON type_spec SEMICOLON
        {
            ASTNode *n = ast_new_node(NODE_VAR_DECL, line_number);
            n->value.sval = $2;
            n->children[0] = $4;
            n->num_children = 1;
            $$ = n;
        }
    | VAR ID COLON type_spec ASSIGN expr SEMICOLON
        {
            ASTNode *n = ast_new_node(NODE_VAR_DECL, line_number);
            n->value.sval = $2;
            n->children[0] = $4;
            n->children[1] = $6;
            n->num_children = 2;
            $$ = n;
        }
    ;

/* ---- Function declaration ---- */
func_decl
    : FUNC ID LPAREN param_list RPAREN ARROW type_spec block
        {
            ASTNode *n = ast_new_node(NODE_FUNC_DECL, line_number);
            n->value.sval = $2;
            n->children[0] = $4;   /* params */
            n->children[1] = $7;   /* return type */
            n->children[2] = $8;   /* body */
            n->num_children = 3;
            $$ = n;
        }
    | FUNC ID LPAREN RPAREN ARROW type_spec block
        {
            ASTNode *n = ast_new_node(NODE_FUNC_DECL, line_number);
            n->value.sval = $2;
            n->children[0] = NULL;
            n->children[1] = $6;
            n->children[2] = $7;
            n->num_children = 3;
            $$ = n;
        }
    ;

param_list
    : param_list COMMA param_item
        { $$ = $1; ASTNode *t = $1; while(t->next) t=t->next; t->next = $3; }
    | param_item  { $$ = $1; }
    ;

param_item
    : ID COLON type_spec
        {
            ASTNode *n = ast_new_node(NODE_PARAM, line_number);
            n->value.sval = $1;
            n->children[0] = $3;
            n->num_children = 1;
            $$ = n;
        }
    ;

/* ---- Block ---- */
block
    : LBRACE stmt_list RBRACE
        {
            ASTNode *n = ast_new_node(NODE_BLOCK, line_number);
            n->children[0] = $2;
            n->num_children = 1;
            $$ = n;
        }
    | LBRACE RBRACE
        {
            ASTNode *n = ast_new_node(NODE_BLOCK, line_number);
            n->num_children = 0;
            $$ = n;
        }
    ;

/* ---- Control flow ---- */
if_stmt
    : IF LPAREN expr RPAREN block
        {
            ASTNode *n = ast_new_node(NODE_IF, line_number);
            n->children[0] = $3; n->children[1] = $5;
            n->num_children = 2;
            $$ = n;
        }
    | IF LPAREN expr RPAREN block ELSE block
        {
            ASTNode *n = ast_new_node(NODE_IF, line_number);
            n->children[0] = $3; n->children[1] = $5; n->children[2] = $7;
            n->num_children = 3;
            $$ = n;
        }
    ;

while_stmt
    : WHILE LPAREN expr RPAREN block
        {
            ASTNode *n = ast_new_node(NODE_WHILE, line_number);
            n->children[0] = $3; n->children[1] = $5;
            n->num_children = 2;
            $$ = n;
        }
    ;

for_stmt
    : FOR LPAREN assignment SEMICOLON expr SEMICOLON assignment RPAREN block
        {
            ASTNode *n = ast_new_node(NODE_FOR, line_number);
            n->children[0] = $3; n->children[1] = $5;
            n->children[2] = $7; n->children[3] = $9;
            n->num_children = 4;
            $$ = n;
        }
    ;

return_stmt
    : RETURN expr
        {
            ASTNode *n = ast_new_node(NODE_RETURN, line_number);
            n->children[0] = $2; n->num_children = 1;
            $$ = n;
        }
    ;

print_stmt
    : PRINT LPAREN expr RPAREN
        {
            ASTNode *n = ast_new_node(NODE_PRINT, line_number);
            n->children[0] = $3; n->num_children = 1;
            $$ = n;
        }
    ;

input_stmt
    : INPUT LPAREN ID RPAREN
        {
            ASTNode *n = ast_new_node(NODE_INPUT, line_number);
            n->value.sval = $3; n->num_children = 0;
            $$ = n;
        }
    ;

/* ---- Assignment ---- */
assignment
    : ID ASSIGN expr
        {
            ASTNode *n = ast_new_node(NODE_ASSIGN, line_number);
            n->children[0] = ast_id($1, line_number);
            n->children[1] = $3;
            n->num_children = 2;
            free($1);
            $$ = n;
        }
    ;

/* ---- Function call ---- */
func_call
    : ID LPAREN arg_list RPAREN
        {
            ASTNode *n = ast_new_node(NODE_FUNC_CALL, line_number);
            n->value.sval = $1;
            n->children[0] = $3;
            n->num_children = 1;
            $$ = n;
        }
    | ID LPAREN RPAREN
        {
            ASTNode *n = ast_new_node(NODE_FUNC_CALL, line_number);
            n->value.sval = $1;
            n->num_children = 0;
            $$ = n;
        }
    ;

arg_list
    : arg_list COMMA expr
        { $$ = $1; ASTNode *t=$1; while(t->next) t=t->next; t->next=$3; }
    | expr  { $$ = $1; }
    ;

/* ---- Expressions ---- */
expr
    : expr OR  term  { $$ = ast_binop(OP_OR,  $1, $3, line_number); }
    | expr AND term  { $$ = ast_binop(OP_AND, $1, $3, line_number); }
    | expr EQ  term  { $$ = ast_binop(OP_EQ,  $1, $3, line_number); }
    | expr NEQ term  { $$ = ast_binop(OP_NEQ, $1, $3, line_number); }
    | expr LT  term  { $$ = ast_binop(OP_LT,  $1, $3, line_number); }
    | expr GT  term  { $$ = ast_binop(OP_GT,  $1, $3, line_number); }
    | expr LEQ term  { $$ = ast_binop(OP_LEQ, $1, $3, line_number); }
    | expr GEQ term  { $$ = ast_binop(OP_GEQ, $1, $3, line_number); }
    | NOT  expr      { $$ = ast_unop(OP_NOT, $2, line_number); }
    | term           { $$ = $1; }
    ;

term
    : term PLUS  factor { $$ = ast_binop(OP_ADD, $1, $3, line_number); }
    | term MINUS factor { $$ = ast_binop(OP_SUB, $1, $3, line_number); }
    | factor            { $$ = $1; }
    ;

factor
    : factor MULT  primary { $$ = ast_binop(OP_MUL, $1, $3, line_number); }
    | factor DIV   primary { $$ = ast_binop(OP_DIV, $1, $3, line_number); }
    | factor MOD   primary { $$ = ast_binop(OP_MOD, $1, $3, line_number); }
    | MINUS primary %prec UMINUS { $$ = ast_unop(OP_NEG, $2, line_number); }
    | primary              { $$ = $1; }
    ;

primary
    : INT_LIT        { $$ = ast_int_lit($1, line_number); }
    | FLOAT_LIT      { $$ = ast_float_lit($1, line_number); }
    | BOOL_LIT       { $$ = ast_bool_lit($1, line_number); }
    | STRING_LIT     { $$ = ast_string_lit($1, line_number); free($1); }
    | ID             { $$ = ast_id($1, line_number); free($1); }
    | func_call      { $$ = $1; }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

/* ---- Type specifiers ---- */
type_spec
    : INT_TYPE    { $$ = ast_new_node(NODE_TYPE_INT,    line_number); }
    | FLOAT_TYPE  { $$ = ast_new_node(NODE_TYPE_FLOAT,  line_number); }
    | BOOL_TYPE   { $$ = ast_new_node(NODE_TYPE_BOOL,   line_number); }
    | STRING_TYPE { $$ = ast_new_node(NODE_TYPE_STRING, line_number); }
    ;

%%

void yyerror(const char *msg) {
    fprintf(stderr, "Syntax error at line %d: %s\n", line_number, msg);
    exit(1);
}
