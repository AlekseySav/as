#include "as.h"

int rr, sr, modrm, nargs;
struct value* rm_disp;

static struct arg {
    int flags;
    union {
        int modrm;
        int sr;
    };
    struct value value;
} args[30];

static inline int gpreg(const char* name) {
    static const char* gpregs[] = {
        "al", "cl", "dl", "bl", "ah", "ch", "dh", "bh",
        "ax", "cx", "dx", "bx", "sp", "bp", "si", "di"
    };
    for (const char** s = gpregs; s < gpregs + 16; s++)
        if (!strcmp(name, *s))
            return s - gpregs;
    return -1;
}

static inline int sreg(const char* name) {
    static const char* sregs[] = {
        "es", "cs", "ss", "ds", "fs", "gs"
    };
    for (const char** s = sregs; s < sregs + 6; s++)
        if (!strcmp(name, *s))
            return s - sregs;
    return -1;
}

static int get_modrm(struct value* disp) {
    int rm = 1, tmp;
    for (;;) {
        if (!lex() && (tmp = gpreg(lbuf)) != -1)
            rm = rm << 3 | tmp;
        else {
            unlex();
            if (!disp) error("double displacement in mod r/m byte");
            *disp = eval();
            disp = NULL;
        }
        if (trylex(')')) break;
        if (lex() != ',') error("expected ','");
    }
    if (!disp) rm |= 0200;
    switch (rm) {
        case 0136: case 0163: return 0000;      /* (bx,si) */
        case 0137: case 0173: return 0001;      /* (bx,di) */
        case 0156: case 0165: return 0002;      /* (bp,si) */
        case 0157: case 0175: return 0003;      /* (bp,di) */
        case 0016:            return 0004;      /* (si) */
        case 0017:            return 0005;      /* (di) */
        case 0201:            return 0006;      /* (disp) */
        case 0013:            return 0007;      /* (bx) */
        case 0336: case 0363: return 0100;      /* (bx,si,disp) */
        case 0337: case 0373: return 0101;      /* (bx,di,disp) */
        case 0356: case 0365: return 0102;      /* (bp,si,disp) */
        case 0357: case 0375: return 0103;      /* (bp,di,disp) */
        case 0216:            return 0104;      /* (si,disp) */
        case 0217:            return 0105;      /* (di,disp) */
        case 0215:            return 0106;      /* (bp,disp) */
        case 0213:            return 0107;      /* (bx,disp) */
        default: error("bad mod r/m byte");
    }
}

bool readarg(void) {
    if (trylex(';')) return false;
    if (nargs && lex() != ',') error("expected ','");
    struct arg* a = &args[nargs++];
    switch (lex()) {
        case '(':                               /* memory */
            a->modrm = get_modrm(&a->value);
            a->flags = ARG_MEM;
            break;
        case 0:
            if ((a->sr = gpreg(lbuf)) != -1) {  /* gp-reg */
                a->flags = 1 << a->sr;
                a->modrm = a->sr & 7 | 0300;
                break;
            }
            if ((a->sr = sreg(lbuf)) != -1) {   /* segment */
                a->flags = ARG_SEG;
                break;
            }
            unlex();
            a->value = eval();                  /* constant */
            if (trylex('(')) {                  /* memory */
                a->modrm = get_modrm(NULL);
                a->flags = ARG_MEM;
                break;
            }
            a->flags = ARG_IMM;                 /* immediate */
            break;
        default:                                /* immediate */
            unlex();
            a->value = eval();
            a->flags = ARG_IMM;
            break;
    }
    return true;
}

static inline bool compare(struct arg* arg, int expect) {
    if (expect & ARG_BYTE || expect & ARG_DEFAULT && size == 0) {
        expect &= ~ARG_BYTE;
        if (value_size(&arg->value, expect & ARG_SIGN, expect & ARG_UNSIGN) >= 1)
            return false;
    }
    if (!(arg->flags & expect)) return false;

    if (expect & ARG_MEM)
        modrm = arg->modrm, rm_disp = &arg->value;
    else if (expect & ARG_REG)
        rr = arg->modrm & 7;
    else if (expect & ARG_SEG)
        sr = arg->sr;
    return true;
}

bool cmpargs(int n, ...) {
    int savsiz = size;
    if (n > nargs) return false;
    va_list ap;
    va_start(ap, n);
    for (int i = 0; i < n; i++) {               /* get instruction size */
        if (size == 1 && va_arg(ap, int) & ARG_DEFSIZ && args[i].flags & ARG_RR1) {
            size = 0;
            break;
        }
    }
    va_end(ap);
    va_start(ap, n);
    for (int arg = 0, i = 0; i < nargs; i++) {  /* compare args */
        if (!(arg & ARG_VAARG)) {
            if (!n) return false;
            arg = va_arg(ap, int), n--;
        }
        if (!compare(&args[i], arg & ~(ARG_VAARG | ARG_DEFSIZ))) {
            size = savsiz;
            return false;
        }
    }
    va_end(ap);
    return true;
}

struct value* arg(int n) {
    return &args[n].value;
}

void put_rm(int rr) {
    if (rr == -1) return put_value(rm_disp, false, 1);
    int mrm = modrm | (rr & 7) << 3;
    if (modrm == 0006) {
        putb(mrm);
        put_value(rm_disp, false, 1);
    }
    else if (modrm >> 6 == 1) {
        int size = value_size(rm_disp, true, false);
        mrm = mrm & 0077 | 0100 << size;
        putb(mrm);
        put_value(rm_disp, false, size);
        optimize(OT_MRMDISP);
    }
    else putb(mrm);
}
