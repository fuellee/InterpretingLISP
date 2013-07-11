all: my_lisp_interpreter

my_lisp_interpreter: main.o read.o eval.o print.o
	gcc -o my_lisp_interpreter -lm main.o read.o eval.o print.o 
main.o:	main.c
	gcc -Wall -c main.c 
read.o: read.c
	gcc -Wall -c read.c 
eval.o: eval.c
	gcc -Wall -c eval.c 
print.o: print.c
	gcc -Wall -c print.c 

.PHONY: clean

clean: 
	rm -f *.o
