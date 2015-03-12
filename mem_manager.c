#include "struct.h"
#include "mem_manager.h"

//mem_manager.c need:
extern int16 ct; /* eval depth count */
extern int16 tracesw;/* trace switch */

extern int16 nx[atom_table_size];/* the number hash index table */
extern int16 nf;/* the number table free space list head pointer */
	/* the number table mark array nmark is used in garbage collection to
   	   mark words not to be returned to the free space list */
extern char nmark[atom_table_size]; /* an array of 1-bit entries would suffice */

extern int16 fp;/* the list area free space list head pointer */
extern int32 numf; /* Number of free list-nodes */

extern void error(char *msg);
extern void print(int32 ptr);


/*----------------------------------------------------------------------------
  This function prints out the input && the result for each successive
  invocation of eval() when tracing is requested.
  ----------------------------------------------------------------------------*/
void traceprint(int32 v, int16 osw)
{/* int32 v; the object to be printed
  * int16 osw; 1 for eval() output, 0 for eval() input */
	if (tracesw>0)
	{
		osw? sprintf(sout,"%d result:",ct--) : sprintf(sout,"%d eval:",++ct);
		ourprint(sout); 
		
		print(v); ourprint("\n");
	}
}

/*--------------------------------------------------------------------------
  Allocates && loads the fields of a new location in the list area, with
  CAR()= X, CDR()= Y. The index of the new location is returned.
  -------------------------------------------------------------------------*/
int32 newCONS(int32 x, int32 y)
{
	int32 j;

	if (fp<0) 
	{
		gcmark(x); gcmark(y); gc(); 
		if(fp<0) 
			error("out of space");
	}
	j= fp; fp= CDR(j); CAR(j)= x; CDR(j)= y; numf--; 
	return(j);
}

//macros for gc() and gcmark(ptr)
#define marked(p)    ((CAR(p) & 0x08000000)!=0)
#define marknode(p)  (CAR(p) |= 0x08000000)
#define unmark(p)    (CAR(p) &= 0xf7ffffff)
#define marknum(p)   if(type(p) == 9) nmark[ptrv(p)]= 1
#define listp(t)     ((t) == 0 || (t)>11)/*dotted-pairs non-primitive function,special-form*/

/*--------------------------------------------------------------------------
  Garbage collector for number table and listarea
  --------------------------------------------------------------------------*/
void gc(void)
{
	int32 i,t;

	for (i= 0; i<atom_table_size; i++)
	{gcmark(Atab[i].L); gcmark(Atab[i].bl); gcmark(Atab[i].plist);}

	for (i= 0; i<atom_table_size; i++) nx[i]= -1;

	for (nf= -1,i= 0; i<atom_table_size; i++)
		if (nmark[i] == 0) {Ntab[i].nlink= nf; nf= i;}
		else  /* restore num[i] */
		{t= hashnum(Ntab[i].num);
			while (nx[t]!=-1) if ((++t) == atom_table_size) t= 0;
			nx[t]= i; nmark[i]= 0;
		}

	/* build the new list-node free-space list */
	fp= -1; numf= 0;
	for (i=1; i<list_area_size; i++) 
		if (! marked(i)) {CDR(i)= fp; fp= i; numf++;} else unmark(i);
}

/*--------------------------------------------------------------------------
  Mark the S-expression given by the typed-pointer p.
  --------------------------------------------------------------------------*/
void gcmark(int32 p)
{
	static int32 s,t;
start:
	if (listp( type(p) ))
	{
		p=ptrv(p); 
		if(marked(p)) 
			return; 
		marknode(p);
		t=CAR(p); 
		if(! listp(type(t))) 
		{
			marknum(t); 
			p=CDR(p); 
			goto start;
		}
		s=CDR(p); 
		if (! listp(type(s))) 
		{
			marknum(s); 
			p=t; 
			goto start;
		}
		gcmark(t); 
		p=CDR(p); 
		goto start; /* Equivalent to the recursive call: gcmark(CDR(p)) */
	}
	else marknum(p);
}
