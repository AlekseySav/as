#pragma once

#include "../as.h"

/* sym.c */
struct symbol* lookup(const char* name);
void clearsyms(void);

/* as.c */
void error(short at, const char* msg, ...);
