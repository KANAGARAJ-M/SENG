
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "interp.h"
#include "compiler.h"
#include "vm.h"
#include "packages.h"
#include <string.h>
#include <stdio.h>

static void print_usage(void) {
    printf(
        "seng v1.0.2 — Simple English Programming Language\n"
        "\n"
        "Usage:\n"
        "  seng <file.se>              Run a .se source file\n"
        "  seng compile <file.se>      Compile to <file.sec> bytecode\n"
        "  seng run <file.sec>         Execute a compiled .sec file\n"
        "  seng disasm <file.sec>      Disassemble a .sec file\n"
        "  seng repl                   Start an interactive shell\n"
        "  seng help                   Show this help\n"
        "\n"
        "File extensions:\n"
        "  .se   — seng source file\n"
        "  .sec  — seng compiled bytecode\n"
    );
}

static void run_repl(void) {
    printf("seng REPL v" SENG_VERSION "\nType 'stop' or Ctrl+D to exit.\n");
    Interp *in = interp_new();
    char line[2048];
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        if (strcmp(line, "stop\n") == 0) break;
        if (line[0] == '\n') continue;
        
        Lexer  *lex  = lexer_new(line);
        Parser *par  = parser_new(lex);
        Node   *prog = parse(par);
        if (prog) {
            interp_exec(in, prog);
            node_free(prog);
        }
        parser_free(par);
        lexer_free(lex);
    }
    printf("\n");
    interp_free(in);
}

/* build output path: replace .se → .sec */
static char *make_sec_path(const char *src_path) {
    size_t len = strlen(src_path);
    char  *out;
    if (len > 3 && strcmp(src_path + len - 3, ".se") == 0) {
        out = (char *)xmalloc(len + 2);
        memcpy(out, src_path, len);
        out[len]   = 'c';
        out[len+1] = '\0';
    } else {
        out = (char *)xmalloc(len + 5);
        sprintf(out, "%s.sec", src_path);
    }
    return out;
}

/* Run source file through interpreter */
static void run_source(const char *path) {
    char *src = read_file(path);
    if (!src) fatal("cannot open file '%s'", path);

    Lexer  *lex  = lexer_new(src);
    Parser *par  = parser_new(lex);
    Node   *prog = parse(par);
    parser_free(par);
    lexer_free(lex);
    free(src);

    Interp *interp = interp_new();
    interp_exec(interp, prog);
    interp_free(interp);
    node_free(prog);
}

/* Compile source file to .sec */
static void compile_source(const char *path) {
    char *src = read_file(path);
    if (!src) fatal("cannot open file '%s'", path);

    Lexer  *lex  = lexer_new(src);
    Parser *par  = parser_new(lex);
    Node   *prog = parse(par);
    parser_free(par);
    lexer_free(lex);
    free(src);

    char *out = make_sec_path(path);
    compile_to_sec(prog, out);
    printf("Compiled '%s' → '%s'\n", path, out);
    free(out);
    node_free(prog);
}

int main(int argc, char *argv[]) {
    pkg_set_args(argc, argv);
    if (argc < 2) { print_usage(); return 0; }

    const char *arg1 = argv[1];

    /* seng help */
    if (strcmp(arg1, "help") == 0 || strcmp(arg1, "--help") == 0
        || strcmp(arg1, "-h") == 0) {
        print_usage();
        return 0;
    }

    /* seng repl */
    if (strcmp(arg1, "repl") == 0) {
        run_repl();
        return 0;
    }

    /* seng compile <file.se> */
    if (strcmp(arg1, "compile") == 0) {
        if (argc < 3) fatal("usage: seng compile <file.se>");
        compile_source(argv[2]);
        return 0;
    }

    /* seng run <file.sec> */
    if (strcmp(arg1, "run") == 0) {
        if (argc < 3) fatal("usage: seng run <file.sec>");
        vm_run_file(argv[2]);
        return 0;
    }

    /* seng disasm <file.sec> */
    if (strcmp(arg1, "disasm") == 0) {
        if (argc < 3) fatal("usage: seng disasm <file.sec>");
        vm_disasm(argv[2]);
        return 0;
    }

    /* seng <file.se>  — direct interpret */
    size_t len = strlen(arg1);
    if (len > 4 && strcmp(arg1 + len - 4, ".sec") == 0) {
        vm_run_file(arg1);
    } else {
        run_source(arg1);
    }

    return 0;
}
