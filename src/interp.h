
#ifndef SENG_INTERP_H
#define SENG_INTERP_H

#include "ast.h"
#include "value.h"
#include "env.h"

typedef struct Interp Interp;

Interp *interp_new (void);
void    interp_free(Interp *i);
void    interp_exec(Interp *i, Node *program);   /* run a ND_PROGRAM node */

#endif /* SENG_INTERP_H */
