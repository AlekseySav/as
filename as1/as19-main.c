#include <as1.h>
#include <stdio.h>

int main(int argc, char** argv) {
    init_builtins();
    init_io();
    for (int i = 1; i < argc; i++) {
        stdin = freopen(argv[i], "rt", stdin);
        current_file.line = 1;
        current_file.name = argv[i];
        while (assem());
    }
    p_segment(SEG_BSS);
    exec.a_bss = dot->value;
    exec.a_magic = A_OMAGIC;
    save_and_quit();
    return 0;
}
