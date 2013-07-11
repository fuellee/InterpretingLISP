#include "struct.h"

#include "read.h"

/* the put-back variable */
static int32 put_back= 0;


/*----------------------------------------------------------------------------
  Gets a line from stream && puts it into s (at most lim chars).  
  '\n's are dorped;TABs are maped to blank.
  returns the length of readed string;returns -1 (EOF) If there are no characters but EOF.
  -----------------------------------------------------------------------------*/
int16 fgetline(char *s/*read to*/, int16 lim, FILE *stream/*read from*/)
{
	int16 c,N_readed;
#define TAB 9
	for (N_readed=0; N_readed<lim && (c=fgetc(stream))!=EOF && c!='\n'; ++N_readed)
	{
		if (c == TAB) c= ' '; 
		s[N_readed]= c;
	}
	s[N_readed]= '\0';
	if (c == EOF && N_readed == 0)//reached the end of file
		return(-1); //return EOF (-1)
	else return(N_readed);
}

/*----------------------------------------------------------------------------
  Read a line into g[]. A line starting with a "/" is a comment line.
  -----------------------------------------------------------------------------*/
/* '\n' droped in fgetline; Tab maped to ' ' in fgetline;comment ignored in fillg*/
void fillg(void)
{
	/*pg:start of unused char	pge:end of the g buffer*/
	while (pg>=pge)/*if g buffer has been all used,fresh it*/
	{
sprompt: if (filep == stdin) //only if read form stdio,print the prompt char.
		 {
			 sprintf(sout,"%c",prompt); 
			 ourprint(sout);
		 }
		 if (fgetline(g,200,filep)<0) 
			 return;//end of file the buffer g is empty!!!!
		 //read source from usr input,write to logfile;from file,no write
		 if (filep == stdin)
		 {
			 fprintf(logfilep,"%s\n",g); fflush(logfilep);
		 }
		 if (*g == '/')//ignore the line start with '/' 
			 goto sprompt;
		 pg= g; pge= g+strlen(g); *pge++= ' '; *pge= '\0'; prompt= '>';
	}
}

/*----------------------------------------------------------------------------
  Fill the buffer g if needed;
  remove && return the next character from the input.
  -----------------------------------------------------------------------------*/
char getgchar(void)
{fillg(); return(*pg++);}

/*----------------------------------------------------------------------------
  Fill the buffer string g if needed; 
  return a copy of the next character in the input, but don't advance pg..
 * -----------------------------------------------------------------------------*/
char lookgchar(void)
{fillg(); return(*pg);}

/*----------------------------------------------------------------------------
  E is a lexical token scanning routine which has no input && returns
  1 if the token is '('
  2 if the token is '''
  3 if the token is '.'
  4 if the token is ')'
  or a negative typed-pointer to an entry in the atom-table or number-table.
  -----------------------------------------------------------------------------*/
int32 e(void)
{
	double v,f,k,sign;
	int32 t,c;
	char nc[50], *np;
	struct Insave *tb;

#define SINGLEQ '\''
#define CHVAL(c) (c-'0')
#define TOUPPER(c) ((c) + 'A'-'a')
#define DIGIT_P(c) ('0'<=(c) && (c)<='9')
#define ISLOWER(c) ((c)>='a' && (c)<='z')

	if (put_back!=0)//has something put back by read(),return the put_back first 
	{
		t= put_back; 
		put_back= 0; 
		return(t);
	}
start:
	while ((c= getgchar()) == ' ');  /* remove blanks */

	if(c == '(') return 1;
	else if(c == '\0')
	{
		if (topInsave == NULL) 
		{
			fclose(logfilep); 
			exit(0);
		}
		/* restore the previous input stream */
		fclose(filep);
		strcpy(g,topInsave->g); pg= topInsave->pg; pge= topInsave->pge;
		filep= topInsave->filep; topInsave= topInsave->link;
		if (prompt == '@') prompt= '>';
		goto start;
	}
	else if(c == SINGLEQ) return(2);
	else if(c == ')') return(4);
	else if(c == '.')
	{
		if (DIGIT_P(lookgchar()))// .[0-9] 
		{
			sign= 1.0; v= 0.0; goto fraction;
		} 
		return(3);// .[^0-9] not a number
	}
	//if  not([0-9+-][0-9])      means not a number
	if(! (DIGIT_P(c)||((c=='+'||c=='-')&&(DIGIT_P(lookgchar())||lookgchar()=='.'))))
	{
		np= nc;
		*np++= c;    /* put c in nc[0] */
		for(c= lookgchar(); c!=' '&&c!='\0'&&c!='('&&c!=')'; c= lookgchar())
			*(np++)= getgchar(); /* add a character */
		*np= '\0'; /* nc is now a string */
		if (*nc == '@')/* command:  switch input streams */
		{	/* save the current input stream */
			tb= (struct Insave *)calloc(1,sizeof(struct Insave));
			tb->link= topInsave; topInsave= tb;
			strcpy(tb->g,g); tb->pg= pg; tb->pge= pge; tb->filep= filep;

			/* set up the new input stream */
			*g= '\0'; pg= pge= g; 
			prompt= '@';
			filep= fopen(nc+1,"r"); /* skip over the @ */
			if (filep == NULL) 
			{
				printf("@file:%s:\n",nc+1);
				error("Cannot open @file!");
			}
			goto start;
		}
		/* convert the string nc to upper case */
		for (np= nc; *np!='\0'; np++)
			if (ISLOWER((int16)*np)) 
				*np= (char)TOUPPER((int16)*np);
		return(ordatom(nc));
	}


//rest: number constructor 			to be reconstructored as a function
	if(c == '-')//nagetive number 
	{
		v= 0.0;//initial number value 
		sign= -1.0;
	} 
	else //positive number
	{
		v= CHVAL(c); 
		sign= 1.0;
	}
	while(DIGIT_P(lookgchar())) 
		v= 10.0*v+CHVAL(getgchar());
	if (lookgchar() == '.')
	{
		getgchar();//drop '.'
		if (DIGIT_P(lookgchar()))//.[0-9]
		{/*handle the fraction part of number*/
fraction:	k= 1.0; f= 0.0;
			do{
				k=10.* k;
				f=10.* f+CHVAL(getgchar());
			}while (DIGIT_P(lookgchar()));
			v= v+f/k;
		}
	}
	return(numatom(sign*v));
}

/*----------------------------------------------------------------------------
  returns a typed pointer to the S-expression it constructed.
  
  Scans input string g using a lexical token scanning routine e();
  The token found by e() is stripped from the front of g.
  --------------------------------------------------------------------------*/
int32 read(void)
{
	int32 j,k,t,c;

	if ((c= e())<=0) return(c);
	if (c == 1)/* '(' */
		if ((k=e()) == 4) return(nilptr);/* ()->nil */ 
		else put_back= k;//put it back


#define skLp Atab[sk].L
	/* to permit recursion, skLp is a list of lists. */
	skLp= newCONS(nilptr,skLp);
	CAR(skLp)= j= k= newCONS(nilptr,nilptr);

	/* we will return k, but we will fill node j first */
	if(c==1)
	{
scan:	CAR(j)=read();
next:	if ((c= e())<=2)// ' ( atom number
		{
			t= newCONS(nilptr,nilptr); CDR(j)= t; j= t;
			if (c<=0) //atom/number
			{
				CAR(j)= c; 
				goto next;
			}
			put_back= c; 
			goto scan;
		}
		if (c!=4) /*not ) */
		{
			CDR(j)= read(); 
			if (e()!=4) 
				error("syntax error");
		}
		skLp= CDR(skLp); return(k);
	}

	if (c == 2)/* ' */
	{
		CAR(j)= quoteptr; 
		CDR(j)= t= newCONS(nilptr,nilptr); 
		CAR(t)= read();
		skLp= CDR(skLp); 
		return(k);
	}

	error("bad syntax");
	return -1;//should never come here.
}
