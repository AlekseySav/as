#include <as1.h>

static char reg2mod[8] = { 0, 0, 0, 7, 0, 6, 4, 5 };

static int get_modrm(int arg_size) {
    int mod = lval.sym->value;
    int rm = arg_size << 6;
    if (lval.sym->builtin_id == I_REGSUM) 
        return mod | rm;
    if (lval.sym->builtin_id != I_REGW)
        error("bad mod r/m mask");
    if (!(mod = reg2mod[mod]))
        error("bad mod r/m byte");
    if ((mod | rm) == 6)
        error("bad (bp) mod r/m byte");
    return mod | rm;
}

static enum arg raw_arg(struct arg_value* v) {
    int arg_size = 2;
    if (trylex('*')) arg_size = 1;
    if (trylex(L_SYM)) {    // get reg
        if (XSYM_BUILTIN(lval.sym)) {
            v->reg = lval.sym->value;
            if (lval.sym->builtin_id == I_REGB) return A_RB;
            if (lval.sym->builtin_id == I_REGW) return A_RW;
            if (lval.sym->builtin_id == I_SREG) return A_SR;
            error("bad (builtin) argument");
        }
        unlex(L_SYM);
    }
    if (!trylex('('))
        v->value = expr();
    else {
        arg_size = 0;
        unlex('(');
    }
    if (!trylex('('))   // get imm
        return A_IB << (arg_size - 1);
    if (trylex(L_SYM)) {    // get mem
        if (XSYM_BUILTIN(lval.sym)) {
            v->modrm = get_modrm(arg_size);
            if (!trylex(')')) error("missing ')'");
            return A_MM;
        }
        unlex(L_SYM);
    }
    if (arg_size) error("bad mod r/m byte: double offset");
    v->value = expr();
    v->modrm = 6;
    if (!trylex(')')) error("missing ')'");
    return A_MM;
}

enum arg arg(enum arg mask, struct arg_value* v) {
    enum arg raw = raw_arg(v);
    if (raw & A_RR) v->modrm = 0300 | v->reg;
    if (!(mask & raw)) {
        error("invalid combination of opcodes and operand");
        raw = mask;
    }
    return raw;
}

enum arg next_arg(enum arg mask, struct arg_value* v) {
    if (!trylex(',')) error("missed comma ','");
    return arg(mask, v);
}

void put_modrm(struct arg_value* mod, char rr, bool disponly) {
    if (disponly && mod->modrm != 6)
        error("required displacement-only mod r/m");
    if (!disponly)
        putbyte(mod->modrm | rr << 3);
    if (mod->modrm >> 6 != 3 && mod->modrm >> 6 != 0 || mod->modrm == 6)
        put_value(mod->value, false, mod->modrm == 6 || (mod->modrm >> 6 == 2));
}
