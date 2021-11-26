run: lex
	./lex < ./test.c

w-s: lex.yy.c
	gcc -o $@ $<

lex.yy.c: lex.l
	flex $<