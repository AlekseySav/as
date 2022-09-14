as1n = as.o sym.o lex.o value.o args.o debug.o preprocess.o eval.o opcodes.o
as2n = as.o sym.o
as1o = $(as1n:%=src/as1/%)
as2o = $(as2n:%=src/as2/%)
temp = src/as1/eval.c src/as1/opcodes.c

CFLAGS += -g

.PHONY: test run clean

as1: $(as1o)
	@gcc -o $@ $^

as2: $(as2o)
	@gcc -o $@ $^

test: as1 as2
	@test/expr
	@test/link
	@test/main

run: as1 as2 tmp/boot tmp/kern
	@cat tmp/boot tmp/kern >1
	@rm tmp/boot tmp/kern
	@truncate -s 32K 1
	@qemu-system-x86_64 1
	@hexdump -C 1

clean:
	@rm -f $(as1o) $(as2o) $(temp) as1 as2 test/tmp*

src/as1/eval.c: tools/eval src/as1/eval.yaml
	$^ >$@

src/as1/opcodes.c: tools/opcodes src/as1/opcodes.yaml
	$^ >$@

%: %.s
	tools/as $< >$@

tmp/kern: tmp/kern.s tmp/console.s
	tools/as $^ >$@
