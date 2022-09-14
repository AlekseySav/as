#include "as.h"

static int textsize;
static struct exec exec;
static struct reloc relocs[60000];
static int nrelocs;
static char text[64 * 1024];

static struct symbol* filesyms[4000];
static int nfilesyms;

void error(short at, const char* msg, ...) {
    va_list ap;
    va_start(ap, msg);
    fprintf(stderr, "text+%d: ", at);
    vfprintf(stderr, msg, ap);
    fputc('\n', stderr);
    va_end(ap);
    exit(1);
}

static void readfile(const char* filename, int n) {
    FILE* f;
    nfilesyms = 0;
    if (!(f = fopen(filename, "rt"))) perror("");
    if (!fread(&exec, sizeof(struct exec), 1, f)) perror("");
    fread(text, 1, exec.text, f);
    if (exec.text % 2) getc(f);
    if (n == 0) fwrite(text, 1, exec.text, stdout);
    short syms = 0;
    while (syms < exec.syms) {
        struct symbol sym;
        fread(&sym, 1, sizeof(sym), f);
        syms += 12;
        nfilesyms++;
        if (sym.isshort) {
            syms += sym_size(&sym) - sizeof(struct symbol);
            fseek(f, sym_size(&sym) - sizeof(struct symbol), SEEK_CUR);
        }
        if (sym.islocal && sym.defined && n) continue;
        if (!sym.islocal && !n) continue;
        struct symbol* s = lookup(sym.name);
        if (s->defined && sym.defined) error(0, "symbol '%s' redefined", s->name);
        if (!s->defined) {
            memcpy(s, &sym, sym_size(&sym));
            if (s->filerel) s->value += textsize;
        }
        filesyms[nfilesyms - 1] = s;
    }
    short rel = 0;
    while (rel < exec.reloc) {
        struct reloc* r;
        rel += fread(r = &relocs[nrelocs++], 1, sizeof(struct reloc), f);
        r->ptr += textsize;
    }
    textsize += exec.text;
    fclose(f);
}

static void relocate(int n) {
    for (struct reloc* r = relocs; r < &relocs[nrelocs]; r++) {
        struct symbol* s = filesyms[r->sym];
        if (!s->name[0]) continue;
        if (!n && !s->defined) continue;
        if (!s->defined) error(r->ptr, "reference to undefined symbol");
        if (s->mutable) error(r->ptr, "reference to mutable symbol");
        fseek(stdout, r->ptr, SEEK_SET);
        short size = 1 << (1 & r->size);
        short data = s->value;
        if (r->pcrel) data -= r->ptr + size;
        fwrite(&data, size, 1, stdout);
    }
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        readfile(argv[i], 0);
        relocate(0);
        nrelocs = 0;
        clearsyms();
    }
    textsize = 0;
    for (int i = 1; i < argc; i++)
        readfile(argv[i], 1);
    relocate(1);
}
