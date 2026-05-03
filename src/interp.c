
#include "interp.h"
#include "lexer.h"
#include "parser.h"
#include "packages.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* ── control-flow signals ────────────────────────────────────── */
typedef enum { SIG_NONE, SIG_RETURN, SIG_STOP, SIG_SKIP } Signal;

struct Interp {
    Env   *globals;
    Signal signal;
    Value *ret_val;
    Node **imported;     /* imported ASTs kept alive so body_ref stays valid */
    int    import_count;
    int    import_cap;
};

/* forward decls */
static Value  *eval (Interp *in, Env *e, Node *n);
static Signal  exec (Interp *in, Env *e, Node *n);
static Signal  exec_block(Interp *in, Env *e, NodeList *bl);

/* ── helpers ─────────────────────────────────────────────────── */

static double num_of(Value *v, int line) {
    if (!v || v->type != VAL_NUM)
        fatal("line %d: expected a number", line);
    return v->num;
}

static char *concat_vals(Value *a, Value *b, int line) {
    (void)line;
    char *sa = val_to_string(a);
    char *sb = val_to_string(b);
    char *r  = (char *)xmalloc(strlen(sa) + strlen(sb) + 1);
    strcpy(r, sa); strcat(r, sb);
    free(sa); free(sb);
    return r;
}

/* ── eval ───────────────────────────────────────────────────── */

static Value *eval(Interp *in, Env *e, Node *n) {
    if (!n) return val_null();
    switch (n->kind) {
        case ND_NUMBER:  return val_num(n->num);
        case ND_BOOL:    return val_bool(n->bool_val);
        case ND_NOTHING: return val_null();
        case ND_STRING:  return val_str(n->str);

        case ND_IDENT: {
            Value *v = env_get(e, n->str);
            if (!v) fatal("line %d: '%s' is not defined", n->line, n->str);
            return val_copy(v);
        }

        case ND_NEGATE: {
            Value *v = eval(in, e, n->unary);
            double d = num_of(v, n->line);
            val_deref(v);
            return val_num(-d);
        }

        case ND_NOT: {
            Value *v = eval(in, e, n->unary);
            int    t = val_truthy(v);
            val_deref(v);
            return val_bool(!t);
        }

        case ND_BINOP: {
            Value *l = eval(in, e, n->binop.left);
            Value *r = eval(in, e, n->binop.right);
            Value *res;
            /* string concat with + */
            if (n->binop.op == BINOP_ADD &&
                (l->type == VAL_STR || r->type == VAL_STR)) {
                char *s = concat_vals(l, r, n->line);
                res = val_str(s); free(s);
            } else {
                double a = num_of(l, n->line);
                double b = num_of(r, n->line);
                switch (n->binop.op) {
                    case BINOP_ADD: res = val_num(a + b); break;
                    case BINOP_SUB: res = val_num(a - b); break;
                    case BINOP_MUL: res = val_num(a * b); break;
                    case BINOP_DIV:
                        if (b == 0) fatal("line %d: division by zero", n->line);
                        res = val_num(a / b); break;
                    case BINOP_MOD:
                        if (b == 0) fatal("line %d: modulo by zero", n->line);
                        res = val_num(fmod(a, b)); break;
                    default: res = val_null();
                }
            }
            val_deref(l); val_deref(r);
            return res;
        }

        case ND_CMP: {
            Value *l = eval(in, e, n->cmp.left);
            Value *r = eval(in, e, n->cmp.right);
            int    result;
            /* string comparisons */
            if (l->type == VAL_STR && r->type == VAL_STR) {
                int c = strcmp(l->str, r->str);
                switch (n->cmp.op) {
                    case CMP_EQ:  result = (c == 0); break;
                    case CMP_NEQ: result = (c != 0); break;
                    case CMP_GT:  result = (c  > 0); break;
                    case CMP_LT:  result = (c  < 0); break;
                    case CMP_GTE: result = (c >= 0); break;
                    case CMP_LTE: result = (c <= 0); break;
                    default:      result = 0;
                }
            } else if (l->type == VAL_BOOL && r->type == VAL_BOOL) {
                switch (n->cmp.op) {
                    case CMP_EQ:  result = (l->bool_val == r->bool_val); break;
                    case CMP_NEQ: result = (l->bool_val != r->bool_val); break;
                    default:      result = 0;
                }
            } else if (l->type == VAL_NULL || r->type == VAL_NULL) {
                result = (n->cmp.op == CMP_EQ)
                         ? (l->type == VAL_NULL && r->type == VAL_NULL)
                         : !(l->type == VAL_NULL && r->type == VAL_NULL);
            } else {
                double a = num_of(l, n->line);
                double b = num_of(r, n->line);
                switch (n->cmp.op) {
                    case CMP_EQ:  result = (a == b); break;
                    case CMP_NEQ: result = (a != b); break;
                    case CMP_GT:  result = (a  > b); break;
                    case CMP_LT:  result = (a  < b); break;
                    case CMP_GTE: result = (a >= b); break;
                    case CMP_LTE: result = (a <= b); break;
                    default:      result = 0;
                }
            }
            val_deref(l); val_deref(r);
            return val_bool(result);
        }

        case ND_AND: {
            Value *l = eval(in, e, n->logical.left);
            int    t = val_truthy(l); val_deref(l);
            if (!t) return val_bool(0);
            Value *r = eval(in, e, n->logical.right);
            int    u = val_truthy(r); val_deref(r);
            return val_bool(u);
        }

        case ND_OR: {
            Value *l = eval(in, e, n->logical.left);
            int    t = val_truthy(l); val_deref(l);
            if (t)  return val_bool(1);
            Value *r = eval(in, e, n->logical.right);
            int    u = val_truthy(r); val_deref(r);
            return val_bool(u);
        }

        case ND_LIST_LEN: {
            Value *v = env_get(e, n->list_len);
            if (!v || v->type != VAL_LIST)
                fatal("line %d: '%s' is not a list", n->line, n->list_len);
            return val_num((double)v->list->count);
        }

        case ND_LIST_GET: {
            Value *v = env_get(e, n->list_get.name);
            if (!v || v->type != VAL_LIST)
                fatal("line %d: '%s' is not a list", n->line, n->list_get.name);
            Value *idx_v = eval(in, e, n->list_get.index);
            int    idx   = (int)num_of(idx_v, n->line) - 1; /* 1-based */
            val_deref(idx_v);
            if (idx < 0 || idx >= v->list->count)
                fatal("line %d: list index %d out of range (size %d)",
                      n->line, idx + 1, v->list->count);
            return val_copy(v->list->items[idx]);
        }

        case ND_CALL_EXPR: {
            Value *fv = env_get(e, n->call.name);
            if (!fv || (fv->type != VAL_FUNC && fv->type != VAL_NATIVE))
                fatal("line %d: '%s' is not a function", n->line, n->call.name);

            /* native (built-in) function */
            if (fv->type == VAL_NATIVE) {
                SengNative *nat = fv->native;
                int argc = n->call.args.count;
                if (nat->arity >= 0 && argc != nat->arity)
                    fatal("line %d: '%s' expects %d argument(s), got %d",
                          n->line, nat->name, nat->arity, argc);
                Value **args = (Value **)xmalloc(sizeof(Value *) * (size_t)(argc > 0 ? argc : 1));
                for (int i = 0; i < argc; i++)
                    args[i] = eval(in, e, n->call.args.items[i]);
                Value *rv = nat->fn(args, argc);
                for (int i = 0; i < argc; i++) val_deref(args[i]);
                free(args);
                return rv;
            }

            /* user-defined function */
            SengFunc *fn = fv->func;
            if (n->call.args.count != fn->param_count)
                fatal("line %d: '%s' expects %d argument(s), got %d",
                      n->line, fn->name, fn->param_count, n->call.args.count);

            Env *fenv = env_new(in->globals);
            for (int i = 0; i < fn->param_count; i++) {
                Value *av = eval(in, e, n->call.args.items[i]);
                env_set(fenv, fn->params[i], av);
                val_deref(av);
            }

            NodeList *body = (NodeList *)fn->body_ref;
            exec_block(in, fenv, body);
            env_free(fenv);

            Value *rv = in->ret_val ? in->ret_val : val_null();
            in->ret_val = NULL;
            in->signal  = SIG_NONE;
            return rv;
        }

        default:
            fatal("line %d: cannot evaluate node kind %d", n->line, n->kind);
            return val_null();
    }
}

/* ── exec ──────────────────────────────────────────────────── */

static Signal exec_block(Interp *in, Env *e, NodeList *bl) {
    for (int i = 0; i < bl->count; i++) {
        Signal s = exec(in, e, bl->items[i]);
        if (s != SIG_NONE) return s;
    }
    return SIG_NONE;
}

static Signal exec(Interp *in, Env *e, Node *n) {
    if (!n) return SIG_NONE;
    switch (n->kind) {
        case ND_SET: {
            Value *v = eval(in, e, n->set.expr);
            env_update(e, n->set.name, v);  /* update existing or create new */
            val_deref(v);
            return SIG_NONE;
        }

        case ND_SAY: {
            Value *v = eval(in, e, n->say);
            val_print(v);
            val_deref(v);
            return SIG_NONE;
        }

        case ND_ASK: {
            printf("%s", n->ask.prompt);
            fflush(stdout);
            char buf[1024] = {0};
            if (fgets(buf, sizeof buf, stdin)) {
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n') buf[len - 1] = '\0';
            }
            Value *v = val_str(buf);
            env_set(e, n->ask.name, v);
            val_deref(v);
            return SIG_NONE;
        }

        case ND_IF: {
            for (int i = 0; i < n->if_stmt.count; i++) {
                Node *cond = n->if_stmt.conds.items[i];
                int   take;
                if (cond == NULL) {
                    take = 1;  /* else branch */
                } else {
                    Value *cv = eval(in, e, cond);
                    take      = val_truthy(cv);
                    val_deref(cv);
                }
                if (take) {
                    /* reuse parent env — if/else doesn't introduce new scope */
                    Signal s = exec_block(in, e, &n->if_stmt.blocks[i]);
                    return s;
                }
            }
            return SIG_NONE;
        }

        case ND_REPEAT: {
            Value *cv = eval(in, e, n->repeat.count);
            int    cnt = (int)num_of(cv, n->line);
            val_deref(cv);
            for (int i = 0; i < cnt; i++) {
                Signal s = exec_block(in, e, &n->repeat.body);  /* share env */
                if (s == SIG_RETURN) return SIG_RETURN;
                if (s == SIG_STOP)   break;
                /* SIG_SKIP → continue loop */
            }
            return SIG_NONE;
        }

        case ND_WHILE: {
            while (1) {
                Value *cv = eval(in, e, n->while_stmt.cond);
                int    t  = val_truthy(cv);
                val_deref(cv);
                if (!t) break;
                Signal s = exec_block(in, e, &n->while_stmt.body);  /* share env */
                if (s == SIG_RETURN) return SIG_RETURN;
                if (s == SIG_STOP)   break;
            }
            return SIG_NONE;
        }

        case ND_DEFINE: {
            SengFunc *fn = (SengFunc *)xcalloc(1, sizeof(SengFunc));
            fn->name        = xstrdup(n->define.name);
            fn->param_count = n->define.param_count;
            fn->params      = (char **)xmalloc(sizeof(char *) *
                              (size_t)(fn->param_count ? fn->param_count : 1));
            for (int i = 0; i < fn->param_count; i++)
                fn->params[i] = xstrdup(n->define.params[i]);
            fn->body_ref = &n->define.body;  /* points into AST */
            Value *fv = val_func(fn);
            env_set(e, fn->name, fv);
            val_deref(fv);
            return SIG_NONE;
        }

        case ND_CALL_STMT: {
            Value *fv = env_get(e, n->call.name);
            if (!fv || (fv->type != VAL_FUNC && fv->type != VAL_NATIVE))
                fatal("line %d: '%s' is not a function", n->line, n->call.name);

            /* native (built-in) function */
            if (fv->type == VAL_NATIVE) {
                SengNative *nat = fv->native;
                int argc = n->call.args.count;
                if (nat->arity >= 0 && argc != nat->arity)
                    fatal("line %d: '%s' expects %d argument(s), got %d",
                          n->line, nat->name, nat->arity, argc);
                Value **args = (Value **)xmalloc(sizeof(Value *) * (size_t)(argc > 0 ? argc : 1));
                for (int i = 0; i < argc; i++)
                    args[i] = eval(in, e, n->call.args.items[i]);
                Value *rv = nat->fn(args, argc);
                for (int i = 0; i < argc; i++) val_deref(args[i]);
                free(args);
                val_deref(rv);
                return SIG_NONE;
            }

            /* user-defined function */
            SengFunc *fn = fv->func;
            if (n->call.args.count != fn->param_count)
                fatal("line %d: '%s' expects %d argument(s), got %d",
                      n->line, fn->name, fn->param_count, n->call.args.count);

            Env *fenv = env_new(in->globals);
            for (int i = 0; i < fn->param_count; i++) {
                Value *av = eval(in, e, n->call.args.items[i]);
                env_set(fenv, fn->params[i], av);
                val_deref(av);
            }

            NodeList *body = (NodeList *)fn->body_ref;
            exec_block(in, fenv, body);
            env_free(fenv);

            if (in->ret_val) { val_deref(in->ret_val); in->ret_val = NULL; }
            in->signal = SIG_NONE;
            return SIG_NONE;
        }

        case ND_RETURN: {
            Value *v = eval(in, e, n->ret);
            if (in->ret_val) val_deref(in->ret_val);
            in->ret_val = v;
            in->signal  = SIG_RETURN;
            return SIG_RETURN;
        }

        case ND_MAKE_LIST: {
            Value *v = val_list();
            env_set(e, n->list_name, v);
            val_deref(v);
            return SIG_NONE;
        }

        case ND_ADD_LIST: {
            Value *lv = env_get(e, n->add_list.name);
            if (!lv || lv->type != VAL_LIST)
                fatal("line %d: '%s' is not a list", n->line, n->add_list.name);
            Value *item = eval(in, e, n->add_list.val);
            SengList *sl = lv->list;
            if (sl->count >= sl->cap) {
                sl->cap   = sl->cap ? sl->cap * 2 : 4;
                sl->items = (Value **)xrealloc(sl->items,
                            sizeof(Value *) * (size_t)sl->cap);
            }
            sl->items[sl->count++] = item;
            return SIG_NONE;
        }

        case ND_IMPORT: {
            char *src = read_file(n->import_path);
            if (!src)
                fatal("line %d: cannot open file '%s'", n->line, n->import_path);
            Lexer  *lex  = lexer_new(src);
            Parser *par  = parser_new(lex);
            Node   *prog = parse(par);
            parser_free(par);
            lexer_free(lex);
            free(src);
            exec_block(in, in->globals, &prog->program);
            /* keep prog alive: function body_refs point into this AST */
            if (in->import_count >= in->import_cap) {
                in->import_cap   = in->import_cap ? in->import_cap * 2 : 4;
                in->imported     = (Node **)xrealloc(in->imported,
                                   sizeof(Node *) * (size_t)in->import_cap);
            }
            in->imported[in->import_count++] = prog;
            return SIG_NONE;
        }

        case ND_IMPORT_PKG: {
            if (!pkg_register(in->globals, n->str))
                fatal("line %d: unknown package '%s'  (available: math, string, io, type, http, sys, json)",
                      n->line, n->str);
            return SIG_NONE;
        }

        case ND_STOP:  in->signal = SIG_STOP; return SIG_STOP;
        case ND_SKIP:  in->signal = SIG_SKIP; return SIG_SKIP;

        case ND_PROGRAM:
            return exec_block(in, e, &n->program);

        default:
            fatal("line %d: cannot execute node kind %d", n->line, n->kind);
            return SIG_NONE;
    }
}

/* ── public API ──────────────────────────────────────────────── */

Interp *interp_new(void) {
    Interp *in    = (Interp *)xcalloc(1, sizeof(Interp));
    in->globals   = env_new(NULL);
    return in;
}

void interp_free(Interp *in) {
    env_free(in->globals);
    if (in->ret_val) val_deref(in->ret_val);
    for (int i = 0; i < in->import_count; i++) node_free(in->imported[i]);
    free(in->imported);
    free(in);
}

void interp_exec(Interp *in, Node *program) {
    exec_block(in, in->globals, &program->program);
}
