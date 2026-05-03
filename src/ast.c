
#include "ast.h"

void node_list_push(NodeList *nl, Node *n) {
    if (nl->count >= nl->cap) {
        nl->cap = nl->cap ? nl->cap * 2 : 4;
        nl->items = (Node **)xrealloc(nl->items, sizeof(Node *) * (size_t)nl->cap);
    }
    nl->items[nl->count++] = n;
}

Node *node_new(NodeKind kind, int line) {
    Node *n = (Node *)xcalloc(1, sizeof(Node));
    n->kind = kind;
    n->line = line;
    return n;
}

static void node_list_free(NodeList *nl);

void node_free(Node *n) {
    if (!n) return;
    switch (n->kind) {
        case ND_STRING: case ND_IDENT:    free(n->str);  break;
        case ND_SET:    free(n->set.name); node_free(n->set.expr); break;
        case ND_SAY:    node_free(n->say); break;
        case ND_ASK:    free(n->ask.name); free(n->ask.prompt); break;
        case ND_REPEAT: node_free(n->repeat.count); node_list_free(&n->repeat.body); break;
        case ND_WHILE:  node_free(n->while_stmt.cond); node_list_free(&n->while_stmt.body); break;
        case ND_RETURN: node_free(n->ret); break;
        case ND_MAKE_LIST: free(n->list_name); break;
        case ND_ADD_LIST: node_free(n->add_list.val); free(n->add_list.name); break;
        case ND_IMPORT:     free(n->import_path); break;
        case ND_IMPORT_PKG: free(n->str); break;
        case ND_BINOP:  node_free(n->binop.left); node_free(n->binop.right); break;
        case ND_NEGATE: case ND_NOT: node_free(n->unary); break;
        case ND_CMP:    node_free(n->cmp.left); node_free(n->cmp.right); break;
        case ND_AND: case ND_OR:
            node_free(n->logical.left); node_free(n->logical.right); break;
        case ND_LIST_GET: node_free(n->list_get.index); free(n->list_get.name); break;
        case ND_LIST_LEN: free(n->list_len); break;
        case ND_CALL_STMT: case ND_CALL_EXPR:
            free(n->call.name); node_list_free(&n->call.args); break;
        case ND_DEFINE:
            free(n->define.name);
            for (int i = 0; i < n->define.param_count; i++) free(n->define.params[i]);
            free(n->define.params);
            node_list_free(&n->define.body);
            break;
        case ND_PROGRAM: node_list_free(&n->program); break;
        case ND_IF:
            node_list_free(&n->if_stmt.conds);
            for (int i = 0; i < n->if_stmt.count; i++)
                node_list_free(&n->if_stmt.blocks[i]);
            free(n->if_stmt.blocks);
            break;
        default: break;
    }
    free(n);
}

static void node_list_free(NodeList *nl) {
    for (int i = 0; i < nl->count; i++) node_free(nl->items[i]);
    free(nl->items);
    nl->items = NULL; nl->count = 0; nl->cap = 0;
}
