
#include "value.h"
#include <math.h>
#include <stdio.h>

static Value *val_alloc(ValType t) {
    Value *v = (Value *)xcalloc(1, sizeof(Value));
    v->type     = t;
    v->refcount = 1;
    return v;
}

Value *val_num   (double d)       { Value *v = val_alloc(VAL_NUM);    v->num      = d; return v; }
Value *val_bool  (int    b)       { Value *v = val_alloc(VAL_BOOL);   v->bool_val = !!b; return v; }
Value *val_null  (void)           { return val_alloc(VAL_NULL); }
Value *val_func  (SengFunc *f)    { Value *v = val_alloc(VAL_FUNC);   v->func   = f; return v; }
Value *val_native(SengNative *n)  { Value *v = val_alloc(VAL_NATIVE); v->native = n; return v; }

Value *val_str(const char *s) {
    Value *v = val_alloc(VAL_STR);
    v->str   = xstrdup(s);
    return v;
}

Value *val_list(void) {
    Value    *v = val_alloc(VAL_LIST);
    SengList *l = (SengList *)xcalloc(1, sizeof(SengList));
    v->list = l;
    return v;
}

Value *val_class(SengClass *c) {
    Value *v = val_alloc(VAL_CLASS);
    v->klass = c;
    return v;
}

Value *val_instance(SengInstance *i) {
    Value *v = val_alloc(VAL_INSTANCE);
    v->instance = i;
    return v;
}

void val_ref (Value *v) { if (v) v->refcount++; }
void val_deref(Value *v) {
    if (!v) return;
    if (--v->refcount > 0) return;
    if (v->type == VAL_STR)  free(v->str);
    if (v->type == VAL_LIST) {
        for (int i = 0; i < v->list->count; i++) val_deref(v->list->items[i]);
        free(v->list->items);
        free(v->list);
    }
    if (v->type == VAL_FUNC) {
        SengFunc *fn = v->func;
        free(fn->name);
        for (int i = 0; i < fn->param_count; i++) free(fn->params[i]);
        free(fn->params);
        free(fn);
    }
    if (v->type == VAL_NATIVE) {
        /* SengNative structs are usually static/constant in SENG packages, 
           but if they were dynamic we'd free them here. 
           Actually, they are pointers to static structs in packages.c. 
           So we don't free them. */
    }
    if (v->type == VAL_CLASS) {
        SengClass *c = v->klass;
        free(c->name);
        for (int i = 0; i < c->field_count; i++) free(c->fields[i]);
        free(c->fields);
        for (int i = 0; i < c->method_count; i++) {
            free(c->methods[i].name);
            val_deref(c->methods[i].method);
        }
        free(c->methods);
        free(c);
    }
    if (v->type == VAL_INSTANCE) {
        SengInstance *inst = v->instance;
        for (int i = 0; i < inst->klass->field_count; i++) val_deref(inst->fields[i]);
        free(inst->fields);
        free(inst);
    }
    free(v);
}

int val_truthy(const Value *v) {
    if (!v) return 0;
    switch (v->type) {
        case VAL_BOOL:   return v->bool_val;
        case VAL_NUM:    return v->num != 0.0;
        case VAL_STR:    return v->str && v->str[0] != '\0';
        case VAL_NULL:   return 0;
        case VAL_LIST:   return v->list->count > 0;
        case VAL_NATIVE: return 1;
        default:         return 1;
    }
}

char *val_to_string(const Value *v) {
    if (!v) return xstrdup("nothing");
    char buf[64];
    switch (v->type) {
        case VAL_NULL: return xstrdup("nothing");
        case VAL_BOOL: return xstrdup(v->bool_val ? "true" : "false");
        case VAL_STR:  return xstrdup(v->str);
        case VAL_FUNC: {
            snprintf(buf, sizeof buf, "<function %s>", v->func ? v->func->name : "?");
            return xstrdup(buf);
        }
        case VAL_NATIVE: {
            snprintf(buf, sizeof buf, "<builtin %s>", v->native ? v->native->name : "?");
            return xstrdup(buf);
        }
        case VAL_CLASS: {
            snprintf(buf, sizeof buf, "<blueprint %s>", v->klass ? v->klass->name : "?");
            return xstrdup(buf);
        }
        case VAL_INSTANCE: {
            snprintf(buf, sizeof buf, "<instance of %s>", (v->instance && v->instance->klass) ? v->instance->klass->name : "?");
            return xstrdup(buf);
        }
        case VAL_NUM: {
            /* print without trailing .0 when it's a whole number */
            if (v->num == (long long)v->num && v->num >= -1e15 && v->num <= 1e15)
                snprintf(buf, sizeof buf, "%lld", (long long)v->num);
            else
                snprintf(buf, sizeof buf, "%g", v->num);
            return xstrdup(buf);
        }
        case VAL_LIST: {
            /* [1, 2, 3] */
            char *res = xstrdup("[");
            for (int i = 0; i < v->list->count; i++) {
                char *s = val_to_string(v->list->items[i]);
                size_t rl = strlen(res), sl = strlen(s);
                res = (char *)xrealloc(res, rl + sl + 3);
                strcat(res, s);
                if (i < v->list->count - 1) strcat(res, ", ");
                free(s);
            }
            size_t rl = strlen(res);
            res = (char *)xrealloc(res, rl + 2);
            strcat(res, "]");
            return res;
        }
        default: return xstrdup("?");
    }
}

void val_print(const Value *v) {
    char *s = val_to_string(v);
    printf("%s\n", s);
    free(s);
}

Value *val_copy(const Value *src) {
    if (!src) return val_null();
    val_ref((Value *)src);
    return (Value *)src;
}
