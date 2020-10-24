/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>  /* for mode_t */

extern int TDH_err(), TDH_value_subst(), TDH_getvalue();
extern int GL_getchunk(), GL_goodnum(), GL_member(), GL_getseg();
extern int GL_substitute(), GL_ranger(), GL_wildcmp(), GL_contains(), GL_addmember(), GL_substring();
extern int GL_autoround(), TDH_setvalue(), GL_make_unique_string(), GL_changechars(), GL_deletechars(), GL_deletemember();
extern int GL_urlencode(), GL_urldecode();
extern int GL_commonmembers(), GL_checksum_functions();

extern int DT_datefunctions(), DT_timefunctions();
extern int TDH_shfunctions(), TDH_showvars(), TDH_errmode();
extern int atoi(), sleep(), geteuid(), getegid(), chmod(), chdir(), unlink(), rename(), mkdir(); /* sure thing or return value not used */
/* extern char *crypt(); */ /* linux may require -lcrypt */


#ifdef PLOTICUS
  extern int PL_custom_function();
#else
  extern int custom_function();
#endif

#ifdef TDH_DB
  extern int TDH_dbfunctions();
#endif

#define PATH_SLASH '/'
#define MAXARGS 64

#define NUMBER 0
#define ALPHA 1

extern char *TDH_dat, *TDH_recid;
#define err(a,b,c) 		TDH_err(a,b,c)

/* the following is used by $tmpfilename.. */
extern char TDH_tmpdir[];
extern char TDH_scriptdir[];
#if TDH_DB == 2
extern char SHSQL_projdir[];
#endif

static int evalflag;
static char Sep[5] = ","; /* for args that are commalists */
static int textsaved = 0;
static int eval_function();

char *GL_getok();

/* =============================================== */
int
TDH_functioncall_initstatic()
{
strcpy( Sep, "," );
textsaved = 0;
return( 0 );
}


/* =============================================== */
/* FUNCTION_CALL - parse function name, and any arguments, then call
   function evaluator.  V is returned with the result value. */

int
TDH_function_call( v, typ, eval )
char *v;
int *typ;  /* sent back.. describes basic type of result.. either NUMBER or ALPHA */
int eval;  /* 1 if unevaluated @variables possible, 0 otherwise */
{
int i, ix, len, status;
char name[80];
char tok[256];
char argbuf[ 1024 ];
int nargs;
int alen;
char *arg[MAXARGS];

evalflag = eval;

/* get function name */
ix = 0;
len = strlen( v );
GL_getchunk( name, v, &ix, " (" );
if( name[0] == '\0' ) {
	err( 1601, "$function expected, nothing found", "" );
	return( 1 );
	}
/* get function parameters, parsing them into arg pointer array */
alen = 0;
for( i = 0; i < MAXARGS; i++ ) {
	if( ix >= len ) break;
	GL_getchunk( tok, v, &ix, " ,()" );
	arg[i] = &argbuf[ alen ];
	if( evalflag ) TDH_value_subst( &argbuf[ alen ], tok, TDH_dat, TDH_recid, 0, 0 );
	else strcpy( &argbuf[ alen ], tok );
	alen += strlen( &argbuf[ alen ] ) + 1;
	}
nargs = i;

nargs--; /* above parsing always picks up an extra arg for closing paren */

status = eval_function( name, arg, nargs, v, typ );
return( status );
}

/* ======================================== */
/* EVAL_FUNCTION - implement general purpose condex functions. 

	Return 0 if ok, 1 if named function doesn't exist or some other error. 
	Functions that produce a result which should always be taken as
   	alpha rather than number, should append a space to the end of the result. 

   NOTE........... to generate new hash numbers use tdhkit/bin/wordhash 

*/
static int
eval_function( name, arg, nargs, result, typ )
char *name; 	/* function name */
char *arg[];    /* arg list */
int nargs;	/* number of args */
char *result;	/* the result value gets copied into this string */
int *typ;	/* basic type of result */
{
int stat;
double accum;
char curop;
double atof();
int i, j;
char tok[256];
char *s;
int hash;
char fmt[40];

/* divert custom functions.. */
if( name[0] == '$' && name[1] == '$' ) goto CUSTOM;

/* generate a hash key by adding up chars in name. (skip leading $) */  
s = &name[1];
hash = s[0];
for( i = 1; s[i] != '\0'; i++ ) hash += ( i * (s[i] - 80) );


/* use ~/scg/wordhash to check new function names.. */

*typ = ALPHA; /* fallback */


if( hash < 1000 ) {
  if( hash < 200 ) {
	if( hash == 165 ) {     /* $def( varname ) - return 1 if variable has been defined, 0 if not */
		if( TDH_getvalue( result, arg[0], TDH_dat, TDH_recid ) == 0 ) sprintf( result, "1" );
		else sprintf( result, "0" );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 179 ) {	/* $ref( varname ) - return the contents of variable varname */
		TDH_getvalue( result, arg[0], TDH_dat, TDH_recid );
		return( 0 );
		}

	if( hash == 189 ) {   /* $len( s ) - return string length of s */
		LEN:
		sprintf( result, "%d", (int) strlen( arg[0] ) );
		*typ = NUMBER;
		return( 0 );
		}
	}

  else if( hash < 300 ) {
	if( hash == 203 ) {   /* $inr(n,low,hi) - see if n is in range low to high */
		i = 0;
		if( !GL_goodnum( arg[0], &i ) ) strcpy( result, "0" );
		else if( atof( arg[0] ) >= atof( arg[1] ) && atof( arg[0] ) <= atof( arg[2] ) ) strcpy( result, "1" );
		else strcpy( result, "0" );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 259 ) { /* $char(n,s) - get the nth character of string s */
		int n;
		n = atoi( arg[0] );
		if( n < 1 || n > (strlen( arg[1] )-1) ) return( -1 );
		sprintf( result, "%c", arg[1][ n-1 ] );
		return( 0 );
		}

	if( hash == 270 ) {  /* $math(what,f,[g],[fmt]) - return math function of f */
		/* mag and pow added scg 9/30/03 */
		double fabs(), sqrt(), exp(), log(), fmod(), pow();
		if( nargs == 4 ) strcpy( fmt, arg[3] );
		else if( nargs == 3 && arg[3][0] == '%' ) strcpy( fmt, arg[2] );
		else strcpy( fmt, "%g" );
		if( strcmp( arg[0], "mag" )==0 ) sprintf( result, fmt, atof( arg[1] ) * pow( 10.0, atof( arg[2] ) ) );
		else if( strcmp( arg[0], "abs" )==0 ) sprintf( result, fmt, fabs( atof( arg[1] ) ) );
		else if( strcmp( arg[0], "mod" )==0 ) sprintf( result, fmt, fmod( atof( arg[1] ), atof( arg[2] ) ) );
		else if( strcmp( arg[0], "div" )==0 ) sprintf( result, "%d", atoi( arg[1] ) / atoi( arg[2] ) );
		else if( strcmp( arg[0], "sqrt" )==0 ) sprintf( result, fmt, sqrt( atof( arg[1] ) ) );
		else if( strcmp( arg[0], "exp-1" )==0 ) sprintf( result, fmt, exp( atof( arg[1] ) )-1.0 );
		else if( strcmp( arg[0], "log+1" )==0 ) sprintf( result, fmt, log( atof( arg[1] ) )+1.0 );
		else if( strcmp( arg[0], "pow" )==0 ) sprintf( result, fmt, pow( atof( arg[1] ), atof( arg[2] ) ) );
		/* future: others may be added here... might want a format arg like $arith() uses.. */
		*typ = NUMBER;
		return( 0 );
		}
	if( hash == 262 ) goto EXT_TIME; /* $time() */
	}
  else if( hash < 500 ) {
	if( hash == 354 ) {  /* $chmod( file, mode ) */   /* support common modes.. */
		if( strcmp( arg[1], "664" )==0 ) chmod( arg[0], 00664 );  
		else if( strcmp( arg[1], "644" )==0 ) chmod( arg[0], 00644 );  
		else if( strcmp( arg[1], "666" )==0 ) chmod( arg[0], 00666 );  
		else if( strcmp( arg[1], "444" )==0 ) chmod( arg[0], 00644 );  
		else if( strcmp( arg[1], "660" )==0 ) chmod( arg[0], 00660 );  
		else if( strcmp( arg[1], "640" )==0 ) chmod( arg[0], 00640 );  
		else if( strcmp( arg[1], "755" )==0 ) chmod( arg[0], 00755 );  
		else if( strcmp( arg[1], "775" )==0 ) chmod( arg[0], 00775 );  
		return( 0 );
		}
	if( hash == 374 ) {  /* chdir( dir ) */
		chdir( arg[0] );
		return( 0 );
		}
	if( hash == 385 ) { /* $arith() and $arithl() - L to R arithmetic expression evaluator */
		/* improved to accomodate scientific notation operands eg 1.27e-4  .... scg 7/29/04 */
		int j;
		ARITH:
		i = 0;
		accum = 0.0;
		curop = '+';
		s = arg[0];
		strcpy( fmt, arg[1] );
		if( s[i] == '-' ) { tok[0] = '-'; i++; } /* check for unary minus on first arg */
		else tok[0] = '+';
		while( 1 ) {

			/* parse next numeric operand */
			for( j = 1; s[i] != '\0'; i++ ) {
				if( GL_member( s[i] , "+-/*%" ) && ( i > 0 && tolower(s[i-1]) != 'e' )) break;
				else tok[j++] = s[i];
				}
			tok[j] = '\0';
					
			if( tok[1] == '\0' ) break; /* end of expression */

			/* check that operand is a valid numeric (allow w/ $arithl.. atof() will return 0.0 below) */
			if( ! GL_goodnum( tok, &stat ) && name[6] != 'l' ) { err( 1603, "arith: bad value", tok ); return( 1 ); }

			/* do arithmetic operation.. */
			if( curop == '+' ) accum = accum + atof( tok );
			else if( curop == '-' ) accum = accum - atof( tok );
			else if( curop == '/' ) {
				if( atof( tok ) == 0.0 ) {  /* added scg 8/4/01 */
					err( 1605, "arith: divide by zero", "" );
					return( 1 );
					}
				accum = accum / atof( tok );
				}
			else if( curop == '*' ) accum = accum * atof( tok );
			else if( curop == '%' ) accum = (int)(accum) % (int)(atof( tok ));
			else { err( 1604, "arith:bad operator", "" ); return( 1 ); }

			/* get next arith operator..  and check for unary minus on next operand, if any.. */
			curop = s[i];
			if( s[i+1] == '-' ) { tok[0] = '-'; i++; }
			else tok[0] = '+';

			i++;
			}
		if( nargs > 1 ) sprintf( result, fmt, accum );
		else sprintf( result, "%g", accum );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 387 ) { /* $mkdir( newdirname ) */
		stat = mkdir( arg[0], 00755 );
		if( stat == 0 ) { strcpy( result, "0" ); return( 0 ); }
		else { strcpy( result, "1" ); return( 1 ); }
		}

	if( hash == 399 ) { /* $capit( str ) - capitalize first letter in string and return it */
		arg[0][0] = toupper( arg[0][0] );
		sprintf( result, "%s", arg[0] );
		return( 0 );
		}

	if( hash == 400 ) goto EXT_TIME;     /*  $tomin() */
	if( hash == 402 ) goto EXT_DATE;     /*  $today() */

	if( hash == 438 ) {     /*  $count(str,list) - count the number of times 'str' appears in 'list'.  */
				/* if str is '*', result is the number of members in list */
		int n, j, len;
		n = 0;
		for( j = 1; j < nargs; j++ ) {  /* allow comma lists in several args.. */
			s = arg[j];
			i = 0;
			len = strlen( s );
			while( i < len ) {
				GL_getseg( tok, s, &i, Sep );   /* parse list */
				/* if( strlen( tok ) < 1 ) break; */
				if( strcmp( arg[0], "*" )==0 ) n++;
				else if( strcmp( tok, arg[0] )==0 ) n++;
				}
			}
		sprintf( result, "%d", n );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 444 ) {   /* $change(s1,s2,line) - in line, substitute every occurance of s1 with s2. */
		stat = GL_substitute( arg[0], arg[1], arg[2] ); /* modifies argbuf */
		sprintf( result, "%s", arg[2] );
		return( 0 );
		}

	if( hash == 467 ) {   /* rename( pathname, newpathname )  */
		if( nargs != 2 ) return( -1 );
		rename( arg[0], arg[1] );
		strcpy( result, "0" );
		return( 0 );
		}

	if( hash == 476 ) {   /* $expand(s) - expand all vars embedded in s */
		TDH_value_subst( result, arg[ 0 ], TDH_dat, TDH_recid, 0, 0 );
		return( 0 );
		}

	if( hash == 492 ) goto EXT_DATE;  /* julian() */

#ifndef WIN32
	if( hash == 497 ) {  /* $whoami() - return a string containing euid,egid */
		sprintf( result, "%d,%d", geteuid(), getegid() );
		return( 0 );
		}
#endif
	}

  else if( hash < 600 ) {

	if( hash == 503 ) { /* $isleep() */
		sleep( atoi( arg[0] ) );
		strcpy( result, "" );
		return( 0 );
		}

	if( hash == 507 ) {  /* $upperc(string) - result is the lower-case equivalent of string */
		int slen;
		strcpy( result, arg[0] );
		for( i = 0, slen = strlen( result ); i < slen; i++ ) result[i] = toupper( result[i] );
		return( 0 );
		}

	if( hash == 511 ) { /* $lowerc(string) - result is the lower-case equivalent of string */
		int slen;
		strcpy( result, arg[0] );
		for( i = 0, slen = strlen( result ); i < slen; i++ ) result[i] = tolower( result[i] );
		return( 0 );
		}

	if( hash == 514 ) { /* $ranger( rangespec )  - take a rangespec and return list of all members */
		int ilist[256], nlist, rlen;
		if( !isdigit( (int) arg[0][0] ) ) { /* just return the arg without expanding it.. */
			strcpy( result, arg[0] );
			return( 0 ); 
			}
		nlist = 255;
		strcpy( result, "" );
		stat = GL_ranger( arg[0], ilist, &nlist );
		if( stat != 0 ) return( stat );
		rlen = 0;
		for( i = 0; i < nlist; i++ ) {
			sprintf( tok, "%d,", ilist[i] );
			strcpy( &result[rlen], tok );
			rlen += strlen( tok );
			}
		result[rlen] = '\0';
		return( 0 );
		}

	if( hash == 520 ) { /* $random() - return a random number between 0 and 1 */
		double GL_rand();
		sprintf( result, "%g", GL_rand() );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 523 ) { /* $ntoken( n, s, [c] ) - 
			if c is unspecified return the nth whitespace-delimited token in s.
			if c is given (single char) it is understood to be the delimiter char, return the nth field in s.
			c can also be given as a set of delimiter chars, eg "|,"
			*/
		int ix, n;
		n = atoi( arg[0] );
		ix = 0;
		if( nargs == 3 ) {
			for( i = 0; i < n; i++ ) GL_getseg( result, arg[1], &ix, arg[2] );  /* delimited by specified char */
			}
		else	{
			for( i = 0; i < n; i++ ) strcpy( result, GL_getok( arg[1], &ix ) );  /* whitespace delmited */
			}
		return( 0 );
		}

	if( hash == 524 ) { /* $strcat(s,t) - concatenate s and t */
		sprintf( result, "%s%s", arg[0], arg[1] );
		return( 0 );
		}

	if( hash == 525 ) goto ARITH;   /* $arithl() */

	if( hash == 533 ) { /* unlink( pathname ) */
		if( nargs != 1 ) return( -1 );
		unlink( arg[0] );
		return( 0 );
		}

	if( hash == 537 ) goto LEN;	/* $strlen() - same as $len() */
	if( hash == 540 ) goto EXT_DATE; /* $dateadd() */
	if( hash == 551 ) {  /* $fflush() */
		fflush( stdout );
		strcpy( result, "0" );
		return( 0 );
		}
	if( hash == 552 ) { /* $strcmp(s,t) - return difference of s and t */
		sprintf( result, "%d", strcmp( arg[0], arg[1] ) );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 569 ) { 	/*  $getenv(envvarname) - get the contents of shell environment variable */
		char *getenv(), *s;
		s = getenv( arg[0] );
		if( s == NULL ) strcpy( result, "" );
		else 	{ 
			strncpy( result, s, 255 );
			result[ 255 ] = '\0';
			}
		return( 0 );
		}

	}

  else if( hash < 800 ) {

	if( hash == 621 ) goto EXT_TIME;  /* $timesec() */
	if( hash == 625 ) goto EXT_SQL; /* $sqlrow() */

	if( hash == 649 ) { /* $nmember(n,list) - return nth member of commalist.  */
		int j, n;
		n = atoi( arg[0] );
		s = arg[1];
		j = 0; i = 0;
		while( 1 ) {
			j++;
			GL_getseg( tok, s, &i, Sep );  /* parse list */
			if( tok[0] == '\0' ) { strcpy( result, "" ); break; }
			else if( j == n ) { sprintf( result, "%s", tok ); break; }
			}
		return( 0 );
		}


	if( hash == 665 ) goto EXT_DATE;  /* $datecmp() */
	if( hash == 673 ) {   /* $wildcmp( s, t ) .. mostly for testing wildcmp() */
		sprintf( result, "%d", GL_wildcmp( arg[0], arg[1], strlen( arg[1] ), 0 ) );
		return( 0 );
		}
	if( hash == 706 ) goto EXT_TIME;  /* $frommin() */
	if( hash == 753 ) goto EXT_TIME;  /* $timediff() */
	if( hash == 795 ) goto EXT_DATE;  /* $daysdiff() */

	}

  else if( hash < 1000 ) {

/* #ifdef UNIX
 * #ifndef PLOTICUS
 *	if( hash == 811 ) { // $encrypt(s,salt) 
 *		if( arg[1][0] == '\0' ) strcpy( arg[1], "sG" );
 *		sprintf( result, "%s", crypt( arg[0], arg[1] ) );  // linux may require -lcrypt 
 *		return( 0 );
 *		}
 * #endif
 * #endif
 */

	if( hash == 827 ) {  /* $stripws(s,mode) - remove white space from string.. 2 modes */
		int state;

		if( strcmp( arg[1], "any" )==0 ) {  /*  mode == any ... remove whitespace anywhere in string - added scg 5/10/06 */
			for( i = 0, j = 0; arg[0][i] != '\0'; i++ ) {
				if( isspace( (int) arg[0][i] )) continue;
				else result[j++] = arg[0][i];
				}
			result[j] = '\0';
			return( 0 );
			}

		/* otherwise, remove leading and trailing whitespace only (original use) */
		for( i = 0, j = 0, state = 0; arg[0][i] != '\0'; i++ ) {
			if( state == 0 && isspace( (int) arg[0][i] ) ) continue;
			else	{
				result[j++] = arg[0][i];
				state = 1;
				}
			}
		result[j] = '\0';
		/* now strip off trailing blanks.. */
		for( i = j; i >= 0; i-- ) {
			if( !isspace( (int) result[i] ) && result[i] != '\0' ) {
				result[i+1] = '\0';
				break;
				}
			}
		return( 0 );
		}

	if( hash == 881 ) {  /* $isnumber(s) - result is 1 if argument is a valid number, 0 if not. */
		if( name[1] == 'y' ) goto EXT_DATE; /* $yearsold() */
		*typ = NUMBER;
		if(  arg[0][0] == '\0' ) { sprintf( result, "0" ) ; return( 0 ); }
		stat = GL_goodnum( arg[0], &i );
		sprintf( result, "%d", stat );
		return( 0 );
		}

	if( hash == 881 ) goto EXT_DATE; /* $yearsold() */

	if( hash == 913 ) goto EXT_SQL; /* $sqltabdef() */

	if( hash == 916 ) { /* $contains(clist,s) - if string s contains any of chars in clist, result is position
 			     *	(first=1) of first occurance.  Result is 0 if none found. */
		sprintf( result, "%d", GL_contains( arg[0], arg[1] ) );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 940 ) { /* $urldecode( s ) - perform url DEcoding on s */
		GL_urldecode( arg[0], result );
		return( 0 );
		}

	if( hash == 942 ) { /* $makelist( s ) - convert white-space separated list to commalist */
		int ix;
		strcpy( result, "" );
		ix = 0;
		while( 1 ) {
			strcpy( tok, GL_getok( arg[0], &ix ) );
			if( tok[0] == '\0' ) break;
			if( tok[ strlen( tok ) - 1 ] == ',' ) tok[ strlen( tok ) - 1 ] = '\0'; 
			GL_addmember( tok, result );
			}
		return( 0 );
		}

	if( hash == 963 ) { /* $cleanname( s ) - remove everything but chars, digits, and underscores.. */
		strcpy( result, "" );
		for( i = 0, j = 0; arg[0][i] != '\0'; i++ ) {
			if( isalnum( (int) arg[0][i] ) || arg[0][i] == '_' ) result[j++] = arg[0][i];
			}
		result[j] = '\0';
		return( 0 );
		}
	if( hash == 979 ) { /* $urlencode( s ) - perform url encoding on s */
		GL_urlencode( arg[0], result );
		return( 0 );
		}

	if( hash == 992 ) goto EXT_DATE; /* $datevalid() */

	if( hash == 997 ) goto EXT_SQL; /* $sqlerror() */
	}
  }
else {

  if( hash < 1200 ) {

	if( hash == 1000 ) { /* $addmember(newmem,list) - append newmem to the end of comma-delimited list. */
		strcpy( result, arg[1] );
		GL_addmember( arg[0], result ); 
		return( 0 );
		}

	if( hash == 1002 ) goto EXT_TIME; /* $timevalid() */

	if( hash == 1004 ) {  /* $showvars() - print a list of all existing variable names to stdout, for debugging - added scg 7/2/07 */
		if( arg[0][0] == 'v' ) TDH_showvars(1); /* with values */
		else TDH_showvars(0); /* names only */
		return( 0 );
		}

	if( hash == 1006 ) goto EXT_SH;   /* $shellrow() */

	if( hash == 1011 ) { /* $numgroup( f, h, mode )   truncate f to the nearest multiple of h that is less than f.  
 			* mode may be either low, middle, or high.   For example, if f is 73 and h is 10, function
			* returns 70, 75, or 80 for modes low, middle, high respectively */
		double f, h, GL_numgroup();
		f = atof( arg[0] );
		h = atof( arg[1] );
		sprintf( result, "%g", GL_numgroup( f, h, arg[2] ) );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 1053 ) goto EXT_DATE; /* $jultodate() */

	if( hash == 1071 ) {  /* $errormode( mode )   mode may be either "stdout" or "stderr" */
		TDH_errmode( arg[0] );
		return( 0 );
		}

	if( hash == 1085 ) {  /* $textsaved()  */
		if( textsaved ) strcpy( result, "1" );
		else strcpy( result, "" );
		return( 0 );
		}

	if( hash == 1151 ) { /* $substring(s,from,nchar) - result is a substring, e.g. $substring(abcde,3,99) --> cde */
		GL_substring( result, arg[0], atoi( arg[1] ), atoi( arg[2] ) );
		return( 0 );
		}

	if( hash == 1168 ) goto EXT_SQL; /* $sqlprefix() */

	if( hash == 1182 ) { /* $autoround(val,d) - round val to reasonable precision */
		strcpy( result, arg[0] );
		GL_autoround( result, atoi( arg[1] ) );
		*typ = NUMBER;
		return( 0 );
		}
	
	}

  else if( hash < 1400 ) {

	if( hash == 1205 ) { /* $htmlquote( v1, .. vN ) - convert embedded quotes (") to &quot; for all variables given */
		for( i = 0; i < nargs; i++ ) {
        		stat = TDH_getvalue( tok, arg[i], TDH_dat, TDH_recid );
        		GL_substitute( "\"", "&quot;", tok );
        		stat = TDH_setvalue( arg[i], tok, TDH_dat, TDH_recid );
			}
		sprintf( result, "0" );
		return( 0 );
		}

	if( hash == 1215 ) goto EXT_DATE; /* $dategroup() */
	if( hash == 1252 ) goto EXT_DATE; /* $formatdate() */

	if( hash == 1269 ) { /* $uniquename() - result is a unique name based on date, time, and pid() */
		GL_make_unique_string( result, 0 );
		return( 0 );
		}

	if( hash == 1293 ) goto EXT_DATE; /* $todaysdate() */
	if( hash == 1348 ) goto EXT_TIME; /* $formattime() */
	if( hash == 1352 ) goto EXT_DATE; /* $setdatefmt() */

#ifdef CUT
	if( hash == 1380 ) { /* $fuzzymatch(s,t,tightness) */
		stat = GL_fuzzymatch( arg[0], arg[1], strlen(arg[1]), atoi( arg[2] ) );
		sprintf( result, "%d", stat );
		*typ = NUMBER;
		return( 0 );
		}
#endif
	
	if( hash == 1397 ) goto EXT_TIME; /* $settimefmt() */

	}

  else if( hash < 1600 ) {

	if( hash == 1461 ) { /* $extractnum( s ) - extract the first numeric entity embedded anywhere in s */
		int len;
		for( i = 0, len = strlen( arg[0] ); i < len; i++ ) if( isdigit( (int) arg[0][i] ) ) break;
		if( i == len ) strcpy( result, "" );
		else sprintf( result, "%g", atof( &arg[0][i] ) );
		return( 0 );
		}

	if( hash == 1458 ) { /* $tmpfilename(tag) - return a tmp file name using tmpdir from config file */
		GL_make_unique_string( tok, 0 );
		if( arg[0][0] != '\0' ) strcat( arg[0], "." );
		sprintf( result, "%s%c%s%s", TDH_tmpdir, PATH_SLASH, arg[0], tok ); 
		return( 0 );
		}


	if( hash == 1518 ) { /* $changechars(clist,s,newchar) - if string s contains any of chars in clist, 
			      * change that character to newchar */
		strcpy( result, arg[1] );
		GL_changechars( arg[0], result, arg[2] );
		/* sprintf( result, "%s", tok ); */
		return( 0 );
		}

	if( hash == 1523 ) goto EXT_SQL;  /* $sqlwritable() */

	if( hash == 1528 ) { /* $fileexists( dir, name ) - return 1 if the requested file can be opened, 0 if not.  
			      * dir is a symbol indicating what directory name is relative to (see docs) */
		FILE *testfp;
		if( strcmp( arg[0], "/" )==0 ) sprintf( tok, "%s", arg[1] );
		else if( strcmp( arg[0], "scriptdir" )==0 ) sprintf( tok, "%s%c%s", TDH_scriptdir, PATH_SLASH, arg[1] );
#if TDH_DB == 2
		else if( strcmp( arg[0], "datadir" )==0 ) sprintf( tok, "%s%cdata%c%s", 
								SHSQL_projdir, PATH_SLASH, PATH_SLASH, arg[1] );
#endif
		else if( strcmp( arg[0], "tmpdir" )==0 ) sprintf( tok, "%s%c%s", TDH_tmpdir, PATH_SLASH, arg[1] );
		else return( err( 7923, "fileexists: unrecognized dir", arg[0] ) );
		testfp = fopen( tok, "r" );
		if( testfp == NULL ) strcpy( result, "0" );
		else	{
			strcpy( result, "1" );
			fclose( testfp );
			}
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 1563 ) { /* $deletechars(clist,s) - if string s contains any of chars in clist, 
			      * delete that character */
		strcpy( tok, arg[1] );
		GL_deletechars( arg[0], tok );
		sprintf( result, "%s", tok );
		return( 0 );
		}
	}

  else if( hash < 2000 ) {

	if( hash == 1604 ) goto EXT_SQL; /* $sqlpushrow() */

	if( hash == 1607 ) goto EXT_SQL; /* $sqlgetnames */

	if( hash == 1625 ) { /* $formatfloat(n,format) */
		sprintf( result, arg[1], atof( arg[0] ) );
		return( 0 );
		}

	if( hash == 1708 ) { /* $htmldisplay( var1, .. varN ) - convert embedded HTML problem chars (<,>,&) 
			      * to their html esc sequences for all variables given */
		/* char *strchr(); */
		for( i = 0; i < nargs; i++ ) {
        		stat = TDH_getvalue( tok, arg[i], TDH_dat, TDH_recid );
			if( strchr( tok, '&' ) != NULL ) GL_substitute( "&", "&amp;", tok );
			if( strchr( tok, '<' ) != NULL ) GL_substitute( "<", "&lt;", tok );
			if( strchr( tok, '>' ) != NULL ) GL_substitute( ">", "&gt;", tok );
        		stat = TDH_setvalue( arg[i], tok, TDH_dat, TDH_recid );
			}
		sprintf( result, "0" );
		return( 0 );
		}

	if( hash == 1741 ) { /* $homogeneous( s ) - if all members of s are same, return 1 */
		int ix;
		char firsttok[256];
		ix = 0;
		*typ = NUMBER;
		GL_getseg( firsttok, arg[0], &ix, Sep );  /* parse list */
		if( firsttok[0] == '\0' ) { sprintf( result, "0" ); return( 0 ); }
		while( 1 ) {
			GL_getseg( tok, arg[0], &ix, Sep );  /* parse list */
			if( tok[0] == '\0' ) break;
			if( strcmp( tok, firsttok )!=0 ) { sprintf( result, "0" ); return( 0 ); }
			}
		sprintf( result, "1" );
		return( 0 );
		}

	if( hash == 1781 ) { /* counttokens( s [, c] ) - return number of ws-delimited [or c-delimited] tokens in s */
		int ix;
		ix = 0;
		if( nargs == 2 ) for( i = 0; ; i++ ) { 
				GL_getseg( result, arg[0], &ix, arg[1] ); 
				if( result[0] == '\0' ) break;
				}
		else if( nargs == 1 ) {
			for( i = 0; ; i++ ) { if( strcmp( GL_getok( arg[0], &ix ), "" ) == 0 ) break; }
			}	
		*typ = NUMBER;
		sprintf( result, "%d", i );
		return( 0 );
		}

	if( hash == 1788 ) { /* $deletemember( mem, list ) - delete member(s) matching mem from list.
							mem may contain wild cards */
		GL_deletemember( arg[0], arg[1], result );
		return( 0 );
		}

	if( hash == 1882 ) goto EXT_SQL;  /* $sqlrowcount() */

	if( hash == 1983 ) goto EXT_DATE; /* $setdateparms() */
	}

  else	{
	if( hash == 2138 ) goto EXT_SH; /* $shellexitcode() */
	if( hash == 2155 ) goto EXT_CHKSUM; /* $checksumnext() */
	if( hash == 2182 ) goto EXT_CHKSUM; /* $checksumvalid() */

	if( hash == 2268 ) { /* $commonmembers( list1, list2 ) - return # of members in common between 2 lists */
		int mode;
		if( strcmp( arg[2], "count" )==0 ) mode = 0;
		else mode = 1;
		sprintf( result, "%d", GL_commonmembers( arg[0], arg[1], mode ) );
		*typ = NUMBER;
		return( 0 );
		}

	if( hash == 2412 ) goto EXT_CHKSUM; /* $checksumencode() */
	if( hash == 2554 ) goto EXT_SH; /* $shellreadheader() */
	if( hash == 2569 ) goto EXT_SH; /* $shellrowcount() */
	if( hash == 2686 ) goto EXT_SH; /* $shellfielddelim() */

	if( hash == 2831 ) goto EXT_SQL;  /* $sqlstripprefix() */
	if( hash == 3084 ) goto EXT_SH;  /* $shellstripchars() */
	if( hash == 3953 ) goto EXT_SH;  /* $shellfieldconvert() */

	}
  }

/* if we reach here, we have a custom function.. */
CUSTOM:
#ifdef PLOTICUS
  if( name[1] == '$' ) stat = PL_custom_function( &name[2], arg, nargs, result, typ );
  else stat = PL_custom_function( &name[1], arg, nargs, result, typ );
#else
  if( name[1] == '$' ) stat = custom_function( &name[2], arg, nargs, result, typ );
  else stat = custom_function( &name[1], arg, nargs, result, typ );
#endif
if( stat != 0 ) err( 1602, "unrecognized function", name );


return( 0 );


EXT_DATE:
return( DT_datefunctions( hash, name, arg, nargs, result, typ ) );

EXT_TIME:
return( DT_timefunctions( hash, name, arg, nargs, result, typ ) );

EXT_SQL:
#ifdef TDH_DB
return( TDH_dbfunctions( hash, name, arg, nargs, result, typ ) );
#endif

EXT_SH:
return( TDH_shfunctions( hash, name, arg, nargs, result, typ ) );
/*was: return( TDH_shfunctions( hash, name, arg, nargs, result, typ, TDH_dat, TDH_recid ) ); */

EXT_CHKSUM:
#ifndef PLOTICUS
  return( GL_checksum_functions( hash, name, arg, nargs, result, typ ) );
#else
  return( -1 );
#endif

}

/* ==================== */
int
TDH_function_listsep( sepchar )
char sepchar;
{
Sep[0] = sepchar;
Sep[1] = '\0';
return( 0 );
}

/* ====================== */
int
TDH_function_set( what, val )
char *what;
int val;
{
if( strcmp( what, "textsaved" )==0 ) textsaved = val;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
