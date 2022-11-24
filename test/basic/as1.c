#include <as1.h>
#include <stdio.h>
#include <string.h>

const char* _scope;

char buf_stderr[4096 * 2];
short buf_relocs[4096];
short buf_seg_fd[3][4096];
short buf_reloc_fd[2][4096];

#define test_scope(name) ((_scope = name), fprintf(stderr, "%s...", _scope))
#define end_scope() fprintf(stderr, " ok\n");
#define test(expr) (expr) ? 0 : fprintf(stderr, "%s (line %d) failed\n", _scope, __LINE__);
#define new_stdin(c) { char buf[] = c; stdin = fmemopen(buf, strlen(buf), "rt"); }
#define new_write(f) { memset(buf_##f, 0, 4096 * 2); f = fmemopen(buf_##f, 4096 * 2, "wb"); }
#define file_eq(a, ...) ({ short b[] = { __VA_ARGS__ }; _file_eq((char*)a, (char*)b, sizeof(b)); })
#define file_eqb(a, ...) ({ char b[] = { __VA_ARGS__ }; _file_eq((char*)a, (char*)b, sizeof(b)); })

bool _file_eq(char* a, char* b, int len) {
    while (len--)
        if (a[len] != b[len]) {
            fprintf(stderr, "missed: %hhx->%hhx\n", a[len], b[len]);
            return false;
        }
    return true;
}

int main(int argc, char** argv) {
    init_builtins();
    FILE* _stdin = stdin, * _stderr = stderr;

    test_scope("symbol table");
    struct x_symbol* x = lookup("hello");
    test(!strcmp(x->name, "hello"));
    x->value = 7;
    test(!strcmp(lookup("hello1")->name, "hello1"));
    test(!strcmp(lookup("hello2")->name, "hello2"));
    test(!strcmp(lookup("hello3")->name, "hello3"));
    test(!strcmp(lookup("hello4")->name, "hello4"));
    test(!strcmp(lookup("hello5")->name, "hello5"));
    test(!strcmp(lookup("hello6")->name, "hello6"));
    test(!strcmp(lookup("hello7")->name, "hello7"));
    test(!strcmp(lookup("hello8")->name, "hello8"));
    test(!strcmp(lookup("hello9")->name, "hello9"));
    test(!strcmp(lookup("hello0")->name, "hello0"));
    test(!strcmp(lookup("helloa")->name, "helloa"));
    test(!strcmp(lookup("hellob")->name, "hellob"));
    test(!strcmp(lookup("helloc")->name, "helloc"));
    test(!strcmp(lookup("hellod")->name, "hellod"));
    test(lookup(".") == dot);
    test(lookup("..") == ddot);
    test(!strcmp(lookup("hello")->name, "hello"));
    test(lookup("hello") == x);
    test(lookup("hello")->value == 7);
    test(XSYM_UNNAMED(lookup(NULL)));
    end_scope();

    test_scope("lexer");
    current_file.line = 1;
    new_stdin(
"()[]+-*/,|&!=:?"
"/*any comment*/\n\n"
"x/*multiline comment\n*/\n"
"//single line comment\n\n"
"123 12 012 0x123 0xabc 's '\\n '\\s '\\t '\\r '\\0 "
"lalala, lala, la"
"//comment in the end:)"
);
    test(lex() == '(');
    test(lex() == ')');
    test(lex() == '[');
    test(lex() == ']');
    test(lex() == '+');
    test(lex() == '-');
    test(lex() == '*');
    test(lex() == '/');
    test(lex() == ',');
    test(lex() == '|');
    test(lex() == '&');
    test(lex() == '!');
    test(lex() == '=');
    test(lex() == ':');
    test(lex() == '?');
    test(lex() == ';' && current_file.line == 2);
    test(lex() == ';' && current_file.line == 3);
    test(lex() == L_SYM && lval.sym == lookup("x"));
    test(lex() == ';' && current_file.line == 5);
    test(lex() == ';' && current_file.line == 6);
    test(lex() == ';' && current_file.line == 7);
    test(lex() == L_NUM && lval.num == 123);
    test(lex() == L_NUM && lval.num == 12);
    test(lex() == L_NUM && lval.num == 012);
    test(lex() == L_NUM && lval.num == 0x123);
    test(lex() == L_NUM && lval.num == 0xabc);
    test(lex() == L_NUM && lval.num == 's');
    test(lex() == L_NUM && lval.num == '\n');
    test(lex() == L_NUM && lval.num == ' ');
    test(lex() == L_NUM && lval.num == '\t');
    test(lex() == L_NUM && lval.num == '\r');
    test(lex() == L_NUM && lval.num == '\0');
    test(lex() == L_SYM && lval.sym == lookup("lalala"));
    test(lex() == ',');
    test(lex() == L_SYM && lval.sym == lookup("lala"));
    test(lex() == ',');
    test(lex() == L_SYM && lval.sym == lookup("la"));
    test(lex() == L_EOF);
    fclose(stdin);
    end_scope();

    test_scope("expr parser");
    new_stdin(
"[1] "
"[1+2+3+4+5] "
"[1*2+3-4+4/5] "
"1*2+3-4+4/5 "
"s+2 "
"2+s "
"y-s*2 "
"qqq "
"."
);
    struct value v;
    test((v = expr()).value == 1 && v.defined && v.segment == SEG_CONST);
    test((v = expr()).value == 15 && v.defined && v.segment == SEG_CONST);
    test((v = expr()).value == 1 && v.defined && v.segment == SEG_CONST);
    lookup("s")->defined = 1; lookup("s")->segment = SEG_TEXT;
    lookup("y")->defined = 1; lookup("y")->segment = SEG_TEXT;
    lookup("s")->value = 17;
    lookup("y")->value = 18;
    test((v = expr()).value == 1 && v.defined && v.segment == SEG_CONST);
    test((v = expr()).value == 19 && v.defined && v.segment == SEG_TEXT);
    test((v = expr()).value == 19 && v.defined && v.segment == SEG_TEXT);
    test((v = expr()).value == 2 && v.defined && v.segment == SEG_CONST);
    test(!(v = expr()).defined);
    test((v = expr()).defined && v.segment == SEG_TEXT);
    test(lex() == L_EOF);
    fclose(stdin);
    end_scope();

    test_scope("arg");
    new_stdin(
"al cl dl bl ah ch dh bh "
"ax cx dx bx sp bp si di "
"*11 12 ;"
"(123) "
"1(bp_si), *2(di) "
", -1(si), *(-5)"
);
    struct arg_value av;
    test(arg(~0, &av) == A_RB && av.reg == 0);
    test(arg(~0, &av) == A_RB && av.reg == 1);
    test(arg(~0, &av) == A_RB && av.reg == 2);
    test(arg(~0, &av) == A_RB && av.reg == 3);
    test(arg(~0, &av) == A_RB && av.reg == 4);
    test(arg(~0, &av) == A_RB && av.reg == 5);
    test(arg(~0, &av) == A_RB && av.reg == 6);
    test(arg(~0, &av) == A_RB && av.reg == 7);
    test(arg(~0, &av) == A_RW && av.reg == 0);
    test(arg(~0, &av) == A_RW && av.reg == 1);
    test(arg(~0, &av) == A_RW && av.reg == 2);
    test(arg(~0, &av) == A_RW && av.reg == 3);
    test(arg(~0, &av) == A_RW && av.reg == 4);
    test(arg(~0, &av) == A_RW && av.reg == 5);
    test(arg(~0, &av) == A_RW && av.reg == 6);
    test(arg(~0, &av) == A_RW && av.reg == 7);
    test(arg(~0, &av) == A_IB && av.value.value == 11 && av.value.defined);
    test(arg(~0, &av) == A_IW && av.value.value == 12 && av.value.defined);
    test(lex() == ';');
    test(arg(~0, &av) == A_MM && av.value.value == 123 && av.value.defined && av.modrm == 0006);
    test(arg(~0, &av) == A_MM && av.value.value == 1 && av.value.defined && av.modrm == 0202);
    test(next_arg(~0, &av) == A_MM && av.value.value == 2 && av.value.defined && av.modrm == 0105);
    test(next_arg(~0, &av) == A_MM && av.value.value == (word)-1 && av.value.defined && av.modrm == 0204);
    test(next_arg(~0, &av) == A_MM && av.value.value == (word)-5 && av.value.defined && av.modrm == 0006);
    fclose(stdin);
    end_scope();

    return 0;
}
