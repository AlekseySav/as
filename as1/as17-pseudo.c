#include <as1.h>
#include <stdio.h>

void p_error(int n) {
    char buf[512], c, * p = buf;
    while ((c = get()) != '\n' && c != EOF)
        *p++ = c;
    *p = '\0';
    if (c == EOF) unget(c);
    if (!iffalse) error(p);
}

static int if_level = 0;

void p_if(int n) {
    if (if_level++)
        error("embeded .if not supported yet");
    iffalse = !cexpr();    
}

void p_else(int n) {
    if (!if_level)
        error(".else without .if");
    iffalse = !iffalse;
}

void p_endif(int n) {
    if (!if_level)
        error(".endif without .if");
    iffalse = false;
}

void p_segment(int n) {
    if (iffalse) return;
    static int saved_dot_values[4];
    saved_dot_values[current_segment] = dot->value;
    dot->segment = ddot->segment = current_segment = n;
    dot->value = saved_dot_values[current_segment];
    if (current_segment == SEG_CONST) dot->value = 0;
}

void p_even(int n) {
    if (iffalse) return;
    if (dot->value % 2) {
        if (current_segment < 2) putbyte(0);
        else dot->value++;
    }
}

void p_mut(int n) {
    if (if_level) {
        error("cannot use .mut inside .if");
        return;
    }
define:
    if (!trylex(L_SYM)) {
        error("required symbol name");
        return;
    }
    struct x_symbol* sym = lval.sym;
    if (sym->defined) error("already exists!");
    sym->mutable = 1;
    if (trylex('=')) assign(sym, expr());
    if (trylex(',')) goto define;
}

void p_byte(int n) {
byte:
    putbyte(cexpr());
    if (trylex(',')) goto byte;
}

void p_mm(int n) {
    struct arg_value v;
repeat:
    arg(A_RM, &v);
    put_modrm(&v, 0, false);
    if (trylex(',')) goto repeat;
}

void p_fill(int n) {
    int rep, val = 0;
    rep = cexpr();
    if (trylex(',')) val = cexpr();
    while (rep--) putbyte(val);
}

void p_func(int n) {
    if (!trylex(L_SYM)) {
        error("name expected");
        return;
    }
    if (iffalse) return;
    assign(lval.sym, (struct value){.defined = true, .segment = SEG_DATA, .sym = NULL, .value = exec.a_data});
    p_segment(SEG_DATA);
    putword(exec.a_text);
    p_segment(SEG_TEXT);
    bytes_written = 0;
    putbyte(0x55), putbyte(0x89), putbyte(0xe5);
}
