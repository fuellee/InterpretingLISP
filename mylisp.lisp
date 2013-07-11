(setq define setq)

(setq if 
	  (special (condition then-do else-do) 
			   (cond (condition then-do) 
					 (T else-do))))

(setq define 
  (special (def_name def_body)
	   (cond ((atom def_name)
			  (eval '(setq def_name def_body)))
			 (T (eval (setq (car def_name) (list 'lambda (cdr def_name) def_body)))))))

(define = eq)

(define * times)

(define - difference)

(define factorial 
  (lambda (x) 
	(if (= x 0) 
	  1 
	  (* x (factorial (- x 1))))))
