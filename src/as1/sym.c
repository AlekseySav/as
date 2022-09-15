/* symtable */

#include "as.h"

#define SYMSIZE 1000

static struct symbol symtab[SYMSIZE];
static struct symbol* hshed[SYMSIZE];
static struct symbol* f[10], * b[10];
static int nsyms;

static inline unsigned hash(const char* s) {
    unsigned n = 0;
    while (*s) n = (n * 71 + *s++ - '.') % SYMSIZE;
    return n;
}

const char* counter(void) {
    static char r[2] = { 1, 0 };
    if (!++r[1]) {
        r[0]++;
        if (r[0] >= ' ') error("counter overflow");
    }
    return r;
}

struct symbol* lookup(const char* name) {
    unsigned n, t, i;
    struct symbol* e;
    if (name[0] == '.' && name[1] != '.' && name[1] || '0' <= name[0] && name[0] <= '9')
        error("bad symbol name");
    for (t = n = hash(name), i = 0; e = hshed[n]; i++, n = (n + i * i) % SYMSIZE) {
        if (!strncmp(name, e->name, sym_len(e)))
            return e;
        if (i && t == n) error("symtab overflow");
    }
    e = hshed[n] = &symtab[nsyms++];
    if (strlen(name) <= 2) e->isshort = true;
    strncpy(e->name, name, sym_len(e));
    if (!autoglobal) e->islocal = true;
    return e;
}

struct symbol* shortsym(int n, char bf) {
    switch (bf) {
        case 'b':
            if (!b[n]) error("short symbol (b) not defined");
            return b[n];
        case 'f':
            if (!f[n]) f[n] = lookup(counter());
            return f[n];
        default:
            b[n] = f[n], f[n] = NULL;
            if (!b[n]) b[n] = lookup(counter());
            return b[n];
    }
}

int flush_syms(void) {
    int size = 0;
    for (int i = 0; i < 10; i++)
        if (f[i]) error("short symbol (f) not defined");
    for (struct symbol* e = symtab; e < &symtab[nsyms]; e++) {
        int i = strlen(e->name) + sizeof(struct symbol) - 7;
        size += fwrite(e, 1, i < sym_size(e) ? i : sym_size(e), stdout);
    }
    return size;
}

int symid(struct symbol* s) {
    return s - symtab;
}

struct symbol* symbol(int id) {
    return &symtab[id];
}
