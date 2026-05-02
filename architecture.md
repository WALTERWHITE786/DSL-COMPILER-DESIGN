# Architecture & Module Documentation

## DSL Compiler — Compiler-Design-DSL_RA2311026050171_WASIM SHARAFATH M Y

---

## Overview

This compiler transforms programs written in a custom DSL (Domain-Specific Language) into LLVM Intermediate Representation (IR), which is subsequently compiled to native machine code using the `llc` backend.

---

## Compilation Pipeline

```
┌─────────────────────────────────────────────────────────────────┐
│                    DSL COMPILER PIPELINE                        │
│                                                                 │
│  source.dsl ──► Lexer ──► Parser ──► AST ──► Semantic ──► IR  │
│                (Flex)  (Bison)   (C)    Analysis   (LLVM)      │
│                                                                 │
│                                              IR (.ll) ──► llc  │
│                                                         ──► .s  │
│                                                         ──► bin │
└─────────────────────────────────────────────────────────────────┘
```

---

## Module Explanations

### 1. Lexer (`src/lexer.l`) — Flex

**Purpose:** Converts raw source text into a stream of tokens.

**Key responsibilities:**
- Recognises keywords: `program`, `var`, `func`, `if`, `else`, `while`, `for`, `print`, `input`, `return`, `end`
- Recognises types: `int`, `float`, `bool`, `string`
- Tokenises literals: integers, floats, booleans, quoted strings
- Recognises operators: `+`, `-`, `*`, `/`, `%`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`
- Skips whitespace and `#`-prefixed comments
- Tracks line numbers for error reporting

**Token categories:**
```
KEYWORD     → program, var, func, if, else, while, for, ...
LITERAL     → INT_LIT, FLOAT_LIT, BOOL_LIT, STRING_LIT
IDENTIFIER  → ID (matches [a-zA-Z_][a-zA-Z0-9_]*)
OPERATOR    → PLUS, MINUS, MULT, DIV, EQ, NEQ, ...
PUNCTUATION → LPAREN, RPAREN, LBRACE, RBRACE, SEMICOLON, ...
```

---

### 2. Parser (`src/parser.y`) — Bison

**Purpose:** Validates token sequences against the grammar and drives AST construction.

**Grammar style:** LALR(1) — one token lookahead

**Key grammar rules:**
```
program   → PROGRAM ID '{' stmt_list '}' END
stmt_list → stmt_list stmt | stmt
stmt      → var_decl | func_decl | assignment | if_stmt
          | while_stmt | for_stmt | return_stmt | print_stmt
expr      → expr OP term | term
term      → term OP factor | factor
factor    → factor OP primary | primary
primary   → literal | ID | func_call | '(' expr ')'
```

**Operator precedence (low → high):**
```
OR  <  AND  <  NOT  <  relational  <  additive  <  multiplicative  <  unary
```

---

### 3. AST (`src/ast.h`, `src/ast.c`)

**Purpose:** Represents the program structure as a tree of typed nodes.

**Node structure:**
```c
struct ASTNode {
    NodeType  type;          // e.g. NODE_BINOP, NODE_IF, NODE_VAR_DECL
    DataType  data_type;     // resolved during semantic analysis
    int       line;          // source line for error messages
    union { int ival; double fval; char *sval; OpType op; } value;
    ASTNode  *children[4];   // up to 4 sub-nodes
    int       num_children;
    ASTNode  *next;          // sibling (for stmt/param lists)
};
```

**Node types cover:** literals, identifiers, declarations, all statement forms, binary/unary operators, type specifiers, and list nodes.

---

### 4. Symbol Table (`src/symtable.h`, `src/symtable.c`)

**Purpose:** Tracks declared variables and functions with scope support.

**Design:**
- Flat array of `Symbol` records, each carrying a `scope_level`
- Scope stack — `enter_scope()` pushes a bookmark; `exit_scope()` pops and frees symbols
- `lookup()` searches from the most-recently-inserted symbol outward (innermost scope wins)

**Symbol fields:** name, type (DataType enum), is_func, scope_level, is_initialized

---

### 5. Semantic Analyser (`src/semantic.h`, `src/semantic.c`)

**Purpose:** Enforces language semantics beyond what the grammar can express.

**Checks performed:**
1. **Undeclared variables** — every `ID` reference is looked up in the symbol table
2. **Duplicate declarations** — catches re-declaration in the same scope
3. **Type mismatches** — checks RHS type matches LHS in assignments and initialisations
4. **Condition types** — `if` and `while` conditions must be `bool`
5. **Undeclared functions** — verifies function calls against declarations
6. **Arithmetic promotion** — `int + float` resolves to `float`

The analyser performs a recursive post-order traversal of the AST, attaching resolved `data_type` to every expression node.

---

### 6. Code Generator (`src/codegen.h`, `src/codegen.c`)

**Purpose:** Emits textual LLVM IR from the annotated AST.

**Key LLVM IR patterns used:**

| DSL construct | LLVM IR equivalent |
|---|---|
| `var x : int = 5` | `%x = alloca i32` + `store i32 5, i32* %x` |
| `result = a + b` | `%t0 = add i32 %a_val, %b_val` |
| `if (cond) {…}` | `br i1 %cond, label %L0, label %L1` |
| `while (cond) {…}` | branch + loop label structure |
| `print(x)` | `call i32 @printf(…)` |
| `func f(…) -> int` | `define i32 @f(…) { … }` |

**SSA registers** are named `%t0, %t1, …` using a monotonic counter.
**Labels** for branches use `L0, L1, …` similarly.

---

## Data Types

| DSL type | LLVM IR type | C internal |
|----------|-------------|------------|
| `int`    | `i32`       | `TYPE_INT`    |
| `float`  | `double`    | `TYPE_FLOAT`  |
| `bool`   | `i1`        | `TYPE_BOOL`   |
| `string` | `i8*`       | `TYPE_STRING` |

---

## Error Handling

| Phase    | Error type           | Behaviour                       |
|----------|---------------------|---------------------------------|
| Lexer    | Unknown character   | Print line + exit               |
| Parser   | Syntax error        | `yyerror()` + exit              |
| Semantic | Type/scope error    | Accumulate + abort after phase  |
| Codegen  | llc not installed   | Warns, keeps .ll file           |

---

## Sample Output

**Input:**
```
program Demo {
    var n : int = 42;
    print(n);
} end
```

**LLVM IR output:**
```llvm
; LLVM IR generated by DSL Compiler
source_filename = "dsl_program"
target triple = "x86_64-pc-linux-gnu"

@.fmt_int = private constant [4 x i8] c"%d\0A\00"
declare i32 @printf(i8*, ...)

define i32 @main() {
entry:
  %n = alloca i32
  %t0 = add i32 0, 42
  store i32 %t0, i32* %n
  %t1 = load i32, i32* %n
  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.fmt_int, i32 0, i32 0), i32 %t1)
  ret i32 0
}
```

---

*Document prepared for SRMIST Trichy — Compiler Design Lab Assessment*
*WASIM SHARAFATH M Y | RA2311026050171 | III AIML A*
