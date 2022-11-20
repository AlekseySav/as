#include <as1.h>

static void check_const(struct value v, bool rel) {
    if (!v.sym) return;
    if (v.defined && (rel || v.segment == SEG_CONST)) return;
    error("math operations on non-const value");
}

static struct value value(void) {
    char t = lex();
    struct value v;
    struct x_symbol* e;
    switch (t) {
        case L_SYM:
            e = lval.sym;
            if (XSYM_BUILTIN(e)) error("used builtin symbol as value");
            return (struct value){.sym = e, .value = e->value, .segment = e->segment, .defined = e->defined};
        case L_NUM:
            return (struct value){.sym = NULL, .value = lval.num, .segment = SEG_CONST, .defined = 1};
        case '[':
            v = expr();
            if (lex() != ']') error("missing closing bracket ']'");
            break;
        case '-':
            v = value();
            check_const(v, false);
            v.value = -v.value;
        case '+':
            break;
        case '!':
            v = value();
            check_const(v, false);
            v.value = !v.value;
            break;
        case '?':
            v = value();
            v.value = (bool)(v.defined);
            v.defined = 1;
            break;
        default:
            error("bad unary operator");
            return (struct value){NULL, 0, SEG_CONST, 1};
    }
    return v;
}

#define o b = value(); check_const(a, false); check_const(b, false); a.sym = NULL;

struct value expr(void) {
    struct value a, b;
    int t;
    a = value();
    for (;;) {
        switch (t = lex()) {
            case '+':
                b = value();
                check_const(a, true);
                check_const(b, true);
                if (a.segment != SEG_CONST && b.segment != SEG_CONST)
                    error("adding two relative values");
                a.value += b.value;
                if (a.segment == SEG_CONST)
                    a.segment = b.segment;
                a.sym = NULL;
                break;
            case '-':
                b = value();
                check_const(a, true);
                check_const(b, true);
                if (b.segment != SEG_CONST) {
                    if (b.segment != a.segment) error("subtracting badly-relative values");
                    a.segment = SEG_CONST;
                }
                a.value -= b.value;
                a.sym = NULL;
                break;
            case '*': o; a.value *= b.value; break;
            case '/': o; a.value /= b.value; break;
            case '|': o; a.value |= b.value; break;
            case '&': o; a.value &= b.value; break;
            case '=': o; a.value = (a.value == b.value); break;
            default:
                unlex(t);
                return a;
        }
    }
}

word cexpr(void) {
    struct value v = expr();
    check_const(v, false);
    return v.value;
}

void put_value(struct value v, bool pc_rel, int size_log) {
    if (!pc_rel && v.defined && v.segment == SEG_CONST) {
        size_log ? putword(v.value) : putbyte(v.value);
        return;
    }
    if (!v.sym || v.sym->mutable) {
        if (v.sym && !v.defined)
            error("using undefined mutable symbol");
        struct x_symbol* e = lookup(NULL);
        e->defined = 1;
        e->value = v.value;
        v.sym = e;
    }
    struct reloc r;
    r.symbol = v.sym - symtab;
    r.pcrel = pc_rel;
    r.size = size_log;
    r.addr = A_SEGSIZE(exec, current_segment);
    put_reloc(r);
    size_log ? putword(0) : putbyte(0);
}
