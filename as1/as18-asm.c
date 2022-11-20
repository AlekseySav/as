#include <as1.h>

static enum arg a1, a2;
static struct arg_value v1, v2;

bool sreg_is_ok;

#define A_BYTEMASK ((size ? A_RM : A_MB) | A_IM)
#define A_SIZEMASK ((size ? A_MW : A_MB) | A_IM)

#define one_arg \
    a1 = arg(A_BYTEMASK ^ A_IM | (sreg_is_ok ? A_SR : 0), &v1); \
    if (a1 & A_RB) size = 0; \

#define two_args \
    one_arg \
    a2 = next_arg(A_BYTEMASK | (sreg_is_ok ? A_SR : 0), &v2); \
    if (a1 & A_MM && a2 & A_MM) error("too many memory operands"); \
    if (a1 & A_RB && a2 & A_RW || a1 & A_RW && a2 & A_RB) \
        error("registers of different sizes"); \
    if (a2 & A_RB) size = 0;

#define swap_args ({ \
    int ___tm; struct arg_value ___tmv; \
    ___tm = a1, a1 = a2, a2 = ___tm; \
    ___tmv = v1, v1 = v2, v2 = ___tmv; }) \

void o_onebyte(int n)   { putbyte(n); }
void o_string(int n)    { putbyte(n + size - 1); }
void o_argbyte(int n)   { putbyte(n); put_value(expr(), false, 0); }

void o_segment(int n) {
    arg(A_SR, &v1);
    if (v1.reg < 4) putbyte(v1.reg << 3 | 0x26);
    else putbyte(0x60 + v1.reg);
}

void o_sys(int n) {
    arg(A_IW, &v1);
    putbyte(0xb0);
    put_value(v1.value, false, 0);
    putword(0x20cd);
}

// add or adc sbb and sub xor cmp
void o_math0(int n) {
    two_args
    if (a2 & A_RR)
        return putbyte(n << 3 | 0 | size), put_modrm(&v1, v2.reg, false);
    if (a2 & A_MM)
        return putbyte(n << 3 | 2 | size), put_modrm(&v2, v1.reg, false);
    if (a2 & A_IB && size == 1)
        return putbyte(0x83), put_modrm(&v1, n, false), put_value(v2.value, false, 0);
    if (a1 & A_RR && v1.reg == 0)
        return putbyte(n << 3 | 4 | size), put_value(v2.value, false, size);
    putbyte(0x80 | size), put_modrm(&v1, n, false), put_value(v2.value, false, size);
}

// rol ror rcl rcr sal shl shr sar
void o_math1(int n) {
    one_arg
    a2 = next_arg(A_RB | A_IM, &v2);
    if (a2 & A_IM) {
        if (v2.value.defined && v2.value.segment == SEG_CONST && v2.value.value == 1)
            return putbyte(0xd0 | size), put_modrm(&v1, n, false);
        return putbyte(0xc0 | size), put_modrm(&v1, n, false), put_value(v2.value, false, 0);
    }
    if (v2.reg != 1) error("only immediate value or CL can be used");
    putbyte(0xd2 | size), put_modrm(&v1, n, false);
}

// not neg mul imul div idiv
void o_math2(int n) {
    one_arg
    putbyte(0xf6 | size), put_modrm(&v1, n, false);
}

void o_test(int n) {
    two_args
    if (a2 & A_MM) swap_args;
    if (a2 & A_RM)
        return putbyte(0x84 | size), put_modrm(&v1, v2.reg, false);
    if (a1 & A_RR && v1.reg == 0)
        return putbyte(0xa8 | size), put_value(v2.value, false, size);
    putbyte(0xf6 | size), put_modrm(&v1, 0, false), put_value(v2.value, false, size);
}

void o_incdec(int n) {
    one_arg
    if (size && a1 & A_RR)
        return putbyte(0x40 | n << 3 | v1.reg);
    putbyte(0xfe | size), put_modrm(&v1, n, false);
}

void o_stack(int n) {
    int a = arg(A_RW | A_IM | A_MM | A_SR, &v1);
    if (a & A_IM) {
        if (a & A_IB) size = 0;
        if (n == 1) error("cannot pop into immediate");
        return putbyte(0x6a - size * 2), put_value(v1.value, false, size);
    }
    if (a & A_RW)
        return putbyte(0x50 | n << 3 | v1.reg);
    if (a & A_SR) {
        if (v1.reg < 4) return putbyte(0x06 | v1.reg << 3 | n);
        return putbyte(0x0f), putbyte(0xa0 | v1.reg - 4  << 3 | n);
    }
    if (n == 0)
        return putbyte(0xff), put_modrm(&v1, 6, false);
    putbyte(0x8f), put_modrm(&v1, 0, false);
}

void o_ret(int n) {
    if (trylex(';')) return putbyte(n + 1);
    arg(A_IW, &v1);
    putbyte(n);
    put_value(v1.value, false, 1);
}

void o_inout(int n) {
    if (trylex(';')) return putbyte(0xe4 | 8 | n | size);
    arg(A_IM, &v1);
    putbyte(0xe4 | n | size);
    put_value(v1.value, false, 0);
}

void o_move(int n) {
    sreg_is_ok = true;
    two_args
    sreg_is_ok = false;
    if (a2 & A_IM) {
        if (a1 & A_RR)
            return putbyte(0xb0 | size << 3 | v1.reg), put_value(v2.value, false, size);
        return putbyte(0xc6 | size), put_modrm(&v1, 0, false), put_value(v2.value, false, size);
    }
    int adj = 0;
    if (a2 & A_MM || a1 & A_SR) {
        adj = 2;
        swap_args;
    }
    if (a2 & A_SR)
        return putbyte(0x8c | adj), put_modrm(&v1, v2.reg, false);
    if (a2 & A_RR && v2.reg == 0 && a1 & A_MM && v1.modrm == 6)
        return putbyte(0xa0 | 2 - adj | size), put_modrm(&v1, 0, true);
    putbyte(0x88 | adj | size), put_modrm(&v1, v2.reg, false);
}

void o_xchg(int n) {
    two_args
    if (a2 & A_IM) error("cannot use immediate value as xchg arg");
    if (a1 & A_RW && a2 & A_RW && v1.reg == 0)
        return putbyte(0x90 | v2.reg);
    if (a1 & A_RW && a2 & A_RW && v2.reg == 0)
        return putbyte(0x90 | v1.reg);
    if (a1 & A_MM) swap_args;
    putbyte(0x86 | size), put_modrm(&v2, v1.reg, false);
}

void o_cjump(int n) {
    putbyte(n);
    arg(A_IW, &v1);
    put_value(v1.value, true, 0);
}

void o_cbranch(int n) {
    putbyte(0x0f);
    putbyte(n);
    arg(A_IW, &v1);
    put_value(v1.value, true, 1);
}

// j jmp call ljmp lcall
void o_jump(int n) {
    a1 = arg(A_RM | A_IM, &v1);
    if (a1 & A_IB) size = 0;
    if (trylex(',')) {
        a2 = arg(A_IW, &v2);
        if (a1 != A_IW) error("bad far jump arguments");
        putbyte(n <= 3 ? 0x9a : 0xea);
        return put_value(v2.value, false, 1), put_value(v1.value, false, 1);
    }
    if (n > 3 && size == 0) {
        if (!(a1 & A_IM)) error("bad short jump argument");
        return putbyte(0xeb), put_value(v1.value, true, 0);
    }
    if (a1 & A_RM)
        return putbyte(0xff), put_modrm(&v1, n, false);
    if (n & 1) error("bad far jump");
    putbyte(n <= 3 ? 0xe8 : 0xe9), put_value(v1.value, true, 1);
}

void o_ascii(int n) {
    putbyte(0xd4 + n);
    if (!trylex(';')) {
        arg(A_IW, &v1);
        put_value(v1.value, false, 0);
    }
    else putbyte(10);
}

void o_memory(int n) {
    if (n <= 1) {
        putbyte(0x0f);
        n += 0xb4;
    }
    putbyte(n);
    arg(A_RW, &v1);
    next_arg(A_MM, &v2);
    put_modrm(&v2, v1.reg, false);
}

void o_regmem(int n) {
    putbyte(0x0f);
    putbyte(n);
    arg(A_RW, &v1);
    next_arg(A_RM, &v2);
    put_modrm(&v2, v1.reg, false);
}

void o_enter(int n) {
    putbyte(0xc8);
    arg(A_IW, &v1);
    next_arg(A_IM, &v2);
    put_value(v1.value, false, 1);
    put_value(v2.value, false, 0);
}

void o_setflags(int n) {
    arg(A_MB, &v1);
    putbyte(0x0f);
    putbyte(n);
    put_modrm(&v1, 0, false);
}
