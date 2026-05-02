
#include "env.h"
#include <string.h>

#define ENV_BUCKET 64

typedef struct EnvEntry {
    char         *key;
    Value        *val;
    struct EnvEntry *next;
} EnvEntry;

struct Env {
    EnvEntry *buckets[ENV_BUCKET];
    Env      *parent;
};

static unsigned int hash_str(const char *s) {
    unsigned int h = 5381;
    while (*s) h = ((h << 5) + h) + (unsigned char)*s++;
    return h % ENV_BUCKET;
}

Env *env_new(Env *parent) {
    Env *e = (Env *)xcalloc(1, sizeof(Env));
    e->parent = parent;
    return e;
}

void env_free(Env *e) {
    if (!e) return;
    for (int i = 0; i < ENV_BUCKET; i++) {
        EnvEntry *en = e->buckets[i];
        while (en) {
            EnvEntry *nx = en->next;
            free(en->key);
            val_deref(en->val);
            free(en);
            en = nx;
        }
    }
    free(e);
}

void env_set(Env *e, const char *name, Value *v) {
    unsigned int h = hash_str(name);
    EnvEntry *en = e->buckets[h];
    while (en) {
        if (strcmp(en->key, name) == 0) {
            val_deref(en->val);
            val_ref(v);
            en->val = v;
            return;
        }
        en = en->next;
    }
    EnvEntry *ne = (EnvEntry *)xcalloc(1, sizeof(EnvEntry));
    ne->key  = xstrdup(name);
    ne->val  = v;
    val_ref(v);
    ne->next = e->buckets[h];
    e->buckets[h] = ne;
}

Value *env_get(Env *e, const char *name) {
    Env *cur = e;
    while (cur) {
        unsigned int h = hash_str(name);
        EnvEntry *en = cur->buckets[h];
        while (en) {
            if (strcmp(en->key, name) == 0) return en->val;
            en = en->next;
        }
        cur = cur->parent;
    }
    return NULL;
}

void env_update(Env *e, const char *name, Value *v) {
    Env *cur = e;
    while (cur) {
        unsigned int h = hash_str(name);
        EnvEntry *en = cur->buckets[h];
        while (en) {
            if (strcmp(en->key, name) == 0) {
                val_deref(en->val);
                val_ref(v);
                en->val = v;
                return;
            }
            en = en->next;
        }
        cur = cur->parent;
    }
    /* not found — create in current scope */
    env_set(e, name, v);
}
