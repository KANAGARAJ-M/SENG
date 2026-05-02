
#include "lexer.h"

/* ── keyword table ───────────────────────────────────────────── */
static const struct { const char *word; TkType type; } kw_table[] = {
    {"add",     TK_ADD},     {"and",     TK_AND},    {"ask",     TK_ASK},
    {"back",    TK_BACK},    {"by",      TK_BY},     {"call",    TK_CALL},
    {"define",  TK_DEFINE},  {"divided", TK_DIVIDED},{"else",    TK_ELSE},
    {"end",     TK_END},     {"equal",   TK_EQUAL},  {"false",   TK_FALSE},
    {"for",     TK_FOR},     {"give",    TK_GIVE},   {"greater", TK_GREATER},
    {"if",      TK_IF},      {"import",  TK_IMPORT}, {"is",      TK_IS},
    {"item",    TK_ITEM},    {"length",  TK_LENGTH}, {"less",    TK_LESS},
    {"list",    TK_LIST},    {"make",    TK_MAKE},   {"minus",   TK_MINUS},
    {"mod",     TK_MOD},     {"not",     TK_NOT},    {"note",    TK_NOTE},
    {"nothing", TK_NOTHING}, {"of",      TK_OF},     {"or",      TK_OR},
    {"plus",    TK_PLUS},    {"repeat",  TK_REPEAT}, {"result",  TK_RESULT},
    {"say",     TK_SAY},     {"set",     TK_SET},    {"skip",    TK_SKIP},
    {"stop",    TK_STOP},    {"than",    TK_THAN},   {"then",    TK_THEN},
    {"times",   TK_TIMES},   {"to",      TK_TO},     {"true",    TK_TRUE},
    {"while",   TK_WHILE},   {"with",    TK_WITH},
    {NULL,      TK_EOF}
};

static TkType lookup_kw(const char *word) {
    for (int i = 0; kw_table[i].word; i++)
        if (strcmp(kw_table[i].word, word) == 0)
            return kw_table[i].type;
    return TK_IDENT;
}

/* ── lexer struct ────────────────────────────────────────────── */
struct Lexer {
    const char *src;
    size_t      pos;
    int         line;
    Token       peeked;
    int         has_peeked;
};

Lexer *lexer_new(const char *src) {
    Lexer *l = (Lexer *)xcalloc(1, sizeof(Lexer));
    l->src  = src;
    l->pos  = 0;
    l->line = 1;
    return l;
}

void lexer_free(Lexer *l) {
    free(l);
}

/* helper: skip spaces & tabs (NOT newlines) */
static void skip_spaces(Lexer *l) {
    while (l->src[l->pos] == ' ' || l->src[l->pos] == '\t' || l->src[l->pos] == '\r')
        l->pos++;
}

/* produce one raw token from the source stream */
static Token scan(Lexer *l) {
    skip_spaces(l);
    int   line = l->line;
    char  c    = l->src[l->pos];
    Token t;

    /* ── EOF ─────── */
    if (c == '\0') { t.type = TK_EOF; t.value = NULL; t.line = line; return t; }

    /* ── comment: # ── */
    if (c == '#') {
        while (l->src[l->pos] && l->src[l->pos] != '\n') l->pos++;
        return scan(l);
    }

    /* ── newline ─────── */
    if (c == '\n') {
        l->pos++; l->line++;
        t.type = TK_NEWLINE; t.value = NULL; t.line = line; return t;
    }

    /* ── string literal ─ */
    if (c == '"') {
        l->pos++;
        size_t start = l->pos;
        while (l->src[l->pos] && l->src[l->pos] != '"' && l->src[l->pos] != '\n')
            l->pos++;
        char *val = xstrndup(l->src + start, l->pos - start);
        if (l->src[l->pos] == '"') l->pos++;
        t.type = TK_STRING; t.value = val; t.line = line; return t;
    }

    /* ── number ─── */
    if (isdigit((unsigned char)c)) {
        size_t start = l->pos;
        while (isdigit((unsigned char)l->src[l->pos])) l->pos++;
        if (l->src[l->pos] == '.') {
            l->pos++;
            while (isdigit((unsigned char)l->src[l->pos])) l->pos++;
        }
        t.type = TK_NUMBER; t.value = xstrndup(l->src + start, l->pos - start);
        t.line = line; return t;
    }

    /* ── identifier / keyword ─── */
    if (isalpha((unsigned char)c) || c == '_') {
        size_t start = l->pos;
        while (isalnum((unsigned char)l->src[l->pos]) || l->src[l->pos] == '_')
            l->pos++;
        char  *word = xstrndup(l->src + start, l->pos - start);
        TkType tp   = lookup_kw(word);

        /* 'note' → rest of line is a comment */
        if (tp == TK_NOTE) {
            free(word);
            while (l->src[l->pos] && l->src[l->pos] != '\n') l->pos++;
            return scan(l);
        }
        /* for non-literal keywords, value is not needed */
        if (tp != TK_IDENT && tp != TK_NUMBER && tp != TK_STRING) {
            free(word);
            t.type = tp; t.value = NULL; t.line = line; return t;
        }
        t.type = TK_IDENT; t.value = word; t.line = line; return t;
    }

    /* ── operators ─── */
    l->pos++;
    t.value = NULL; t.line = line;
    switch (c) {
        case '+': t.type = TK_PLUS_OP;  return t;
        case '-': t.type = TK_MINUS_OP; return t;
        case '*': t.type = TK_STAR;     return t;
        case '/': t.type = TK_SLASH;    return t;
        case '%': t.type = TK_PERCENT;  return t;
        case '(': t.type = TK_LPAREN;   return t;
        case ')': t.type = TK_RPAREN;   return t;
        default:
            t.type = TK_ERROR; return t;
    }
}

Token lexer_advance(Lexer *l) {
    if (l->has_peeked) {
        l->has_peeked = 0;
        return l->peeked;
    }
    return scan(l);
}

Token lexer_peek(Lexer *l) {
    if (!l->has_peeked) {
        l->peeked = scan(l);
        l->has_peeked = 1;
    }
    return l->peeked;
}

/* ── debug name ─── */
const char *tk_name(TkType t) {
    static const char *names[] = {
        "NUMBER","STRING","TRUE","FALSE","NOTHING","IDENT",
        "add","and","ask","back","by","call","define","divided",
        "else","end","equal","for","give","greater","if","import",
        "is","item","length","less","list","make","minus","mod",
        "not","note","of","or","plus","repeat","result","say","set",
        "skip","stop","than","then","times","to","while","with",
        "+","-","*","/","%","(",")",
        "NEWLINE","EOF","ERROR"
    };
    if (t < TK_COUNT) return names[t];
    return "?";
}
