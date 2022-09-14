/* symtable */

#include "as.h"

#define SYMSIZE 8000

static struct symbol symtab[SYMSIZE];
static struct symbol* hshed[SYMSIZE];
static int nsyms;

static inline unsigned hash(const char* s) {
    unsigned n = 0;
    while (*s) n = (n * 71 + *s++ - '.') % SYMSIZE;
    return n;
}

struct symbol* lookup(const char* name) {
    unsigned n, t, i;
    struct symbol* e;
    if (name[0] == '.' && name[1] != '.' && name[1] || '0' <= name[0] && name[0] <= '9')
        error(0, "bad symbol name");
    for (t = n = hash(name), i = 0; e = hshed[n]; i++, n = (n + i * i) % SYMSIZE) {
        if (!strncmp(name, e->name, sym_len(e)))
            return e;
        if (i && t == n) error(0, "symtab overflow");
    }
    e = hshed[n] = &symtab[nsyms++];
    return e;
}

void clearsyms(void) {
    memset(hshed, 0, SYMSIZE * sizeof(struct symbol*));
    memset(symtab, 0, nsyms * sizeof(struct symbol));
    nsyms = 0;
}
