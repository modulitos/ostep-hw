/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* error msg handler */

#include <stdio.h>
#include <string.h>
extern int GL_sysdate( int *mon, int *day, int *yr );
extern int GL_systime( int *hr, int *min, int *sec );
extern int GL_slmember( char *str, char *list );

static char emode[20] = "stderr";
static char progname[80] = "";
static char errlog[256] = "";
static int progsticky = 0;
static FILE *errfp = NULL;

/* ========================================== */
int
TDH_err_initstatic()
{
strcpy( emode, "stderr" );
strcpy( progname, "" );
strcpy( errlog, "" );
errfp = NULL;
return( 0 );
}

/* ========================================== */

int
TDH_err( errno, msg, parm )
int errno;
char *msg, *parm;
{
char op[4], cp[4];
char *getenv();
int say_error;

strcpy( op, "" );
strcpy( cp, "" );
if( parm[0] != '\0' ) {
	strcpy( op, "(" );
	strcpy( cp, ")" );
	}

say_error = 1;
#ifndef BAREBONES
  if( GL_slmember( msg, "note:* warning:*" )) say_error = 0;
#endif


if( strcmp( emode, "cgi" )==0 ) {
	/* first \n is important in case it hasn't been delivered yet by quisp.. (http content-type statement) */
	printf( "\n<br><h4>%s %d: %s %s%s%s</h4><br>\n", (say_error)?"Error":"", errno, msg, op, parm, cp );
	fflush( stdout );
	/* took out progname */
	}
else if( strcmp( emode, "stderr" )==0 ) 
	fprintf( stderr, "%s: %s %d: %s %s%s%s\n", progname, (say_error)?"error":"", errno, msg, op, parm, cp );

else 	{
	fprintf( errfp, "%s: %s %d: %s %s%s%s\n", progname, (say_error)?"error":"", errno, msg, op, parm, cp );
	fflush( errfp );
	}

/* if an error log file was specified, write a line to this file .. */

#ifndef BAREBONES

  if( errlog[0] != '\0' ) {
	FILE *logfp;
	int mon, day, yr, hr, min, sec;
	char *qs, *ref, *host, *ip;
	GL_sysdate( &mon, &day, &yr );
	GL_systime( &hr, &min, &sec );

	logfp = fopen( errlog, "a" );
	if( logfp != NULL ) {
		fprintf( logfp, "20%02d/%02d/%02d %02d:%02d:%02d error %d: %s (%s) ", 
                	yr, mon, day, hr, min, sec, errno, msg, parm );
		if( strcmp( emode, "cgi" )==0 ) {  /* give web context info  ... added scg 3/26/10 */
			qs = getenv( "QUERY_STRING" );
			ref = getenv( "HTTP_REFERER" );
			host = getenv( "REMOTE_HOST" );
			ip = getenv( "REMOVE_ADDR" );
			if( host != NULL ) fprintf( logfp, "h=%.30s ", host );
			else if( ip != NULL ) fprintf( logfp, "h=%s ", ip );
			if( qs != NULL ) fprintf( logfp, "qs=%.80s ", qs );
			if( ref != NULL ) fprintf( logfp, "r=%.80s ", ref );
			}
		fprintf( logfp, "\n" );
		fclose( logfp );
		}
	}
#endif

return( errno );
}


/* ===================================== */
int
TDH_errprog( prog )
char *prog; 
{
if( progsticky ) return( 0 );
strcpy( progname, prog);
return( 0 );
}

/* ===================================== */
/* set the errprog.. and can only be reset via this routine (ignore subsequent errprog calls)
	...this allows user control over errprog */
int
TDH_errprogsticky( prog )
char *prog;
{
progsticky = 1;
strcpy( progname, prog);
return( 0 );
}

/* ===================================== */
int
TDH_geterrprog( prog )
char *prog; 
{
strcpy( prog, progname );
return( 0 );
}

/* ===================================== */
int
TDH_errmode( mode )
char *mode;
{
strcpy( emode, mode );
return( 0 );
}

/* ===================================== */
int
TDH_errfile( fp )
FILE *fp;
{
errfp = fp;
strcpy( emode, "file" );
return( 0 );
}

/* ===================================== */
int
TDH_geterrmode( mode )
char *mode;
{
strcpy( mode, emode );
return( 0 );
}

/* ===================================== */
int
TDH_errlogfile( filename )
char *filename;
{
strcpy( errlog, filename );
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
