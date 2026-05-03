
#ifndef SENG_PACKAGES_H
#define SENG_PACKAGES_H

#include "env.h"

/* Load a built-in package into the given environment.
   Returns 1 on success, 0 if the package name is unknown. */
int pkg_register(Env *globals, const char *name);
void pkg_set_args(int argc, char **argv);

#endif /* SENG_PACKAGES_H */
