#include <as1.h>

void bad_opcode(int n) {
    error("bad opcode: %s", lval.sym->name);
}

void x_chsize(int n) {
    size = 0;
    lval.sym--;
    opcodes[lval.sym->builtin_id](lval.sym->value);
}

bool size;

static void define(int t, union lval lval, struct value value) {
    struct x_symbol* sym = lval.sym;
    if (t == L_NUM) sym = define_fb(lval.num);
    assign(sym, value);
}

void assign(struct x_symbol* sym, struct value value) {
    if (iffalse) return;
    if (sym->defined && !sym->mutable)
        error("re-assigning immutable variable");
    sym->defined = value.defined;
    sym->segment = value.segment;
    sym->value = value.value;
}

void do_assem(void) {
    if (trylex(L_SYM)) {
        if (XSYM_BUILTIN(lval.sym))
            return opcodes[lval.sym->builtin_id](lval.sym->value);
        unlex(L_SYM);
    }
    int t = lex();
    union lval bakup = lval;
    if (t == L_SYM || t == L_NUM) {
        while (trylex(L_SYM) || trylex(L_NUM)) error("repeated value");
        if (trylex(':'))
            return define(t, bakup, (struct value){.sym = dot, .value = dot->value, .segment = dot->segment, .defined = 1});
        else if (trylex('='))
            return define(t, bakup, expr());
    }
    unlex(t);
    put_value(expr(), false, 1);
}

static void put_string(void) {
    bool esc;
    char c;
    while ((c = strchar(&esc)) != '>' || esc) {
        if (c == '\n' && !esc) {
            error("non-terminated string");
            return;
        }
        putbyte(c);
    }
}

bool assem(void) {
    if (trylex(L_EOF)) return false;
    if (trylex(';')) return assem();
    size = 1;
    if (trylex('<')) put_string();
    else do_assem();
    dot->value += bytes_written;
    bytes_written = 0;
    return true;
}
