
#ifndef SENG_LEXER_H
#define SENG_LEXER_H

#include "common.h"

/* ── token types ─────────────────────────────────────────────── */
typedef enum {
    /* literals */
    TK_NUMBER, TK_STRING, TK_TRUE, TK_FALSE, TK_NOTHING,
    /* identifier */
    TK_IDENT,

    /* ── keywords ── */
    TK_ADD,     TK_AND,    TK_ASK,    TK_BACK,   TK_BY,
    TK_CALL,    TK_DEFINE, TK_DIVIDED,
    TK_ELSE,    TK_END,    TK_EQUAL,
    TK_FOR,     TK_GIVE,   TK_GREATER,
    TK_IF,      TK_IMPORT, TK_IS,     TK_ITEM,
    TK_LENGTH,  TK_LESS,   TK_LIST,
    TK_MAKE,    TK_MINUS,  TK_MOD,
    TK_NOT,     TK_NOTE,
    TK_OF,      TK_OR,     TK_PLUS,
    TK_REPEAT,  TK_RESULT, TK_SAY,    TK_SET,
    TK_SKIP,    TK_STOP,
    TK_THAN,    TK_THEN,   TK_TIMES,  TK_TO,
    TK_WHILE,   TK_WITH,

    /* ── operators ── */
    TK_PLUS_OP,   /* +  */
    TK_MINUS_OP,  /* -  */
    TK_STAR,      /* *  */
    TK_SLASH,     /* /  */
    TK_PERCENT,   /* %  */
    TK_LPAREN,    /* (  */
    TK_RPAREN,    /* )  */

    /* ── special ── */
    TK_NEWLINE,
    TK_EOF,
    TK_ERROR,

    TK_COUNT   /* sentinel */
} TkType;

typedef struct {
    TkType  type;
    char   *value;   /* heap string for TK_NUMBER / TK_STRING / TK_IDENT */
    int     line;
} Token;

typedef struct Lexer Lexer;

Lexer      *lexer_new    (const char *src);
void        lexer_free   (Lexer *l);
Token       lexer_advance(Lexer *l);  /* consume next token          */
Token       lexer_peek   (Lexer *l);  /* look ahead without consuming */
const char *tk_name      (TkType t);

#endif /* SENG_LEXER_H */
