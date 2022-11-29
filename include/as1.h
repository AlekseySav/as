#pragma once
#include "as.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdnoreturn.h>

struct file {
    const char* name;
    int line;
};

union lval {
    word num;
    struct x_symbol* sym;
};

enum token {
    L_EOF,
    L_SYM,
    L_NUM
};

struct value {
    struct x_symbol* sym;
    word value;
    word segment: 2;
    word defined: 1;
};

enum arg {
    A_RB = 1,
    A_RW = 2,
    A_SR = 4,
    A_MM = 8,
    A_IB = 16,
    A_IW = 32,
    A_RR = A_RB | A_RW,
    A_IM = A_IB | A_IW,
    A_MB = A_RB | A_MM,
    A_MW = A_RW | A_MM,
    A_RM = A_RR | A_MM,
};

struct arg_value {
    struct value value;
    int modrm;
    int reg;
};

/*
 * as10.c -- common data, used in many files
 */
extern bool iffalse; // in .if [false] section
extern int current_segment;
extern struct exec exec;
extern struct file current_file;
extern struct x_symbol* dot, * ddot; // . and .. symbols
extern struct x_symbol symbol_pool[SYMTAB_MAX_SIZE];
extern void (*opcodes[I_N_OPCODES])(int n);

/*
 * as11-io.c
 */
void init_io();
void save_and_quit();
// output functions
extern int bytes_written;
void putbyte(byte v);
void putword(word v);
void put_reloc(struct reloc r);
// input functions
char get();
void unget(char c);
// error handlers
void error(const char* fmt, ...);
noreturn void fatal(const char* message);

/*
 * as12-sym.c
 */
extern struct x_symbol* symtab;
extern char strtab[STRTAB_MAX_SIZE];
void init_builtins();
struct x_symbol* lookup(const char* name);
struct x_symbol* get_fb(word n, char c);
struct x_symbol* define_fb(int n);

/*
 * as13-lex.c
 */
extern union lval lval;
int lex();
void unlex(int token);
bool trylex(int to);
char strchar(bool* escaped);

/*
 * as14-expr.c
 */
struct value expr(void);
word cexpr(void);
void put_value(struct value v, bool pc_rel, int size_log);

/*
 * as15-assem.c
 */
extern bool size; // =1 if word, =0 if byte
void bad_opcode(int n);
void x_chsize(int n);
void assign(struct x_symbol* sym, struct value value);
bool assem(void);

/*
 * as17-arg.c
 */
#define modrm_disp_len(modrm) (((modrm) == 6 || ((modrm) >> 6 == 2)) ? 2 : (((modrm) >> 6 == 1) ? 1 : 0))
enum arg arg(enum arg mask, struct arg_value* v);
enum arg next_arg(enum arg mask, struct arg_value* v);
void put_modrm(struct arg_value* mod, char rr, bool disponly);

/*
 * as16-pseudo.c
 */
void p_error(int n);
void p_if(int n);
void p_else(int n);
void p_endif(int n);
void p_segment(int n);
void p_even(int n);
void p_mut(int n);
void p_byte(int n);
void p_mm(int n);
void p_fill(int n);
void p_func(int n);

/*
 * as18-asm.c
 */
void segment_override(int seg);

void o_onebyte(int n);
void o_string(int n);
void o_argbyte(int n);
void o_segment(int n);
void o_sys(int n);
void o_math0(int n);
void o_math1(int n);
void o_math2(int n);
void o_test(int n);
void o_incdec(int n);
void o_stack(int n);
void o_ret(int n);
void o_inout(int n);
void o_move(int n);
void o_xchg(int n);
void o_cjump(int n);
void o_cbranch(int n);
void o_jump(int n);
void o_ascii(int n);
void o_memory(int n);
void o_regmem(int n);
void o_enter(int n);
void o_setflags(int n);

/*
 * as19-main.c
 */
int main(int argc, char** argv);
