
#ifndef SENG_VALUE_H
#define SENG_VALUE_H

#include "common.h"

typedef enum { 
    VAL_NUM, VAL_STR, VAL_BOOL, VAL_NULL, VAL_LIST, VAL_FUNC, VAL_NATIVE,
    VAL_CLASS, VAL_INSTANCE
} ValType;

typedef struct Value Value;

/* Native (C-backed) function */
typedef Value* (*NativeFn)(Value **args, int argc);
typedef struct {
    const char *name;
    NativeFn    fn;
    int         arity;  /* expected arg count; -1 = variadic */
} SengNative;

/* Function value: points to ND_DEFINE node */
typedef struct SengFunc {
    char   *name;
    char  **params;
    int     param_count;
    void   *body_ref;  /* NodeList* in AST */
} SengFunc;

/* List value: dynamic array of Values */
typedef struct {
    Value **items;
    int     count;
    int     cap;
} SengList;

typedef struct SengClass {
    char   *name;
    struct SengClass *parent;
    char  **fields;
    int    *field_hidden;
    int     field_count;
    struct {
        char  *name;
        Value *method;
        int    hidden;
    } *methods;
    int     method_count;
} SengClass;

typedef struct {
    SengClass *klass;
    Value    **fields; /* mapped to klass->fields index */
} SengInstance;

struct Value {
    ValType type;
    int     refcount;
    union {
        double      num;
        char       *str;
        int         bool_val;
        SengList   *list;
        SengFunc   *func;
        SengNative *native;
        SengClass  *klass;
        SengInstance *instance;
    };
};

Value *val_num   (double d);
Value *val_str   (const char *s);
Value *val_bool  (int b);
Value *val_null  (void);
Value *val_list  (void);
Value *val_func  (SengFunc *f);
Value *val_native(SengNative *n);
Value *val_class (SengClass *c);
Value *val_instance(SengInstance *i);
int    find_field(SengClass *c, const char *name);
Value *find_method(SengClass *c, const char *name);
int    is_method_hidden(SengClass *c, const char *name);
int    val_is_circular(Value *v);

void   val_ref  (Value *v);
void   val_deref(Value *v);

Value *val_copy (const Value *v);
int    val_truthy(const Value *v);
char  *val_to_string(const Value *v);   /* heap, caller frees */
void   val_print(const Value *v);

#endif /* SENG_VALUE_H */
