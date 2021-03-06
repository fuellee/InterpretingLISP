/filename: 386 C:\lisp\lispinit              revision date: October 15, 1988

(SETQ APPEND (LAMBDA (X Y) (COND ((EQ X NIL) Y)
   ((ATOM X) (CONS X Y))
   (T (CONS (CAR X) (APPEND (CDR X) Y)) )) ))

(SETQ REVERSE (LAMBDA (X) (COND ((ATOM X) X)
   (T (APPEND (REVERSE (CDR X)) (CONS (CAR X) NIL )))) ))

(SETQ EQUAL (LAMBDA (X Y) (COND ((OR (ATOM X) (ATOM Y)) (EQ X Y))
   ((EQUAL (CAR X) (CAR Y)) (EQUAL (CDR X) (CDR Y)))
   (T NIL)) ))

(SETQ NOT NULL)

(SETQ MEMBER (LAMBDA (A S) (COND ((EQ S NIL) NIL) ((EQUAL A (CAR S)) T)
(T (MEMBER A (CDR S))) )))

(SETQ INTO (LAMBDA (G L) (COND ((NULL L) L) (T (CONS (G (CAR L))
(INTO G (CDR L)))))))

(SETQ ONTO (LAMBDA (G L) (COND ((NULL L) L) (T (CONS (G L)
(ONTO G (CDR L)))))))

(SETQ APPLY (SPECIAL ($G $X) (EVAL (CONS $G $X))))

(SETQ SORT (LAMBDA (X)
   (COND ((NULL X) X) (T (MERGE (CAR X) (SORT (CDR X)))))))

(SETQ MERGE (LAMBDA (V L) (COND ((OR (NULL L) (LESSP V (CAR L))) (CONS V L))
   (T (CONS (CAR L) (MERGE V (CDR L)))))))

(SETQ GETPROP (LAMBDA (A P) (ASSOC (GETPLIST A) P)))

(SETQ ASSOC (LAMBDA (L P) (COND ((NULL L) NIL)
   (T (COND ((EQUAL P (CAR (CAR L))) (CDR (CAR L)))
   (T (ASSOC (CDR L) P)))))))

(SETQ PUTPROP (LAMBDA (A P W) (PUTPLIST A
   (CONS (CONS P W) (GETPLIST (REMPROP A P W))))))

(SETQ REMPROP (LAMBDA (A P W) (PUTPLIST A (NAX (GETPLIST A) (CONS P W)))))

(SETQ NAX (LAMBDA (L P) (COND
   ((NULL L) NIL) ((EQUAL (CAR L) P) (CDR L))
   (T (DO (NX L P) L)))))

(SETQ NX (LAMBDA (L P) (COND ((NULL (CDR L)) NIL)
   ((EQUAL P (CAR (CDR L))) (RPLACD L (CDR L))))))






(setq define setq)

(setq IF 
  (SPECIAL (CONDITION THEN-DO ELSE-DO) 
		   (COND (CONDITION THEN-DO) 
				 (t ELSE-DO))))

(define = eq)

(define * times)

(define - difference)

(define factorial 
  (lambda (x) 
	(if (= x 0) 
	  	1 
		(* x (factorial (- x 1))))))
