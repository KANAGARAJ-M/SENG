
#ifndef SENG_ENV_H
#define SENG_ENV_H

#include "value.h"

/* ── simple hash-map based variable environment ─────────────── */
typedef struct Env Env;

Env   *env_new   (Env *parent);
void   env_free  (Env *e);
void   env_set   (Env *e, const char *name, Value *v);   /* sets in current scope */
Value *env_get   (Env *e, const char *name);              /* searches up the chain */
void   env_update(Env *e, const char *name, Value *v);   /* updates nearest binding */

#endif /* SENG_ENV_H */
