#pragma once

typedef unsigned char byte;
typedef unsigned short word;

struct exec {
    word a_magic;
    word a_text, a_data, a_bss;
    word a_symtab;
    word a_trel, a_drel;
    word a_strtab;
};

#define A_OMAGIC        0xfeeb // object file, as1 output
#define A_XMAGIC        0x0eeb // executable file, as2 output
#define A_BADMAGIC(x)   ((x).a_magic != A_OMAGIC && (x).a_magic != A_XMAGIC)

#define A_SEGSIZE(x, n) ((&(x).a_text)[n])

#define A_TEXTOFF(x)    ((word)(sizeof(struct exec)))
#define A_DATAOFF(x)    (A_TEXTOFF(x) + (x).a_text)
#define A_BSSOFF(x)     (A_DATAOFF(x) + (x).a_data)
#define A_SYMOFF(x)     (A_BSSOFF(x)) // bss not stored in file
#define A_TRELOFF(x)    (A_SYMOFF(x)  + (x).a_symtab)
#define A_DRELOFF(x)    (A_TRELOFF(x) + (x).a_trel)
#define A_STROFF(x)     (A_DRELOFF(x) + (x).a_drel)
#define A_EOFOFF(x)     (A_STROFF(x)  + (x).a_strtab)

struct symbol {
    word name: 14;
    word segment: 2;
    word value;
};

#define SEG_TEXT        0
#define SEG_DATA        1
#define SEG_BSS         2
#define SEG_CONST       3
#define SYM_NONAME      ((1 << 14) - 1)
#define SYM_UNNAMED(s)  ((s).name == SYM_NONAME)
#define SYM_ISCONST(s)  ((s).segment == SEG_CONST)

struct reloc {
    word symbol: 14;
    word pcrel: 1;
    word size: 1;
    word addr;
};
