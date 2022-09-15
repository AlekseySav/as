#pragma once

#include "../as.h"

struct ex_reloc {
    struct reloc r;
    struct symbol* s;
};

/* sym.c */
struct symbol* lookup(int g, const char* name);
void clearsyms(void);

/* as.c */
void error(short at, const char* msg, ...);
