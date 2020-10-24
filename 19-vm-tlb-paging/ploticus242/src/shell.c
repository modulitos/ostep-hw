/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* SHELL.C  - script shell command interface */

#include "tdhkit.h"
#include <ctype.h>

extern int TDH_setvarcon(), TDH_setvar(), TDH_getvar(), TDH_setshellfdelim(), GL_deletechars();

#define NL 0
#define WS 1
#define TAB 2

static FILE *shellfp = NULL;
static char *fn[MAXITEMS];
static char namebuf[512]; /* was MAXRECORDLEN but seemed like overkill.. */
static int nfn = 0;
static int indelim = NL;
static int nrows = 0;
static int exitcode = 0;
static int fconvertflag = 0;   /* added 4/17/03 scg */

static int parsefields(), checkexit();


/* =================================== */
int
TDH_shell_initstatic()
{
shellfp = NULL;
nfn = 0;
indelim = NL;
nrows = 0;
exitcode = 0;
return( 0 );
}


/* =================================== */
int
TDH_shellcommand( command )
char *command;
{
FILE *popen();

nfn = 0;
nrows = 0;

#ifndef WIN32
strcat( command, "\necho \"%-exitcode-% $?\" \n" );
#endif

shellfp = popen( command, "r" );
if( shellfp == NULL ) return( 1 );

return( 0 );
}

/* =================================== */
int
TDH_shellreadheader( )
{
int stat;
char *s;

s = fgets( namebuf, SCRIPTLINELEN-1, shellfp );

stat = checkexit( s, namebuf );
if( stat != 0 ) return( stat );

parsefields( namebuf, fn, &nfn );
return( 0 );
}

/* =================================== */
int
TDH_shellresultrow( buf, fields, nfields, maxlen )
char *buf;
char *fields[MAXITEMS];
int *nfields;
int maxlen;
{
char *s;
int stat;

if( shellfp == NULL ) return( 1 );

s = fgets( buf, maxlen, shellfp );

stat = checkexit( s, buf );
if( stat != 0 ) {
	TDH_setshellfdelim( 0 ); /* reset shell delimiter to NL.. added scg 8/3/06 */
	return( stat );
	}

nrows++;

parsefields( buf, fields, nfields );

return( 0 );
}

/* ===================================== */
int
TDH_shellclose()
{
if( shellfp == NULL ) return( 1 );
pclose( shellfp ); shellfp = NULL;
return( 0 );
}

/* ===================================== */
/* SETDELIM - allow program to set/reset the field delimiter character.  
 *  Code may be one of 0 (NL)  1 (WS)  2 (TAB) */
int
TDH_setshellfdelim( code ) 
int code;
{
indelim = code;
return( 0 );
}

/* ===================================== */
static int
checkexit( s, buf )
char *s; /* return from an fgets */
char *buf;
{
if( s == NULL ) {
	pclose( shellfp ); shellfp = NULL;
	return( 1 );
	}
if( strncmp( buf, "%-exitcode-% ", 13 )==0 ) {
	sscanf( buf, "%*s %d", &exitcode );
	pclose( shellfp ); shellfp = NULL;
	return( 1 );
	}
return( 0 );
}

/* =================================== */
/* parse out shell fields */
static int
parsefields( buf, f, nf )
char *buf;
char *f[MAXITEMS];
int *nf;
{
int i, j, len, startfld, sp;

i = 0;
len = strlen( buf );
startfld = 1;
for( i = 0, j = 0; i < len; i++ ) {
	sp = 0;
	if( indelim == TAB && (buf[i] == '\t' || buf[i] == '\n' ) ) sp = 1;
	else if( indelim == NL && buf[i] == '\n' ) sp = 1;
	else if( indelim == WS ) sp = isspace( (int) buf[i] );
	if( !sp && startfld ) {
		f[j++] = &buf[i];
		startfld = 0;
		}
	if( sp ) {
		buf[i] = '\0';
		startfld = 1;
		}
	}
*nf = j;
return( 0 );
}


/* ========================================== */
/* SHFUNCTIONS - TDH script access to shell commands */

int
TDH_shfunctions( hash, name, arg, nargs, result, typ )
int hash;
char *name;
char *arg[];
int nargs;
char *result;
int *typ;
{
char *f[MAXITEMS];
char fname[50];
char buf[MAXRECORDLEN];
int i, n, len, stat;

*typ = 0; /* numeric */


if( hash == 1006 ) { /* $shellrow() - fetch a row of results.  
 		      * Return 0 = normal, 1 = no row fetched and no more results, >1 = error. */

	/* get next result row.. */
	NEXTROW:
	if( nargs > 1 && indelim == NL ) indelim = WS; /* more than one var given.. guess whitespace delim */
        stat = TDH_shellresultrow( buf, f, &n, SCRIPTLINELEN );
        if( stat == 0 ) {
		if( nfn > 0 ) { /* names already defined in a header.. */
			for( i = 0; i < nfn; i++ ) TDH_setvarcon( fn[i], f[i], fconvertflag );
			}
		else if( nargs > 0 && strcmp( arg[0], "#varvaluepair" )==0 ) { /* tag-value pair */
			strcpy( fname, f[0] );
			len = strlen( fname );
			if( len == 0 ) goto NEXTROW; /* blank name.. skip.. */
			if( fname[ len -1 ] == ':' ) fname[ len-1] = '\0';
			TDH_setvarcon( fname, f[1], fconvertflag );
			} 
		else if( nargs > 0 ) { /* names given as function arguments */
			for( i = 0; i < nargs; i++ ) {
				if( i >= n ) TDH_setvar( arg[i], "" );
				else TDH_setvarcon( arg[i], f[i], fconvertflag );
				}
			}
		else 	{    /* error - no name(s) specified */
			strcpy( result, "5690" );
			return( 0 );
			}
                }
        else for( i = 0; i < nfn; i++ ) TDH_setvar( fn[i], "" );

	/* check return status.. non-zero indicates no more rows.. */
	sprintf( result, "%d", stat );
	return( 0 );
	}

if( hash == 2569 ) { /* shellrowcount() - return number of rows presented or affected by last sql command. */
	sprintf( result, "%d", nrows );
	return( 0 );
	}

if( hash == 3084 ) { /* $shellstripchars( chars, varname1 .. varnamen ) - remove characters that could be dangerous
		      in shell commands.  Chars arg may be omitted to use a standard set of characters.  */

			/* DEPRECATED - this is now automatically done in value_subst when within #shell/#endshell */
	int start;
	if( isalpha( (int) arg[0][0] )) start = 0;
	else start = 1;
	for( i = start; i < nargs; i++ ) {
        	stat = TDH_getvar( buf, arg[i] );
		if( start == 0 ) GL_deletechars( "\"'`$\\;", buf );
        	else GL_deletechars( arg[0], buf );
        	stat = TDH_setvar( arg[i], buf );
		}
	sprintf( result, "0" );
	return( 0 );
	}

if( hash == 2554 ) { /* $shellreadheader() - load field name header */
	if( arg[0][0] == 't' ) indelim = TAB;   /* added scg 3/10/06 */
	else indelim = WS; 			/* added scg 3/10/06 */
	stat = TDH_shellreadheader();
	sprintf( result, "%d", stat );
	return( 0 );
	}


if( hash == 2686 ) { /* $shellfielddelim() */
	if( arg[0][0] == 'w' ) indelim = WS;
	else if( arg[0][0] == 't' ) indelim = TAB;
	else indelim = NL;
	sprintf( result, "0" );
	return( 0 );
	}

if( hash == 3953 ) { /* $shellfieldconvert() - specify conversions to perform on incoming fields */
	if( strcmp( arg[0], "shsql" )==0 ) fconvertflag = 1;
	else fconvertflag = 0;
	sprintf( result, "0" );
	return( 0 );
	}

if( hash == 2138 ) { /* $shellexitcode() */
	sprintf( result, "%d", exitcode );
	return( 0 );
	}


return( err( 197, "unrecognized function", name )); /* not found */

}
/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
