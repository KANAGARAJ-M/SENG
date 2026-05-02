
#include "vm.h"
#include "bytecode.h"
#include "value.h"
#include "env.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ── read helpers ────────────────────────────────────────────── */
static uint8_t  read_u8 (FILE *f) { uint8_t  v; fread(&v, 1, 1, f); return v; }
static uint32_t read_u32(FILE *f) { uint32_t v; fread(&v, 4, 1, f); return v; }
static int32_t  read_i32(FILE *f) { int32_t  v; fread(&v, 4, 1, f); return v; }
static double   read_f64(FILE *f) { double   v; fread(&v, 8, 1, f); return v; }

typedef struct { uint8_t op; int32_t arg; } VmInstr;

/* ── vm stack ────────────────────────────────────────────────── */
#define STACK_MAX 1024
typedef struct {
    Value *data[STACK_MAX];
    int    top;
} Stack;

static void push(Stack *s, Value *v) {
    if (s->top >= STACK_MAX) fatal("stack overflow");
    s->data[s->top++] = v;
}
#if defined(__GNUC__)
__attribute__((noinline))
#endif
static Value *pop(Stack *s) {
    if (s->top <= 0) fatal("stack underflow");
    int idx = s->top - 1;
    s->top = idx;
    return s->data[idx];
}

/* ── function record in VM ───────────────────────────────────── */
typedef struct {
    char    *name;
    int      body_start;  /* instruction index */
    int      param_count;
    int     *param_idxs;  /* constant pool indices for param names */
} VmFunc;

void vm_run_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) fatal("cannot open '%s'", path);

    /* ── read header ── */
    char magic[5] = {0};
    fread(magic, 1, 4, f);
    if (strcmp(magic, SEC_MAGIC) != 0) fatal("not a valid .sec file");
    uint8_t ver = read_u8(f);
    if (ver != SEC_VERSION) fatal("unsupported .sec version %d", ver);

    /* ── read constant pool ── */
    uint32_t pool_count = read_u32(f);
    char   **pool_str   = (char   **)xcalloc(pool_count, sizeof(char *));
    double  *pool_num   = (double  *)xcalloc(pool_count, sizeof(double));
    int     *pool_is_str= (int     *)xcalloc(pool_count, sizeof(int));

    for (uint32_t i = 0; i < pool_count; i++) {
        uint8_t tp = read_u8(f);
        pool_is_str[i] = tp;
        if (tp) {
            uint32_t len = read_u32(f);
            char *s = (char *)xmalloc(len + 1);
            fread(s, 1, len, f);
            s[len]       = '\0';
            pool_str[i]  = s;
        } else {
            pool_num[i] = read_f64(f);
        }
    }

    /* ── read instructions ── */
    uint32_t code_count = read_u32(f);
    VmInstr *code = (VmInstr *)xmalloc(sizeof(VmInstr) * code_count);
    for (uint32_t i = 0; i < code_count; i++) {
        code[i].op  = read_u8(f);
        code[i].arg = read_i32(f);
    }
    fclose(f);

    /* ── scan for function definitions ── */
    int      func_count = 0, func_cap = 0;
    VmFunc  *funcs = NULL;

    for (uint32_t i = 0; i < code_count; i++) {
        if (code[i].op == OP_DEF_FUNC) {
            int name_idx   = code[i].arg;      i++;
            int body_start = code[i].arg;       i++;
            int pcount     = code[i].arg;

            if (func_count >= func_cap) {
                func_cap = func_cap ? func_cap * 2 : 4;
                funcs = (VmFunc *)xrealloc(funcs, sizeof(VmFunc) * (size_t)func_cap);
            }
            VmFunc fn;
            fn.name        = pool_str[name_idx];
            fn.body_start  = body_start;
            fn.param_count = pcount;
            fn.param_idxs  = (int *)xmalloc(sizeof(int) * (size_t)(pcount ? pcount : 1));
            for (int pi = 0; pi < pcount; pi++) {
                i++;
                fn.param_idxs[pi] = code[i].arg;
            }
            funcs[func_count++] = fn;
        }
    }

    /* ── execution ── */
    Stack  stk  = {0};
    Env   *globals = env_new(NULL);

    /* register functions */
    for (int i = 0; i < func_count; i++) {
        SengFunc *sf = (SengFunc *)xcalloc(1, sizeof(SengFunc));
        sf->name        = xstrdup(funcs[i].name);
        sf->param_count = funcs[i].param_count;
        sf->params      = (char **)xmalloc(sizeof(char *) *
                          (size_t)(sf->param_count ? sf->param_count : 1));
        for (int pi = 0; pi < sf->param_count; pi++)
            sf->params[pi] = xstrdup(pool_str[funcs[i].param_idxs[pi]]);
        sf->body_ref = (void *)(intptr_t)funcs[i].body_start;  /* not used in VM path */
        Value *fv = val_func(sf);
        env_set(globals, sf->name, fv);
        val_deref(fv);
    }

    /* ── call stack for functions ── */
    typedef struct { int ret_pc; Env *ret_env; int ret_sp; } CallFrame;
    CallFrame call_stack[256];
    int       call_top = 0;
    Env      *cur_env  = globals;

    int32_t pc = 0;
    while ((uint32_t)pc < code_count) {
        uint8_t op  = code[pc].op;
        int32_t arg = code[pc].arg;
        pc++;

        switch (op) {
            case OP_PUSH_NUM:  push(&stk, val_num(pool_num[arg])); break;
            case OP_PUSH_STR:  push(&stk, val_str(pool_str[arg])); break;
            case OP_PUSH_TRUE: push(&stk, val_bool(1)); break;
            case OP_PUSH_FALSE:push(&stk, val_bool(0)); break;
            case OP_PUSH_NULL: push(&stk, val_null()); break;

            case OP_LOAD: {
                const char *nm = pool_str[arg];
                Value *v = env_get(cur_env, nm);
                if (!v) fatal("'%s' is not defined", nm);
                push(&stk, val_copy(v));
                break;
            }
            case OP_STORE: {
                Value *v = pop(&stk);
                env_set(cur_env, pool_str[arg], v);
                val_deref(v);
                break;
            }
            case OP_POP: {
                Value *v = pop(&stk);
                val_deref(v);
                break;
            }

            case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV: case OP_MOD: {
                Value *b = pop(&stk);
                Value *a = pop(&stk);
                Value *res;
                if (op == OP_ADD && (a->type == VAL_STR || b->type == VAL_STR)) {
                    char *sa = val_to_string(a), *sb = val_to_string(b);
                    char *s  = (char *)xmalloc(strlen(sa) + strlen(sb) + 1);
                    strcpy(s, sa); strcat(s, sb);
                    res = val_str(s); free(s); free(sa); free(sb);
                } else {
                    double da = a->num, db = b->num;
                    switch (op) {
                        case OP_ADD: res = val_num(da + db); break;
                        case OP_SUB: res = val_num(da - db); break;
                        case OP_MUL: res = val_num(da * db); break;
                        case OP_DIV:
                            if (db == 0) fatal("division by zero");
                            res = val_num(da / db); break;
                        case OP_MOD:
                            if (db == 0) fatal("modulo by zero");
                            res = val_num(fmod(da, db)); break;
                        default: res = val_null();
                    }
                }
                val_deref(a); val_deref(b);
                push(&stk, res);
                break;
            }

            case OP_NEG: {
                Value *v = pop(&stk);
                push(&stk, val_num(-v->num));
                val_deref(v);
                break;
            }
            case OP_NOT: {
                Value *v = pop(&stk);
                push(&stk, val_bool(!val_truthy(v)));
                val_deref(v);
                break;
            }

            case OP_CMP_EQ: case OP_CMP_NEQ: case OP_CMP_GT:
            case OP_CMP_LT: case OP_CMP_GTE: case OP_CMP_LTE: {
                Value *b = pop(&stk), *a = pop(&stk);
                int r;
                if (a->type == VAL_STR && b->type == VAL_STR) {
                    int c = strcmp(a->str, b->str);
                    switch (op) {
                        case OP_CMP_EQ:  r = (c == 0); break;
                        case OP_CMP_NEQ: r = (c != 0); break;
                        case OP_CMP_GT:  r = (c  > 0); break;
                        case OP_CMP_LT:  r = (c  < 0); break;
                        case OP_CMP_GTE: r = (c >= 0); break;
                        case OP_CMP_LTE: r = (c <= 0); break;
                        default: r = 0;
                    }
                } else {
                    double da = a->num, db = b->num;
                    switch (op) {
                        case OP_CMP_EQ:  r = (da == db); break;
                        case OP_CMP_NEQ: r = (da != db); break;
                        case OP_CMP_GT:  r = (da  > db); break;
                        case OP_CMP_LT:  r = (da  < db); break;
                        case OP_CMP_GTE: r = (da >= db); break;
                        case OP_CMP_LTE: r = (da <= db); break;
                        default: r = 0;
                    }
                }
                val_deref(a); val_deref(b);
                push(&stk, val_bool(r));
                break;
            }

            case OP_AND: {
                Value *b = pop(&stk), *a = pop(&stk);
                push(&stk, val_bool(val_truthy(a) && val_truthy(b)));
                val_deref(a); val_deref(b);
                break;
            }
            case OP_OR: {
                Value *b = pop(&stk), *a = pop(&stk);
                push(&stk, val_bool(val_truthy(a) || val_truthy(b)));
                val_deref(a); val_deref(b);
                break;
            }

            case OP_PRINT: {
                Value *v = pop(&stk);
                val_print(v);
                val_deref(v);
                break;
            }
            case OP_INPUT: {
                Value *prompt = pop(&stk);
                char *ps = val_to_string(prompt);
                printf("%s", ps); free(ps);
                fflush(stdout);
                val_deref(prompt);
                char buf[1024] = {0};
                if (fgets(buf, sizeof buf, stdin)) {
                    size_t len = strlen(buf);
                    if (len > 0 && buf[len-1] == '\n') buf[len-1] = '\0';
                }
                Value *v = val_str(buf);
                env_set(cur_env, pool_str[arg], v);
                val_deref(v);
                break;
            }

            case OP_JUMP:       pc = arg; break;
            case OP_JUMP_FALSE: {
                Value *v = pop(&stk);
                if (!val_truthy(v)) pc = arg;
                val_deref(v);
                break;
            }

            case OP_CALL: {
                /* top of stack = function value, below = args (arg = count) */
                Value *fv = pop(&stk);
                if (!fv || fv->type != VAL_FUNC)
                    fatal("attempted to call a non-function");
                SengFunc *fn = fv->func;
                if (arg != fn->param_count)
                    fatal("'%s': expected %d args, got %d",
                          fn->name, fn->param_count, arg);

                /* find body start */
                int body = -1;
                for (int fi = 0; fi < func_count; fi++) {
                    if (strcmp(funcs[fi].name, fn->name) == 0) {
                        body = funcs[fi].body_start; break;
                    }
                }
                if (body < 0) fatal("function '%s' body not found", fn->name);

                /* save state */
                if (call_top >= 256) fatal("call stack overflow");
                call_stack[call_top++] = (CallFrame){pc, cur_env, stk.top - arg};

                /* new scope */
                Env *fenv = env_new(globals);
                /* bind params (they are in order on stack) */
                for (int pi = fn->param_count - 1; pi >= 0; pi--) {
                    Value *av = pop(&stk);
                    env_set(fenv, fn->params[pi], av);
                    val_deref(av);
                }
                cur_env = fenv;
                pc = body;
                val_deref(fv);
                break;
            }

            case OP_RET: {
                Value *rv = pop(&stk);
                if (call_top <= 0) { push(&stk, rv); goto done; }
                CallFrame fr = call_stack[--call_top];
                env_free(cur_env);
                cur_env = fr.ret_env;
                pc      = fr.ret_pc;
                push(&stk, rv);
                break;
            }

            case OP_HALT: goto done;

            case OP_LIST_NEW: {
                Value *v = val_list();
                env_set(cur_env, pool_str[arg], v);
                val_deref(v);
                break;
            }
            case OP_LIST_PUSH: {
                Value *item = pop(&stk);
                Value *lv   = env_get(cur_env, pool_str[arg]);
                if (!lv || lv->type != VAL_LIST)
                    fatal("'%s' is not a list", pool_str[arg]);
                SengList *sl = lv->list;
                if (sl->count >= sl->cap) {
                    sl->cap   = sl->cap ? sl->cap * 2 : 4;
                    sl->items = (Value **)xrealloc(sl->items,
                                sizeof(Value *) * (size_t)sl->cap);
                }
                sl->items[sl->count++] = item;
                break;
            }
            case OP_LIST_GET: {
                Value *idx_v = pop(&stk);
                int    idx   = (int)idx_v->num - 1; /* 1-based */
                val_deref(idx_v);
                Value *lv = env_get(cur_env, pool_str[arg]);
                if (!lv || lv->type != VAL_LIST)
                    fatal("'%s' is not a list", pool_str[arg]);
                if (idx < 0 || idx >= lv->list->count)
                    fatal("list index out of range");
                push(&stk, val_copy(lv->list->items[idx]));
                break;
            }
            case OP_LIST_LEN: {
                Value *lv = env_get(cur_env, pool_str[arg]);
                if (!lv || lv->type != VAL_LIST)
                    fatal("'%s' is not a list", pool_str[arg]);
                push(&stk, val_num((double)lv->list->count));
                break;
            }

            case OP_DEF_FUNC:
                /* already scanned; skip the extra OP_DEF_FUNC operand instructions */
                /* peek arg = body_start, then skip param_count + params */
                {
                    pc++;                         /* skip body_start instr  */
                    int pc2 = code[pc].arg; pc++; /* get param count        */
                    pc += pc2;                    /* skip param name instrs */
                }
                break;

            default:
                fatal("unknown opcode %d", op);
        }
    }
done:
    /* clean up stack */
    while (stk.top > 0) { Value *v = pop(&stk); val_deref(v); }
    env_free(globals);
    for (int i = 0; i < func_count; i++) free(funcs[i].param_idxs);
    free(funcs);
    for (uint32_t i = 0; i < pool_count; i++) if (pool_is_str[i]) free(pool_str[i]);
    free(pool_str); free(pool_num); free(pool_is_str);
    free(code);
}
