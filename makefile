all: my_lisp_interpreter

my_lisp_interpreter: *.c
	$(CC) *.c -lm -o my_lisp_interpreter

.PHONY: clean

clean:
	rm -f *.o
	rm my_lisp_interpreter
