/*****************************************************************************

  LISP  INTERPRETER
  -----------------

  This progam is a LISP interpreter.  This interpreter consists of
  three major functions: READ, EVAL, and PRINT.  READ scans the
  input string for input S-expressions (atoms && dotted pairs) and
  returns a corresponding typed-pointer. The EVAL function takes as
  input a typed-pointer p to an input S-expression && evaluates it and
  returns a typed pointer to its result. PRINT takes as input the
  typed pointer returned from EVAL && prints out the result.

  LISP input lines beginning with a "/" are comment lines.  Indirect
  input text is taken from a file Z to replace the directive of the form
  "@Z".  EVAL tracing can be turned on by using the directive "!trace",
  && turned off with the directive "!notrace".

 *****************************************************************************/
#include "struct.h"

#include "read.h"
#include "print.h"

/* extern references */
extern int32 eval(int32 i);
extern void initlisp(void);
extern int32 read(void);
extern void gc(void);
extern void gcmark(int32 p);
extern void error(char *s);


jmp_buf env;         /* struct to hold environment for longjump */
char *sout;          /* general output buffer pointer */

/*Atab:atom table */
struct Atom_node Atab[atom_table_size];

/*Ntab:number table*/
union Number_node Ntab[atom_table_size];

/* the number hash index table */
int16 nx[atom_table_size];

/* the number table free space list head pointer */
int16 nf= -1;

/* the number table mark array nmark is used in garbage collection to
   mark words not to be returned to the free space list */
char nmark[atom_table_size]; /* an array of 1-bit entries would suffice */

/* The list area */
struct Cons_cell *P;

/* the list area free space list head pointer */
int16 fp= -1;

/* The input string && related pointers */
char *g/*input buffer string*/,*pg/*start of string g*/,*pge/*end of string g*/;

/* the input stream stack structure head pointer */
struct Insave *topInsave;

char prompt; /* the input prompt character */

/* eval depth count && trace switch */
int16 ct= 0, tracesw= 0;

/* Global ordinary atom typed-pointers */
int32 nilptr,tptr,currentin,eaL,quoteptr,sk,traceptr;

/* Number of free list-nodes */
int32 numf;

/* variables used in file operations */
FILE *filep;
FILE *logfilep;

/*---------------------------------------------------------------------------
  For debugging to see if we are leaking list-nodes.
  We are to protect r from garbage-collection.
  This function can be called from within the main loop.
  ----------------------------------------------------------------------------*/
void spacerpt(int32 r)
{
	char s[60]; 
	int16 t;

	sprintf(s,"entering spacerpt: r=%lx, numf=%ld\n", r, numf); ourprint(s);

	t = type(r);
	if (namedfsf_P(t)) r = ptrv(Atab[ptrv(r)].L); /* dereference r */
	if (builtin_P(t)) r = nilptr; /*do not try to mark a builtin */
	gcmark(r);
	gc();

	sprintf(s,"leaving spacerpt: numf=%ld\n", numf); ourprint(s);
}

/*---------------------------------------------------------------------------
  The main read/eval/print loop.
  ----------------------------------------------------------------------------*/
int main(void)
{
	int32 r;
	initlisp();

	setjmp(env);/*calling error() returns to here by longjmp()*/

	while(1)
	{
		ourprint("\n");
		prompt= '*';
		r=read();
		r=eval(r);
		print(r);  /* print uses/frees no list-nodes. */
	}
	return 0;
}

/*---------------------------------------------------------------------------
  Type-out the message msg && do longjmp() to top level
  ----------------------------------------------------------------------------*/
void error(char *msg)
{
	int32 i,t;

	/* discard all input S-expression && argument list stacks */
	Atab[currentin].L= nilptr; 
	Atab[eaL].L= nilptr; 
	Atab[sk].L= nilptr;

	/* reset all atoms to their top-level values */
	for(i= 0; i< atom_table_size; i++) 
		if((t= Atab[i].bl)!=nilptr)
		{
			while(CDR(t)!=nilptr)//while not top-level 
				t= CDR(t); 			//go forward
			Atab[i].L= CAR(t);	//setq atom top-level-value
			Atab[i].bl= nilptr;	//empty the binding list
		}

	ct= 0;//reset eval depth counter
	ourprint("::"); ourprint(msg); ourprint("\n");
	longjmp(env,-1);
}

/*--------------------------------------------------------------------------
  Print the string s to the terminal, && also in the logfile, lisp.log
  --------------------------------------------------------------------------*/
void ourprint(char *s)
{
	printf("%s",s); fflush(stdout); 
	fprintf(logfilep,"%s",s); fflush(logfilep);
}

/*---------------------------------------------------------------------------
  Installs all builtin functions && special forms into the atom table. 
  Initializes the number table && list area.
  --------------------------------------------------------------------------*/
void initlisp(void)
{
	int32 i;
	static char *BIname[]= {
		"CAR","CDR","CONS","LAMBDA","SPECIAL","SETQ","ATOM","NUMBERP","QUOTE",
		"LIST","DO","COND","PLUS","TIMES","DIFFERENCE","QUOTIENT","POWER",
		"FLOOR","MINUS","LESSP","GREATERP","EVAL","EQ","AND","OR","SUM","PRODUCT",
		"PUTPLIST","GETPLIST","READ","PRINT","PRINTCR","MKATOM","BODY","RPLACA",
		"RPLACD","TSETQ", "NULL", "SET" };
	static char BItype[]= {
		10,10,10,11,11,11,10,10,11,10,
		10,11,10,10,10,10,10,10,10,10,
		10,10,10,11,11,10,10,10,10,10,
		10,10,10,10,10,10,11,10,11 };

/* number of builtin's in BIname[] && BItype[] above */
#define NBI 39

	/* allocate a global character array for messages */
	sout= (char *)calloc(80,sizeof(char));

	/* allocate the input string */
	g= (char *)calloc(202,sizeof(char));

	/* allocate the list area */
	P= (struct Cons_cell *)calloc(list_area_size,sizeof(struct Cons_cell));

	/* initialize atom table names && the number table */
	for (i= 0; i<atom_table_size; i++)
	{
		Atab[i].name[0]='\0'; nmark[i]=0; nx[i]= -1; 
		Ntab[i].nlink=nf; nf=i;
	}

	/* install typed-case numbers for builtin functions && special forms into
	   the atom table */
	for (i= 0; i<NBI; i++)                  /*the nth builtin start from 1*/
		Atab[ptrv(ordatom(BIname[i]))].L= tp((((int32)BItype[i])<<28),(i+1));

	nilptr= ordatom("NIL"); Atab[ptrv(nilptr)].L= nilptr;
	tptr= ordatom("T");     Atab[ptrv(tptr)].L= tptr;
	quoteptr= ordatom("QUOTE");

	/* Creating these lists in the atom-table ensures that we protect
	   them during garbage-collection. Make CURRENTIN && EAL ! upper-case
	   to keep them private.*/
	currentin= ptrv(ordatom("CURRENTIN")); Atab[currentin].L= nilptr;
	eaL= ptrv(ordatom("EAL")); Atab[eaL].L= nilptr;
	sk= ptrv(ordatom("readlist")); Atab[sk].L= nilptr;

#define ciLp Atab[currentin].L
#define eaLLp Atab[eaL].L

	/* initialize the bindlist (bl) && plist fields */
	for(i= 0; i<atom_table_size; i++) 
		Atab[i].bl= Atab[i].plist= nilptr;

	/* set up the list area free-space list */
	for(i= 1; i<list_area_size; i++){
		CDR(i)= fp; fp= i;} 
	numf = list_area_size-1;

	/* Prepare to read in predefined functions && special forms from the
	   lispinit.lisp file: these are APPEND, REVERSE, EQUAL, APPLY, INTO,
	   ONTO, NOT, NULL, ASSOC, NPROP, PUTPROP, GETPROP, && REMPROP */

	/* open the logfile */
	logfilep= fopen("lisp.log","w");
	ourprint(" ENTERING THE LISP INTERPRETER \n");

	/* establish the input buffer && the input stream stack */
	topInsave= NULL;
	strcpy(g,"@lispinit.lisp ");
	pg= g; pge= g+strlen(g);/* initialize start & end pointers to string g */
	filep= stdin;
}

/*----------------------------------------------------------------------------
  The number r is looked-up in the number table && stored there as a lazy
  number atom if it is not already present.  
  The typed-pointer to this number atom is returned.
  ----------------------------------------------------------------------------*/
int32 numatom(double r)
{
	int32 j;
	/* hashnum(r): ((*(1+(int32 *)(&r)) & 0x7fffffff) % atom_table_size)*/
	j= hashnum(r);

	while (nx[j]!=-1)
		if (Ntab[nx[j]].num == r) 
		{
			j= nx[j]; 
			goto ret;
		} 
		else if(++j == atom_table_size) 
			j= 0;

	if (nf<0) 
	{
		gc(); 
		if (nf<0) 
			error("The number table is full");
	}
	nx[j]= nf; j= nf; nf= Ntab[nf].nlink; Ntab[j].num= r;
ret: return(nu(j));
}

/*----------------------------------------------------------------------------
  The ordinary atom whose name is given as the argument string s is looked-up
  in the atom table && stored there as an atom with the value undefined if it
  is not already present.  The typed-pointer to this ordinary atom is then
  returned.
  ----------------------------------------------------------------------------*/
			/*hashname:  	s[first]<<16 + s[last]<<8 + length(s)*/
int32 ordatom (char *s)
{
	int32 j,c;
	j= hashname(s); c= 0;
//DEBUG(printf("ordatom: `%s' hashes to %d. k=%d, atom_table_size=%d\n",s,j,k,atom_table_size););
	while (Atab[j].name[0]!='\0')
	{
		if(strcmp(Atab[j].name,s) == 0) 
			goto ret;
		else if(++j >= atom_table_size) 
		{
			j= 0; 
			if (++c>1) 
				error("atom table is full");
		}
	}
	strcpy(Atab[j].name,s); Atab[j].L= ud(j);
ret:return(oa(j));
}
