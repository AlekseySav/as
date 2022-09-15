#pragma once

#include "../as.h"

struct value {
    short constant;
    short symbol;
    bool defined: 1;
    bool filerel: 1;
    bool hasname: 1;
};

struct opcode {
    void (*group)(int n);
    int n;
};

/* sym.c */
const char* counter(void);                      /* generate unique name */
struct symbol* lookup(const char* name);
struct symbol* shortsym(int n, char bf);        /* 1f 1b 1, etc. */
int flush_syms(void);
int symid(struct symbol* s);
struct symbol* symbol(int id);

/* lex.c */
extern int line;
extern char lbuf[];
int lex(void);
char lexch(bool* escaped);
void unlex(void);
bool trylex(int to);

/* value.c */
void put_value(struct value* v, bool pcrel, int size);
void value(struct value* v);
int flush_relocs(void);
int value_size(struct value* v, bool sign, bool unsign);
void optimize(int id);

/* args.c */
extern int rr, sr, modrm, nargs;
bool readarg(void);
bool cmpargs(int n, ...);
struct value* arg(int n);
void put_rm(int rr);                            /* rr = -1 => put displacement only */

/* as.c */
extern int size, textsize;
void put_const(int v, int size);
void error(const char* msg);
void parse(void);
void endline(void);

/* debug.c */
extern bool autoglobal;
void debug_command(void);

/* preprocess.c */
void preprocess(const char* name);

/* eval.yaml */
struct value eval(void);

/* opcodes.yaml */
struct opcode* opcode(const char* name, int* size);

/* printing functions */
#define putb(n)     put_const(n, 0)
#define putw(n)     put_const(n, 1)
#define putf(n)     put_const(0x0f | (n) << 8, 1)
#define put(n)      put_const(n, size)
#define putb_im(n)  put_value(arg(n), false, 0)
#define putw_im(n)  put_value(arg(n), false, 1)
#define put_im(n)   put_value(arg(n), false, size)
#define putb_dd(n)  put_value(arg(n), true, 0)
#define putw_dd(n)  put_value(arg(n), true, 1)
#define put_dd(n)   put_value(arg(n), true, size)

/* arg flags */
#define ARG_REG1(n) (1 << (n & 7))
#define ARG_REG2(n) (ARG_REG1(n) << 8)
#define ARG_RR1     ((1 << 8) - 1)
#define ARG_RR2     (ARG_RR1 << 8)
#define ARG_REG     (ARG_RR1 | ARG_RR2)

#define ARG_IMM     0x00010000
#define ARG_MEM     0x00020000
#define ARG_SEG     0x00040000
#define ARG_REL     0x00080000

#define ARG_DEFAULT 0x04000000
#define ARG_BYTE    0x08000000

#define ARG_DEFSIZ  0x80000000
#define ARG_UNSIGN  0x40000000
#define ARG_SIGN    0x20000000
#define ARG_VAARG   0x10000000
