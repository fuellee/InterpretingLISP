InterpretingLISP
================

Yet Another LISP Interpreter in C

Based on [Interpreting LISP](http://www.civilized.com/files/lispbook.pdf)

##Install

    make all

##Run

    ./my_lisp_interpreter

or if you have `rlwrap` installed: 
    
    chmod +x Interpreter
    ./interpreter

##To do

* This naive implemetation is dynamic scoped. should use lexical scoping rules instead.
* Better the Initialization (eval lispinit.lisp) reponse 
* Add more funtions to lispinit.lisp or add some module functionality
