
#include "compiler.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ── constant pool ───────────────────────────────────────────── */
typedef struct {
    int    is_str;
    double num;
    char  *str;
} Const;

/* ── instruction ─────────────────────────────────────────────── */
typedef struct { uint8_t op; int32_t arg; } Instr;

/* ── compiler context ────────────────────────────────────────── */
typedef struct {
    Const *pool;
    int    pool_count, pool_cap;
    Instr *code;
    int    code_count, code_cap;
} Ctx;

static int pool_add_num(Ctx *c, double d) {
    for (int i = 0; i < c->pool_count; i++)
        if (!c->pool[i].is_str && c->pool[i].num == d) return i;
    if (c->pool_count >= c->pool_cap) {
        c->pool_cap = c->pool_cap ? c->pool_cap * 2 : 8;
        c->pool = (Const *)xrealloc(c->pool, sizeof(Const) * (size_t)c->pool_cap);
    }
    c->pool[c->pool_count] = (Const){0, d, NULL};
    return c->pool_count++;
}

static int pool_add_str(Ctx *c, const char *s) {
    for (int i = 0; i < c->pool_count; i++)
        if (c->pool[i].is_str && strcmp(c->pool[i].str, s) == 0) return i;
    if (c->pool_count >= c->pool_cap) {
        c->pool_cap = c->pool_cap ? c->pool_cap * 2 : 8;
        c->pool = (Const *)xrealloc(c->pool, sizeof(Const) * (size_t)c->pool_cap);
    }
    c->pool[c->pool_count] = (Const){1, 0.0, xstrdup(s)};
    return c->pool_count++;
}

static int emit(Ctx *c, uint8_t op, int32_t arg) {
    if (c->code_count >= c->code_cap) {
        c->code_cap = c->code_cap ? c->code_cap * 2 : 16;
        c->code = (Instr *)xrealloc(c->code, sizeof(Instr) * (size_t)c->code_cap);
    }
    int pos = c->code_count;
    c->code[c->code_count++] = (Instr){op, arg};
    return pos;
}

static void patch(Ctx *c, int pos, int32_t arg) {
    c->code[pos].arg = arg;
}

/* forward declaration */
static void compile_node(Ctx *c, Node *n);
static void compile_block(Ctx *c, NodeList *bl);
static void compile_expr(Ctx *c, Node *n);

static void compile_expr(Ctx *c, Node *n) {
    if (!n) { emit(c, OP_PUSH_NULL, 0); return; }
    switch (n->kind) {
        case ND_NUMBER:  emit(c, OP_PUSH_NUM, pool_add_num(c, n->num)); break;
        case ND_STRING:  emit(c, OP_PUSH_STR, pool_add_str(c, n->str)); break;
        case ND_BOOL:    emit(c, n->bool_val ? OP_PUSH_TRUE : OP_PUSH_FALSE, 0); break;
        case ND_NOTHING: emit(c, OP_PUSH_NULL, 0); break;
        case ND_IDENT:   emit(c, OP_LOAD, pool_add_str(c, n->str)); break;

        case ND_NEGATE:
            compile_expr(c, n->unary);
            emit(c, OP_NEG, 0);
            break;
        case ND_NOT:
            compile_expr(c, n->unary);
            emit(c, OP_NOT, 0);
            break;

        case ND_BINOP:
            compile_expr(c, n->binop.left);
            compile_expr(c, n->binop.right);
                switch (n->binop.op) {
                case BINOP_ADD: emit(c, OP_ADD, 0); break;
                case BINOP_SUB: emit(c, OP_SUB, 0); break;
                case BINOP_MUL: emit(c, OP_MUL, 0); break;
                case BINOP_DIV: emit(c, OP_DIV, 0); break;
                case BINOP_MOD: emit(c, OP_MOD, 0); break;
                }
            break;

        case ND_CMP:
            compile_expr(c, n->cmp.left);
            compile_expr(c, n->cmp.right);
            switch (n->cmp.op) {
                case CMP_EQ:  emit(c, OP_CMP_EQ,  0); break;
                case CMP_NEQ: emit(c, OP_CMP_NEQ, 0); break;
                case CMP_GT:  emit(c, OP_CMP_GT,  0); break;
                case CMP_LT:  emit(c, OP_CMP_LT,  0); break;
                case CMP_GTE: emit(c, OP_CMP_GTE, 0); break;
                case CMP_LTE: emit(c, OP_CMP_LTE, 0); break;
            }
            break;

        case ND_AND:
            compile_expr(c, n->logical.left);
            compile_expr(c, n->logical.right);
            emit(c, OP_AND, 0);
            break;
        case ND_OR:
            compile_expr(c, n->logical.left);
            compile_expr(c, n->logical.right);
            emit(c, OP_OR, 0);
            break;

        case ND_LIST_LEN:
            emit(c, OP_LIST_LEN, pool_add_str(c, n->list_len));
            break;

        case ND_LIST_GET:
            compile_expr(c, n->list_get.index);
            emit(c, OP_LIST_GET, pool_add_str(c, n->list_get.name));
            break;

        case ND_CALL_EXPR:
            for (int i = 0; i < n->call.args.count; i++)
                compile_expr(c, n->call.args.items[i]);
            emit(c, OP_LOAD, pool_add_str(c, n->call.name));
            emit(c, OP_CALL, n->call.args.count);
            break;

        default:
            /* fall back: treat as statement expression */
            compile_node(c, n);
    }
}

static void compile_block(Ctx *c, NodeList *bl) {
    for (int i = 0; i < bl->count; i++)
        compile_node(c, bl->items[i]);
}

static void compile_node(Ctx *c, Node *n) {
    if (!n) return;
    switch (n->kind) {
        case ND_SAY:
            compile_expr(c, n->say);
            emit(c, OP_PRINT, 0);
            break;

        case ND_SET:
            compile_expr(c, n->set.expr);
            emit(c, OP_STORE, pool_add_str(c, n->set.name));
            break;

        case ND_ASK: {
            int pi = pool_add_str(c, n->ask.prompt);
            int ni = pool_add_str(c, n->ask.name);
            emit(c, OP_PUSH_STR, pi);
            emit(c, OP_INPUT, ni);
            break;
        }

        case ND_IF: {
            int *patch_ends = (int *)xmalloc(sizeof(int) * (size_t)n->if_stmt.count);
            for (int i = 0; i < n->if_stmt.count; i++) {
                Node *cond = n->if_stmt.conds.items[i];
                int skip_pos = -1;
                if (cond) {
                    compile_expr(c, cond);
                    skip_pos = emit(c, OP_JUMP_FALSE, 0); /* patch later */
                }
                compile_block(c, &n->if_stmt.blocks[i]);
                patch_ends[i] = emit(c, OP_JUMP, 0);     /* jump past all branches */
                if (skip_pos >= 0) patch(c, skip_pos, c->code_count);
            }
            int end = c->code_count;
            for (int i = 0; i < n->if_stmt.count; i++) patch(c, patch_ends[i], end);
            free(patch_ends);
            break;
        }

        case ND_REPEAT: {
            compile_expr(c, n->repeat.count);
            /* store counter in __repeat_i__ */
            int ci = pool_add_str(c, "__ri__");
            emit(c, OP_STORE, ci);
            /* emit 0 as current count */
            emit(c, OP_PUSH_NUM, pool_add_num(c, 0.0));
            int cni = pool_add_str(c, "__rn__");
            emit(c, OP_STORE, cni);

            int loop_start = c->code_count;
            /* while __rn__ < __ri__ */
            emit(c, OP_LOAD, cni);
            emit(c, OP_LOAD, ci);
            emit(c, OP_CMP_LT, 0);
            int exit_jump = emit(c, OP_JUMP_FALSE, 0);

            compile_block(c, &n->repeat.body);

            /* __rn__ = __rn__ + 1 */
            emit(c, OP_LOAD, cni);
            emit(c, OP_PUSH_NUM, pool_add_num(c, 1.0));
            emit(c, OP_ADD, 0);
            emit(c, OP_STORE, cni);
            emit(c, OP_JUMP, loop_start);
            patch(c, exit_jump, c->code_count);
            break;
        }

        case ND_WHILE: {
            int loop_start = c->code_count;
            compile_expr(c, n->while_stmt.cond);
            int exit_jump = emit(c, OP_JUMP_FALSE, 0);
            compile_block(c, &n->while_stmt.body);
            emit(c, OP_JUMP, loop_start);
            patch(c, exit_jump, c->code_count);
            break;
        }

        case ND_DEFINE: {
            /* jump over function body */
            int skip = emit(c, OP_JUMP, 0);
            int fn_start = c->code_count;
            compile_block(c, &n->define.body);
            emit(c, OP_PUSH_NULL, 0);
            emit(c, OP_RET, 0);
            patch(c, skip, c->code_count);

            int name_idx = pool_add_str(c, n->define.name);
            emit(c, OP_DEF_FUNC, name_idx);
            emit(c, OP_DEF_FUNC, fn_start);        /* body start */
            emit(c, OP_DEF_FUNC, n->define.param_count);
            for (int i = 0; i < n->define.param_count; i++)
                emit(c, OP_DEF_FUNC, pool_add_str(c, n->define.params[i]));
            break;
        }

        case ND_CALL_STMT:
            for (int i = 0; i < n->call.args.count; i++)
                compile_expr(c, n->call.args.items[i]);
            emit(c, OP_LOAD, pool_add_str(c, n->call.name));
            emit(c, OP_CALL, n->call.args.count);
            emit(c, OP_POP, 0);  /* discard return value */
            break;

        case ND_RETURN:
            compile_expr(c, n->ret);
            emit(c, OP_RET, 0);
            break;

        case ND_MAKE_LIST:
            emit(c, OP_LIST_NEW, pool_add_str(c, n->list_name));
            break;

        case ND_ADD_LIST:
            compile_expr(c, n->add_list.val);
            emit(c, OP_LIST_PUSH, pool_add_str(c, n->add_list.name));
            break;

        case ND_STOP:  emit(c, OP_JUMP, -1); break;  /* -1 = break sentinel */
        case ND_SKIP:  emit(c, OP_JUMP, -2); break;  /* -2 = continue sentinel */

        case ND_PROGRAM:
            compile_block(c, &n->program);
            emit(c, OP_HALT, 0);
            break;

        default: break;
    }
}

/* ── write .sec file ─────────────────────────────────────────── */
static void write_u8 (FILE *f, uint8_t  v) { fwrite(&v, 1, 1, f); }
static void write_u32(FILE *f, uint32_t v) { fwrite(&v, 4, 1, f); }
static void write_i32(FILE *f, int32_t  v) { fwrite(&v, 4, 1, f); }
static void write_f64(FILE *f, double   v) { fwrite(&v, 8, 1, f); }

void compile_to_sec(Node *program, const char *out_path) {
    Ctx c = {0};
    compile_node(&c, program);

    FILE *f = fopen(out_path, "wb");
    if (!f) fatal("cannot write file '%s'", out_path);

    /* header */
    fwrite(SEC_MAGIC, 1, 4, f);
    write_u8(f, SEC_VERSION);

    /* constant pool */
    write_u32(f, (uint32_t)c.pool_count);
    for (int i = 0; i < c.pool_count; i++) {
        if (c.pool[i].is_str) {
            write_u8(f, 1);
            uint32_t len = (uint32_t)strlen(c.pool[i].str);
            write_u32(f, len);
            fwrite(c.pool[i].str, 1, len, f);
        } else {
            write_u8(f, 0);
            write_f64(f, c.pool[i].num);
        }
    }

    /* instructions */
    write_u32(f, (uint32_t)c.code_count);
    for (int i = 0; i < c.code_count; i++) {
        write_u8(f, c.code[i].op);
        write_i32(f, c.code[i].arg);
    }

    fclose(f);

    /* cleanup */
    for (int i = 0; i < c.pool_count; i++) if (c.pool[i].is_str) free(c.pool[i].str);
    free(c.pool);
    free(c.code);
}
