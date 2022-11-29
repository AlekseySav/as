#include <as1.h>

bool iffalse;
int current_segment;
struct exec exec;
struct file current_file;
struct x_symbol* dot, * ddot;

struct x_symbol symbol_pool[SYMTAB_MAX_SIZE] = {
    { ".", 0, 0, 1, 1 },
    { "..", 0, 0, 1, 1 },

    { ".error",     P_ERROR             },
    { ".if",        P_IF,               },
    { ".else",      P_ELSE,     0       },
    { ".endif",     P_ENDIF,    1       },
    { ".text",      P_SEGMENT,  0       },
    { ".data",      P_SEGMENT,  1       },
    { ".bss",       P_SEGMENT,  2       },
    { ".struct",    P_SEGMENT,  3       },
    { ".even",      P_EVEN,             },
    { ".mut",       P_MUT,              },
    { ".byte",      P_BYTE,             },
    { ".mm",        P_MM,               },
    { ".fill",      P_FILL,             },
    { ".func",      P_FUNC,             },

    { "al",         I_REGB,     0       },
    { "cl",         I_REGB,     1       },
    { "dl",         I_REGB,     2       },
    { "bl",         I_REGB,     3       },
    { "ah",         I_REGB,     4       },
    { "ch",         I_REGB,     5       },
    { "dh",         I_REGB,     6       },
    { "bh",         I_REGB,     7       },
    { "ax",         I_REGW,     0       },
    { "cx",         I_REGW,     1       },
    { "dx",         I_REGW,     2       },
    { "bx",         I_REGW,     3       },
    { "sp",         I_REGW,     4       },
    { "bp",         I_REGW,     5       },
    { "si",         I_REGW,     6       },
    { "di",         I_REGW,     7       },
    { "bx_si",      I_REGSUM,   0       },
    { "bx_di",      I_REGSUM,   1       },
    { "bp_si",      I_REGSUM,   2       },
    { "bp_di",      I_REGSUM,   3       },
    { "es",         I_SREG,     0       },
    { "cs",         I_SREG,     1       },
    { "ss",         I_SREG,     2       },
    { "ds",         I_SREG,     3       },
    { "fs",         I_SREG,     4       },
    { "gs",         I_SREG,     5       },

    { "daa",        O_ONEBYTE,  0x27    },
    { "das",        O_ONEBYTE,  0x2f    },
    { "aaa",        O_ONEBYTE,  0x37    },
    { "aas",        O_ONEBYTE,  0x3f    },
    { "pusha",      O_ONEBYTE,  0x60    },
    { "popa",       O_ONEBYTE,  0x61    },
    { "nop",        O_ONEBYTE,  0x90    },
    { "cbw",        O_ONEBYTE,  0x98    },
    { "cwd",        O_ONEBYTE,  0x99    },
    { "pushf",      O_ONEBYTE,  0x9c    },
    { "popf",       O_ONEBYTE,  0x9d    },
    { "sahf",       O_ONEBYTE,  0x9e    },
    { "lahf",       O_ONEBYTE,  0x9f    },
    { "leave",      O_ONEBYTE,  0xc9    },
    { "int3",       O_ONEBYTE,  0xcc    },
    { "into",       O_ONEBYTE,  0xce    },
    { "iret",       O_ONEBYTE,  0xcf    },
    { "xlat",       O_ONEBYTE,  0xd7    },
    { "lock",       O_ONEBYTE,  0xf0    },
    { "repne",      O_ONEBYTE,  0xf2    },
    { "repe",       O_ONEBYTE,  0xf3    },
    { "rep",        O_ONEBYTE,  0xf3    },
    { "hlt",        O_ONEBYTE,  0xf4    },
    { "cmc",        O_ONEBYTE,  0xf5    },
    { "clc",        O_ONEBYTE,  0xf8    },
    { "stc",        O_ONEBYTE,  0xf9    },
    { "cli",        O_ONEBYTE,  0xfa    },
    { "sti",        O_ONEBYTE,  0xfb    },
    { "cld",        O_ONEBYTE,  0xfc    },
    { "std",        O_ONEBYTE,  0xfd    },

    { "insw",       O_STRING,   0x6d    },
    { "insb",       X_CHSIZE            },
    { "outsw",      O_STRING,   0x6f    },
    { "outsb",      X_CHSIZE            },
    { "movsw",      O_STRING,   0xa5    },
    { "movsb",      X_CHSIZE            },
    { "cmpsw",      O_STRING,   0xa7    },
    { "cmpsb",      X_CHSIZE            },
    { "stosw",      O_STRING,   0xab    },
    { "stosb",      X_CHSIZE            },
    { "lodsw",      O_STRING,   0xad    },
    { "lodsb",      X_CHSIZE            },
    { "scasw",      O_STRING,   0xaf    },
    { "scasb",      X_CHSIZE            },

    { "int",        O_ARGBYTE,  0xcd    },

    { "seg",        O_SEGMENT           },
    { "sys",        O_SYS,              },

    { "add",        O_MATH0,    0       },
    { "addb",       X_CHSIZE            },
    { "or",         O_MATH0,    1       },
    { "orb",        X_CHSIZE            },
    { "adc",        O_MATH0,    2       },
    { "adcb",       X_CHSIZE            },
    { "sbb",        O_MATH0,    3       },
    { "sbbb",       X_CHSIZE            },
    { "and",        O_MATH0,    4       },
    { "andb",       X_CHSIZE            },
    { "sub",        O_MATH0,    5       },
    { "subb",       X_CHSIZE            },
    { "xor",        O_MATH0,    6       },
    { "xorb",       X_CHSIZE            },
    { "cmp",        O_MATH0,    7       },
    { "cmpb",       X_CHSIZE            },

    { "rol",        O_MATH1,    0       },
    { "rolb",       X_CHSIZE            },
    { "ror",        O_MATH1,    1       },
    { "rorb",       X_CHSIZE            },
    { "rcl",        O_MATH1,    2       },
    { "rclb",       X_CHSIZE            },
    { "rcr",        O_MATH1,    3       },
    { "rcrb",       X_CHSIZE            },
    { "sal",        O_MATH1,    4       },
    { "salb",       X_CHSIZE            },
    { "shl",        O_MATH1,    4       },
    { "shlb",       X_CHSIZE            },
    { "shr",        O_MATH1,    5       },
    { "shrb",       X_CHSIZE            },
    { "sar",        O_MATH1,    7       },
    { "sarb",       X_CHSIZE            },

    { "not",        O_MATH2,    2       },
    { "notb",       X_CHSIZE            },
    { "neg",        O_MATH2,    3       },
    { "negb",       X_CHSIZE            },
    { "mul",        O_MATH2,    4       },
    { "mulb",       X_CHSIZE            },
    { "imul",       O_MATH2,    5       },
    { "imulb",      X_CHSIZE            },
    { "div",        O_MATH2,    6       },
    { "divb",       X_CHSIZE            },
    { "idiv",       O_MATH2,    7       },
    { "idivb",      X_CHSIZE            },

    { "test",       O_TEST              },
    { "testb",      X_CHSIZE            },

    { "inc",        O_INCDEC,   0       },
    { "incb",       X_CHSIZE            },
    { "dec",        O_INCDEC,   1       },
    { "decb",       X_CHSIZE            },

    { "push",       O_STACK,    0       },
    { "pushb",      X_CHSIZE            },
    { "pop",        O_STACK,    1       },

    { "ret",        O_RET,      0xc2    },
    { "retf",       O_RET,      0xca    },
    
    { "in",         O_INOUT,    0       },
    { "inb",        X_CHSIZE,           },
    { "out",        O_INOUT,    2       },
    { "outb",       X_CHSIZE,           },

    { "mov",        O_MOVE              },
    { "movb",       X_CHSIZE            },

    { "xchg",       O_XCHG              },
    { "xchgb",      X_CHSIZE            },

    { "jo",         O_CJUMP,    0x70    },
    { "jno",        O_CJUMP,    0x71    },
    { "jb",         O_CJUMP,    0x72    },
    { "jnae",       O_CJUMP,    0x72    },
    { "jc",         O_CJUMP,    0x72    },
    { "jae",        O_CJUMP,    0x73    },
    { "jnb",        O_CJUMP,    0x73    },
    { "jnc",        O_CJUMP,    0x73    },
    { "je",         O_CJUMP,    0x74    },
    { "jz",         O_CJUMP,    0x74    },
    { "jne",        O_CJUMP,    0x75    },
    { "jnz",        O_CJUMP,    0x75    },
    { "jbe",        O_CJUMP,    0x76    },
    { "jna",        O_CJUMP,    0x76    },
    { "ja",         O_CJUMP,    0x77    },
    { "jnbe",       O_CJUMP,    0x77    },
    { "js",         O_CJUMP,    0x78    },
    { "jns",        O_CJUMP,    0x79    },
    { "jp",         O_CJUMP,    0x7a    },
    { "jpe",        O_CJUMP,    0x7a    },
    { "jnp",        O_CJUMP,    0x7b    },
    { "jpo",        O_CJUMP,    0x7b    },
    { "jl",         O_CJUMP,    0x7c    },
    { "jge",        O_CJUMP,    0x7d    },
    { "jnl",        O_CJUMP,    0x7d    },
    { "jle",        O_CJUMP,    0x7e    },
    { "jng",        O_CJUMP,    0x7e    },
    { "jg",         O_CJUMP,    0x7f    },
    { "jnle",       O_CJUMP,    0x7f    },
    { "loopne",     O_CJUMP,    0xe0    },
    { "loope",      O_CJUMP,    0xe1    },
    { "loop",       O_CJUMP,    0xe2    },
    { "jcxz",       O_CJUMP,    0xe3    },

    { "blo",        O_CBRANCH,  0x82    },
    { "bhis",       O_CBRANCH,  0x83    },
    { "beq",        O_CBRANCH,  0x84    },
    { "bne",        O_CBRANCH,  0x85    },
    { "blos",       O_CBRANCH,  0x86    },
    { "bho",        O_CBRANCH,  0x87    },
    { "blt",        O_CBRANCH,  0x8c    },
    { "bge",        O_CBRANCH,  0x8d    },
    { "ble",        O_CBRANCH,  0x8e    },
    { "bgt",        O_CBRANCH,  0x8f    },

    { "call",       O_JUMP,     2       },
    { "lcall",      O_JUMP,     3       },
    { "jmp",        O_JUMP,     4       },
    { "j",          X_CHSIZE            },
    { "ljmp",       O_JUMP,     5       },

    { "aam",        O_ASCII,    0       },
    { "aad",        O_ASCII,    1       },

    { "bound",      O_MEMORY,   0x62    },
    { "lea",        O_MEMORY,   0x8d    },
    { "les",        O_MEMORY,   0xc4    },
    { "lds",        O_MEMORY,   0xc5    },
    { "lfs",        O_MEMORY,   0       },
    { "lgs",        O_MEMORY,   1       },

    { "bsf",        O_REGMEM,   0xbc    },
    { "bsr",        O_REGMEM,   0xbd    },

    { "enter",      O_ENTER             },

    { "seto",       O_SETFLAGS, 0x90    },
    { "setno",      O_SETFLAGS, 0x91    },
    { "setb",       O_SETFLAGS, 0x92    },
    { "setc",       O_SETFLAGS, 0x92    },
    { "setnae",     O_SETFLAGS, 0x92    },
    { "setae",      O_SETFLAGS, 0x93    },
    { "setnb",      O_SETFLAGS, 0x93    },
    { "setnc",      O_SETFLAGS, 0x93    },
    { "sete",       O_SETFLAGS, 0x94    },
    { "setz",       O_SETFLAGS, 0x94    },
    { "setne",      O_SETFLAGS, 0x95    },
    { "setnz",      O_SETFLAGS, 0x95    },
    { "setbe",      O_SETFLAGS, 0x96    },
    { "setna",      O_SETFLAGS, 0x96    },
    { "sets",       O_SETFLAGS, 0x98    },
    { "setns",      O_SETFLAGS, 0x99    },
    { "setp",       O_SETFLAGS, 0x9a    },
    { "setpe",      O_SETFLAGS, 0x9a    },
    { "setnp",      O_SETFLAGS, 0x9b    },
    { "setpo",      O_SETFLAGS, 0x9b    },
    { "setl",       O_SETFLAGS, 0x9c    },
    { "setnge",     O_SETFLAGS, 0x9c    },
    { "setge",      O_SETFLAGS, 0x9d    },
    { "setnl",      O_SETFLAGS, 0x9d    },
    { "setle",      O_SETFLAGS, 0x9e    },
    { "setng",      O_SETFLAGS, 0x9e    },
    { "setg",       O_SETFLAGS, 0x9f    },
    { "setnle",     O_SETFLAGS, 0x9f    },

    { NULL }
};

void (*opcodes[I_N_OPCODES])(int n) = {
    bad_opcode,
    p_error, p_if, p_else, p_endif, p_segment, p_even, p_mut, p_byte, p_mm, p_fill, p_func,
    bad_opcode, bad_opcode, bad_opcode, bad_opcode,
    x_chsize,
    o_onebyte, o_string, o_argbyte, o_segment, o_sys,
    o_math0, o_math1, o_math2, o_test, o_incdec,
    o_stack, o_ret, o_inout, o_move, o_xchg,
    o_cjump, o_cbranch, o_jump,
    o_ascii, o_memory, o_regmem,
    o_enter, o_setflags
};
