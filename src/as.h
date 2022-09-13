#pragma once

/*
 * assembler, usage
 * usage:   $ ./as1 <1.s >1.o       # assembler, first pass 
 *          $ ./asx <1.o >1.xo      # (optional) assembler, optimize ; todo
 *          $ ./as2 1.xo ... >1     # assembler, second pass
 */
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdnoreturn.h>

struct exec {
    short text;
    short bss;
    short syms;
    short reloc;
};

struct symbol {
    short defined: 1;
    short filerel: 1;
    short isshort: 1;
    short islocal: 1;
    short mutable: 1;
    short value;
    char name[8];                               /* char name[2]; for short symbol */
};

struct reloc {
    short size: 1;
    short pcrel: 1;
    short optimize: 3;                          /* hardcoded optimizations, 0xeb jump, etc. */
    short ptr;
    short sym;
};

#define sym_len(sym)  ((sym)->isshort ? 2 : 8)
#define sym_size(sym) ((sym)->isshort ? sizeof(struct symbol) - 6 : sizeof(struct symbol))

/* optimizations */
#define OT_MRMDISP      1
