#include "struct.h"

#include "print.h"

//auxiliary functions for print
static inline int list_P(int32 ptr)
{
	while(type(CDR(ptr))==0) 
		ptr= CDR(ptr);
	return (CDR(ptr)==nilptr);//is the last element NIL?
}	

static inline void print_list(int32 ptr)
{
	ourprint("(");
	while(1)
	{
		print(CAR(ptr)); 
		if((ptr= CDR(ptr)) == nilptr) 
			break;//the last element of list need not " ":(... a) 
		ourprint(" ");
	}
	ourprint(")");
}

static inline void print_ordered_pair(int32 ptr)
{
	ourprint("(");
	print(CAR(ptr)); ourprint(" . "); print(CDR(ptr));
	ourprint(")");
}

/*----------------------------------------------------------------------------
  The S-expression pointed to by j is typed out.
  ----------------------------------------------------------------------------*/
void print(int32 ptr)
{
	int32 i= ptrv(ptr);
	switch (type(ptr))
	{
		case 0:/*dotted-pair*/
			list_P(ptr)? print_list(ptr) : print_ordered_pair(ptr); break;
		case  8:/*ordinary atom*/ 
			ourprint(Atab[i].name); break;
		case  9:/*number*/ 
			sprintf(sout,"%-g",Ntab[i].num); ourprint(sout); break;
		case 10:/*builtin function*/ 
			sprintf(sout,"{builtin function: %s}",Atab[i].name);
			ourprint(sout); break;
		case 11:/*builtin special form*/
			sprintf(sout,"{builtin special form: %s}",Atab[i].name);
			ourprint(sout); break;
		case 12:/*user-defined function*/
			sprintf(sout,"{user defined function: %s}",Atab[i].name);
			ourprint(sout); break;
		case 13:/*user-defined special form*/
			sprintf(sout,"{user defined special form: %s}",Atab[i].name);
			ourprint(sout); break;
		case 14:/*unnamed function*/
			ourprint("{unnamed function}"); break;
		case 15:/*unnamed special form*/ 
			ourprint("{unnamed special form}"); break;
	}
}
