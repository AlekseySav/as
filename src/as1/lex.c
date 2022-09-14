#include "as.h"

int line = 1;
char lbuf[20];

static int c, t, t2, qlen;

static short ident[8] = {
    0000000, 0000000, /* special */
    0040000, 0001777, /* . 0-9 */
    0177776, 0103777, /* A-Z _ */
    0177776, 0003777  /* a-z */
};

static short space[8] = {
    0001000, 0000000, /* '\t' */
    0000001, 0000000, /* ' ' */
    0000000, 0000000,
    0000000, 0000000
};

static bool is(short* map, int c) {
    if (c == EOF) return false;
    return map[c >> 4] >> (c & 15) & 1;
}

static char ch(void) {
    if (!c) {
        c = getchar();
        if (c == '\n') line++;
    }
    return c;
}

static char pop(void) {
    char q;
    return q = ch(), c = 0, q;
}

static inline bool comment(void) {
    if ((c = getchar()) != '*') return false;
    c = 0;
    for (;;) {
        while (getchar() != '*');
        if (getchar() == '/')
            return true;
    }
}

static int next(void) {
    char* s = lbuf, x;
    while (is(space, ch())) pop();
    while (is(ident, ch())) *s++ = pop();
    if (s != lbuf) return *s++ = 0;
    if (ch() == EOF) return -1;
    switch (x = pop()) {
        case '\n': return ';';
        case '/': return comment() ? next() : '/';
        case '!': case '<': case '>':
            if (ch() == '=') return pop(), (x == '!' ? 1 : x == '<' ? 2 : 3);
        default:
            if (ch() == x) return pop() | 0x80;
            return x;
    }
}

int lex(void) {
    if (qlen) return --qlen ? t2 : t;
    if (t == -1 && t2 == -1) error("unexpected (eof)");
    t2 = t;
    return t = next();
}

char lexch(bool* escaped) {
    *escaped = ch() == '\\';
    if (!*escaped) return pop();
    pop();
    switch (pop()) {
        case '0': return '\0';
        case 'n': return '\n';
        case 't': return '\t';
        case 'r': return '\r';
        case 'b': return '\b';
        case 's': return ' ';
        case 'e': return '\033';
        case '>': return '>';
        default:  error("bad esc-char");
    }
}

void unlex(void) {
    qlen++;
}

bool trylex(int to) {
    return lex() == to ? true : (unlex(), false);
}
