#include "as.h"

#define eprint(fmt, ...) fprintf(stderr, fmt __VA_OPT__(,) __VA_ARGS__)

bool autoglobal;

void debug_command(void) {
    bool esc;
    char c;
    struct value v;
    switch (c = lexch(&esc)) {
        case 'e':
            v = eval();
            if (!v.defined) eprint("<undefined>\n");
            else eprint("%d%s\n", v.constant, v.filerel ? "+.." : "");
            break;
        case 'v': case 'r':
            v = eval();
            put_value(&v, c == 'r', 1);
            break;
        case 'g':
            autoglobal = !autoglobal;
            break;
        default:
            error("$ <unknown>");
            break;
    }
}
