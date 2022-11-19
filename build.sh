#!/bin/bash

if [[ $1 == "test" || $1 == "debug" ]]; then
    gcc -g -Iinclude as1/as1[!9]*.c test/basic/as1.c -o .bin/as1
else
    gcc -g -Iinclude as1/as*.c -o .bin/as1
fi

if [[ $1 == "test" ]]; then
    .bin/as1
fi

gcc -Iinclude tools/objdump.c -o .bin/objdump
gcc -Iinclude tools/as2.c -o .bin/as2
