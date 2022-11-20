#include <as1.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static void align(int seg) {
    p_segment(seg);
    p_even(0);
}

int main(int argc, char** argv) {
    init_builtins();
    init_io();
    for (int i = 1; i < argc; i++) {
        p_segment(SEG_TEXT);
        stdin = freopen(argv[i], "rt", stdin);
        if (!stdin) {
            error("unable to open: '%s'", argv[i]);
            continue;
        }
        current_file.line = 1;
        current_file.name = argv[i];
        while (assem());
        align(SEG_TEXT);
        align(SEG_DATA);
        align(SEG_BSS);
    }
    p_segment(SEG_BSS);
    exec.a_bss = dot->value;
    exec.a_magic = A_OMAGIC;
    save_and_quit();
    return 0;
}
