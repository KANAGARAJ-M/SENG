
#include "value.h"
#include <math.h>
#include <stdio.h>

static Value *val_alloc(ValType t) {
    Value *v = (Value *)xcalloc(1, sizeof(Value));
    v->type     = t;
    v->refcount = 1;
    return v;
}

Value *val_num (double d)        { Value *v = val_alloc(VAL_NUM);  v->num      = d; return v; }
Value *val_bool(int   b)         { Value *v = val_alloc(VAL_BOOL); v->bool_val = !!b; return v; }
Value *val_null(void)            { return val_alloc(VAL_NULL); }
Value *val_func(SengFunc *f)     { Value *v = val_alloc(VAL_FUNC); v->func = f; return v; }

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
    free(v);
}

int val_truthy(const Value *v) {
    if (!v) return 0;
    switch (v->type) {
        case VAL_BOOL: return v->bool_val;
        case VAL_NUM:  return v->num != 0.0;
        case VAL_STR:  return v->str && v->str[0] != '\0';
        case VAL_NULL: return 0;
        case VAL_LIST: return v->list->count > 0;
        default:       return 1;
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
    switch (src->type) {
        case VAL_NUM:  return val_num(src->num);
        case VAL_BOOL: return val_bool(src->bool_val);
        case VAL_NULL: return val_null();
        case VAL_STR:  return val_str(src->str);
        case VAL_FUNC: { Value *v = val_alloc(VAL_FUNC); v->func = src->func; return v; }
        case VAL_LIST: {
            Value *v = val_list();
            for (int i = 0; i < src->list->count; i++) {
                Value *item = val_copy(src->list->items[i]);
                if (v->list->count >= v->list->cap) {
                    v->list->cap = v->list->cap ? v->list->cap * 2 : 4;
                    v->list->items = (Value **)xrealloc(v->list->items,
                        sizeof(Value *) * (size_t)v->list->cap);
                }
                v->list->items[v->list->count++] = item;
            }
            return v;
        }
        default: return val_null();
    }
}
