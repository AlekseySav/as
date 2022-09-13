        mov     al, '0
        call    coutchar
        call    coutchar
        jmpb    .

screen = 0xb800

coutchar:
        mov     di, (pos)
        push    screen
        pop     es
        mov     ah, (attr)
        stos
        mov     (pos), di
        ret

attr:   0x70

x:      .=.+1
y:      .=.+1
pos:    .=.+2
