#include <as1.h>
#include <stdio.h>

void p_error(int n) {
    char buf[512], c, * p = buf;
    while ((c = get()) != '\n' && c != EOF)
        *p++ = c;
    *p = '\0';
    if (c == EOF) unget(c);
    error(p);
}

void p_if(int n) {
    bool bad;
    int save_line = current_file.line;
    word cond = cexpr();
ifelse:
    while (!trylex(L_SYM)) {
        if (cond) bad = !assem();
        else bad = lex() == L_EOF;
        if (bad) {
            error("non-terminated .if on %d", save_line);
            unlex(L_EOF);
            return;
        }
    }
    if (!XSYM_BUILTIN(lval.sym) || lval.sym->builtin_id != P_ENDIF) {
        unlex(L_SYM);
        goto ifelse;
    }
    cond = !cond;
    if (lval.sym->value == 0)
        goto ifelse;
}

void p_endif(int n) {
    error(".%s without .if", n ? "endif" : "else");
}

void p_segment(int n) {
    static int saved_dot_values[4];
    saved_dot_values[current_segment] = dot->value;
    dot->segment = ddot->segment = current_segment = n;
    dot->value = saved_dot_values[current_segment];
    if (current_segment == SEG_CONST) dot->value = 0;
}

void p_even(int n) {
    if (dot->value % 2) {
        if (current_segment < 2) putbyte(0);
        else dot->value++;
    }
}

void p_mut(int n) {
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
    putbyte(cexpr());
}

void p_fill(int n) {
    int rep, val = 0;
    rep = cexpr();
    if (trylex(',')) val = cexpr();
    while (rep--) putbyte(val);
}
