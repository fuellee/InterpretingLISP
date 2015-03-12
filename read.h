#include "linuxenv.h"

//provide by read.c
extern int32 read(void);

//read.c need
extern void error(char *msg);
extern int32 ordatom (char *s);
extern int32 numatom(double r);
extern int32 newCONS(int32 x, int32 y);

extern struct Insave *topInsave;

	/* The input string && related pointers */
extern char *g/*input buffer string*/,*pg/*start of string g*/,*pge/*end of string g*/;

extern FILE *filep;//try to change this globals local
extern FILE *logfilep;
