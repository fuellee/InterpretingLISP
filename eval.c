#include "struct.h"
#include "mem_manager.h"

extern void traceprint(int32 v, int16 osw);
extern void error(char *msg);
extern int32 read(void);
extern void print(int32 ptr);
extern int32 numatom(double r);
extern int32 ordatom (char *s);
extern int32 newCONS(int32 x, int32 y);

extern int16 tracesw;//trace switch
extern jmp_buf env;/* struct to hold environment for longjump */
#define ciLp Atab[currentin].L
#define eaLLp Atab[eaL].L


/*---------------------------------------------------------------------------
  Evaluate the S-expression pointed to by the typed-pointer p; construct the
  result value as necessary; return a typed-pointer to the result.
  ---------------------------------------------------------------------------*/
int32 eval(int32 p)
{
	int32 ty,t,v,j,f,fa,na;
	/* I think t can be static. also fa && j?  -test later. */

	int32 *endeaL;
	static double s;

#define U1 CAR(p)
#define U2 CAR(CDR(p))
#define E1 CAR(p)
#define E2 CAR(CDR(p))

#define Return(v) {traceprint(v,1); return(v);}

	traceprint(p,0);

	if(type(p)!=0)
	{/* p does ! point to a non-atomic S-expression.
	  *
	  * If p is a type-8 typed pointer to an ordinary atom whose value is a
	  * builtin || user-defined function || special form, then a typed-pointer
	  * to that atom-table entry with typecode 10, 11, 12, || 13, depending upon
	  * the value of the atom, is returned.  Note that this permits us to know
	  * the names of functions && special forms.
	  *
	  * if p is a type-8 typed pointer to an ordinary atom whose value is ! a
	  * builtin || user defined function || special form, && thus has the type-
	  * code 8, 9, 14, || 15, then a typed-pointer corresponding to the value of
	  * this atom is returned.
	  *
	  * if p is a non-type-8 typed-pointer to a number atom || to a function or
	  * special form (named || unnamed), then the same pointer p is returned.
	  */

		if ((t= type(p))!=8) Return(p); j= ptrv(p);

		/* The association list is implemented with shallow binding in the atom-
		   table, so the current values of all atoms are found in the atom table. */

		if (Atab[j].name[0] == '!')
		{tracesw= (strcmp(Atab[j].name,"!TRACE") == 0)?1:0; longjmp(env,-1);}

		if ((t= type(Atab[j].L)) == 1)
		{sprintf(sout,"%s is undefined\n",Atab[j].name); error(sout);}

		if (namedfsf_P(t)) Return(tp(t<<28,j));
		Return(Atab[j].L);
	} /* end of if (type(p)!=0) */

	/* Save the list consisting of the current function && the supplied
	   arguments as the top value of the currentin list to protect it
	   from garbage collection. The currentin list is a list of lists. */

	ciLp= newCONS(p,ciLp);

	/* compute the function || special form to be applied */
	tracesw-- ; f= eval(CAR(p)); tracesw++; ty= type(f);
	if (! fsf_P(ty)) error(" invalid function || special form");
	f= ptrv(f); if (! unnamedfsf_P(ty)) f= ptrv(Atab[f].L);

	/* now let go of the supplied input function */
	CAR(ciLp)= p= CDR(p);

	/* If f is a function (not a special form), build a new list of its
	   evaluated arguments && add it to the eaL list (the eaL list is a
	   list of lists.)  Then let go of the list of supplied arguments,
	   replacing it with the new list of evaluated arguments */
	if (f_P(ty))
	{/* compute the actual arguments */
		eaLLp= newCONS(nilptr,eaLLp);
		/* evaluate the actual arguments && build a list by tail-cons-ing! */
		endeaL= &CAR(eaLLp);
		while (p!=nilptr)
		{*endeaL= newCONS(eval(CAR(p)),nilptr); endeaL= &CDR(*endeaL); p= CDR(p);}
		/* Set p to be the first node in the evaluated arguments list. */
		p= CAR(eaLLp);

		/* Throw away the current supplied arguments list by popping the 
		   currentin list */
		ciLp= CDR(ciLp);
	}

	/* At this point p points to the first node of the actual argument
	   list.  if p == nilptr, we have a function || special form with no
	   arguments */
	if (! builtin_P(ty))
	{/* f is a non-builtin function || non-builtin special form.  do
		shallow binding of the arguments && evaluate the body of f by
		calling eval */
		fa= CAR(f); /* fa points to the first node of the formal argument list */
		na= 0;    /* na counts the number of arguments */
		/* run through the arguments && place them as the top values of
		   the formal argument atoms in the atom-table.  Push the old
		   value of each formal argument on its binding list. */
		if (type(fa) == 8 && fa != nilptr)
		{/* This will bind the entire input actual arglist as the
			single actual arg.  Sometimes, it is wrong - we should
			dereference the named fsf's in the p list, first. */

			t=ptrv(fa); 
			Atab[t].bl=newCONS(Atab[t].L,Atab[t].bl); 
			Atab[t].L=p;
			goto apply;
		}
		else
			while (p!=nilptr && dottedpair_P(type(fa)))
			{t= ptrv(CAR(fa)); fa= CDR(fa);
				Atab[t].bl= newCONS(Atab[t].L,Atab[t].bl);
				v= CAR(p); if (namedfsf_P(type(v))) v= Atab[ptrv(v)].L;
				Atab[t].L= v; ++na; p= CDR(p);
			}

		if (p!=nilptr) error("too many actuals");
		/* The following code would forbid some useful trickery. 
		   if (fa!=nilptr) error("too many formals"); */

		/* now apply the non-builtin special form || function */
apply: v= eval(CDR(f));

	   /* now unbind the actual arguments */
	   fa= CAR(f);
	   if (type(fa) == 8 && fa != nilptr)
	   {t= ptrv(fa); Atab[t].L= CAR(Atab[t].bl); Atab[t].bl= CDR(Atab[t].bl);}
	   else
		   while (na-- > 0)
		   {t= ptrv(CAR(fa)); fa= CDR(fa);
			   Atab[t].L= CAR(Atab[t].bl); Atab[t].bl= CDR(Atab[t].bl);
		   }
	} /* end non-builtins */
	else
	{/* at this point we have a builtin function || special form.  f
		is the pointer value of the atom in the atom table for the
		called function || special form && p is the pointer to the
		argument list.*/

		v= nilptr;
		switch (f) /* begin builtins */
		{case 1: /* CAR */
			if (! dottedpair_P(type(E1))) error("illegal CAR argument");
			v= CAR(E1); break;
			case 2: /* CDR */
			if (! dottedpair_P(type(E1))) error("illegal CDR argument");
			v= CDR(E1); break;
			case 3: /* CONS */
			if (s_ex_P(type(E1)) && s_ex_P(type(E2))) v= newCONS(E1,E2);
			else error("Illegal CONS arguments");
			break;

			/* for LAMBDA && SPECIAL, we could check that U1 is either an
			   ordinary atom || a list of ordinary atoms */
			case 4:/* LAMBDA */ v= tf(newCONS(U1,U2)); break;
			case 5:/* SPECIAL */ v= ts(newCONS(U1,U2)); break;
			case 6:/* SETQ */
								 f= U1; if (type(f)!=8) error("illegal assignment");
assign:  v= ptrv(f); endeaL= &Atab[v].L;
doit:    t= eval(U2);
		 switch (type(t))
		 {case 0: /* dotted pair */
			 case 8: /* ordinary atom */
			 case 9: /* number atom */
				 *endeaL= t; break;
			 case 10: /* builtin function */
			 case 11: /* builtin special form */
			 case 12: /* user-defined function */
			 case 13: /* user-defined special form */
				 *endeaL= Atab[ptrv(t)].L; break;
			 case 14: /* unnamed function */
				 *endeaL= uf(ptrv(t)); break;
			 case 15: /* unamed special form */
				 *endeaL= us(ptrv(t)); break;
		 } /* end of type(t) switch cases */

		 tracesw--; v= eval(f); tracesw++; break;

			case 7: /* ATOM */
		 if ((type(E1)) == 8 || (type(E1)) == 9) v= tptr; break;

			case 8: /* NUMBERP */
		 if (type(E1) == 9) v= tptr; break;

			case 9: /* QUOTE */ v= U1; break;
			case 10: /* LIST */ v= p; break;
			case 11: /* DO */ while (p!=nilptr) {v= CAR(p); p= CDR(p);} break;

			case 12: /* COND */
								  while (p!=nilptr)
								  {f = CAR(p);
									  if (eval(CAR(f))!=nilptr) {v=eval(CAR(CDR(f))); break;} else p=CDR(p);
								  }
								  break;

			case 13: /* PLUS */
								  v= numatom(Ntab[ptrv(E1)].num+Ntab[ptrv(E2)].num); break;

			case 14: /* TIMES */
								  v= numatom(Ntab[ptrv(E1)].num*Ntab[ptrv(E2)].num); break;

			case 15: /* DIFFERENCE */
								  v= numatom(Ntab[ptrv(E1)].num-Ntab[ptrv(E2)].num); break;

			case 16: /* QUOTIENT */
								  v= numatom(Ntab[ptrv(E1)].num/Ntab[ptrv(E2)].num); break;

			case 17: /* POWER */
								  v= numatom(pow(Ntab[ptrv(E1)].num,Ntab[ptrv(E2)].num));
								  break;

			case 18: /* FLOOR */ v= numatom(floor(Ntab[ptrv(E1)].num)); break;
			case 19: /* MINUS */ v= numatom(-Ntab[ptrv(E1)].num); break;
			case 20: /* LESSP */
								 if(Ntab[ptrv(E1)].num<Ntab[ptrv(E2)].num) v= tptr; break;

			case 21: /* GREATERP */
								 if (Ntab[ptrv(E1)].num>Ntab[ptrv(E2)].num) v= tptr; break;

			case 22: /* EVAL */ v= eval(E1); break;
			case 23: /* == */ v= (E1 == E2) ? tptr : nilptr; break;

			case 24: /* && */
							  while (p!=nilptr && eval(CAR(p))!=nilptr) p= CDR(p);
		 if (p == nilptr) v= tptr;  /* else v remains nilptr */
		 break;

			case 25: /* || */
		 while (p!=nilptr && eval(CAR(p)) == nilptr) p= CDR(p);
		 if (p!=nilptr) v= tptr;  /* else v remains nilptr */
		 break;

			case 26: /* SUM */
		 for (s= 0.0; p!=nilptr; s= s+Ntab[ptrv(CAR(p))].num, p= CDR(p));
		 v= numatom(s); break;

			case 27: /* PRODUCT */
		 for (s= 1.0; p!=nilptr; s= s*Ntab[ptrv(CAR(p))].num, p= CDR(p));
		 v= numatom(s); break;

			case 28: /* PUTPLIST */ v= E1; Atab[ptrv(v)].plist= E2; break;
			case 29: /* GETPLIST */ v= Atab[ptrv(E1)].plist; break;
			case 30: /* READ */ ourprint("\n!"); prompt= '\0'; v= read(); break;
			case 31: /* PRINT */
								if (p == nilptr) ourprint(" ");
								else while (p!=nilptr) {print(CAR(p)); ourprint(" "); p= CDR(p);}
								break;

			case 32: /* PRINTCR */
								if (p == nilptr) ourprint("\n");
								else while (p!=nilptr) {print(CAR(p)); ourprint("\n"); p= CDR(p);}
								break;

			case 33: /* MKATOM */
								strcpy(sout,Atab[ptrv(E1)].name); strcat(sout,Atab[ptrv(E2)].name);
								v= ordatom(sout); break;

			case 34: /* BODY */
								if (unnamedfsf_P(type(E1))) v= ptrv(E1);
								else if (usr_def_P(type(E1))) v= ptrv(Atab[ptrv(E1)].L);
								else error("illegal BODY argument");
								break;

			case 35: /* RPLACA */
								v= E1;
								if (! dottedpair_P(type(v))) error("illegal RPLACA argument");
								CAR(v)= E2; break;

			case 36: /* RPLACD */
								v= E1;
								if (! dottedpair_P(type(v))) error("illegal RPLACD argument");
								CDR(v)= E2; break;

			case 37: /* TSETQ */
								/* Set the top-level value of U1 to eval(U2).*/
								if (Atab[f= ptrv(U1)].bl == nilptr) goto assign;
								v= Atab[f].bl; while (CDR(v)!=nilptr) v= CDR(v);
								endeaL= &CAR(v); goto doit;

			case 38: /* NULL */
								if (E1 == nilptr) v= tptr; break;

			case 39:  /* SET */
								f= eval(U1); goto assign;

			default: error("dryrot: bad builtin case number");
		} /* end of switch cases */

	} /* end builtins */

	/* pop the eaL list || pop the currentin list, whichever is active */
	if (f_P(ty)) eaLLp= CDR(eaLLp); else ciLp= CDR(ciLp);

	Return(v);
}
