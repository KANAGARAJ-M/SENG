
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

Value *val_map(void) {
    Value   *v = val_alloc(VAL_MAP);
    SengMap *m = (SengMap *)xcalloc(1, sizeof(SengMap));
    v->map = m;
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
    if (v->type == VAL_MAP) {
        for (int i = 0; i < v->map->count; i++) {
            free(v->map->keys[i]);
            val_deref(v->map->values[i]);
        }
        free(v->map->keys);
        free(v->map->values);
        free(v->map);
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
        case VAL_MAP:    return v->map->count > 0;
        case VAL_NATIVE: return 1;
        default:         return 1;
    }
}

static void append_str(char **dest, const char *src) {
    if (!src) return;
    size_t dl = *dest ? strlen(*dest) : 0;
    size_t sl = strlen(src);
    *dest = (char *)xrealloc(*dest, dl + sl + 1);
    strcpy((*dest) + dl, src);
}

static int check_circular(Value *v, Value **stack, int depth) {
    if (!v || (v->type != VAL_LIST && v->type != VAL_INSTANCE)) return 0;
    if (depth > 128) return 1;
    for (int i = 0; i < depth; i++) if (stack[i] == v) return 1;
    stack[depth] = v;
    if (v->type == VAL_LIST) {
        for (int i = 0; i < v->list->count; i++)
            if (check_circular(v->list->items[i], stack, depth + 1)) return 1;
    } else {
        for (int i = 0; i < v->instance->klass->field_count; i++)
            if (check_circular(v->instance->fields[i], stack, depth + 1)) return 1;
    }
    return 0;
}

int val_is_circular(Value *v) {
    Value *stack[130] = {0};
    return check_circular(v, stack, 0);
}

static char *vts_rec(const Value *v, Value **stack, int depth) {
    if (!v) return xstrdup("nothing");
    if (depth > 32) return xstrdup("...");
    for (int i = 0; i < depth; i++) {
        if (stack[i] == v) return xstrdup(v->type == VAL_LIST ? "[...]" : (v->type == VAL_MAP ? "{...}" : "<...>"));
    }
    stack[depth] = (Value *)v;

    char buf[64];
    switch (v->type) {
        case VAL_NULL: return xstrdup("nothing");
        case VAL_BOOL: return xstrdup(v->bool_val ? "true" : "false");
        case VAL_STR:  return xstrdup(v->str);
        case VAL_FUNC:
            snprintf(buf, sizeof buf, "<function %s>", v->func ? v->func->name : "?");
            return xstrdup(buf);
        case VAL_NATIVE:
            snprintf(buf, sizeof buf, "<builtin %s>", v->native ? v->native->name : "?");
            return xstrdup(buf);
        case VAL_CLASS:
            snprintf(buf, sizeof buf, "<blueprint %s>", v->klass ? v->klass->name : "?");
            return xstrdup(buf);
        case VAL_INSTANCE: {
            char *res = xstrdup("<instance of ");
            append_str(&res, v->instance->klass->name);
            append_str(&res, " {");
            for (int i = 0; i < v->instance->klass->field_count; i++) {
                append_str(&res, v->instance->klass->fields[i]);
                append_str(&res, ": ");
                char *s = vts_rec(v->instance->fields[i], stack, depth + 1);
                append_str(&res, s); free(s);
                if (i < v->instance->klass->field_count - 1) append_str(&res, ", ");
            }
            append_str(&res, "}>");
            return res;
        }
        case VAL_NUM:
            if (v->num == (long long)v->num && v->num >= -1e15 && v->num <= 1e15)
                snprintf(buf, sizeof buf, "%lld", (long long)v->num);
            else
                snprintf(buf, sizeof buf, "%g", v->num);
            return xstrdup(buf);
        case VAL_LIST: {
            char *res = xstrdup("[");
            for (int i = 0; i < v->list->count; i++) {
                char *s = vts_rec(v->list->items[i], stack, depth + 1);
                append_str(&res, s); free(s);
                if (i < v->list->count - 1) append_str(&res, ", ");
            }
            append_str(&res, "]");
            return res;
        }
        case VAL_MAP: {
            char *res = xstrdup("{");
            for (int i = 0; i < v->map->count; i++) {
                append_str(&res, v->map->keys[i]);
                append_str(&res, ": ");
                char *s = vts_rec(v->map->values[i], stack, depth + 1);
                append_str(&res, s); free(s);
                if (i < v->map->count - 1) append_str(&res, ", ");
            }
            append_str(&res, "}");
            return res;
        }
        default: return xstrdup("?");
    }
}

char *val_to_string(const Value *v) {
    Value *stack[64] = {0};
    return vts_rec(v, stack, 0);
}

void val_print(const Value *v) {
    char *s = val_to_string(v);
    printf("%s\n", s);
    free(s);
}

int find_field(SengClass *c, const char *name) {
    for (int i = 0; i < c->field_count; i++)
        if (strcmp(c->fields[i], name) == 0) return i;
    return -1;
}

Value *find_method(SengClass *c, const char *name) {
    while (c) {
        for (int i = 0; i < c->method_count; i++) {
            if (strcmp(c->methods[i].name, name) == 0) return c->methods[i].method;
        }
        c = c->parent;
    }
    return NULL;
}

int is_method_hidden(SengClass *c, const char *name) {
    while (c) {
        for (int i = 0; i < c->method_count; i++) {
            if (strcmp(c->methods[i].name, name) == 0) return c->methods[i].hidden;
        }
        c = c->parent;
    }
    return 0;
}

Value *val_copy(const Value *src) {
    if (!src) return val_null();
    val_ref((Value *)src);
    return (Value *)src;
}

void list_push(Value *v, Value *item) {
    if (!v || v->type != VAL_LIST) return;
    SengList *l = v->list;
    if (l->count >= l->cap) {
        l->cap = l->cap ? l->cap * 2 : 4;
        l->items = (Value **)xrealloc(l->items, sizeof(Value *) * (size_t)l->cap);
    }
    l->items[l->count++] = item;
}

void map_set(Value *v, const char *key, Value *val) {
    if (!v || v->type != VAL_MAP) return;
    SengMap *m = v->map;
    for (int i = 0; i < m->count; i++) {
        if (strcmp(m->keys[i], key) == 0) {
            val_deref(m->values[i]);
            m->values[i] = val;
            return;
        }
    }
    if (m->count >= m->cap) {
        m->cap = m->cap ? m->cap * 2 : 4;
        m->keys = (char **)xrealloc(m->keys, sizeof(char *) * (size_t)m->cap);
        m->values = (Value **)xrealloc(m->values, sizeof(Value *) * (size_t)m->cap);
    }
    m->keys[m->count] = xstrdup(key);
    m->values[m->count] = val;
    m->count++;
}
