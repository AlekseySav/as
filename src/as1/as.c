#include "as.h"

int size, textsize, opsize;
static struct symbol* pc, * begin;

void error(const char* msg) {
    fprintf(stderr, "%d: %s\n", line, msg);
    exit(1);
}

void put_const(int v, int size) {
    size = 1 << (size & 1);
    fwrite(&v, 1, size, stdout);
    opsize += size;
    textsize += size;
}

static inline noreturn void quit(void) {
    if (textsize % 2) putb(0), pc->value++;
    int s = flush_syms();
    int r = flush_relocs();
    struct exec e = {
        .text = textsize,
        .bss = pc->value - textsize,
        .syms = s,
        .reloc = r
    };
    fseek(stdout, 0, SEEK_SET);
    fwrite(&e, sizeof(e), 1, stdout);
    exit(0);
}

static inline void string(void) {
    bool esc = false;
    int c;
    for (;;) {
        c = lexch(&esc);
        if (!esc && c == '>') return;
        putb(c);
    }
}

static void run(struct opcode* o) {
    if (!o) return unlex(), run(opcode("", &size));
    nargs = 0;
    while (readarg());
    o->group(o->n);
    endline();
}

/*
 * '$' debug-command
 * .preprocessor-statement
 * ';'
 * label ':'
 * label '=' expr
 * [ mnemonic ] [arg [, arg]]... ';'
 */

void parse(void) {
    char name[8];
    struct symbol* s;
    struct value v;
    opsize = 0;
    switch (lex()) {
        case 0:   break;
        case -1:  quit();
        case ';': return;
        case '<': string(); endline(); return;
        case '$': debug_command(); return;
        default:  run(NULL); return;
    }
    strncpy(name, lbuf, 8);
    if (name[0] == '.' && name[1] != '.' && name[1] != '\0')
        return preprocess(name + 1);

    switch (lex()) {
        case ':':
            if ('0' <= name[0] && name[0] <= '9' && name[1] == 0)
                s = shortsym(name[0] - '0', 0);
            else if ((s = lookup(name))->defined && !s->mutable)
                error("immutable label redefined");
            s->defined = true;
            s->filerel = pc->filerel || begin->filerel;
            s->value = pc->value + begin->value;
            return;
        case '=':
            s = lookup(name);
            v = eval();
            if (s->defined && !s->mutable) error("immutable label redefined");
            if (!v.defined) error("assigning to undefined value");
            s->defined = true;
            s->filerel = v.filerel;
            s->value = v.constant;
            return;
        default:  unlex(), run(opcode(name, &size)); return;
    }
}

void endline(void) {
    pc->value += opsize;
}

int main(int argc, char** argv) {
    pc = lookup(".");
    begin = lookup("..");
    pc->filerel = begin->filerel = true;
    pc->defined = begin->defined = true;
    pc->islocal = begin->islocal = true;
    pc->mutable = begin->mutable = true;
    fseek(stdout, sizeof(struct exec), SEEK_SET);
    for (;;) parse();
}
