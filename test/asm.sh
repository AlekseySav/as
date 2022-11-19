#
# compare as1 & as2 output with nasm output
#

regb="al|cl|dl|bl|ah|ch|dh|bh"
regw="ax|cx|dx|bx|sp|bp|si|di"
sreg="es|cs|ss|ds|fs|gs"

imm_small="0|1|2|3|4|5|6|7|8|9|10|11|12|13|14|15|16"

immb="0|1|2|3|4|5|10|40|100|120|-55|-5|-1"
immb_as="*0|*1|*2|*3|*4|*5|*10|*40|*100|*120|*-55|*-5|*-1"
ummb="0|1|2|3|4|5|10|40|100|120|200|250|255"
ummb_as="*0|*1|*2|*3|*4|*5|*10|*40|*100|*120|*200|*250|*255"

immw="$immb|1000|2000|3000|4000|10000|30000|50000|65535"
immw_as="$immb_as|1000|2000|3000|4000|10000|30000|50000|*-1"
ummw="$ummb|1000|2000|3000|4000|10000|30000|50000|65535"
ummw_as="$ummb_as|1000|2000|3000|4000|10000|30000|50000|65535"

modrm="[bx+si]|[bx+di]|[bp+si]|[bp+di]|[si]|[di]|[10+bx+si]|[10+bx+di]|[10+bp+si]|[10+bp+di]|[10+si]|[10+di]|[10]|[500+bx+si]|[500+bx+di]|[500+bp+si]|[500+bp+di]|[500+si]|[500+di]|[500]"
modrm_as="(bx_si)|(bx_di)|(bp_si)|(bp_di)|(si)|(di)|*10(bx_si)|*10(bx_di)|*10(bp_si)|*10(bp_di)|*10(si)|*10(di)|*(10)|500(bx_si)|500(bx_di)|500(bp_si)|500(bp_di)|500(si)|500(di)|(500)"

rmb="$regb|$modrm"
rmw="$regw|$modrm"
rmb_as="$regb|$modrm_as"
rmw_as="$regw|$modrm_as"

run-nasm() {
    python3 tools/combine.py \
        "-r/*dx*/=dx" "-r/*ax*/=ax" "-r/*al*/=al" "-r/*,*/=," \
        "$@" $'\n' >test/tmp/1.asm
    echo "times (\$-\$\$) % 2 db 0" >>test/tmp/1.asm
    nasm test/tmp/1.asm
}

run-as() {
    python3 tools/combine.py \
        "-r$immb=$immb_as" "-r$immw=$immw_as" \
        "-r$modrm=$modrm_as" \
        "-r$rmb=$rmb_as" "-r$rmw=$rmw_as" \
        "$@" $'\n' >test/tmp/1.s
    bash test/tools/assem.sh test/tmp/1.s
}

run-test() {
    case $1 in
        b) nasm_size=" byte"; as_size="b" ;;
        B) nasm_size=" "; as_size="b" ;;
        w) nasm_size=" word"; as_size="" ;;
        b+) nasm_size=" byte| byte"; as_size="b|" ;;
        *) nasm_size=""; as_size="";;
    esac
    opcode=$2; shift 2
    run-nasm $opcode "$nasm_size" "$@"
    run-as $opcode $as_size "$@"
    echo -n '.'
    cmp test/tmp/1 test/tmp/1.s.o
    if [ $? -ne 0 ]; then
        exit
    fi
}

ok() {
    echo .. ok
}

echo -n "onebyte"
onebyte="pusha|popa|nop|cbw|cwd|pushf|popf|sahf|lahf|leave|int3|into|iret|xlat|repne|repe|rep|hlt|cmc|clc|stc|cli|sti|cld|std"
run-test - $onebyte
ok

echo -n "string"
string="movsw|movsb|cmpsw|cmpsb|stosw|stosb|lodsw|lodsb|scasw|scasb"
run-test - $string
ok

echo -n "argbyte"
run-test - "int" " " $ummb
ok

echo -n "math0"
math0="add|or|adc|sbb|and|sub|xor|cmp"
run-test b+ $math0 " " $rmb , $regb
run-test w  $math0 " " $rmw , $regw
run-test b+ $math0 " " $regb , $modrm
run-test w  $math0 " " $regw , $modrm
run-test b+ $math0 " " $regb , $immb
run-test b  $math0 " " $modrm , $immb
run-test w  $math0 " " $rmw , $immw
ok

echo -n "math1"
math1="rol|ror|rcl|rcr|sal|shl|shr|sar"
run-test b+ $math1 " " $regb , $imm_small
run-test b  $math1 " " $modrm , $imm_small
run-test w  $math1 " " $rmw , $imm_small
run-test b+ $math1 " " $regb , "cl"
run-test b  $math1 " " $modrm , "cl"
run-test w  $math1 " " $rmw , "cl"
ok

echo -n "math2"
math2="not|neg|mul|imul|div|idiv"
run-test b+ $math2 " " $regb
run-test b  $math2 " " $modrm
run-test w  $math2 " " $rmw
ok

echo -n "test"
run-test b+ "test" " " $regb , $rmb
run-test b+ "test" " " $modrm , $regb
run-test w  "test" " " $regw , $rmw
run-test w  "test" " " $modrm , $regw
run-test b+ "test" " " $regb , $immb
run-test b  "test" " " $modrm , $immb
run-test w  "test" " " $rmw , $immw
ok

echo -n "inc/dec"
run-test b+ "inc|dec" " " $regb
run-test b  "inc|dec" " " $modrm
run-test w  "inc|dec" " " $rmw
ok

echo -n "push/pop"
nostar_immb="0|$immb"
run-test w "push|pop" " " $rmw
run-test w "push|pop" " " "es|ss|ds|fs|gs"
run-test w  "push" " " $immw
run-test b+ "push" " " $immb
run-test b  "push" " " $nostar_immb
ok

echo -n "ret/retf"
nostar_immw="0|$immw"
run-test - "ret|retf"
run-test - "ret|retf" " " $nostar_immw
ok

echo -n "in/out"
nostar_ummb="0|$ummb"
run-test B "out" " " "/*dx*/" "/*,*/" "/*al*/"
run-test B "in" " " "/*al*/" "/*,*/" "/*dx*/"
run-test B "out" " " $nostar_ummb "/*,*/" "/*al*/"
run-test B "in" " " "/*al*/" "/*,*/" $nostar_ummb
ok

echo -n "mov"
run-test b+ "mov" " " $rmb , $regb
run-test b+ "mov" " " $regb , $modrm
run-test w  "mov" " " $rmw , $regw
run-test w  "mov" " " $regw , $modrm
run-test w  "mov" " " $regw , $modrm
run-test w  "mov" " " $regw , $sreg
run-test w  "mov" " " $sreg , $regw
run-test w  "mov" " " $rmw , $immw
run-test b+ "mov" " " $regb , $nostar_immb
run-test b  "mov" " " $modrm , $nostar_immb
ok

echo -n "xchg"
run-test b+ "xchg" " " $rmb , $regb
run-test b+ "xchg" " " $regb , $modrm
run-test w  "xchg" " " $rmw , $regw
run-test w  "xchg" " " $regw , $modrm
ok
