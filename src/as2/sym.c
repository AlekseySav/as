/* symtable */

#include "as.h"

#define SYMSIZE 8000

/* 0=local, 1=global */
static struct symbol symtab[2][SYMSIZE];
static struct symbol* hshed[2][SYMSIZE];
static int nsyms[2];

static inline unsigned hash(const char* s) {
    unsigned n = 0;
    while (*s) n = (n * 71 + *s++ - '.') % SYMSIZE;
    return n;
}

struct symbol* lookup(int g, const char* name) {
    unsigned n, t, i;
    struct symbol* e;
    for (t = n = hash(name), i = 0; e = hshed[g][n]; i++, n = (n + i * i) % SYMSIZE) {
        if (!strncmp(name, e->name, sym_len(e)))
            return e;
        if (i && t == n) error(0, "symtab overflow");
    }
    e = hshed[g][n] = &symtab[g][nsyms[g]++];
    return e;
}

void clearsyms(void) {
    memset(hshed[0], 0, SYMSIZE * sizeof(struct symbol*));
    memset(symtab[0], 0, nsyms[0] * sizeof(struct symbol));
    nsyms[0] = 0;
}
