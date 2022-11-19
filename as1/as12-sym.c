#include <as1.h>
#include <string.h>

struct x_symbol* symtab;
char strtab[STRTAB_MAX_SIZE];

static struct x_symbol* symhash[SYMTAB_MAX_SIZE];

static struct x_symbol* alloc_symbol() {
    if (exec.a_symtab >= SYMTAB_MAX_SIZE)
        fatal("symtab size exceeded");
    return symtab + exec.a_symtab++;
}

static const char* alloc_name(const char* name) {
    char* res = strtab + exec.a_strtab;
    exec.a_strtab += strlen(name) + 1;
    if (exec.a_strtab > STRTAB_MAX_SIZE)
        fatal("strtab size exceeded");
    strcpy(res, name);
    return res;
}

static word hash(const char* s) {
    word res = 0;
    while (*s) res = (res * 79 + *s++ - ' ') % SYMTAB_MAX_SIZE;
    return res;
}

void init_builtins() {
    for (symtab = symbol_pool; symtab->name; symtab++) {
        word n = hash(symtab->name);
        while (symhash[n]) n = (n + 1) % SYMTAB_MAX_SIZE;
        symhash[n] = symtab;
    }
    dot = symbol_pool;
    ddot = symbol_pool + 1;
}

struct x_symbol* lookup(const char* name) {
    if (!name) return alloc_symbol();
    for (word n = hash(name), it = 0; it != SYMTAB_MAX_SIZE; it++, n = (n + 1) % SYMTAB_MAX_SIZE) {
        if (!symhash[n]) {
            symhash[n] = alloc_symbol();
            symhash[n]->name = alloc_name(name);
            return symhash[n];
        }
        if (!strcmp(symhash[n]->name, name))
            return symhash[n];
    }
    dot = symbol_pool;
    ddot = symbol_pool + 1;
    fatal("symtable size exceeded");
}

static struct x_symbol* fb[20];

struct x_symbol* get_fb(word n, char c) {
    if (n >= 10) fatal("bad fb (numeric) symbol (0-9 allowed only)");
    struct x_symbol** s = &fb[n + 10 * (c == 'f')];
    if (!*s) {
        if (c == 'b') error("undefined numeric symbol %db", n);
        *s = alloc_symbol();
    }
    return *s;
}

struct x_symbol* define_fb(int n) {
    if (fb[n + 10]) fb[n] = fb[n + 10];
    else fb[n] = alloc_symbol();
    fb[n + 10] = NULL;
    return fb[n];
}
