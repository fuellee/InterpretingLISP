all: my_lisp_interpreter

my_lisp_interpreter: *.c
	clang -lm *.c -o my_lisp_interpreter

.PHONY: clean

clean:
	rm -f *.o
