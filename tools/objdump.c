#include <as.h>
#include <stdio.h>

static struct exec exec;
static char strtab[STRTAB_MAX_SIZE];
static struct symbol symtab[SYMTAB_MAX_SIZE];

static void syminfo(struct symbol s) {
    if (SYM_UNNAMED(s)) return;
    static const char* p[] = { "[.text]", "[.data]", "[.bss]", "" };
    printf("  %-8s", p[s.segment]);
    printf("%04x  ", s.value);
    printf("%s\n", strtab + s.name);
}

static void relinfo(struct reloc r) {
    printf("  %c%c %04x <- ", (r.size ? 'w' : 'b'), (r.pcrel ? 'r' : ' '), r.addr);
    struct symbol e = symtab[r.symbol];
    if (SYM_UNNAMED(e)) printf("0x%04x", e.value);
    else printf("%s", strtab + e.name);
    printf("\n");
}

static void dump_file(const char* name) {
    FILE* f = fopen(name, "rt");
    fread(&exec, sizeof(exec), 1, f);
    printf("file: %s (%s)\nexec:\n",
        name,
        (exec.a_magic == A_OMAGIC ? "A_OMAGIC" : (exec.a_magic == A_XMAGIC ? "A_XMAGIC" : "???")));
    printf("  %04hx  a_text: %6hd\n  %04hx  a_data: %6hd\n  %04hx  a_bss: %7hd\n",
        A_TEXTOFF(exec), exec.a_text, A_DATAOFF(exec), exec.a_data, A_BSSOFF(exec), exec.a_bss);
    printf("  %04hx  a_symtab: %4hd\n", A_SYMOFF(exec), exec.a_symtab);
    printf("  %04hx  a_trel: %6hd\n  %04hx  a_drel: %6hd\n",
        A_TRELOFF(exec), exec.a_trel, A_DRELOFF(exec), exec.a_drel);
    printf("  %04hx  a_strtab: %4hd\n", A_STROFF(exec), exec.a_strtab);
    printf("  symbols: %lu, relocations: %lu+%lu\n",
        exec.a_symtab / sizeof(struct symbol),
        exec.a_trel / sizeof(struct reloc), exec.a_drel / sizeof(struct reloc));

    fseek(f, A_STROFF(exec), SEEK_SET);
    fread(strtab, 1, exec.a_strtab, f);
    fseek(f, A_SYMOFF(exec), SEEK_SET);
    fread(symtab, 1, exec.a_symtab, f);

    printf("symtab:\n");
    for (int i = 0; i < exec.a_symtab / sizeof(struct symbol); i++)
        syminfo(symtab[i]);

    struct reloc r;
    fseek(f, A_TRELOFF(exec), SEEK_SET);
    printf("text relocations:\n");
    for (int i = 0; i < exec.a_trel / sizeof(struct reloc); i++) {
        fread(&r, sizeof(r), 1, f);
        relinfo(r);
    }
    printf("data relocations:\n");
    for (int i = 0; i < exec.a_drel / sizeof(struct reloc); i++) {
        fread(&r, sizeof(r), 1, f);
        relinfo(r);
    }

    fclose(f);
}

int main(int argc, char** argv) {
    for (int i = 1; i < argc; i++)
        dump_file(argv[i]);
    return 0;
}

