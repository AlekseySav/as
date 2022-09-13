#include "as.h"

static short constexpr(void) {
    struct value v = eval();
    if (!v.defined || v.filerel) error("non-const expression");
    return v.constant;
}

static void do_nop(int n) { }

static void do_ifelse(int isif) {
    if (isif && constexpr()) return;
    for (;;) {
        while (lex());
        if (isif && !strcmp(lbuf, ".else")) return;
        if (!strcasecmp(lbuf, ".endif")) return;
    }
}

static void do_setflag(int n) {
    while (!trylex(';')) {
        if (lex()) error("bad .global arguments");
        struct symbol* s = lookup(lbuf);
        switch (n) {
            case 0: s->islocal = false; break;
            case 1: s->mutable = true; break;
        }
        trylex(',');
    }
}

static void do_proc(int n) {
    if (lex()) error("bad .proc name");
    if (lex() != ':') error("bad .proc format");
    unlex();
    unlex();
    parse();
    putb(0x5e); /* pop si */
    endline();
}


static struct {
    const char name[8];
    void (*exec)(int);
    int n;
} cmd[] = {
    { "endif", do_nop },
    { "else", do_ifelse, 0 },
    { "if", do_ifelse, 1 },
    { "global", do_setflag, 0 },
    { "mutable", do_setflag, 1 },
    { "proc", do_proc }
};

void preprocess(const char* name) {
    for (int i = 0; i < sizeof(cmd) / sizeof(cmd[0]); i++)
        if (!strcmp(cmd[i].name, name))
            return cmd[i].exec(cmd[i].n);
}
