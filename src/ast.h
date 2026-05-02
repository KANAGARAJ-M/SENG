
#ifndef SENG_AST_H
#define SENG_AST_H

#include "common.h"

/* ── node kinds ──────────────────────────────────────────────── */
typedef enum {
    /* statements */
    ND_PROGRAM,   /* root: list of stmts        */
    ND_SET,       /* set x to <expr>            */
    ND_SAY,       /* say <expr>                 */
    ND_ASK,       /* ask x for "prompt"         */
    ND_IF,        /* if/else if/else            */
    ND_REPEAT,    /* repeat <n> times           */
    ND_WHILE,     /* while <cond>               */
    ND_DEFINE,    /* define f with params       */
    ND_CALL_STMT, /* call f with args           */
    ND_RETURN,    /* give back <expr>           */
    ND_MAKE_LIST, /* make list x                */
    ND_ADD_LIST,  /* add <val> to x             */
    ND_IMPORT,    /* import "file.se"           */
    ND_STOP,      /* stop  (break)              */
    ND_SKIP,      /* skip  (continue)           */
    /* expressions */
    ND_NUMBER,    /* 3.14                       */
    ND_STRING,    /* "hello"                    */
    ND_BOOL,      /* true / false               */
    ND_NOTHING,   /* nothing (null)             */
    ND_IDENT,     /* variable name              */
    ND_BINOP,     /* + - * / %                  */
    ND_NEGATE,    /* unary minus                */
    ND_NOT,       /* not <cond>                 */
    ND_CMP,       /* is equal to / greater …    */
    ND_AND,       /* <cond> and <cond>          */
    ND_OR,        /* <cond> or <cond>           */
    ND_CALL_EXPR, /* result of f with args      */
    ND_LIST_GET,  /* item <n> of <list>         */
    ND_LIST_LEN,  /* length of <list>           */
} NodeKind;

/* ── binary/comparison operator tags ────────────────────────── */
typedef enum { BINOP_ADD, BINOP_SUB, BINOP_MUL, BINOP_DIV, BINOP_MOD } BinOp;
typedef enum {
    CMP_EQ, CMP_NEQ, CMP_GT, CMP_LT, CMP_GTE, CMP_LTE
} CmpOp;

/* ── node list (dynamic array) ───────────────────────────────── */
typedef struct Node Node;
typedef struct {
    Node **items;
    int    count;
    int    cap;
} NodeList;

void node_list_push(NodeList *nl, Node *n);

/* ── AST node ─────────────────────────────────────────────────  */
struct Node {
    NodeKind kind;
    int      line;
    union {
        /* ND_NUMBER */           double   num;
        /* ND_STRING/IDENT */     char    *str;
        /* ND_BOOL */             int      bool_val;

        /* ND_PROGRAM */          NodeList program;

        /* ND_SET */              struct { char *name; Node *expr; } set;
        /* ND_SAY */              Node *say;
        /* ND_ASK */              struct { char *name; char *prompt; } ask;

        /* ND_IF: parallel arrays: conditions (NULL→else), bodies */
        struct {
            NodeList conds;   /* NULL entry = else branch */
            NodeList bodies;  /* NodeList each is a block  */
            NodeList *blocks; /* array of NodeList          */
            int       count;
        } if_stmt;

        /* ND_REPEAT */           struct { Node *count; NodeList body; } repeat;
        /* ND_WHILE */            struct { Node *cond;  NodeList body; } while_stmt;

        /* ND_DEFINE */
        struct {
            char     *name;
            char    **params;
            int       param_count;
            NodeList  body;
        } define;

        /* ND_CALL_STMT / ND_CALL_EXPR */
        struct { char *name; NodeList args; } call;

        /* ND_RETURN */           Node *ret;
        /* ND_MAKE_LIST */        char *list_name;
        /* ND_ADD_LIST */         struct { Node *val; char *name; } add_list;
        /* ND_IMPORT */           char *import_path;

        /* ND_BINOP */            struct { BinOp op; Node *left; Node *right; } binop;
        /* ND_NEGATE/ND_NOT */    Node *unary;
        /* ND_CMP */              struct { CmpOp op; Node *left; Node *right; } cmp;
        /* ND_AND/ND_OR */        struct { Node *left; Node *right; } logical;
        /* ND_LIST_GET */         struct { Node *index; char *name; } list_get;
        /* ND_LIST_LEN */         char *list_len;
    };
};

Node *node_new (NodeKind kind, int line);
void  node_free(Node *n);

#endif /* SENG_AST_H */
