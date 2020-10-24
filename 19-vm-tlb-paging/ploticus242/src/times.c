/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* TIMES.C  - time arithmetic and format conversion library */

#include <stdio.h>
#include <string.h>
#include <ctype.h>

extern int TDH_err(), GL_systime(), DT_maketime();

/* Compile flag NO_DT may be used to disable all date/time functionality.

   standard units = (double) minutes since 0:00:00.0
		 --seconds are expressed as decimal portion of 1 minute

  input formats for which arithmetic is supported:    
   format	input			output
   HHMM 	hh:mm or hh:mm:ss     	(output is hh:mm)
   HHMMSS	hh:mm or hh:mm:ss	(output is hh:mm:ss)
   MMSS		mm:ss  			(output is mm:ss)


   output formats:   hh:mm hh:mma  hh:mm:ss hh:mm:ssa mm:ss  
	(a=use am/pm notation; A yeilds capitals e.g. AM or PM)

   For any arithmetic or output format, the decimal format of the seconds
   component can be controlled by using a printf double format spec instead
   of 'ss'.  For example: hh:mm:%02.2f 

   To get current time in minutes past midnight, use tomin( "now", &sec );
   To get current time in current format use frommin( -1.0, result );
*/
#ifndef NO_DT

#define HHMM 0
#define MMSS 1
#define HHMMSS 2
#define stricmp(s,t) strcasecmp(s,t)
#define strnicmp(s,t,u) strncasecmp(s,t,u)
#define err(a,b,c) 		TDH_err(a,b,c)
#define NUMBER 0
#define ALPHA 1

static int format = HHMM;
static int Hr = 0, Min = 0;
static double Sec = 0.0;
static char Dispfmt[30] = "%d:%02d";
static char Curfmt[30] = "hh:mm";
static int Nodaylimit = 0;
#endif

double atof();

/* ============================= */
int
DT_time_initstatic()
{
Hr = 0; Min = 0; Sec = 0.0;
strcpy( Dispfmt, "%d:%02d" );
strcpy( Curfmt, "hh:mm" );
return( 0 );
}


/* ============================= */
/* SETTIMEFMT - set the "current time format" */
int
DT_settimefmt( fmt )
char *fmt;
{
#ifdef NO_DT
  return( err( 7950, "date/time support not included in this build", "" ));
  }
#else

/* hhh = hours not limited to one day, eg. 32:00 or 122:44 would be valid  - scg 6/18/03 */
if( strncmp( fmt, "hhh", 3 )==0 ) {
	Nodaylimit = 1;
	strcpy( fmt, &fmt[1] );
	}

if( stricmp( fmt, "hh:mm" )==0 ) {
	strcpy( Dispfmt, "%d:%02d" );
	format = HHMM;
	}
else if( strnicmp( fmt, "mm:", 3 )==0 ) {
	format = MMSS;
	if( fmt[3] == 's' ) strcpy( Dispfmt, "%d:%02g" );	/* added 3/3/02 */
	else sprintf( Dispfmt, "%%d:%s", &fmt[3] );		/* added 3/3/02 */
	}
else if( strnicmp( fmt, "hh:mm:", 6 )==0 ) {
	format = HHMMSS;
	if( fmt[6] == 's' ) strcpy( Dispfmt, "%d:%02d:%02g" );	/* added 3/3/02 */
	else sprintf( Dispfmt, "%%d:%%02d:%s", &fmt[6] );	/* added 3/3/02 */
	}
else return( -1 );
strcpy( Curfmt, fmt );
return( 0 );
}

/* ============================= */
/* TOMIN - take a time in the "current format" and return # minutes since 00:00:00 */
/* Return 0 on valid input; return non-zero on invalid input */
int
DT_tomin( s, result )
char *s; /* time */
double *result; /* returned */
{
int i;
char t[128];
int nt;
int tlen;

if( stricmp( s, "now" )==0 ) {
	GL_systime( &Hr, &Min, &i );
	Sec = (double)i;
	DT_maketime( Hr, Min, Sec, t );
	}
else strcpy( t, s );

/* validate t */
tlen = strlen( t );
if( !isdigit( (int) t[0]) || !isdigit( (int) t[tlen-1]) /* || !isdigit( (int) t[tlen-2]) */ ) /* relaxed 7/17/01 */
	{ *result = 0.0; return( 1 ); }

if( format == HHMM || format == HHMMSS ) {
	if( tlen < 3 ) { *result = 0.0; return( 2 ); } /* sanity check */
	Sec = 0.0;
	nt = sscanf( t, "%d:%d:%lf", &Hr, &Min, &Sec );
	if( nt == 2 ) *result = (Hr * 60.0) + Min;
	else if( nt == 3 ) *result = (Hr * 60.0) + Min + (Sec/60.0);
	else { *result = 0.0; return( 3 ); } /* error */
	if( Hr < 0 || Min > 59 || Min < 0 || Sec >= 60.0 || Sec < 0.0 ) return( 5 ); /* scg 6/18/03 */
	if( !Nodaylimit && ( Hr > 24 || ( Hr == 24 && Min != 0  ))) return( 5 ); /* scg 6/18/03 */ 
	return( 0 );
	}
else if( format == MMSS ) {
	if( strlen( s ) < 4 ) { *result = 0.0; return( 4 ); } /* sanity check */
	nt = sscanf( s, "%d:%lf", &Min, &Sec );
	if( nt != 2 ) { *result = 0.0; return( 6 ); } /* error */
	if( Min > 59 || Min < 0 || Sec >= 60.0 || Sec < 0.0 ) return( 7 );
	*result = Min + (Sec/60.0);
	}
return( 0 );
}

/* ============================ */
/* FROMMIN - take # minutes since 00:00:00 and return time string in current format */
int
DT_frommin( s, result )
double s; /* sec may be < 0 for "now" */
char *result;
{
int i;
if( s < 0.0 ) {
	GL_systime( &Hr, &Min, &i );
	Sec = (double)i;
	}
else	{
	Hr = (int)(s/60.0);
	Min = ((int)(s)-(Hr*60));
	/* Sec = s - (double)((Hr*60) + (Min)); */ /* fixed scg 3/1/02 */
	Sec = (s - (double)((Hr*60) + (Min))) * 60.0; 
	if( Sec < 0.0000001 ) Sec = 0.0; /* adjust for rounding error */
	}

if( !Nodaylimit && Hr == 24 && Min == 0 && Sec == 0 ) Hr = 0; /* scg 9/29/03 */
	
DT_maketime( Hr, Min, Sec, result );
return( 0 );
}

/* ============================= */
/* GETHMS - get hour, min, sec components of most recent time 
	processed by tomin() for frommin() */
int
DT_gethms( hour, min, sec )
int *hour, *min;
double *sec;
{
*hour = Hr;
*min = Min;
*sec = Sec;
return( 0 );
}

/* ============================= */
/* MAKETIME - given h m s, build time string using current format. */
int
DT_maketime( hr, min, sec, result )
int hr, min;
double sec;
char *result;
{
if( format == HHMM ) sprintf( result, Dispfmt, hr, min );
else if( format == MMSS ) sprintf( result,  Dispfmt, min, sec );
else if( format == HHMMSS ) sprintf( result,  Dispfmt, hr, min, sec ); 
return( 0 );
}


/* ============================== */
/* take time in current format and convert to outformat */
int
DT_formattime( s, outformat, result )
char *s, *outformat, *result;
{
char oldformat[30];
double min;
char c;
char ampm[4];
char fmt[40];
int i;

strcpy( fmt, outformat );

DT_tomin( s, &min );

/* in case we do am/pm */
strcpy( ampm, "" );
c = fmt[ strlen( fmt ) - 1];
if( c == 'a' || c == 'A' ) {

	if( Hr == 12 && Min == 0 ) strcpy( ampm, "N" ); /* scg 2/27/02 */
	else if( Hr >= 12 && Hr < 24 ) strcpy( ampm, "PM" ); /* added < 24  scg 2/27/02 */
	else strcpy( ampm, "AM" );
	if( c == 'a' ) for( i = 0; ampm[i] != '\0'; i++ ) ampm[i] = tolower(ampm[i]);

	fmt[ strlen( fmt ) - 1 ] = '\0';
	if( Hr > 12 ) {
		min -= 720.0;
		Hr -= 12;
		}
	}

if( stricmp( fmt, "hh" )==0 ) sprintf( result, "%02d", Hr );
else if( stricmp( fmt, "h" )==0 ) sprintf( result, "%0d", Hr );
else if( stricmp( fmt, "mm" )==0 ) sprintf( result, "%02d", Min );
else if( stricmp( fmt, "m" )==0 ) sprintf( result, "%d", Min );
else if( stricmp( fmt, "ss" )==0 ) sprintf( result, "%g", Sec );
else if( fmt[0] == '%' ) sprintf( result, fmt, Sec );
else	{
	strcpy( oldformat, Curfmt ); /* scg 3/3/02 */
	DT_settimefmt( fmt );
	DT_frommin( min, result );
	DT_settimefmt( oldformat ); /* scg 3/3/02 */
	/* format = oldformat; */
	}

if( ampm[0] != '\0' ) {
	if( result[0] == '0' && result[1] != ':' ) sprintf( result, "%s%s", &result[1], ampm );
	else strcat( result, ampm );
	}
return( 0 );
}


/* =============================== */
/* TIMEDIFF - find difference between two times */

int
DT_timediff( s1, s2, diff )
char *s1, *s2;
double *diff; /* s1-s2 difference in minutes */
{
double min1, min2;

DT_tomin( s1, &min1 );
DT_tomin( s2, &min2 );
*diff = min1 - min2;
return( 0 );
}
#endif


/* =============================== */
/* TIMEFUNCTIONS - script entry point for time functions.
   Return 0 if function found and executed normally.
   Return 1 on error.
   Return 2 if function not found here.
*/

int
DT_timefunctions( hash, name, arg, nargs, result, typ )
int hash;
char *name;
char *arg[];
int nargs;
char *result;
int *typ;
{
int stat;


#ifdef NO_DT
  return( err( 198, "time functions not supported in this build", "" ));
  }
#else

*typ = ALPHA;


if( hash == 400 ) { /* $tomin(t) take t (a value in the current time notation).  Result is the equivalent,
			expressed in # of minutes since 0:00:00.  Result is float,
			with any seconds expressed as the decimal portion of a minute. */
	double f;
	stat = DT_tomin( arg[0], &f );
	sprintf( result, "%g", f );
	*typ = NUMBER;
	return( 0 );
	}

if( hash == 1348 ) { /* $formattime() */
	stat = DT_formattime( arg[0], arg[1], result );
	if( stat != 0 ) { err( 1697, "$formattime failed", arg[1] ); return( 1 ); }
	return( 0 );
	}

if( hash == 1002 ) { /* $timevalid() */
	double x;
	stat = DT_tomin( arg[0], &x );
	if( stat == 0 ) sprintf( result, "1" );
	else sprintf( result, "0" );
	*typ = NUMBER;
	return( 0 );
	}

if( hash == 706 ) { /* $frommin(m) inverse of $tomin(), where m is a float minutes value.
		     * Result is equivalent time in current notation. */
	stat = DT_frommin( atof( arg[0] ), result );
	return( 0 );
	}

if( hash == 753 ) { /* $timediff(t1,t2) - find the difference between two times (both in 
		     * current notation).  Result is expressed in float minutes (any 
		     * seconds expressed as fraction of a minute) */
	double diff;
	DT_timediff( arg[0], arg[1], &diff );
	sprintf( result, "%g", diff );
	*typ = NUMBER;
	return( 0 );
	}

if( hash == 1397 ) { /* $settimefmt(fmt) - set the "time format" currently being used */
	stat = DT_settimefmt( arg[0] );
	if( stat != 0 ) {
		err( 1614, "$settimefmt - invalid format", arg[0] );
		return( 1 );
		}
	strcpy( result, "0" );
	*typ = NUMBER;
	return( 0 );
	}


if( hash == 262 ) { /* $time() - result is current time in hh:mm:ss format */
	int hr, min, sec;
	GL_systime( &hr, &min, &sec );
	sprintf( result, "%02d:%02d:%02d", hr, min, sec );
	return( 0 );
	}


if( hash == 621 ) { /* $timesec() - return number of seconds since midnight for current time */
	int hr, min, sec;
	GL_systime( &hr, &min, &sec );
	sprintf( result, "%ld", (((hr*(long)60) + min)*(long)60) + sec );
	*typ = NUMBER;
	return( 0 );
	}


return( err( 198, "unrecognized function", name ) );
}
#endif


/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
