#include <as1.h>

static void bad(int n) {
    error("bad opcode: %s", lval.sym->name);
}

static void chsize(int n) {
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
            return define(t, lval, (struct value){.sym = dot, .value = dot->value, .segment = dot->segment, .defined = 1});
        else if (trylex('='))
            return define(t, lval, expr());
        unlex(t);
    }
    put_value(expr(), false, 1);
}

bool assem(void) {
    if (trylex(L_EOF)) return false;
    if (trylex(';')) return assem();
    size = 1;
    do_assem();
    dot->value += bytes_written;
    bytes_written = 0;
    return true;
}

void (*opcodes[I_N_OPCODES])(int n) = {
    bad,
    p_error, p_if, p_endif, p_segment, p_even, p_mut, p_byte, p_fill,
    bad, bad, bad, bad,
    chsize,
    o_onebyte, o_string, o_argbyte, o_segment, o_sys,
    o_math0, o_math1, o_math2, o_test, o_incdec,
    o_stack
};
