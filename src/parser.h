
#ifndef SENG_PARSER_H
#define SENG_PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct Parser Parser;

Parser *parser_new (Lexer *lex);
void    parser_free(Parser *p);
Node   *parse      (Parser *p);   /* returns ND_PROGRAM node */

#endif /* SENG_PARSER_H */
