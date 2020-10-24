/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* 
  Take a conditional expression as a single string argument.
  If the expression is true, 1 is returned.
  If the expression is false, 0 is returned.
  If there was an error in the expression, -1 is returned.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

extern int GL_containswords();
extern int GL_smemberi( char *s, char *t );
extern int GL_wildcmp( char *s1, char *s2, int len, int casecare );
extern int GL_smember( char *s, char *t );
extern int GL_getseg( char *rtn, char *inbuf, int *i, char *sep );
extern int GL_goodnum( char *str, int *prec );
extern int GL_slmember( char *str, char *list );


extern int TDH_err(), TDH_function_call(), TDH_getvalue();

#define stricmp( s, t ) 	strcasecmp( s, t )
#define strnicmp( s, t, u ) 	strncasecmp( s, t, u )
#define err(a,b,c) 		TDH_err(a,b,c)

#define MAXPARMLEN 1024

#define NUMBER 0
#define ALPHA 1

#define NCLAUSE 30
#define NTOKS 30
#define MAXTOK 256


static int evalclause(), evalstmt(), yield();
static char listsep = ',';
extern char *GL_getok();
static int evalflag = 0;
static int nofunc = 0;
static int Matchscore, Matchscore_used;

/* the following is necessary only for var evaluation */
extern char *TDH_dat, *TDH_recid;


/* ================================== */
int
TDH_condex_initstatics()
{
listsep = ',';	/* persist-from-quisp (matters?) */
evalflag = 0;
nofunc = 0;
return( 0 );
}


/* ================================== */
int
TDH_condex( cond, eval )
char cond[];
int eval;  /* 1 = cond contains vars that need to be evaluated  0 = not */
{
int s[NCLAUSE], i, j, k, rtn, ix, negate;
char args[NTOKS][MAXTOK], tok[MAXTOK];
int argc;
double atof();
int condlen;

evalflag = eval;

condlen = strlen( cond );

Matchscore = 0; Matchscore_used = 0; 


/* break cond into tokens */
ix = 0;
/* check for leading "not:" */
strcpy( tok, GL_getok( cond, &ix ) );
if( strcmp( tok, "not:" )==0 ) negate = 1;
else 	{
	negate = 0;
	ix = 0;
	}

for( i = 1; ; i++ ) {
	strcpy( args[ i ], GL_getok( cond, &ix ) );

	/* function may be multiple args - concatenate..*/
	if( !nofunc  && args[i][0] == '$' && ( isalpha( (int) args[i][1] ) || args[i][1] == '$' ) ) {
		while( args[i][ strlen( args[i]) - 1 ] != ')' ) {
			if( ix >= condlen ) break;
			strcat( args[i], GL_getok( cond, &ix ));
			}
		}
	if( args[i][0] == '\0' ) break;
	}
argc = i;

/* for( i = 1; i < argc; i++ ) printf( "[%s]", args[i] );
 * printf( "\n" );
 */







/* do "clauses" */
for( i = 0; i < NCLAUSE; i++ ) s[i] = 0;
i = 0;
j = k = 1;
while( 1 ) {
	if( j==argc && GL_smemberi( args[j], "or ||" )) { 
		err( 1001, "expression error", cond );
		return( -1 ); 
		}
	if( j == argc || GL_smemberi( args[j], "or ||" )) {
		s[ i ] = evalclause( args, k, j-1 );
		if( s[ i ] == -1 ) {
			err( 1002, "expression error", cond );
			return( -1 );
			}
		k = j+1;
		i++;
		}
	j++;
	if( j > argc ) break;
	}
rtn = ( s[0] || s[1] || s[2] || s[3] || s[4] || s[5] || s[6] || s[7] || s[8] || s[9] || s[10] || s[11] );

if( negate ) return( ! rtn );
else return( rtn );
}

/* ================ */
/* EVALCLAUSE - evaluate a clause */
static int
evalclause( args, start, stop )
char args[NTOKS][MAXTOK];
int start, stop;
{
int s[NCLAUSE], i, j, k, rtn;

for( i = 0; i < NCLAUSE; i++ ) s[i] = 1;
i = 0;
j = k = start;
while( 1 ) {
	if( j==stop && GL_smemberi( args[j], "and &&" )) {  return( -1 ); }

	if( j==stop || GL_smemberi( args[j], "and &&" )) {
		s[ i ] = evalstmt( args, k );
		if( s[ i ] == -1 ) return( -1 );
		k = j+1;
		i++;
		}
	j++;
	if( j > stop ) break;
	}
rtn = ( s[0] && s[1] && s[2] && s[3] && s[4] && s[5] && s[6] && s[7] && s[8] && s[9] && s[10] && s[11] );
return( rtn );
}

/* ============ */
/* EVALSTMT - evaluate a statement */
static int
evalstmt( args, start )
char args[NTOKS][MAXTOK];
int start;
{
char r1[MAXTOK], r2[MAXTOK], op[MAXTOK];
int i, t1, t2, i1, i2;
double diff;
double atof(), a2f();
int slen;

if( strcmp( args[start], "@_matchscore" )==0 ) sprintf( r1, "%d", Matchscore ); /* allow capture from a leftward 'contains' */
else strcpy( r1, args[start] ); 
strcpy( op, args[start+1] );
strcpy( r2, args[start+2] );

if( GL_smemberi( r2, "and && or ||" ) || r2[0] == '\0' ) { return( -1 ); } /* got off track */


/* assign data type for value-- types are alpha, number, date */
i1 = yield( r1, &t1 );
i2 = yield( r2, &t2 );
if( i1 < 0 || i2 < 0 ) return( -1 );


/* handle these ops: =  >   >=  <  <=   ... */
if( op[0] == '=' || op[0] == '>' || op[0] == '<' || stricmp( op, "is" )==0 ) {

	if( t1 != t2 ) return( 0 ); /* type mismatch always false for these ops */

	/* compute diff */
	if( t1 == NUMBER && t2 == NUMBER ) diff = atof( r1 ) - atof( r2 );
	else diff = (double) strcmp( r1, r2 );

	/* determine return code 1 or 0 */
	if( op[0] == '=' ) { if( diff == 0 ) return( 1 ); else return( 0 ); }
	else if( op[0] == '>' ) { 
		if( diff > 0 ) return( 1 ); 
		else if( op[1] == '=' && diff == 0 ) return( 1 ); 
		else return( 0 ); 
		}
	else if( op[0] == '<' ) {
		if( diff < 0 ) return( 1 );
		else if( op[1] == '=' && diff == 0 ) return( 1 ); 
		else return( 0 );
		}
	else if( stricmp( op, "is" )==0 ) { if( diff == 0 ) return( 1 ); else return( 0 ); }
	}

/* 'like' ... */
else if( tolower( op[0] ) == 'l' ) return( ! GL_wildcmp( r1, r2, strlen( r2 ), 0 ) );

/* 'contains' */
#ifndef PLOTICUS
else if( strnicmp( op, "contains", 8 )==0 ) {
	int stat;
	stat = GL_containswords( r2, r1 ); /* delimit words on any space/punct */
	if( stat < 0 ) stat = 20; 	/* to keep summation on track.. also so that initial check for 'contains' works */
	if( !Matchscore_used ) Matchscore = stat;
	else Matchscore += stat;	/* if more than one term, accumulate scores */
	Matchscore_used = 1;
	return( (stat < 20 ) );
	}
#endif

/* '!=' ... */
else if( GL_smember( op, "!= <> isnot" )) {
	if( t1 != t2 ) return( 1 ); /* type mismatch always true for != */

	if( t1 == NUMBER && t2 == NUMBER ) diff = atof( r1 ) - atof( r2 );
	else diff = (double) strcmp( r1, r2 );
	
	if( diff != 0 ) return( 1 ); 
	else return( 0 ); 
	}


/* 'inrange' and 'outrange' ... */
else if( strnicmp( op, "inra", 4 )==0 || strnicmp( op, "outr", 4 )==0 ) {
	/* always false if any operands are non-numeric */
	char valtok[80];
	double ff, gg, hh;
	int prec;
	if( t1 != NUMBER ) return( 0 );
	hh = atof( r1 );
	i = 0;
	GL_getseg( valtok, r2, &i, "," );
	if( !GL_goodnum( valtok, &prec ) ) return( 0 );
	ff = atof( valtok );
	GL_getseg( valtok, r2, &i, "," );
	if( !GL_goodnum( valtok, &prec ) ) return( 0 );
	gg = atof( valtok );
	if( tolower( op[0] ) == 'i' && hh >= ff && hh <= gg ) return( 1 );
	else if( tolower( op[0] ) == 'o' && ( hh < ff || hh > gg )) return( 1 );
	else return( 0 );
	}

/* '!like' ... */
else if( GL_smemberi( op, "!like notlike" )) /* return( ! GL_slmember( r1, r2 ) ); */
	 return( abs( GL_wildcmp( r1, r2, strlen( r2 ), 0 )));

/* other list ops... */
for( i = 0, slen = strlen( r2 ); i < slen; i++ ) { if( r2[i] == listsep ) r2[i] = ' ' ; } /* change every comma to a space */

if( stricmp( op, "in" ) == 0 ) return( GL_smemberi( r1, r2 ) );
else if( GL_smemberi( op, "!in notin" )) return( ! GL_smemberi( r1, r2 ) );
else if( GL_smemberi( op, "inlike" )) return( GL_slmember( r1, r2 ) );
else if( GL_smember( op, "!inlike notinlike" )) return( ! GL_slmember( r1, r2 ) );

fprintf( stderr, "[%s?]", op );
return( -1 );
}


/* ========================= */
/* YIELD - determine data type, evaluate functions.
   		Yield is a recursive function. 
   		Returns -1 for bad expression */
static int
yield( v, t )
char v[];
int *t;
{
double atof(), a2f();
int p, status;
char tok[256];

/* if evalflag, v likely contains one or more unevaluated vars */


*t = -1;

/* if v is a $function call, evaluate it .. */
if( !nofunc && v[0] == '$' && (isalpha( (int) v[1] ) || v[1] == '$' ) ) {

	/* shsql always operates in nofunc mode.  This #ifdef avoids function-related
	   references in shsql-only applications.  QUISP, which uses condex for both
	   shsql and script processing, and needs the functions code, must be linked such
	   that tdhkit.a has precidence over libshsql.a
	*/

#ifndef SHSQL
	status = TDH_function_call( v, t, evalflag ); /* v will be modified here */
#else
	status = 1;
#endif
	if( status != 0 ) { 
		err( 1003, "function error in condex", v ); 
		return( -1 ); 
		}
	}

/* variable.. evaluate it.. */
else if( evalflag && v[0] == '@' && v[1] != '@' ) {
	status = TDH_getvalue( tok, &v[1], TDH_dat, TDH_recid );
	if( status == 0 ) strcpy( v, tok );
	/* else @var appears verbatim */
	}

/* determine basic type of v */
if( *t >= 0 ) ;   /* already known from function */
else if( GL_goodnum( v, &p ) ) *t = NUMBER;
else *t = ALPHA;


return( 1 );
}


/* ============================== */
/* ============================== */

/* CONDEX_LISTSEP - allow setting of list delimiter character in case comma is unacceptable */
int
TDH_condex_listsep( c )
char c;
{
listsep = c;
return( 0 );
}

/* ============================== */
/* CONDEX_NOFUNC - don't take special action on tokens beginning with dollar signs */
int
TDH_condex_nofunc( mode )
int mode;
{
nofunc = mode;
return( 0 );
}


/* =============================== */
/* CONDEX_MATCHSCORE - return most recent match score (-1 indicates not used) */
int
TDH_condex_matchscore()
{
if( !Matchscore_used ) return( -1 );
else return( Matchscore );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
