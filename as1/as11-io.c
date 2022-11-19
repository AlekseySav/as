#include <as1.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int bytes_written;

static FILE* seg_fd[4], * rel_fd[4];

void init_io() {
    for (int i = 0; i < 2; i++) {
        seg_fd[i] = tmpfile();
        rel_fd[i] = tmpfile();
        if (!seg_fd[i] || !rel_fd[i])
            fatal("unable to create temporary files");
    }
}

static void copy(FILE* f) {
    char buf[512];
    int len;
    fseek(f, 0, SEEK_SET);
    while ((len = fread(buf, 1, 512, f)) > 0)
        fwrite(buf, 1, len, stdout);
}

void save_and_quit() {
    exec.a_symtab *= sizeof(struct symbol);
    exec.a_trel *= sizeof(struct reloc);
    exec.a_drel *= sizeof(struct reloc);
    fwrite(&exec, sizeof(exec), 1, stdout);
    copy(seg_fd[0]);
    copy(seg_fd[1]);
    for (struct x_symbol* s = symtab; s < symtab + exec.a_symtab / sizeof(struct symbol); s++) {
        if (!s->defined) error("undefined symbol %s", s->name ? s->name : "<unnamed>");
        struct symbol d = {.name = s->name - strtab, .segment = s->segment, .value = s->value};
        fwrite(&d, sizeof(d), 1, stdout);
    }
    copy(rel_fd[0]);
    copy(rel_fd[1]);
    fwrite(strtab, 1, exec.a_strtab, stdout);
    for (int i = 0; i < 2; i++) {
        fclose(seg_fd[i]);
        fclose(rel_fd[i]);
    }
}

static bool bad_segment() {
    if (!seg_fd[current_segment]) {
        error("trying to store data in non-writable section");
        return true;
    }
    return false;
}

void putbyte(byte v) {
    if (bad_segment()) return;
    putc(v, seg_fd[current_segment]);
    bytes_written++;
    A_SEGSIZE(exec, current_segment)++;
}

void putword(word v) {
    if (bad_segment()) return;
    fwrite(&v, 2, 1, seg_fd[current_segment]);
    bytes_written += 2;
    A_SEGSIZE(exec, current_segment) += 2;
}

void put_reloc(struct reloc r) {
    if (bad_segment()) return;
    fwrite(&r, sizeof(r), 1, rel_fd[current_segment]);
    (current_segment == SEG_TEXT) ? exec.a_trel++ : exec.a_drel++;
}

static char peek_c = 0;

char get() {
    char c = peek_c ? peek_c : getchar();
    peek_c = 0;
    if (c == '\n') current_file.line++;
    return c;
}

void unget(char c) {
    peek_c = c;
    if (c == '\n') current_file.line--;
}

void error(const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "as1 error: %s:%d: ", current_file.name, current_file.line);
    vfprintf(stderr, fmt, ap);
    putc('\n', stderr);
    va_end(ap);
}

noreturn void fatal(const char* message) {
    fprintf(stderr, "\e[1mas1: \e[31mfatal error: \e[0m%s\n", message);
    exit(1);
}
