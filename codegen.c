/*
 * codegen.c — LLVM IR Emission
 * Author : WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy
 *
 * Emits textual LLVM IR that can be passed to `llc` to produce
 * native assembly / object code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static int  next_tmp(IRGen *g)   { return g->tmp_counter++;   }
static int  next_lbl(IRGen *g)   { return g->label_counter++; }

static const char *llvm_type(DataType dt) {
    switch (dt) {
        case TYPE_INT:    return "i32";
        case TYPE_FLOAT:  return "double";
        case TYPE_BOOL:   return "i1";
        case TYPE_STRING: return "i8*";
        default:          return "void";
    }
}

/* Forward declaration */
static int emit_expr(IRGen *g, ASTNode *node);
static void emit_stmt(IRGen *g, ASTNode *node);

/* ------------------------------------------------------------------ */
/* Expression emission — returns SSA register number                    */
/* ------------------------------------------------------------------ */

static int emit_expr(IRGen *g, ASTNode *node) {
    if (!node) return -1;
    int t;

    switch (node->type) {

    case NODE_INT_LIT:
        t = next_tmp(g);
        fprintf(g->out, "  %%t%d = add i32 0, %d\n", t, node->value.ival);
        return t;

    case NODE_FLOAT_LIT:
        t = next_tmp(g);
        fprintf(g->out, "  %%t%d = fadd double 0.0, %f\n", t, node->value.fval);
        return t;

    case NODE_BOOL_LIT:
        t = next_tmp(g);
        fprintf(g->out, "  %%t%d = add i1 0, %d\n", t, node->value.ival);
        return t;

    case NODE_STRING_LIT: {
        /* Declare a global string constant */
        static int str_cnt = 0;
        int idx = str_cnt++;
        /* Strip surrounding quotes */
        char *s = node->value.sval;
        int len  = (int)strlen(s);
        char *content = (len >= 2) ? strndup(s + 1, len - 2) : strdup("");
        /* We would normally emit a global here; for brevity, use getelementptr */
        t = next_tmp(g);
        fprintf(g->out,
            "@.str%d = private unnamed_addr constant [%d x i8] c\"%s\\00\"\n",
            idx, (int)strlen(content) + 1, content);
        fprintf(g->out,
            "  %%t%d = getelementptr inbounds [%d x i8], [%d x i8]* @.str%d, i32 0, i32 0\n",
            t, (int)strlen(content) + 1, (int)strlen(content) + 1, idx);
        free(content);
        return t;
    }

    case NODE_ID: {
        Symbol *sym = symtable_lookup(g->st, node->value.sval);
        t = next_tmp(g);
        const char *lt = sym ? llvm_type(sym->type) : "i32";
        fprintf(g->out, "  %%t%d = load %s, %s* %%%s\n",
                t, lt, lt, node->value.sval);
        return t;
    }

    case NODE_BINOP: {
        int l = emit_expr(g, node->children[0]);
        int r = emit_expr(g, node->children[1]);
        t = next_tmp(g);
        DataType dt = node->children[0]->data_type;
        int is_float = (dt == TYPE_FLOAT);
        switch (node->value.op) {
            case OP_ADD: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fadd double" : "add i32", l, r); break;
            case OP_SUB: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fsub double" : "sub i32", l, r); break;
            case OP_MUL: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fmul double" : "mul i32", l, r); break;
            case OP_DIV: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fdiv double" : "sdiv i32", l, r); break;
            case OP_MOD: fprintf(g->out, "  %%t%d = srem i32 %%t%d, %%t%d\n", t, l, r); break;
            case OP_EQ:  fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fcmp oeq double" : "icmp eq i32", l, r); break;
            case OP_NEQ: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fcmp one double" : "icmp ne i32", l, r); break;
            case OP_LT:  fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fcmp olt double" : "icmp slt i32", l, r); break;
            case OP_GT:  fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fcmp ogt double" : "icmp sgt i32", l, r); break;
            case OP_LEQ: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fcmp ole double" : "icmp sle i32", l, r); break;
            case OP_GEQ: fprintf(g->out, "  %%t%d = %s %%t%d, %%t%d\n",
                t, is_float ? "fcmp oge double" : "icmp sge i32", l, r); break;
            case OP_AND: fprintf(g->out, "  %%t%d = and i1 %%t%d, %%t%d\n", t, l, r); break;
            case OP_OR:  fprintf(g->out, "  %%t%d = or i1 %%t%d, %%t%d\n",  t, l, r); break;
            default: t = -1; break;
        }
        return t;
    }

    case NODE_UNOP: {
        int v = emit_expr(g, node->children[0]);
        t = next_tmp(g);
        if (node->value.op == OP_NOT)
            fprintf(g->out, "  %%t%d = xor i1 %%t%d, 1\n", t, v);
        else
            fprintf(g->out, "  %%t%d = sub i32 0, %%t%d\n", t, v);
        return t;
    }

    case NODE_FUNC_CALL: {
        Symbol *sym = symtable_lookup(g->st, node->value.sval);
        const char *rt = sym ? llvm_type(sym->type) : "i32";
        t = next_tmp(g);
        /* Collect args */
        char args_buf[512] = "";
        ASTNode *arg = (node->num_children > 0) ? node->children[0] : NULL;
        int first = 1;
        while (arg) {
            int at = emit_expr(g, arg);
            const char *at_type = llvm_type(arg->data_type);
            char piece[64];
            snprintf(piece, sizeof(piece), "%s%s %%t%d", first ? "" : ", ", at_type, at);
            strncat(args_buf, piece, sizeof(args_buf) - strlen(args_buf) - 1);
            first = 0;
            arg = arg->next;
        }
        fprintf(g->out, "  %%t%d = call %s @%s(%s)\n",
                t, rt, node->value.sval, args_buf);
        return t;
    }

    default:
        return -1;
    }
}

/* ------------------------------------------------------------------ */
/* Statement emission                                                   */
/* ------------------------------------------------------------------ */

static void emit_stmt(IRGen *g, ASTNode *node) {
    if (!node) return;

    switch (node->type) {

    case NODE_VAR_DECL: {
        Symbol *sym = symtable_lookup(g->st, node->value.sval);
        const char *lt = sym ? llvm_type(sym->type) : "i32";
        fprintf(g->out, "  %%%s = alloca %s\n", node->value.sval, lt);
        if (node->num_children == 2) {
            int vt = emit_expr(g, node->children[1]);
            fprintf(g->out, "  store %s %%t%d, %s* %%%s\n",
                    lt, vt, lt, node->value.sval);
        }
        break;
    }

    case NODE_ASSIGN: {
        ASTNode *lhs = node->children[0];
        int vt = emit_expr(g, node->children[1]);
        Symbol *sym = symtable_lookup(g->st, lhs->value.sval);
        const char *lt = sym ? llvm_type(sym->type) : "i32";
        fprintf(g->out, "  store %s %%t%d, %s* %%%s\n",
                lt, vt, lt, lhs->value.sval);
        break;
    }

    case NODE_PRINT: {
        int vt = emit_expr(g, node->children[0]);
        DataType dt = node->children[0]->data_type;
        if (dt == TYPE_INT || dt == TYPE_BOOL)
            fprintf(g->out, "  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.fmt_int, i32 0, i32 0), i32 %%t%d)\n", vt);
        else if (dt == TYPE_FLOAT)
            fprintf(g->out, "  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.fmt_flt, i32 0, i32 0), double %%t%d)\n", vt);
        else
            fprintf(g->out, "  call i32 (i8*, ...) @printf(i8* getelementptr ([4 x i8], [4 x i8]* @.fmt_str, i32 0, i32 0), i8* %%t%d)\n", vt);
        break;
    }

    case NODE_RETURN: {
        int vt = emit_expr(g, node->children[0]);
        DataType dt = node->children[0]->data_type;
        fprintf(g->out, "  ret %s %%t%d\n", llvm_type(dt), vt);
        break;
    }

    case NODE_IF: {
        int cond = emit_expr(g, node->children[0]);
        int lbl_then = next_lbl(g);
        int lbl_else = next_lbl(g);
        int lbl_end  = next_lbl(g);
        fprintf(g->out, "  br i1 %%t%d, label %%L%d, label %%L%d\n",
                cond, lbl_then, lbl_else);
        fprintf(g->out, "L%d:\n", lbl_then);
        emit_stmt(g, node->children[1]);
        fprintf(g->out, "  br label %%L%d\n", lbl_end);
        fprintf(g->out, "L%d:\n", lbl_else);
        if (node->num_children == 3) emit_stmt(g, node->children[2]);
        fprintf(g->out, "  br label %%L%d\n", lbl_end);
        fprintf(g->out, "L%d:\n", lbl_end);
        break;
    }

    case NODE_WHILE: {
        int lbl_cond = next_lbl(g);
        int lbl_body = next_lbl(g);
        int lbl_end  = next_lbl(g);
        fprintf(g->out, "  br label %%L%d\n", lbl_cond);
        fprintf(g->out, "L%d:\n", lbl_cond);
        int cond = emit_expr(g, node->children[0]);
        fprintf(g->out, "  br i1 %%t%d, label %%L%d, label %%L%d\n",
                cond, lbl_body, lbl_end);
        fprintf(g->out, "L%d:\n", lbl_body);
        emit_stmt(g, node->children[1]);
        fprintf(g->out, "  br label %%L%d\n", lbl_cond);
        fprintf(g->out, "L%d:\n", lbl_end);
        break;
    }

    case NODE_FOR: {
        emit_stmt(g, node->children[0]);   /* init */
        int lbl_cond = next_lbl(g);
        int lbl_body = next_lbl(g);
        int lbl_end  = next_lbl(g);
        fprintf(g->out, "  br label %%L%d\n", lbl_cond);
        fprintf(g->out, "L%d:\n", lbl_cond);
        int cond = emit_expr(g, node->children[1]);
        fprintf(g->out, "  br i1 %%t%d, label %%L%d, label %%L%d\n",
                cond, lbl_body, lbl_end);
        fprintf(g->out, "L%d:\n", lbl_body);
        emit_stmt(g, node->children[3]);   /* body */
        emit_stmt(g, node->children[2]);   /* step */
        fprintf(g->out, "  br label %%L%d\n", lbl_cond);
        fprintf(g->out, "L%d:\n", lbl_end);
        break;
    }

    case NODE_FUNC_DECL: {
        /* Collect params */
        char params_buf[512] = "";
        ASTNode *p = node->children[0];
        int first = 1;
        while (p && p->type == NODE_PARAM) {
            char piece[64];
            Symbol *ps = symtable_lookup(g->st, p->value.sval);
            const char *pt = ps ? llvm_type(ps->type) : "i32";
            snprintf(piece, sizeof(piece), "%s%s %%%s",
                     first ? "" : ", ", pt, p->value.sval);
            strncat(params_buf, piece, sizeof(params_buf) - strlen(params_buf) - 1);
            first = 0;
            p = p->next;
        }
        DataType rdt = node_to_dtype(node->children[1]->type);
        /* Guard against infinite recursion from node_to_dtype */
        const char *ret_t = llvm_type(rdt);
        fprintf(g->out, "\ndefine %s @%s(%s) {\nentry:\n",
                ret_t, node->value.sval, params_buf);
        strncpy(g->current_func, node->value.sval, sizeof(g->current_func) - 1);
        emit_stmt(g, node->children[2]);   /* body block */
        /* Ensure a terminator */
        if (rdt == TYPE_VOID) fprintf(g->out, "  ret void\n");
        fprintf(g->out, "}\n");
        break;
    }

    case NODE_BLOCK:
        if (node->children[0]) emit_stmt(g, node->children[0]);
        break;

    case NODE_FUNC_CALL:
        emit_expr(g, node);
        break;

    case NODE_PROGRAM:
        /* Emit global declarations first (format strings for printf) */
        fprintf(g->out, "@.fmt_int = private constant [4 x i8] c\"%%d\\0A\\00\"\n");
        fprintf(g->out, "@.fmt_flt = private constant [4 x i8] c\"%%f\\0A\\00\"\n");
        fprintf(g->out, "@.fmt_str = private constant [4 x i8] c\"%%s\\0A\\00\"\n");
        fprintf(g->out, "declare i32 @printf(i8*, ...)\n");
        fprintf(g->out, "declare i32 @scanf(i8*, ...)\n\n");

        fprintf(g->out, "define i32 @main() {\nentry:\n");
        if (node->children[0]) {
            ASTNode *stmt = node->children[0];
            while (stmt) {
                emit_stmt(g, stmt);
                stmt = stmt->next;
            }
        }
        fprintf(g->out, "  ret i32 0\n}\n");
        break;

    default:
        /* Fallthrough — walk children */
        for (int i = 0; i < node->num_children; i++) emit_stmt(g, node->children[i]);
        if (node->next) emit_stmt(g, node->next);
        break;
    }

    if (node->next && node->type != NODE_PROGRAM) emit_stmt(g, node->next);
}

/* ------------------------------------------------------------------ */
/* Public interface                                                     */
/* ------------------------------------------------------------------ */

IRGen *irgen_new(FILE *out, SymbolTable *st) {
    IRGen *g = calloc(1, sizeof(IRGen));
    g->out          = out;
    g->st           = st;
    g->tmp_counter  = 0;
    g->label_counter = 0;
    return g;
}

void irgen_free(IRGen *g) { free(g); }

void irgen_emit(IRGen *g, ASTNode *root) {
    fprintf(g->out, "; LLVM IR generated by DSL Compiler\n");
    fprintf(g->out, "; Author: WASIM SHARAFATH M Y | RA2311026050171 | SRMIST Trichy\n");
    fprintf(g->out, "source_filename = \"dsl_program\"\n");
    fprintf(g->out, "target triple = \"x86_64-pc-linux-gnu\"\n\n");
    emit_stmt(g, root);
}

/* Helper used inside emit_stmt for func_decl */
DataType node_to_dtype(NodeType nt) {
    switch (nt) {
        case NODE_TYPE_INT:    return TYPE_INT;
        case NODE_TYPE_FLOAT:  return TYPE_FLOAT;
        case NODE_TYPE_BOOL:   return TYPE_BOOL;
        case NODE_TYPE_STRING: return TYPE_STRING;
        default:               return TYPE_VOID;
    }
}
