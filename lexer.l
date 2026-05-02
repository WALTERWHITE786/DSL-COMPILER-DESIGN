%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

int line_number = 1;
%}

%option noyywrap

DIGIT       [0-9]
LETTER      [a-zA-Z_]
ID          {LETTER}({LETTER}|{DIGIT})*
INT         {DIGIT}+
FLOAT       {DIGIT}+"."{DIGIT}*
STRING      \"[^\"]*\"
WHITESPACE  [ \t\r]+

%%

"program"       { return PROGRAM; }
"end"           { return END; }
"var"           { return VAR; }
"int"           { return INT_TYPE; }
"float"         { return FLOAT_TYPE; }
"bool"          { return BOOL_TYPE; }
"string"        { return STRING_TYPE; }
"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }
"for"           { return FOR; }
"print"         { return PRINT; }
"input"         { return INPUT; }
"return"        { return RETURN; }
"func"          { return FUNC; }
"true"          { yylval.ival = 1; return BOOL_LIT; }
"false"         { yylval.ival = 0; return BOOL_LIT; }

{ID}            { yylval.sval = strdup(yytext); return ID; }
{FLOAT}         { yylval.fval = atof(yytext); return FLOAT_LIT; }
{INT}           { yylval.ival = atoi(yytext); return INT_LIT; }
{STRING}        { yylval.sval = strdup(yytext); return STRING_LIT; }

"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }
"%"             { return MOD; }
"="             { return ASSIGN; }
"=="            { return EQ; }
"!="            { return NEQ; }
"<"             { return LT; }
">"             { return GT; }
"<="            { return LEQ; }
">="            { return GEQ; }
"&&"            { return AND; }
"||"            { return OR; }
"!"             { return NOT; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
";"             { return SEMICOLON; }
","             { return COMMA; }
":"             { return COLON; }
"->"            { return ARROW; }

{WHITESPACE}    { /* skip */ }
"\n"            { line_number++; }
"#"[^\n]*       { /* skip comments */ }

.               {
                    fprintf(stderr, "Lexical error at line %d: unknown character '%s'\n",
                            line_number, yytext);
                    exit(1);
                }

%%
