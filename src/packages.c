
#include "packages.h"
#include "value.h"
#include "env.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

/* ── helpers ─────────────────────────────────────────────────── */

static Value *val_strn(const char *s, size_t n) {
    char *t = xstrndup(s, n);
    Value *v = val_str(t);
    free(t);
    return v;
}

static void list_push(Value *list, Value *item) {
    SengList *sl = list->list;
    if (sl->count >= sl->cap) {
        sl->cap   = sl->cap ? sl->cap * 2 : 4;
        sl->items = (Value **)xrealloc(sl->items,
                    sizeof(Value *) * (size_t)sl->cap);
    }
    sl->items[sl->count++] = item;
}

static void require_num(Value *v, int pos, const char *fn) {
    if (!v || v->type != VAL_NUM)
        fatal("'%s': argument %d must be a number", fn, pos);
}
static void require_str(Value *v, int pos, const char *fn) {
    if (!v || v->type != VAL_STR)
        fatal("'%s': argument %d must be a string", fn, pos);
}
static void require_list(Value *v, int pos, const char *fn) {
    if (!v || v->type != VAL_LIST)
        fatal("'%s': argument %d must be a list", fn, pos);
}

/* ═══════════════════════════════════════════════════════════════
   math package
   ═══════════════════════════════════════════════════════════════ */

static Value *nat_sqrt(Value **a, int n) {
    (void)n; require_num(a[0], 1, "sqrt");
    if (a[0]->num < 0) fatal("'sqrt': argument must be non-negative");
    return val_num(sqrt(a[0]->num));
}
static Value *nat_floor(Value **a, int n) {
    (void)n; require_num(a[0], 1, "floor");
    return val_num(floor(a[0]->num));
}
static Value *nat_ceil(Value **a, int n) {
    (void)n; require_num(a[0], 1, "ceil");
    return val_num(ceil(a[0]->num));
}
static Value *nat_round(Value **a, int n) {
    (void)n; require_num(a[0], 1, "round");
    return val_num(round(a[0]->num));
}
static Value *nat_abs(Value **a, int n) {
    (void)n; require_num(a[0], 1, "abs");
    return val_num(fabs(a[0]->num));
}
static Value *nat_power(Value **a, int n) {
    (void)n;
    require_num(a[0], 1, "power"); require_num(a[1], 2, "power");
    return val_num(pow(a[0]->num, a[1]->num));
}
static Value *nat_log(Value **a, int n) {
    (void)n; require_num(a[0], 1, "log");
    if (a[0]->num <= 0) fatal("'log': argument must be positive");
    return val_num(log(a[0]->num));
}
static Value *nat_log10_fn(Value **a, int n) {
    (void)n; require_num(a[0], 1, "log10");
    if (a[0]->num <= 0) fatal("'log10': argument must be positive");
    return val_num(log10(a[0]->num));
}
static Value *nat_sin(Value **a, int n)  { (void)n; require_num(a[0],1,"sin");  return val_num(sin(a[0]->num));  }
static Value *nat_cos(Value **a, int n)  { (void)n; require_num(a[0],1,"cos");  return val_num(cos(a[0]->num));  }
static Value *nat_tan(Value **a, int n)  { (void)n; require_num(a[0],1,"tan");  return val_num(tan(a[0]->num));  }
static Value *nat_min(Value **a, int n) {
    (void)n; require_num(a[0],1,"min"); require_num(a[1],2,"min");
    return val_num(a[0]->num < a[1]->num ? a[0]->num : a[1]->num);
}
static Value *nat_max(Value **a, int n) {
    (void)n; require_num(a[0],1,"max"); require_num(a[1],2,"max");
    return val_num(a[0]->num > a[1]->num ? a[0]->num : a[1]->num);
}
static Value *nat_random(Value **a, int n) {
    (void)a; (void)n;
    return val_num((double)rand() / ((double)RAND_MAX + 1.0));
}
static Value *nat_random_int(Value **a, int n) {
    (void)n; require_num(a[0],1,"random_int"); require_num(a[1],2,"random_int");
    int lo = (int)a[0]->num, hi = (int)a[1]->num;
    if (lo > hi) fatal("'random_int': first argument must be <= second");
    return val_num((double)(lo + rand() % (hi - lo + 1)));
}

static SengNative math_fns[] = {
    {"sqrt",       nat_sqrt,       1},
    {"floor",      nat_floor,      1},
    {"ceil",       nat_ceil,       1},
    {"round",      nat_round,      1},
    {"abs",        nat_abs,        1},
    {"power",      nat_power,      2},
    {"log",        nat_log,        1},
    {"log10",      nat_log10_fn,   1},
    {"sin",        nat_sin,        1},
    {"cos",        nat_cos,        1},
    {"tan",        nat_tan,        1},
    {"min",        nat_min,        2},
    {"max",        nat_max,        2},
    {"random",     nat_random,     0},
    {"random_int", nat_random_int, 2},
    {NULL, NULL, 0}
};

static void pkg_math(Env *g) {
    static int seeded = 0;
    if (!seeded) { srand((unsigned int)time(NULL)); seeded = 1; }
    for (int i = 0; math_fns[i].name; i++) {
        Value *v = val_native(&math_fns[i]);
        env_set(g, math_fns[i].name, v);
        val_deref(v);
    }
    Value *pi = val_num(3.14159265358979323846);
    env_set(g, "pi", pi);
    val_deref(pi);
}

/* ═══════════════════════════════════════════════════════════════
   string package
   ═══════════════════════════════════════════════════════════════ */

static Value *nat_str_len(Value **a, int n) {
    (void)n; require_str(a[0], 1, "str_len");
    return val_num((double)strlen(a[0]->str));
}
static Value *nat_upper(Value **a, int n) {
    (void)n; require_str(a[0], 1, "upper");
    char *s = xstrdup(a[0]->str);
    for (size_t i = 0; s[i]; i++) s[i] = (char)toupper((unsigned char)s[i]);
    Value *v = val_str(s); free(s); return v;
}
static Value *nat_lower(Value **a, int n) {
    (void)n; require_str(a[0], 1, "lower");
    char *s = xstrdup(a[0]->str);
    for (size_t i = 0; s[i]; i++) s[i] = (char)tolower((unsigned char)s[i]);
    Value *v = val_str(s); free(s); return v;
}
static Value *nat_trim(Value **a, int n) {
    (void)n; require_str(a[0], 1, "trim");
    const char *s = a[0]->str;
    while (*s && isspace((unsigned char)*s)) s++;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) len--;
    return val_strn(s, len);
}
static Value *nat_contains(Value **a, int n) {
    (void)n; require_str(a[0],1,"contains"); require_str(a[1],2,"contains");
    return val_bool(strstr(a[0]->str, a[1]->str) != NULL);
}
static Value *nat_starts_with(Value **a, int n) {
    (void)n; require_str(a[0],1,"starts_with"); require_str(a[1],2,"starts_with");
    size_t plen = strlen(a[1]->str);
    return val_bool(strncmp(a[0]->str, a[1]->str, plen) == 0);
}
static Value *nat_ends_with(Value **a, int n) {
    (void)n; require_str(a[0],1,"ends_with"); require_str(a[1],2,"ends_with");
    size_t slen = strlen(a[0]->str), plen = strlen(a[1]->str);
    if (plen > slen) return val_bool(0);
    return val_bool(strcmp(a[0]->str + slen - plen, a[1]->str) == 0);
}
static Value *nat_replace(Value **a, int n) {
    (void)n;
    require_str(a[0],1,"replace"); require_str(a[1],2,"replace"); require_str(a[2],3,"replace");
    const char *src = a[0]->str, *old_s = a[1]->str, *rep = a[2]->str;
    size_t olen = strlen(old_s), rlen = strlen(rep);
    if (olen == 0) return val_str(src);

    size_t cnt = 0;
    const char *p = src;
    while ((p = strstr(p, old_s)) != NULL) { cnt++; p += olen; }

    size_t srclen = strlen(src);
    size_t add  = (rlen > olen) ? (rlen - olen) * cnt : 0;
    size_t sub  = (olen > rlen) ? (olen - rlen) * cnt : 0;
    size_t newlen = srclen + add - sub;
    char *out = (char *)xmalloc(newlen + 1);
    char *dst = out;
    p = src;
    const char *q;
    while ((q = strstr(p, old_s)) != NULL) {
        size_t prefix = (size_t)(q - p);
        memcpy(dst, p, prefix); dst += prefix;
        memcpy(dst, rep, rlen); dst += rlen;
        p = q + olen;
    }
    size_t tail = strlen(p);
    memcpy(dst, p, tail + 1);
    Value *v = val_str(out); free(out); return v;
}
static Value *nat_split(Value **a, int n) {
    (void)n; require_str(a[0],1,"split"); require_str(a[1],2,"split");
    const char *s = a[0]->str, *sep = a[1]->str;
    size_t seplen = strlen(sep);
    Value *list = val_list();
    if (seplen == 0) {
        for (; *s; s++) list_push(list, val_strn(s, 1));
        return list;
    }
    const char *p = s;
    const char *q;
    while ((q = strstr(p, sep)) != NULL) {
        list_push(list, val_strn(p, (size_t)(q - p)));
        p = q + seplen;
    }
    list_push(list, val_str(p));
    return list;
}
static Value *nat_join(Value **a, int n) {
    (void)n; require_list(a[0],1,"join"); require_str(a[1],2,"join");
    SengList *lst = a[0]->list;
    const char *sep = a[1]->str;
    size_t seplen = strlen(sep);
    /* compute total length */
    size_t total = 0;
    char **parts = (char **)xmalloc(sizeof(char *) * (size_t)(lst->count + 1));
    for (int i = 0; i < lst->count; i++) {
        parts[i] = val_to_string(lst->items[i]);
        total += strlen(parts[i]);
        if (i < lst->count - 1) total += seplen;
    }
    char *out = (char *)xmalloc(total + 1);
    char *dst = out;
    for (int i = 0; i < lst->count; i++) {
        size_t plen = strlen(parts[i]);
        memcpy(dst, parts[i], plen); dst += plen;
        if (i < lst->count - 1) { memcpy(dst, sep, seplen); dst += seplen; }
        free(parts[i]);
    }
    *dst = '\0';
    free(parts);
    Value *v = val_str(out); free(out); return v;
}
static Value *nat_str_num(Value **a, int n) {
    (void)n; require_str(a[0],1,"str_num");
    char *end; double d = strtod(a[0]->str, &end);
    if (end == a[0]->str)
        fatal("'str_num': cannot convert \"%s\" to a number", a[0]->str);
    return val_num(d);
}
static Value *nat_num_str(Value **a, int n) {
    (void)n; require_num(a[0],1,"num_str");
    char *s = val_to_string(a[0]);
    Value *v = val_str(s); free(s); return v;
}
static Value *nat_str_repeat(Value **a, int n) {
    (void)n; require_str(a[0],1,"str_repeat"); require_num(a[1],2,"str_repeat");
    int times = (int)a[1]->num;
    if (times < 0) times = 0;
    size_t slen = strlen(a[0]->str);
    size_t total = slen * (size_t)times;
    char *out = (char *)xmalloc(total + 1);
    char *dst = out;
    for (int i = 0; i < times; i++) { memcpy(dst, a[0]->str, slen); dst += slen; }
    *dst = '\0';
    Value *v = val_str(out); free(out); return v;
}
static Value *nat_char_at(Value **a, int n) {
    (void)n; require_str(a[0],1,"char_at"); require_num(a[1],2,"char_at");
    int idx = (int)a[1]->num - 1;
    int len = (int)strlen(a[0]->str);
    if (idx < 0 || idx >= len)
        fatal("'char_at': index %d out of range (length %d)", idx + 1, len);
    return val_strn(a[0]->str + idx, 1);
}
static Value *nat_sub_str(Value **a, int n) {
    (void)n;
    require_str(a[0],1,"sub_str"); require_num(a[1],2,"sub_str"); require_num(a[2],3,"sub_str");
    int start = (int)a[1]->num - 1;
    int end   = (int)a[2]->num;
    int len   = (int)strlen(a[0]->str);
    if (start < 0) start = 0;
    if (end > len) end = len;
    if (start >= end) return val_str("");
    return val_strn(a[0]->str + start, (size_t)(end - start));
}

static SengNative str_fns[] = {
    {"str_len",    nat_str_len,    1},
    {"upper",      nat_upper,      1},
    {"lower",      nat_lower,      1},
    {"trim",       nat_trim,       1},
    {"contains",   nat_contains,   2},
    {"starts_with",nat_starts_with,2},
    {"ends_with",  nat_ends_with,  2},
    {"replace",    nat_replace,    3},
    {"split",      nat_split,      2},
    {"join",       nat_join,       2},
    {"str_num",    nat_str_num,    1},
    {"num_str",    nat_num_str,    1},
    {"str_repeat", nat_str_repeat, 2},
    {"char_at",    nat_char_at,    2},
    {"sub_str",    nat_sub_str,    3},
    {NULL, NULL, 0}
};

static void pkg_string(Env *g) {
    for (int i = 0; str_fns[i].name; i++) {
        Value *v = val_native(&str_fns[i]);
        env_set(g, str_fns[i].name, v);
        val_deref(v);
    }
}

/* ═══════════════════════════════════════════════════════════════
   io package
   ═══════════════════════════════════════════════════════════════ */

static Value *nat_read_file(Value **a, int n) {
    (void)n; require_str(a[0],1,"read_file");
    char *src = read_file(a[0]->str);
    if (!src) fatal("'read_file': cannot open file '%s'", a[0]->str);
    Value *v = val_str(src); free(src); return v;
}
static Value *nat_write_file(Value **a, int n) {
    (void)n; require_str(a[0],1,"write_file"); require_str(a[1],2,"write_file");
    FILE *f = fopen(a[0]->str, "w");
    if (!f) fatal("'write_file': cannot write file '%s'", a[0]->str);
    fputs(a[1]->str, f);
    fclose(f);
    return val_bool(1);
}
static Value *nat_file_exists(Value **a, int n) {
    (void)n; require_str(a[0],1,"file_exists");
    FILE *f = fopen(a[0]->str, "r");
    if (f) { fclose(f); return val_bool(1); }
    return val_bool(0);
}
static Value *nat_print_inline(Value **a, int n) {
    (void)n; require_str(a[0],1,"print_inline");
    printf("%s", a[0]->str);
    fflush(stdout);
    return val_null();
}

static SengNative io_fns[] = {
    {"read_file",    nat_read_file,   1},
    {"write_file",   nat_write_file,  2},
    {"file_exists",  nat_file_exists, 1},
    {"print_inline", nat_print_inline,1},
    {NULL, NULL, 0}
};

static void pkg_io(Env *g) {
    for (int i = 0; io_fns[i].name; i++) {
        Value *v = val_native(&io_fns[i]);
        env_set(g, io_fns[i].name, v);
        val_deref(v);
    }
}

/* ═══════════════════════════════════════════════════════════════
   type package
   ═══════════════════════════════════════════════════════════════ */

static Value *nat_to_num(Value **a, int n) {
    (void)n;
    if (!a[0]) fatal("'to_num': argument is nothing");
    switch (a[0]->type) {
        case VAL_NUM:  return val_num(a[0]->num);
        case VAL_BOOL: return val_num(a[0]->bool_val ? 1.0 : 0.0);
        case VAL_STR: {
            char *end; double d = strtod(a[0]->str, &end);
            if (end == a[0]->str)
                fatal("'to_num': cannot convert \"%s\" to a number", a[0]->str);
            return val_num(d);
        }
        default: fatal("'to_num': cannot convert this value to a number"); return val_null();
    }
}
static Value *nat_to_str(Value **a, int n) {
    (void)n; char *s = val_to_string(a[0]); Value *v = val_str(s); free(s); return v;
}
static Value *nat_is_num(Value **a, int n)      { (void)n; return val_bool(a[0] && a[0]->type == VAL_NUM);    }
static Value *nat_is_str(Value **a, int n)      { (void)n; return val_bool(a[0] && a[0]->type == VAL_STR);    }
static Value *nat_is_bool(Value **a, int n)     { (void)n; return val_bool(a[0] && a[0]->type == VAL_BOOL);   }
static Value *nat_is_list_val(Value **a, int n) { (void)n; return val_bool(a[0] && a[0]->type == VAL_LIST);   }
static Value *nat_is_nothing(Value **a, int n)  { (void)n; return val_bool(!a[0] || a[0]->type == VAL_NULL);  }
static Value *nat_type_of(Value **a, int n) {
    (void)n;
    if (!a[0]) return val_str("nothing");
    switch (a[0]->type) {
        case VAL_NUM:    return val_str("number");
        case VAL_STR:    return val_str("string");
        case VAL_BOOL:   return val_str("boolean");
        case VAL_NULL:   return val_str("nothing");
        case VAL_LIST:   return val_str("list");
        case VAL_FUNC:   return val_str("function");
        case VAL_NATIVE: return val_str("function");
        default:         return val_str("unknown");
    }
}

static SengNative type_fns[] = {
    {"to_num",      nat_to_num,      1},
    {"to_str",      nat_to_str,      1},
    {"is_num",      nat_is_num,      1},
    {"is_str",      nat_is_str,      1},
    {"is_bool",     nat_is_bool,     1},
    {"is_list_val", nat_is_list_val, 1},
    {"is_nothing",  nat_is_nothing,  1},
    {"type_of",     nat_type_of,     1},
    {NULL, NULL, 0}
};

static void pkg_type(Env *g) {
    for (int i = 0; type_fns[i].name; i++) {
        Value *v = val_native(&type_fns[i]);
        env_set(g, type_fns[i].name, v);
        val_deref(v);
    }
}

/* ═══════════════════════════════════════════════════════════════
   public registry
   ═══════════════════════════════════════════════════════════════ */

int pkg_register(Env *globals, const char *name) {
    if (strcmp(name, "math")   == 0) { pkg_math(globals);   return 1; }
    if (strcmp(name, "string") == 0) { pkg_string(globals); return 1; }
    if (strcmp(name, "io")     == 0) { pkg_io(globals);     return 1; }
    if (strcmp(name, "type")   == 0) { pkg_type(globals);   return 1; }
    return 0;
}
