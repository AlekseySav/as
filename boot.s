fatcomp = 1
kernseg = 0x1000
bootseg = 0x07c0

.if fatcomp
jmp .+128; resb 128+..-.                /* prevent corrupting low bytes */
.endif

        mov     ax, kernseg             /* setup stack & s-regs */
        cli
        mov     ss, ax
        xor     sp, sp
        sti
        mov     ds, ax
        mov     es, ax
        xor     ax, ax
        xor     di, di
        mov     cx, 0x8000
        cld; rep; stos                  /* clear kernseg */
1:      mov     ax, 0x0207              /* read 3,5 KiB */
        mov     cx, 0x0002
        xorb    dh, dh
        xor     bx, bx
        int     0x13
        jc      1b                      /* retry on error */
        ljmp    kernseg, 0
        resb    510+..-.; 0xaa55        /* magic */
