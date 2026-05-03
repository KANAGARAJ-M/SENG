
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Parser {
    Lexer *lex;
    Token  cur;      /* current (just consumed)  */
    Token  peek;     /* look-ahead               */
    int    had_peek; /* peek is valid            */
};

/* ── helpers ─────────────────────────────────────────────────── */

static Token p_peek(Parser *p) {
    if (!p->had_peek) {
        p->peek     = lexer_advance(p->lex);
        p->had_peek = 1;
    }
    return p->peek;
}

static Token p_advance(Parser *p) {
    if (p->had_peek) {
        p->cur      = p->peek;
        p->had_peek = 0;
    } else {
        p->cur = lexer_advance(p->lex);
    }
    return p->cur;
}

static int p_check(Parser *p, TkType t) {
    return p_peek(p).type == t;
}

static int p_match(Parser *p, TkType t) {
    if (p_check(p, t)) { p_advance(p); return 1; }
    return 0;
}

static Token p_expect(Parser *p, TkType t) {
    Token tk = p_peek(p);
    if (tk.type != t) {
        fatal("line %d: expected '%s' but got '%s'%s%s",
              tk.line, tk_name(t), tk_name(tk.type),
              tk.value ? " " : "", tk.value ? tk.value : "");
    }
    p_advance(p);
    return p->cur;
}

/* skip blank lines */
static void skip_newlines(Parser *p) {
    while (p_check(p, TK_NEWLINE)) p_advance(p);
}

/* consume a statement-terminating newline (or EOF) */
static void eat_newline(Parser *p) {
    if (p_check(p, TK_NEWLINE)) { p_advance(p); return; }
    if (p_check(p, TK_EOF))     return;
    Token tk = p_peek(p);
    fatal("line %d: expected end of line", tk.line);
}

/* forward declarations */
static Node  *parse_stmt    (Parser *p);
static Node  *parse_expr    (Parser *p);
static Node  *parse_cond    (Parser *p);
static NodeList parse_block (Parser *p); /* until end / else / EOF */

/* ── expression parsing ───────────────────────────────────────── */

static Node *parse_primary(Parser *p) {
    Token tk = p_peek(p);
    int   ln = tk.line;

    /* number */
    if (tk.type == TK_NUMBER) {
        p_advance(p);
        Node *n  = node_new(ND_NUMBER, ln);
        n->num   = atof(p->cur.value);
        free(p->cur.value);
        return n;
    }
    /* string */
    if (tk.type == TK_STRING) {
        p_advance(p);
        Node *n  = node_new(ND_STRING, ln);
        n->str   = p->cur.value;   /* ownership transferred */
        return n;
    }
    /* true / false */
    if (tk.type == TK_TRUE)  { p_advance(p); Node *n = node_new(ND_BOOL, ln); n->bool_val = 1; return n; }
    if (tk.type == TK_FALSE) { p_advance(p); Node *n = node_new(ND_BOOL, ln); n->bool_val = 0; return n; }
    /* nothing */
    if (tk.type == TK_NOTHING) { p_advance(p); return node_new(ND_NOTHING, ln); }

    /* ( expr ) */
    if (tk.type == TK_LPAREN) {
        p_advance(p);
        Node *n = parse_cond(p);
        p_expect(p, TK_RPAREN);
        return n;
    }

    /* length of <list> */
    if (tk.type == TK_LENGTH) {
        p_advance(p);
        p_expect(p, TK_OF);
        Token nm = p_expect(p, TK_IDENT);
        Node *n  = node_new(ND_LIST_LEN, ln);
        n->list_len = nm.value;
        return n;
    }

    /* item <expr> of <list> */
    if (tk.type == TK_ITEM) {
        p_advance(p);
        Node *idx = parse_expr(p);
        p_expect(p, TK_OF);
        Token nm  = p_expect(p, TK_IDENT);
        Node *n   = node_new(ND_LIST_GET, ln);
        n->list_get.index = idx;
        n->list_get.name  = nm.value;
        return n;
    }

    /* result of <func> [with <arg> [and <arg>]*] */
    if (tk.type == TK_RESULT) {
        p_advance(p);
        p_expect(p, TK_OF);
        Token nm = p_expect(p, TK_IDENT);
        Node *n  = node_new(ND_CALL_EXPR, ln);
        n->call.name = nm.value;
        if (p_match(p, TK_WITH)) {
            node_list_push(&n->call.args, parse_expr(p));
            while (p_match(p, TK_AND))
                node_list_push(&n->call.args, parse_expr(p));
        }
        return n;
    }

    /* identifier */
    if (tk.type == TK_IDENT) {
        p_advance(p);
        Node *n = node_new(ND_IDENT, ln);
        n->str  = p->cur.value;
        return n;
    }

    fatal("line %d: unexpected token '%s'%s%s in expression",
          ln, tk_name(tk.type),
          tk.value ? " '" : "", tk.value ? tk.value : "");
    return NULL; /* unreachable */
}

static Node *parse_unary(Parser *p) {
    Token tk = p_peek(p);
    if (tk.type == TK_MINUS_OP) {
        p_advance(p);
        Node *n = node_new(ND_NEGATE, tk.line);
        n->unary = parse_unary(p);
        return n;
    }
    return parse_primary(p);
}

/* term: * / % (also keyword 'times' 'divided by' 'mod') */
static Node *parse_term(Parser *p) {
    Node *left = parse_unary(p);
    while (1) {
        Token tk = p_peek(p);
        BinOp op;
        if      (tk.type == TK_STAR || tk.type == TK_TIMES) op = BINOP_MUL;
        else if (tk.type == TK_SLASH)                             op = BINOP_DIV;
        else if (tk.type == TK_PERCENT || tk.type == TK_MOD)     op = BINOP_MOD;
        else if (tk.type == TK_DIVIDED) {
            p_advance(p); p_expect(p, TK_BY);
            Node *right = parse_unary(p);
            Node *n = node_new(ND_BINOP, tk.line);
            n->binop.op = BINOP_DIV; n->binop.left = left; n->binop.right = right;
            left = n; continue;
        }
        else break;
        p_advance(p);
        Node *right = parse_unary(p);
        Node *n = node_new(ND_BINOP, tk.line);
        n->binop.op = op; n->binop.left = left; n->binop.right = right;
        left = n;
    }
    return left;
}

/* expr: + - (also keyword 'plus' 'minus') */
static Node *parse_expr(Parser *p) {
    Node *left = parse_term(p);
    while (1) {
        Token tk = p_peek(p);
        BinOp op;
        if      (tk.type == TK_PLUS_OP  || tk.type == TK_PLUS)  op = BINOP_ADD;
        else if (tk.type == TK_MINUS_OP || tk.type == TK_MINUS)  op = BINOP_SUB;
        else break;
        p_advance(p);
        Node *right = parse_term(p);
        Node *n = node_new(ND_BINOP, tk.line);
        n->binop.op = op; n->binop.left = left; n->binop.right = right;
        left = n;
    }
    return left;
}

/* comparison: expr [is cmp_op expr] */
static Node *parse_comparison(Parser *p) {
    Node *left = parse_expr(p);
    if (!p_check(p, TK_IS)) return left;
    p_advance(p);   /* consume 'is' */
    Token tk = p_peek(p);
    int   ln = tk.line;
    CmpOp op;

    if (tk.type == TK_EQUAL) {                       /* is equal to */
        p_advance(p); p_expect(p, TK_TO);            op = CMP_EQ;
    } else if (tk.type == TK_NOT) {                  /* is not equal to */
        p_advance(p); p_expect(p, TK_EQUAL); p_expect(p, TK_TO); op = CMP_NEQ;
    } else if (tk.type == TK_GREATER) {              /* is greater than [or equal to] */
        p_advance(p); p_expect(p, TK_THAN);
        if (p_match(p, TK_OR)) { p_expect(p, TK_EQUAL); p_expect(p, TK_TO); op = CMP_GTE; }
        else op = CMP_GT;
    } else if (tk.type == TK_LESS) {                 /* is less than [or equal to] */
        p_advance(p); p_expect(p, TK_THAN);
        if (p_match(p, TK_OR)) { p_expect(p, TK_EQUAL); p_expect(p, TK_TO); op = CMP_LTE; }
        else op = CMP_LT;
    } else {
        fatal("line %d: expected comparison operator after 'is'", ln);
        return NULL;
    }
    Node *right = parse_expr(p);
    Node *n = node_new(ND_CMP, ln);
    n->cmp.op = op; n->cmp.left = left; n->cmp.right = right;
    return n;
}

/* not */
static Node *parse_not(Parser *p) {
    Token tk = p_peek(p);
    if (tk.type == TK_NOT) {
        p_advance(p);
        Node *n = node_new(ND_NOT, tk.line);
        n->unary = parse_not(p);
        return n;
    }
    return parse_comparison(p);
}

/* and / or  */
static Node *parse_cond(Parser *p) {
    Node *left = parse_not(p);
    while (1) {
        Token tk = p_peek(p);
        NodeKind kind;
        if      (tk.type == TK_AND) kind = ND_AND;
        else if (tk.type == TK_OR)  kind = ND_OR;
        else break;
        p_advance(p);
        Node *right = parse_not(p);
        Node *n = node_new(kind, tk.line);
        n->logical.left = left; n->logical.right = right;
        left = n;
    }
    return left;
}

/* ── statement parsing ───────────────────────────────────────── */

/* block: reads statements until end / else / EOF */
static NodeList parse_block(Parser *p) {
    NodeList bl = {0};
    skip_newlines(p);
    while (1) {
        TkType t = p_peek(p).type;
        if (t == TK_END || t == TK_ELSE || t == TK_EOF) break;
        Node *s = parse_stmt(p);
        if (s) node_list_push(&bl, s);
        skip_newlines(p);
    }
    return bl;
}

static Node *parse_stmt(Parser *p) {
    skip_newlines(p);
    Token tk = p_peek(p);
    int   ln = tk.line;

    /* ── set ── */
    if (tk.type == TK_SET) {
        p_advance(p);
        Token nm = p_expect(p, TK_IDENT);
        p_expect(p, TK_TO);
        Node *expr = parse_cond(p);
        eat_newline(p);
        Node *n = node_new(ND_SET, ln);
        n->set.name = nm.value; n->set.expr = expr;
        return n;
    }

    /* ── say ── */
    if (tk.type == TK_SAY) {
        p_advance(p);
        Node *expr = parse_cond(p);
        eat_newline(p);
        Node *n = node_new(ND_SAY, ln);
        n->say = expr;
        return n;
    }

    /* ── ask ── */
    if (tk.type == TK_ASK) {
        p_advance(p);
        Token nm     = p_expect(p, TK_IDENT);
        p_expect(p, TK_FOR);
        Token prompt = p_expect(p, TK_STRING);
        eat_newline(p);
        Node *n = node_new(ND_ASK, ln);
        n->ask.name   = nm.value;
        n->ask.prompt = prompt.value;
        return n;
    }

    /* ── if ── */
    if (tk.type == TK_IF) {
        p_advance(p);
        Node *n = node_new(ND_IF, ln);
        n->if_stmt.count  = 0;
        n->if_stmt.blocks = NULL;

        /* first condition */
        Node *cond = parse_cond(p);
        p_expect(p, TK_THEN);
        eat_newline(p);
        NodeList body = parse_block(p);

        node_list_push(&n->if_stmt.conds, cond);
        n->if_stmt.count++;
        n->if_stmt.blocks = (NodeList *)xrealloc(n->if_stmt.blocks,
                            sizeof(NodeList) * (size_t)n->if_stmt.count);
        n->if_stmt.blocks[n->if_stmt.count - 1] = body;

        /* else if / else */
        while (p_check(p, TK_ELSE)) {
            p_advance(p);
            if (p_check(p, TK_IF)) {
                p_advance(p);
                Node *ec = parse_cond(p);
                p_expect(p, TK_THEN);
                eat_newline(p);
                NodeList eb = parse_block(p);
                node_list_push(&n->if_stmt.conds, ec);
                n->if_stmt.count++;
                n->if_stmt.blocks = (NodeList *)xrealloc(n->if_stmt.blocks,
                                    sizeof(NodeList) * (size_t)n->if_stmt.count);
                n->if_stmt.blocks[n->if_stmt.count - 1] = eb;
            } else {
                /* plain else — NULL sentinel in conds */
                eat_newline(p);
                NodeList eb = parse_block(p);
                node_list_push(&n->if_stmt.conds, NULL);
                n->if_stmt.count++;
                n->if_stmt.blocks = (NodeList *)xrealloc(n->if_stmt.blocks,
                                    sizeof(NodeList) * (size_t)n->if_stmt.count);
                n->if_stmt.blocks[n->if_stmt.count - 1] = eb;
            }
        }
        p_expect(p, TK_END);
        eat_newline(p);
        return n;
    }

    /* ── repeat ── */
    if (tk.type == TK_REPEAT) {
        p_advance(p);
        Node *cnt = parse_primary(p);  /* 'repeat 5 times' — just a simple value */
        p_expect(p, TK_TIMES);
        eat_newline(p);
        NodeList body = parse_block(p);
        p_expect(p, TK_END);
        eat_newline(p);
        Node *n = node_new(ND_REPEAT, ln);
        n->repeat.count = cnt; n->repeat.body = body;
        return n;
    }

    /* ── while ── */
    if (tk.type == TK_WHILE) {
        p_advance(p);
        Node *cond = parse_cond(p);
        eat_newline(p);
        NodeList body = parse_block(p);
        p_expect(p, TK_END);
        eat_newline(p);
        Node *n = node_new(ND_WHILE, ln);
        n->while_stmt.cond = cond; n->while_stmt.body = body;
        return n;
    }

    /* ── define ── */
    if (tk.type == TK_DEFINE) {
        p_advance(p);
        Token nm = p_expect(p, TK_IDENT);
        Node *n  = node_new(ND_DEFINE, ln);
        n->define.name = nm.value;
        if (p_match(p, TK_WITH)) {
            Token p1 = p_expect(p, TK_IDENT);
            n->define.params = (char **)xmalloc(sizeof(char *));
            n->define.params[0] = p1.value;
            n->define.param_count = 1;
            while (p_match(p, TK_AND)) {
                Token px = p_expect(p, TK_IDENT);
                n->define.params = (char **)xrealloc(n->define.params,
                    sizeof(char *) * (size_t)(n->define.param_count + 1));
                n->define.params[n->define.param_count++] = px.value;
            }
        }
        eat_newline(p);
        n->define.body = parse_block(p);
        p_expect(p, TK_END);
        eat_newline(p);
        return n;
    }

    /* ── call (statement) ── */
    if (tk.type == TK_CALL) {
        p_advance(p);
        Token nm = p_expect(p, TK_IDENT);
        Node *n  = node_new(ND_CALL_STMT, ln);
        n->call.name = nm.value;
        if (p_match(p, TK_WITH)) {
            node_list_push(&n->call.args, parse_expr(p));
            while (p_match(p, TK_AND))
                node_list_push(&n->call.args, parse_expr(p));
        }
        eat_newline(p);
        return n;
    }

    /* ── give back ─ */
    if (tk.type == TK_GIVE) {
        p_advance(p);
        p_expect(p, TK_BACK);
        Node *val = parse_cond(p);
        eat_newline(p);
        Node *n = node_new(ND_RETURN, ln);
        n->ret = val;
        return n;
    }

    /* ── make list ── */
    if (tk.type == TK_MAKE) {
        p_advance(p);
        p_expect(p, TK_LIST);
        Token nm = p_expect(p, TK_IDENT);
        eat_newline(p);
        Node *n = node_new(ND_MAKE_LIST, ln);
        n->list_name = nm.value;
        return n;
    }

    /* ── add <val> to <list> ── */
    if (tk.type == TK_ADD) {
        p_advance(p);
        Node *val = parse_expr(p);
        p_expect(p, TK_TO);
        Token nm  = p_expect(p, TK_IDENT);
        eat_newline(p);
        Node *n = node_new(ND_ADD_LIST, ln);
        n->add_list.val  = val;
        n->add_list.name = nm.value;
        return n;
    }

    /* ── import ── */
    if (tk.type == TK_IMPORT) {
        p_advance(p);
        if (p_check(p, TK_STRING)) {
            /* import "file.se" */
            Token path = p_expect(p, TK_STRING);
            eat_newline(p);
            Node *n = node_new(ND_IMPORT, ln);
            n->import_path = path.value;
            return n;
        } else {
            /* import math  (built-in package) */
            Token nm = p_expect(p, TK_IDENT);
            eat_newline(p);
            Node *n = node_new(ND_IMPORT_PKG, ln);
            n->str = nm.value;
            return n;
        }
    }

    /* ── stop / skip ── */
    if (tk.type == TK_STOP)  { p_advance(p); eat_newline(p); return node_new(ND_STOP, ln); }
    if (tk.type == TK_SKIP)  { p_advance(p); eat_newline(p); return node_new(ND_SKIP, ln); }

    /* ── blank line ── */
    if (tk.type == TK_NEWLINE) { p_advance(p); return NULL; }
    if (tk.type == TK_EOF)     return NULL;

    fatal("line %d: unexpected token '%s'%s%s",
          ln, tk_name(tk.type),
          tk.value ? " '" : "", tk.value ? tk.value : "");
    return NULL;
}

/* ── public API ──────────────────────────────────────────────── */

Parser *parser_new(Lexer *lex) {
    Parser *p = (Parser *)xcalloc(1, sizeof(Parser));
    p->lex = lex;
    return p;
}

void parser_free(Parser *p) {
    free(p);
}

Node *parse(Parser *p) {
    Node *root = node_new(ND_PROGRAM, 1);
    skip_newlines(p);
    while (!p_check(p, TK_EOF)) {
        Node *s = parse_stmt(p);
        if (s) node_list_push(&root->program, s);
        skip_newlines(p);
    }
    return root;
}
