#include "as.h"

static int textsize;
static struct exec exec;
static struct ex_reloc relocs[60000];
static int nrelocs, prelocs;
static char text[64 * 1024];

static struct symbol* filesyms[1000];
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

static void readfile(const char* filename) {
    FILE* f;
    memset(filesyms, 0, nfilesyms * sizeof(struct symbol*));
    nfilesyms = 0;
    if (!(f = fopen(filename, "rt"))) perror("");
    if (!fread(&exec, sizeof(struct exec), 1, f)) perror("");

    fread(text, 1, exec.text, f);
    fwrite(text, 1, exec.text, stdout);
    if (exec.text % 2) getc(f);

    short syms = 0;
    while (syms < exec.syms) {
        struct symbol sym;
        int i;
        fread(&sym, 1, sizeof(sym), f);
        syms += 12;
        nfilesyms++;
        if (sym.isshort) {
            syms += sym_size(&sym) - sizeof(struct symbol);
            fseek(f, sym_size(&sym) - sizeof(struct symbol), SEEK_CUR);
        }
        else if ((i = 7 - strlen(sym.name)) > 0) {
            syms -= i;
            fseek(f, -i, SEEK_CUR);
        }
        if (!sym.defined) sym.islocal = false;
        struct symbol* s = lookup(!sym.islocal, sym.name);
        if (s->defined && sym.defined)
            error(0, "symbol '%s' redefined", s->name);
        if (!s->defined) {
            memcpy(s, &sym, sym_size(&sym));
            if (s->filerel) s->value += textsize;
        }
        filesyms[nfilesyms - 1] = s;
    }
    short rel = 0;
    while (rel < exec.reloc) {
        struct ex_reloc* r;
        rel += fread(r = &relocs[nrelocs++], 1, sizeof(struct reloc), f);
        r->r.ptr += textsize;
        r->s = filesyms[r->r.sym];
    }
    textsize += exec.text;
    fclose(f);
}

static void relocate(int n) {
    for (struct ex_reloc* r = relocs + prelocs; r < &relocs[nrelocs + prelocs]; r++) {
        struct symbol* s = r->s;
        if (!s && !n) continue;
        if (!s) error(r->r.ptr, "reference to undefined symbol");
        if (!s->name[0]) continue;
        if (!n && !s->defined) continue;
        if (!s->defined) error(r->r.ptr, "reference to undefined symbol");
        if (s->mutable) error(r->r.ptr, "reference to mutable symbol");
        fseek(stdout, r->r.ptr, SEEK_SET);
        short size = 1 << (1 & r->r.size);
        short data = s->value;
        if (r->r.pcrel) data -= r->r.ptr + size;
        fwrite(&data, size, 1, stdout);
    }
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        readfile(argv[i]);
        fflush(stdout);
        relocate(0);
        fseek(stdout, textsize, SEEK_SET);
        prelocs = nrelocs;
        clearsyms();
    }
    prelocs = 0;
    relocate(1);
}
