/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* Secondary script ops such as "sql" and "shell".  These ops can be multi-line and are 
 * subordinate to flow-of-control operators.
 */
#include "tdhkit.h"
#include <ctype.h>

extern int TDH_sqlcommand(), TDH_sinterp(), TDH_shellcommand();
extern int atoi();

#define SHELL 0
#define SQL 1

static char bigbuf[ MAXRECORDLEN ];
static int bblen;
static int prohibit_shell = 0;

int
TDH_secondaryops( buf, ss, recordid, data )
char *buf;
struct sinterpstate *ss;
char *recordid;
char data[][DATAMAXLEN+1];
{
char tok[ DATAMAXLEN+1 ];
int ix;
int buflen;
int stat;

buflen = strlen( buf );

strcpy( tok, "" );
sscanf( buf, "%s", tok );

if( ( strncmp( tok, "#sql", 4 )==0 && (tok[4] == '\0' || isdigit( (int) tok[4] ))) || strcmp( tok, "#shell" )==0 ) {  
	/* #sql [ load | processrows | dump | dumptab | dumphtml | dumpsilent] sql command [#endsql] */
	/* #shell [ #processrows | #dump | #dumptab | #dumphtml | #dumpsilent ] command [#endshell] */
	int ixhold, capmode, opmode;
	if( strncmp( tok, "#sql", 4 )==0 ) {
		opmode = SQL;
		capmode = 0;   /* if no #sql mode given, default is processrows */
		if( tok[4] == '\0' ) ss->dbc = 0;
		else ss->dbc =  tok[4] - '1';
		}
	else 	{
		opmode = SHELL;
		capmode = 'n';  /* if no #shell mode given, default is dump */
		TDH_valuesubst_settings( "omit_shell_meta", 1 );  /* omit shell metachars in conversion */
		}
	ix = 0;
	GL_getok( buf, &ix ); /* skip over 1st token..*/

	SQL_DBC:
	ixhold = ix;
	strcpy( tok, GL_getok( buf, &ix ) );  

	if( tok[0] == '\0' ) ; /* no 2nd token  - added scg 5/12/04 */

	else if( opmode == SQL && atoi( tok ) > 1 ) {	 /* allow channel number to be a separate token - added scg 2/24/04 */
								/* opmode == SQL added scg 5/12/04 */
		ss->dbc = atoi( tok ) - 1;
		goto SQL_DBC;
		}
	else if( tok[0] == '#' && atoi( &tok[1] ) > 1 ) {
		ss->dbc = atoi( &tok[1] ) - 1;
		goto SQL_DBC;
		}

	else if( strcmp( tok, "load" )==0 || strcmp( tok, "#load" )==0 ) {
		capmode = 'l';
		ixhold = ix;
		strcpy( tok, GL_getok( buf, &ix ) );  
		}

	else if( strncmp( tok, "dump", 4 )==0 || strncmp( tok, "#dump", 5 )==0 ) {  
		if( tok[0] == '#' ) capmode = tok[5];
		else capmode = tok[4];
		if( capmode == '\0' ) capmode = 'n'; /* default is nosep */
		ixhold = ix;
		strcpy( tok, GL_getok( buf, &ix ) );  
		}

	else if( strcmp( tok, "processrows" )==0 || strcmp( tok, "#processrows" )==0 ) {
		capmode = 0;
		ixhold = ix;
		strcpy( tok, GL_getok( buf, &ix ) );  
		}
		 

	if( tok[0] != '\0' ) {
		strcpy( bigbuf, &buf[ixhold+1] ); /* 1 line command.. */

		if( opmode == SHELL ) {
			/* don't allow single-line construct - so we can treat vars during scan for cgi security */
			/* return( err( 2488, "#shell: single line construct not allowed", "" )); */
			err( 2488, "#shell: single line construct not allowed", "" ); 
			return( SINTERP_END );
			}

		else if( opmode == SQL ) {
			ss->doingsqlresult = capmode;

			/* submit the sql.. */
			stat = TDH_sqlcommand( ss->dbc, bigbuf ); 
			if( stat != 0 ) {
				ss->doingsqlresult = 0;
				/* return( err( stat, "error on sql submit", "" )); */
				err( stat, "error on sql submit", "" );
				return( SINTERP_END );
				}
			}

		return( SINTERP_END ); /* no more calls to sec needed this time.. */
		}

	/* Note: all error returns before this point should return SINTERP_END, to avoid race condition
	 *		(after this point a new line is read by TDH_sinterp)  scg 1/25/08
	 */
		
	bblen = 0;
	while( 1 ) {
		stat = TDH_sinterp( buf, ss, recordid, data );
		if( stat != SINTERP_MORE ) return( stat );
		buflen = strlen( buf );
		strcpy( tok, "" );
		sscanf( buf, "%s", tok );

		if( strcmp( tok, "#endsql" )==0 ) {

			ss->doingsqlresult = capmode;  /* set ss->doingsqlresult to the capture mode (load, processrows, etc);
							  then this comes into play in sinterp.c */

			/* submit the sql.. */
			stat = TDH_sqlcommand( ss->dbc, bigbuf ); 
			if( stat != 0 ) {
				int jj;
				ss->doingsqlresult = 0;
				for( jj = 0; bigbuf[jj] != '\0'; jj++ ) if( bigbuf[jj] == '\n' ) { bigbuf[jj] = '\0'; break; }
				sprintf( buf, "sql error %d, first line is: %s", stat, bigbuf );
				return( stat );
				}

			return( SINTERP_END ); /* no more calls to sec needed this time */
			}

		else if( strcmp( tok, "#endshell" )==0 ) {
			ss->doingshellresult = capmode;

			/* submit the shell command.. */
			if( prohibit_shell ) return( err( 1231, "attempt to #shell but shell commands prohibited", "" ));
			else stat = TDH_shellcommand( bigbuf );
			if( stat != 0 ) ss->doingshellresult = 0;

			TDH_valuesubst_settings( "omit_shell_meta", 0 ); /* restore */

			return( SINTERP_END );
			}


		else	{
			/* add more sql to bigbuf.. */
			if( bblen + buflen > (MAXRECORDLEN-2) ) return( err( 1263, "missing #endsql or #endshell", "" ) );
			strcpy( &bigbuf[bblen], buf );
			bblen += buflen;
			continue;
			}
		}
	}


else 	{
	/* other #ops will just be ignored.. control returns to the caller */
	return( SINTERP_END_BUT_PRINT );  /* no more calls to sec needed this time */
	}
}

/* ================================ */
/* If mode is passed as 1, #shell commands are disabled (safety).  If mode is 0, #shell commands are re-enabled. */
int
TDH_prohibit_shell( mode )
int mode;
{
prohibit_shell = mode;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
