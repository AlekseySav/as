#include <as.h>
#include <stdio.h>

static struct exec exec;
static struct symbol symtab[SYMTAB_MAX_SIZE];
static struct reloc* current_reloc, reloc_buf;
int reloc_off;

static struct reloc* get_reloc(FILE* f) {
    if (reloc_off >= A_STROFF(exec))
        reloc_buf.addr = 0;
    else {
        fseek(f, reloc_off, SEEK_SET);
        fread(&reloc_buf, sizeof(reloc_buf), 1, f);
        if (reloc_off >= A_DRELOFF(exec))
            reloc_buf.addr += exec.a_text;
        reloc_off += sizeof(struct reloc);
    }
    return &reloc_buf;
}

static int put(FILE* f, word at) {
    if (!current_reloc) current_reloc = get_reloc(f);
    if (current_reloc->addr == at) {
        struct symbol s = symtab[current_reloc->symbol];
        word v = s.value;
        int size = current_reloc->size + 1;
        if (s.segment == SEG_DATA) v += exec.a_text;
        if (s.segment == SEG_BSS) v += exec.a_text + exec.a_data;
        if (current_reloc->pcrel) v -= current_reloc->addr + size;
        fwrite(&v, size, 1, stdout);
        current_reloc = NULL;
        return size;
    }
    fseek(f, A_TEXTOFF(exec) + at, SEEK_SET);
    int buf[1];
    return fwrite(buf, 1, fread(buf, 1, 1, f), stdout);
}

int main(int argc, char** argv) {
    // if (argc != 3) {
    //     fprintf(stderr,
    //         "usage: [rhs] file [>output]\n"
    //         "  r  -- raw binary\n"
    //         "  h  -- keep header, merge segments into one\n"
    //         "  s  -- keep header, keep segment organization\n"
    //     );
    //     return 1;
    // }
    FILE* f = fopen(argv[1], "rb");
    fread(&exec, sizeof(exec), 1, f);
    if (A_BADMAGIC(exec)) {
        fprintf(stderr, "bad magic: %4x\n", exec.a_magic);
        return 1;
    }
    fseek(f, A_SYMOFF(exec), SEEK_SET);
    fread(symtab, 1, exec.a_symtab, f);
    reloc_off = A_TRELOFF(exec);
    for (int i = 0; i < exec.a_text + exec.a_data;)
        i += put(f, i);
    fclose(f);
    return 0;
}
