echo ---- test symtab/lexer/parser ----
./build.sh test
echo ---- test asm opcodes ----
./build.sh && bash test/asm.sh
echo ---- test pseudo instructions ----
echo NOTE: NO PSEUDO INSTRUCTIONS TESTS
