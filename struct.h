#include "linuxenv.h"

#if !defined(NULL)
#  define NULL 0L
#endif
#define EOF (-1)

#define atom_table_size	1000/*at most 1000 ordinary atom entries*/
#define list_area_size 	6000/*at most 6000 dotted-pair entries*/

/*--------------------------------------------------------------------------- 
 							 global macros 
 ---------------------------------------------------------------------------*/
#define CAR(j)         	P[j].car
#define CDR(j)         	P[j].cdr

#define type(f)        	(((f)>>28) & 0xf)
#define ptrv(f)        	(0x0fffffff & (f))

#define s_ex_P(t)      	((t) == 0 || (t) == 8 || (t) == 9)
#define fsf_P(t)		((t)>9)
#define builtin_P(t)    ((t) == 10 || (t) == 11)
#define usr_def_P(t)    ((t) == 12 || (t) == 13)
#define dottedpair_P(t) ((t) == 0)
#define f_P(t)         	((t) == 10 || (t) == 12 || (t) == 14)
#define unnamedfsf_P(t) ((t)>13)
#define namedfsf_P(t)   ((t)>9 && (t)<14)

#define tp(t,j)        	((t) | (j))
#define ud(j)          	(0x10000000 | (j))//1   udefined
#define dp(j)          	(0x00000000 | (j))//0   dotted-pair
#define oa(j)          	(0x80000000 | (j))//8   ordinary atom
#define nu(j)          	(0x90000000 | (j))//9   number
#define bf(j)          	(0xa0000000 | (j))//10  builtin function
#define bs(j)          	(0xb0000000 | (j))//11  builtin special form
#define uf(j)          	(0xc0000000 | (j))//12  user defined function
#define us(j)          	(0xd0000000 | (j))//13  user defined special form
#define tf(j)          	(0xe0000000 | (j))//14  unnamed function
#define ts(j)          	(0xf0000000 | (j))//15  unnamed special form

#define hashnum(r) ((*(1+(int32 *)(&r)) & 0x7fffffff) % atom_table_size)
		/*hashname:   s[first]<<16 + s[last]<<8 + length(s)*/
#define hashname(s) (abs((s[0]<<16)+(s[(j=strlen(s))-1]<<8)+j) % atom_table_size)

/*---------------------------------------------------------------------------
 								global structs
 ---------------------------------------------------------------------------*/
/* atom table */
struct Atom_node {
	char name[16]; 
	int32 L;//value (typed pointer) 
	int32 bl;//binding list
	int32 plist;//porperty list
};

/* number table:storing floating point numbers.*/
union Number_node {
	double num; 
	int16 nlink;//linking number-table nodes on the number-table free space list. 
};

/* list area */
struct Cons_cell {
	int32 car; 
	int32 cdr;
};

/* the input stream stack structure && head pointer */
struct Insave {
	struct Insave *link; 
	char *pg, *pge; 
	char g[202]; 
	FILE *filep;
};

/*---------------------------------------------------------------------------
 							 global variables
 ---------------------------------------------------------------------------*/
extern struct Atom_node Atab[atom_table_size];
extern union Number_node Ntab[atom_table_size];
extern struct Cons_cell *P; /* The list area */

extern char prompt; 		/* the input prompt character */
extern char *sout;         /* general output buffer pointer */

extern int32 nilptr,tptr,currentin,eaL,quoteptr,sk,traceptr; /* Global ordinary atom typed-pointers */

extern int32 nilptr;
extern int32 quoteptr;

/*---------------------------------------------------------------------------
 							 global functions
 ---------------------------------------------------------------------------*/
extern void ourprint(char *s);
