#include <as1.h>
#include <stdio.h>

#define IDENT_MAX 100

union lval lval;

static int token_queue[3], queue_len = 0;

#define I 16 // ident
#define S 17 // space
#define B 18 // bad
#define L 19 // line break (new line)
#define O 20 // operator ( ) [ ] + - * / , | & ! = : ?
#define Q 21 // quote
#define E 22 // (eof)

static char ctype[129] = {
    E,
    B,  B,  B,  B,  B,  B,  B,  B,  B,  S,  L,  B,  B,  B,  B,  B,
    B,  B,  B,  B,  B,  B,  B,  B,  B,  B,  B,  B,  B,  B,  B,  B,
    S,  O,  B,  B,  B,  O,  O,  Q,  O,  O,  O,  O,  O,  O,  I,  O,
    0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  O,  L,  O,  O,  O,  O,
    B, 10, 11, 12, 13, 14, 15,  I,  I,  I,  I,  I,  I,  I,  I,  I,
    I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  O,  B,  O,  B,  I,
    B, 10, 11, 12, 13, 14, 15,  I,  I,  I,  I,  I,  I,  I,  I,  I,
    I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  I,  B,  O,  B,  I,  B
};

#define ctype(c) (ctype[(c) + 1])

static bool comment(char c) {
    if (c != '/') return false;
    c = get();
    if (c == '/') {
        while ((c = get()) != '\n' && c != EOF);
        unget(c);
        return true;
    }
    if (c != '*') {
        unget(c);
        return false;
    }
    int save_line = current_file.line;
com:
    c = get();
com1:
    if (c == EOF) {
        error("non-terminated comment (started at line %d)", save_line);
        unget(c);
        return true;
    }
    if (c != '*') goto com;
    if ((c = get()) != '/') goto com1;
    return true;
}

static void ident(char c) {
    char buf[IDENT_MAX];
    char* p = buf;
    p = buf;
    do {
        *p++ = c;
        if (p >= buf + IDENT_MAX)
            fatal("exceeded max symbol length");
    } while (ctype(c = get()) <= I);
    *p = '\0';
    unget(c);
    lval.sym = lookup(buf);
}

static int number() {
    char c;
    word base = 10, t;
    if (!lval.num) { /* base of 8 ? */
        base = 8;
        c = get();
        if (c == 'x' || c == 'X') /* base of 16 ? */
            base = 16;
        else unget(c);
    }
    while ((t = ctype(c = get())) < base)
        lval.num = lval.num * base + t;
    
    if (c == 'b' || c == 'f') {
        lval.sym = get_fb(lval.num, c);
        return L_SYM;
    }
    unget(c);
    return L_NUM;
}

int lex() {
    int t;
    char c;
    if (queue_len)
        return token_queue[--queue_len];
lex:
    c = get();
    switch (t = ctype(c)) {
        case E: return L_EOF;
        case B: error("bad character: '%c'", c);
        case S: goto lex;
        case L: return ';';
        case O: return comment(c) ? lex() : c;
        case Q: lval.num = strchar(NULL); return L_NUM;
        case 10: case 11: case 12: case 13: case 14: case 15: case I:
            ident(c);
            return L_SYM;
        default: /* number */
            lval.num = t;
            return number();
    }
}

void unlex(int token) {
    if (queue_len >= 2)
        fatal("token queue exceeded");
    token_queue[queue_len++] = token;
}

bool trylex(int to) {
    if ((token_queue[queue_len] = lex()) == to)
        return true;
    queue_len++;
    return false;
}

char strchar(bool* escaped) {
    char c = get();
    if (escaped) *escaped = c == '\\';
    if (c != '\\') return c;
    switch (get()) {
        case 'n': return '\n';
        case 'r': return '\r';
        case 'b': return '\b';
        case 'e': return '\e';
        case 's': return ' ';
        case 't': return '\t';
        case '0': return '\0';
        case '>': return '>';
        case '\\': return '\\';
        case '^':
            c = get();
            if (c < '@' || c > '_')
                error("bad control character");
            return c - '@';
        default:
            error("bad escape character");
            unget(c);
            return '\\';
    }
}
