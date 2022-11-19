#pragma once

#include "a.out.h"

#define SYMTAB_MAX_SIZE 3181    // including builtins, 3181 is prime
#define STRTAB_MAX_SIZE 10000   // cannot be >16384 -- only 14 bits for name offset

enum builtins {
    P_ERROR = 1, P_IF, P_ENDIF, P_SEGMENT, P_EVEN, P_MUT, P_BYTE, P_FILL,
    I_REGB, I_REGW, I_REGSUM, I_SREG,
    X_CHSIZE,
    O_ONEBYTE, O_STRING, O_ARGBYTE, O_SEGMENT, O_SYS,
    O_MATH0, O_MATH1, O_MATH2, O_TEST, O_INCDEC,
    O_STACK, O_CJUMP, O_CBRANCH, O_JUMP, O_RET,
    O_PORT, O_MOVE, O_XCHG,
    I_N_OPCODES
};

struct x_symbol {
    const char* name;
    enum builtins builtin_id;
    word value;
    word defined: 1;
    word mutable: 1;
    word segment: 2;
};

#define XSYM_UNNAMED(s) ((s)->name == NULL)
#define XSYM_BUILTIN(s) ((s)->builtin_id != 0)
