/* c:\mks\bin\yacc -D hashtok.h -o hashtest.c hashtest.y */
#define IDENT	1
#define INTLIT	2
#define ASSIGN	3
#define SEMI	4
#define DELETE	5
#define INSERT	6
#define LIST	7
#ifdef YYTRACE
#define YYDEBUG 1
#else
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#endif
/*
 * Portable way of defining ANSI C prototypes
 */
#ifndef YY_ARGS
#if __STDC__
#define YY_ARGS(x)	x
#else
#define YY_ARGS(x)	()
#endif
#endif

#if YYDEBUG
typedef struct yyNamedType_tag {	/* Tokens */
	char	* name;		/* printable name */
	short	token;		/* token # */
	short	type;		/* token type */
} yyNamedType;
typedef struct yyTypedRules_tag {	/* Typed rule table */
	char	* name;		/* compressed rule string */
	short	type;		/* rule result type */
} yyTypedRules;

#endif

#line 4 "hashtest.y"

#include "lg.h"
#include "error.h" 
#include <string.h> 
#include "hash.h" 
#include "hashtest.h"
//#include "mprintf.h"
#define yyerror printf
Hashtable TestHash;
extern bool my_iter_func(void* elem, void* data);
extern int yychar, yyerrflag;
#ifndef YYSTYPE
#define YYSTYPE int
#endif
extern YYSTYPE yylval;
#if YYDEBUG
yyTypedRules yyRules[] = {
	{ "&00: %01 &00",  0},
	{ "%01: %02",  0},
	{ "%02: %03 &05",  0},
	{ "%02: %02 %03 &05",  0},
	{ "%03: %04",  0},
	{ "%03: %05",  0},
	{ "%03: %06",  0},
	{ "%03: %07",  0},
	{ "%03: %08",  0},
	{ "%03: %09",  0},
	{ "%07:",  0},
	{ "%04: &02 &04 &03",  0},
	{ "%05: &07 &02 &04 &03",  0},
	{ "%06: &06 &02",  0},
	{ "%08: &02",  0},
	{ "%09: &08",  0},
	{ "%09: &08 &02",  0},
{ "$accept",  0},{ "error",  0}
};
yyNamedType yyTokenTypes[] = {
	{ "$end",  0,  0},
	{ "error",  256,  0},
	{ "IDENT",  1,  0},
	{ "INTLIT",  2,  0},
	{ "ASSIGN",  3,  0},
	{ "SEMI",  4,  0},
	{ "DELETE",  5,  0},
	{ "INSERT",  6,  0},
	{ "LIST",  7,  0}

};
#endif
static short yydef[] = {

	  -1,    4,    3,   -5,  -11
};
static short yyex[] = {

	   4,   17,   -1,    1,    0,   18,    4,   17,   -1,    1, 
	   0,    0,   -1,    1
};
static short yyact[] = {

	  -3,  -12,  -11,   -2,    7,    6,    5,    1,  -13,    1, 
	 -14,    1,   -9,    1,   -8,    3,  -17,    4,   -6,    3, 
	 -16,    2,  -18,    4,  -15,    2,   -1
};
static short yypact[] = {

	   4,    9,   15,    4,   18,   25,   23,   21,   19,   17, 
	  13,   11
};
static short yygo[] = {

	  -5,   -4,   -7,  -10,    3,  -19,  -20,  -21,  -22,  -23, 
	 -24,   -1
};
static short yypgo[] = {

	   0,    0,    0,    9,   10,   10,    7,    6,    5,    1, 
	   1,    3,    3,    3,    3,    3,    3,    8,    0,    0
};
static short yyrlen[] = {

	   0,    0,    0,    1,    1,    2,    2,    4,    3,    2, 
	   3,    1,    1,    1,    1,    1,    1,    0,    1,    2
};
#define YYS0	0
#define YYDELTA	7
#define YYNPACT	12
#define YYNDEF	5

#define YYr17	0
#define YYr18	1
#define YYr19	2
#define YYr14	3
#define YYr15	4
#define YYr16	5
#define YYr13	6
#define YYr12	7
#define YYr11	8
#define YYrACCEPT	YYr17
#define YYrERROR	YYr18
#define YYrLR2	YYr19
#if YYDEBUG
char * yysvar[] = {
	"$accept",
	"file",
	"sequence",
	"statement",
	"_set",
	"_insert",
	"_delete",
	"_null",
	"_lookup",
	"_list",
	0
};
short yyrmap[] = {

	  17,   18,   19,   14,   15,   16,   13,   12,   11,    2, 
	   3,    4,    5,    6,    7,    8,    9,   10,    1,    0
};
short yysmap[] = {

	   0,    1,    4,   12,   13,   20,   19,   17,   16,   11, 
	   3,    2,   14,   15,   23,   21,   18,   22,   10,    9, 
	   8,    7,    6,    5
};
int yyntoken = 9;
int yynvar = 10;
int yynstate = 24;
int yynrule = 20;
#endif

#if YYDEBUG
/*
 * Package up YACC context for tracing
 */
typedef struct yyTraceItems_tag {
	int	state, lookahead, errflag, done;
	int	rule, npop;
	short	* states;
	int	nstates;
	YYSTYPE * values;
	int	nvalues;
	short	* types;
} yyTraceItems;
#endif

#line 2 "c:/mks/etc/yyparse.c"

/*
 * Copyright 1985, 1990 by Mortice Kern Systems Inc.  All rights reserved.
 * 
 * Automaton to interpret LALR(1) tables.
 *
 * Macros:
 *	yyclearin - clear the lookahead token.
 *	yyerrok - forgive a pending error
 *	YYERROR - simulate an error
 *	YYACCEPT - halt and return 0
 *	YYABORT - halt and return 1
 *	YYRETURN(value) - halt and return value.  You should use this
 *		instead of return(value).
 *	YYREAD - ensure yychar contains a lookahead token by reading
 *		one if it does not.  See also YYSYNC.
 *	YYRECOVERING - 1 if syntax error detected and not recovered
 *		yet; otherwise, 0.
 *
 * Preprocessor flags:
 *	YYDEBUG - includes debug code if 1.  The parser will print
 *		 a travelogue of the parse if this is defined as 1
 *		 and yydebug is non-zero.
 *		yacc -t sets YYDEBUG to 1, but not yydebug.
 *	YYTRACE - turn on YYDEBUG, and undefine default trace functions
 *		so that the interactive functions in 'ytrack.c' will
 *		be used.
 *	YYSSIZE - size of state and value stacks (default 150).
 *	YYSTATIC - By default, the state stack is an automatic array.
 *		If this is defined, the stack will be static.
 *		In either case, the value stack is static.
 *	YYALLOC - Dynamically allocate both the state and value stacks
 *		by calling malloc() and free().
 *	YYSYNC - if defined, yacc guarantees to fetch a lookahead token
 *		before any action, even if it doesnt need it for a decision.
 *		If YYSYNC is defined, YYREAD will never be necessary unless
 *		the user explicitly sets yychar = -1
 *
 * Copyright (c) 1983, by the University of Waterloo
 */
/*
 * Prototypes
 */

extern int yylex YY_ARGS((void));

#if YYDEBUG

#include <stdlib.h>		/* common prototypes */
#include <string.h>

extern char *	yyValue YY_ARGS((YYSTYPE, int));	/* print yylval */
extern void yyShowState YY_ARGS((yyTraceItems *));
extern void yyShowReduce YY_ARGS((yyTraceItems *));
extern void yyShowGoto YY_ARGS((yyTraceItems *));
extern void yyShowShift YY_ARGS((yyTraceItems *));
extern void yyShowErrRecovery YY_ARGS((yyTraceItems *));
extern void yyShowErrDiscard YY_ARGS((yyTraceItems *));

extern void yyShowRead YY_ARGS((int));
#endif

/*
 * If YYDEBUG defined and yydebug set,
 * tracing functions will be called at appropriate times in yyparse()
 * Pass state of YACC parse, as filled into yyTraceItems yyx
 * If yyx.done is set by the tracing function, yyparse() will terminate
 * with a return value of -1
 */
#define YY_TRACE(fn) { \
	yyx.state = yystate; yyx.lookahead = yychar; yyx.errflag =yyerrflag; \
	yyx.states = yys+1; yyx.nstates = yyps-yys; \
	yyx.values = yyv+1; yyx.nvalues = yypv-yyv; \
	yyx.types = yytypev+1; yyx.done = 0; \
	yyx.rule = yyi; yyx.npop = yyj; \
	fn(&yyx); \
	if (yyx.done) YYRETURN(-1); }

#ifndef I18N
#define	gettext(x)	x
#endif

#ifndef YYSSIZE
# define YYSSIZE	150
#endif

#define YYERROR		goto yyerrlabel
#define yyerrok		yyerrflag = 0
#if YYDEBUG
#define yyclearin	{ if (yydebug) yyShowRead(-1); yychar = -1; }
#else
#define yyclearin	yychar = -1
#endif
#define YYACCEPT	YYRETURN(0)
#define YYABORT		YYRETURN(1)
#define YYRECOVERING()	(yyerrflag != 0)
#ifdef YYALLOC
# define YYRETURN(val)	{ retval = (val); goto yyReturn; }
#else
# define YYRETURN(val)	return(val)
#endif
#if YYDEBUG
/* The if..else makes this macro behave exactly like a statement */
# define YYREAD	if (yychar < 0) {					\
			if ((yychar = yylex()) < 0)			\
				yychar = 0;				\
			if (yydebug)					\
				yyShowRead(yychar);			\
		} else
#else
# define YYREAD	if (yychar < 0) {					\
			if ((yychar = yylex()) < 0)			\
				yychar = 0;				\
		} else
#endif

#define YYERRCODE	256		/* value of `error' */
#define	YYQYYP	yyq[yyq-yyp]

YYSTYPE	yyval;				/* $ */
YYSTYPE	*yypvt;				/* $n */
YYSTYPE	yylval;				/* yylex() sets this */

int	yychar,				/* current token */
	yyerrflag,			/* error flag */
	yynerrs;			/* error count */

#if YYDEBUG
int yydebug = 0;		/* debug if this flag is set */
extern char	*yysvar[];	/* table of non-terminals (aka 'variables') */
extern yyNamedType yyTokenTypes[];	/* table of terminals & their types */
extern short	yyrmap[], yysmap[];	/* map internal rule/states */
extern int	yynstate, yynvar, yyntoken, yynrule;

extern int	yyGetType YY_ARGS((int));	/* token type */
extern char	*yyptok YY_ARGS((int));	/* printable token string */
extern int	yyExpandName YY_ARGS((int, int, char *, int));
				  /* expand yyRules[] or yyStates[] */
static char *	yygetState YY_ARGS((int));

#define yyassert(condition, msg, arg) \
	if (!(condition)) { \
		printf(gettext("\nyacc bug: ")); \
		printf(msg, arg); \
		YYABORT; }
#else /* !YYDEBUG */
#define yyassert(condition, msg, arg)
#endif

#line 63 "hashtest.y"
int my_hash_func(void* duh)
{
  HashElem* elem = (HashElem*)duh;
  int i,sum = 0;
  for (i = 0; i < HASHELEMSIZE; i++)
    if (elem->key[i] == '\0') return sum;
    else sum += elem->key[i];
  return sum;
}

int my_equ_func(void* arg1, void* arg2)
{
   HashElem  *elem1 = (HashElem*)arg1, 
             *elem2 = (HashElem*)arg2;
   return strncmp(elem1->key,elem2->key,HASHELEMSIZE);
}

bool my_iter_func(void* elem, void* data)
{
  HashElem *e = (HashElem*)elem;
  char* s = (char*)data;
  printf("%s: %d\n",e->key,e->val);
  return data != NULL && strncmp(e->key,s,HASHELEMSIZE) == 0;
}

main()
{
  errtype err;
//  DbgMonoConfig();
  err = hash_init(&TestHash,sizeof(HashElem),3,my_hash_func,my_equ_func);
  if (err != OK) printf("Error on init: %d\n",err);
  while(TRUE) yyparse();
}


yyparse()
{
	register short		yyi, *yyp;	/* for table lookup */
	register short		*yyps;		/* top of state stack */
	register short		yystate;	/* current state */
	register YYSTYPE	*yypv;		/* top of value stack */
	register short		*yyq;
	register int		yyj;
#if YYDEBUG
	yyTraceItems	yyx;			/* trace block */
	short	* yytp;
	int	yyruletype = 0;
#endif
#ifdef YYSTATIC
	static short	yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];
#if YYDEBUG
	static short	yytypev[YYSSIZE+1];	/* type assignments */
#endif
#else /* ! YYSTATIC */
#ifdef YYALLOC
	YYSTYPE *yyv;
	short	*yys;
#if YYDEBUG
	short	*yytypev;
#endif
	YYSTYPE save_yylval;
	YYSTYPE save_yyval;
	YYSTYPE *save_yypvt;
	int save_yychar, save_yyerrflag, save_yynerrs;
	int retval;
#else
	short		yys[YYSSIZE + 1];
	static YYSTYPE	yyv[YYSSIZE + 1];	/* historically static */
#if YYDEBUG
	short	yytypev[YYSSIZE+1];		/* mirror type table */
#endif
#endif /* ! YYALLOC */
#endif /* ! YYSTATIC */


#ifdef YYALLOC
	yys = (short *) malloc((YYSSIZE + 1) * sizeof(short));
	yyv = (YYSTYPE *) malloc((YYSSIZE + 1) * sizeof(YYSTYPE));
#if YYDEBUG
	yytypev = (short *) malloc((YYSSIZE+1) * sizeof(short));
#endif
	if (yys == (short *)0 || yyv == (YYSTYPE *)0
#if YYDEBUG
		|| yytypev == (short *) 0
#endif
	) {
		yyerror("Not enough space for parser stacks");
		return 1;
	}
	save_yylval = yylval;
	save_yyval = yyval;
	save_yypvt = yypvt;
	save_yychar = yychar;
	save_yyerrflag = yyerrflag;
	save_yynerrs = yynerrs;
#endif

	yynerrs = 0;
	yyerrflag = 0;
	yyclearin;
	yyps = yys;
	yypv = yyv;
	*yyps = yystate = YYS0;		/* start state */
#if YYDEBUG
	yytp = yytypev;
	yyi = yyj = 0;			/* silence compiler warnings */
#endif

yyStack:
	yyassert((unsigned)yystate < yynstate, gettext("state %d\n"), yystate);
	if (++yyps > &yys[YYSSIZE]) {
		yyerror("Parser stack overflow");
		YYABORT;
	}
	*yyps = yystate;	/* stack current state */
	*++yypv = yyval;	/* ... and value */
#if YYDEBUG
	*++yytp = yyruletype;	/* ... and type */

	if (yydebug)
		YY_TRACE(yyShowState)
#endif

	/*
	 *	Look up next action in action table.
	 */
yyEncore:
#ifdef YYSYNC
	YYREAD;
#endif
	if (yystate >= sizeof yypact/sizeof yypact[0]) 	/* simple state */
		yyi = yystate - YYDELTA;	/* reduce in any case */
	else {
		if(*(yyp = &yyact[yypact[yystate]]) >= 0) {
			/* Look for a shift on yychar */
#ifndef YYSYNC
			YYREAD;
#endif
			yyq = yyp;
			yyi = yychar;
			while (yyi < *yyp++)
				;
			if (yyi == yyp[-1]) {
				yystate = ~YYQYYP;
#if YYDEBUG
				if (yydebug) {
					yyruletype = yyGetType(yychar);
					YY_TRACE(yyShowShift)
				}
#endif
				yyval = yylval;	/* stack what yylex() set */
				yyclearin;		/* clear token */
				if (yyerrflag)
					yyerrflag--;	/* successful shift */
				goto yyStack;
			}
		}

		/*
	 	 *	Fell through - take default action
	 	 */

		if (yystate >= sizeof yydef /sizeof yydef[0])
			goto yyError;
		if ((yyi = yydef[yystate]) < 0)	 { /* default == reduce? */
			/* Search exception table */
			yyassert((unsigned)~yyi < sizeof yyex/sizeof yyex[0],
				gettext("exception %d\n"), yystate);
			yyp = &yyex[~yyi];
#ifndef YYSYNC
			YYREAD;
#endif
			while((yyi = *yyp) >= 0 && yyi != yychar)
				yyp += 2;
			yyi = yyp[1];
			yyassert(yyi >= 0,
				 gettext("Ex table not reduce %d\n"), yyi);
		}
	}

	yyassert((unsigned)yyi < yynrule, gettext("reduce %d\n"), yyi);
	yyj = yyrlen[yyi];
#if YYDEBUG
	if (yydebug)
		YY_TRACE(yyShowReduce)
	yytp -= yyj;
#endif
	yyps -= yyj;		/* pop stacks */
	yypvt = yypv;		/* save top */
	yypv -= yyj;
	yyval = yypv[1];	/* default action $ = $1 */
#if YYDEBUG
	yyruletype = yyRules[yyrmap[yyi]].type;
#endif

	switch (yyi) {		/* perform semantic action */
		
case YYr11: {	/* _set :  IDENT ASSIGN INTLIT */
#line 26 "hashtest.y"

                 HashElem elem;
                 strncpy(elem.key,(char*)yypvt[-2],HASHELEMSIZE);
                 elem.val = yypvt[0];
                 hash_set(&TestHash,&elem);
               
} break;

case YYr12: {	/* _insert :  INSERT IDENT ASSIGN INTLIT */
#line 34 "hashtest.y"

                 HashElem elem;
                 strncpy(elem.key,(char*)yypvt[-2],HASHELEMSIZE);
                 elem.val = yypvt[0];
                 hash_insert(&TestHash,&elem);
               
} break;

case YYr13: {	/* _delete :  DELETE IDENT */
#line 42 "hashtest.y"

                 HashElem elem;
                 strncpy(elem.key,(char*)yypvt[0],HASHELEMSIZE);
                 hash_delete(&TestHash,&elem);
               
} break;

case YYr14: {	/* _lookup :  IDENT */
#line 49 "hashtest.y"

                 HashElem elem,*result;
                 strncpy(elem.key,(char*)yypvt[0],HASHELEMSIZE);
                 hash_lookup(&TestHash,&elem,&result);
                 if (result != NULL)
                         printf("%s: %d\n",yypvt[0],result->val);
                      else printf("%s not found\n",yypvt[0]);
               
} break;

case YYr15: {	/* _list :  LIST */
#line 58 "hashtest.y"
 hash_iter(&TestHash,my_iter_func,NULL); 
} break;

case YYr16: {	/* _list :  LIST IDENT */
#line 59 "hashtest.y"
 hash_iter(&TestHash,my_iter_func,(void*)yypvt[0]); 
} break;
#line 314 "c:/mks/etc/yyparse.c"
	case YYrACCEPT:
		YYACCEPT;
	case YYrERROR:
		goto yyError;
	}

	/*
	 *	Look up next state in goto table.
	 */

	yyp = &yygo[yypgo[yyi]];
	yyq = yyp++;
	yyi = *yyps;
	while (yyi < *yyp++)
		;

	yystate = ~(yyi == *--yyp? YYQYYP: *yyq);
#if YYDEBUG
	if (yydebug)
		YY_TRACE(yyShowGoto)
#endif
	goto yyStack;

yyerrlabel:	;		/* come here from YYERROR	*/
/*
#pragma used yyerrlabel
 */
	yyerrflag = 1;
	if (yyi == YYrERROR) {
		yyps--;
		yypv--;
#if YYDEBUG
		yytp--;
#endif
	}

yyError:
	switch (yyerrflag) {

	case 0:		/* new error */
		yynerrs++;
		yyi = yychar;
		yyerror("Syntax error");
		if (yyi != yychar) {
			/* user has changed the current token */
			/* try again */
			yyerrflag++;	/* avoid loops */
			goto yyEncore;
		}

	case 1:		/* partially recovered */
	case 2:
		yyerrflag = 3;	/* need 3 valid shifts to recover */
			
		/*
		 *	Pop states, looking for a
		 *	shift on `error'.
		 */

		for ( ; yyps > yys; yyps--, yypv--
#if YYDEBUG
					, yytp--
#endif
		) {
			if (*yyps >= sizeof yypact/sizeof yypact[0])
				continue;
			yyp = &yyact[yypact[*yyps]];
			yyq = yyp;
			do
				;
			while (YYERRCODE < *yyp++);

			if (YYERRCODE == yyp[-1]) {
				yystate = ~YYQYYP;
				goto yyStack;
			}
				
			/* no shift in this state */
#if YYDEBUG
			if (yydebug && yyps > yys+1)
				YY_TRACE(yyShowErrRecovery)
#endif
			/* pop stacks; try again */
		}
		/* no shift on error - abort */
		break;

	case 3:
		/*
		 *	Erroneous token after
		 *	an error - discard it.
		 */

		if (yychar == 0)  /* but not EOF */
			break;
#if YYDEBUG
		if (yydebug)
			YY_TRACE(yyShowErrDiscard)
#endif
		yyclearin;
		goto yyEncore;	/* try again in same state */
	}
	YYABORT;

#ifdef YYALLOC
yyReturn:
	yylval = save_yylval;
	yyval = save_yyval;
	yypvt = save_yypvt;
	yychar = save_yychar;
	yyerrflag = save_yyerrflag;
	yynerrs = save_yynerrs;
	free((char *)yys);
	free((char *)yyv);
	return(retval);
#endif
}

		
#if YYDEBUG
/*
 * Return type of token
 */
int
yyGetType(tok)
int tok;
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->type;
	return 0;
}
/*
 * Print a token legibly.
 */
char *
yyptok(tok)
int tok;
{
	yyNamedType * tp;
	for (tp = &yyTokenTypes[yyntoken-1]; tp > yyTokenTypes; tp--)
		if (tp->token == tok)
			return tp->name;
	return "";
}

/*
 * Read state 'num' from YYStatesFile
 */
#ifdef YYTRACE
static FILE *yyStatesFile = (FILE *) 0;
static char yyReadBuf[YYMAX_READ+1];

static char *
yygetState(num)
int num;
{
	int	size;

	if (yyStatesFile == (FILE *) 0
	 && (yyStatesFile = fopen(YYStatesFile, "r")) == (FILE *) 0)
		return "yyExpandName: cannot open states file";

	if (num < yynstate - 1)
		size = (int)(yyStates[num+1] - yyStates[num]);
	else {
		/* length of last item is length of file - ptr(last-1) */
		if (fseek(yyStatesFile, 0L, 2) < 0)
			goto cannot_seek;
		size = (int) (ftell(yyStatesFile) - yyStates[num]);
	}
	if (size < 0 || size > YYMAX_READ)
		return "yyExpandName: bad read size";
	if (fseek(yyStatesFile, yyStates[num], 0) < 0) {
	cannot_seek:
		return "yyExpandName: cannot seek in states file";
	}

	(void) fread(yyReadBuf, 1, size, yyStatesFile);
	yyReadBuf[size] = '\0';
	return yyReadBuf;
}
#endif /* YYTRACE */
/*
 * Expand encoded string into printable representation
 * Used to decode yyStates and yyRules strings.
 * If the expansion of 's' fits in 'buf', return 1; otherwise, 0.
 */
int
yyExpandName(num, isrule, buf, len)
int num, isrule;
char * buf;
int len;
{
	int	i, n, cnt, type;
	char	* endp, * cp;
	char	*s;

	if (isrule)
		s = yyRules[num].name;
	else
#ifdef YYTRACE
		s = yygetState(num);
#else
		s = "*no states*";
#endif

	for (endp = buf + len - 8; *s; s++) {
		if (buf >= endp) {		/* too large: return 0 */
		full:	(void) strcpy(buf, " ...\n");
			return 0;
		} else if (*s == '%') {		/* nonterminal */
			type = 0;
			cnt = yynvar;
			goto getN;
		} else if (*s == '&') {		/* terminal */
			type = 1;
			cnt = yyntoken;
		getN:
			if (cnt < 100)
				i = 2;
			else if (cnt < 1000)
				i = 3;
			else
				i = 4;
			for (n = 0; i-- > 0; )
				n = (n * 10) + *++s - '0';
			if (type == 0) {
				if (n >= yynvar)
					goto too_big;
				cp = yysvar[n];
			} else if (n >= yyntoken) {
			    too_big:
				cp = "<range err>";
			} else
				cp = yyTokenTypes[n].name;

			if ((i = strlen(cp)) + buf > endp)
				goto full;
			(void) strcpy(buf, cp);
			buf += i;
		} else
			*buf++ = *s;
	}
	*buf = '\0';
	return 1;
}
#ifndef YYTRACE
/*
 * Show current state of yyparse
 */
void
yyShowState(tp)
yyTraceItems * tp;
{
	short * p;
	YYSTYPE * q;

	printf(
	    gettext("state %d (%d), char %s (%d)\n"),
	      yysmap[tp->state], tp->state,
	      yyptok(tp->lookahead), tp->lookahead);
}
/*
 * show results of reduction
 */
void
yyShowReduce(tp)
yyTraceItems * tp;
{
	printf("reduce %d (%d), pops %d (%d)\n",
		yyrmap[tp->rule], tp->rule,
		tp->states[tp->nstates - tp->npop],
		yysmap[tp->states[tp->nstates - tp->npop]]);
}
void
yyShowRead(val)
int val;
{
	printf(gettext("read %s (%d)\n"), yyptok(val), val);
}
void
yyShowGoto(tp)
yyTraceItems * tp;
{
	printf(gettext("goto %d (%d)\n"), yysmap[tp->state], tp->state);
}
void
yyShowShift(tp)
yyTraceItems * tp;
{
	printf(gettext("shift %d (%d)\n"), yysmap[tp->state], tp->state);
}
void
yyShowErrRecovery(tp)
yyTraceItems * tp;
{
	short	* top = tp->states + tp->nstates - 1;

	printf(
	gettext("Error recovery pops state %d (%d), uncovers %d (%d)\n"),
		yysmap[*top], *top, yysmap[*(top-1)], *(top-1));
}
void
yyShowErrDiscard(tp)
yyTraceItems * tp;
{
	printf(gettext("Error recovery discards %s (%d), "),
		yyptok(tp->lookahead), tp->lookahead);
}
#endif	/* ! YYTRACE */
#endif	/* YYDEBUG */
