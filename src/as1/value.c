#include "as.h"

static struct reloc relocs[8000];
static int nrelocs;

void put_value(struct value* v, bool pcrel, int size) {
    if (v->defined && !v->filerel && !pcrel)
        return put_const(v->constant, size);
    struct reloc* r = &relocs[nrelocs++];
    if (!v->hasname || symbol(v->symbol)->mutable) {
        struct symbol* s = lookup(counter());
        s->value = v->constant;
        s->filerel = v->hasname && symbol(v->symbol)->filerel;
        s->defined = true;
        v->hasname = true;
        v->symbol = symid(s);
    }
    r->pcrel = pcrel;
    r->size = size;
    r->sym = v->symbol;
    r->ptr = textsize;
    put_const(0, size);
}

int flush_relocs(void) {
    return fwrite(relocs, 1, sizeof(struct reloc) * nrelocs, stdout);
}

void value(struct value* v) {
    bool esc;
    v->defined = true;
    v->filerel = false;
    v->hasname = false;
    if (trylex('\'')) {
        v->constant = lexch(&esc);
        return;
    }
    if (lex()) error("bad value");
    char* p;
    v->constant = strtol(lbuf, &p, 0);
    if (!*p) return;
    struct symbol* s;
    if ('0' <= lbuf[0] && lbuf[0] <= '9' && lbuf[1] == 'b' || lbuf[1] == 'f' && !lbuf[2])
        s = shortsym(lbuf[0] - '0', lbuf[1]);
    else s = lookup(lbuf);
    v->defined = s->defined;
    v->filerel = s->filerel;
    v->hasname = true;
    v->constant = s->value;
    v->symbol = symid(s);
}

int value_size(struct value* v, bool sign, bool unsign) {
    if (v->filerel || !v->defined) return 1;
    if (sign && v->constant < 128 && v->constant >= -128)
        return 0;
    if (unsign && v->constant < 256 && v->constant >= 0)
        return 0;
    return 1;
}

void optimize(int id) {
    relocs[nrelocs - 1].optimize = id;
}
