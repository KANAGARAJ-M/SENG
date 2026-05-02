
#ifndef SENG_COMPILER_H
#define SENG_COMPILER_H

#include "ast.h"
#include "bytecode.h"

/* Compile a ND_PROGRAM AST node → write .sec file to path */
void compile_to_sec(Node *program, const char *out_path);

#endif /* SENG_COMPILER_H */
