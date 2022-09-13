#pragma once

#include "../as.h"

/* sym.c */
struct symbol* lookup(const char* name);
int symid(struct symbol* s);
struct symbol* symbol(int id);
void clearsyms(void);

/* as.c */
void error(short at, const char* msg, ...);
