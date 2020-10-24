/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"

static int Supress_convmsg = 0;
static int N_convmsg = 0;
static int beginnum();

static int dump_ok = 0;



/* ====================== */
int
PL_lib_initstatic()
{
Supress_convmsg = 0;
N_convmsg = 0;
dump_ok = 0;
return( 0 );
}

/* ====================== */
/* GETYN */
int
PL_getyn( val )
char *val;
{
if( strnicmp( val, "y", 1 )==0 ) return( 1 );
else return( 0 );
}

/* ====================== */
/* FDA - access D as a 2-D array; return as double; 
   For non-plottables 0.0 is returned, but Econv_error may be called to see if 
   there was a conversion error */
double 
PL_fda( r, c, ax )
int r, c;
char ax;
{
return( Econv( ax, da( r, c ) ) );
}

/* ====================== */
/* NUM - convert string to double, and ensure that it is proper numeric
   Result is s converted to double.
   Return is 0 on ok, 1 on non-numeric. */
int
PL_num( s, result )
char s[];
double *result;
{
int nt;

nt = sscanf( s, "%lf", result );
if( nt > 0 ) return( 0 );
else	{
	*result = 0.0;
	return( 1 );
	}
}

/* ============================= */
/* GETCOORDS - extract a coordinate pair (two posex's) from val */
int
PL_getcoords( parmname, val, x, y )
char *parmname, *val;
double *x, *y;
{
char px[40], py[40];
int nt;
int stat;

nt = sscanf( val, "%s %s", px, py );
if( nt < 2 ) return( Eerr( 55, "Two values expected", parmname ) );

stat = Eposex( px, X, x );
stat += Eposex( py, Y, y );
if( stat != 0 ) return( Eerr( 201, "Error on coord pair", parmname ) );
else return( 0 );
}

/* ============================= */
/* GETBOX - extract two coordinate pairs (four posex's) from val */
int
PL_getbox( parmname, val, x1, y1, x2, y2 )
char *parmname, *val;
double *x1, *y1, *x2, *y2;
{
char px1[40], py1[40], px2[40], py2[40];
int nt;
int stat;

nt = sscanf( val, "%s %s %s %s", px1, py1, px2, py2 );
if( nt < 4 ) return( Eerr( 55, "Four values expected", parmname ) );
stat = Eposex( px1, X, x1 );
stat += Eposex( py1, Y, y1 );
stat += Eposex( px2, X, x2 );
stat += Eposex( py2, Y, y2 );
if( stat != 0 ) return( Eerr( 201, "Error on box specification", parmname ) );
else return( 0 );
}


/* ================== */
/* GETRANGE - get a low/high range */
int
PL_getrange( lineval, lo, hi, ax, deflo, defhi )
char *lineval;
double *lo, *hi;
char ax;
double deflo, defhi;
{
char s1[80], s2[80];
int nt;
nt = sscanf( lineval, "%s %s", s1, s2 );
if( nt == 2 ) {
	*lo = Econv( ax, s1 );
	*hi = Econv( ax, s2 );
	return( 0 );
	}
else if( nt == 1 ) {
	*lo = Econv( ax, s1 );
	*hi = defhi;
	return( 0 );
	}
else if( nt <= 0 ) {
	*lo = deflo;
	*hi = defhi;
	return( 0 );
	}
return( 0 );
}
	

/* =============================== */
/* FILE_TO_BUF - read file or execute command and place contents into buf.
   Shell expandable file name is ok.
   Returns 0 if ok, 1 if file not available */

int
PL_file_to_buf( filename, mode, result, buflen )
char *filename;
int mode; /* 1 = file   2 = command */
char *result;
int buflen;
{
FILE *fp, *popen();
char buf[1000];


if( mode == 1 ) fp = fopen( filename, "r" );
else 	{
	if( PLS.noshell ) return( Eerr( 7203, "-noshell prohibits shell command", "" ));
	fp = popen( filename, "r" );
	}
if( fp == NULL ) return( 1 );
strcpy( result, "" );
while( fgets( buf, 999, fp ) != NULL ) {
	if( strlen( result ) + strlen( buf ) >= buflen ) {
		Eerr( 7254, "warning: truncated, capacity reached", filename );
		return( 2 );
		}
	strcat( result, buf );
	}
if( mode == 1 ) fclose( fp );
else pclose( fp );
return( 0 );
}

/* ========================= */
/* SETFLOATVAR - set a TDH var to a double value */
int
PL_setfloatvar( varname, f, fmt )
char *varname, *fmt;
double f;
{
char buf[80];
int stat;
sprintf( buf, fmt, f );
stat = TDH_setvar( varname, buf );
if( stat != 0 ) return( Eerr( stat, "Error on setting variable", varname ) );
return( 0 );
}
/* ========================= */
/* SETINTVAR - set a TDH var to an integer value */
int
PL_setintvar( varname, n )
char *varname;
int n;
{
char buf[80];
int stat;
sprintf( buf, "%d", n );
stat = TDH_setvar( varname, buf );
if( stat != 0 ) return( Eerr( stat, "Error on setting variable", varname ) );
return( 0 );
}

/* ========================== */
/* SETCHARVAR - set a TDH var to a char string value */
int
PL_setcharvar( varname, s )
char *varname;
char *s;
{
int stat;
stat = TDH_setvar( varname, s );
if( stat != 0 ) return( Eerr( stat, "Error on setting variable", varname ) );
return( 0 );
}

/* =========================== */
/* CONV_MSG - print a message to errfp for a data item of invalid type */
int
PL_conv_msg( row, col, aname )
int row, col;
char *aname;
{
char progname[80];
N_convmsg++;
if( Supress_convmsg ) return( 0 );
TDH_geterrprog( progname );
fprintf( PLS.errfp, "%s: warning, %s skipping unplottable '%s' (rec=%d field=%d)\n",
	progname, aname, da(row,col), row+1, col+1 );
return( 0 );
}

/* =========================== */
/* SUPPRESS_CONVMSG - suppress invalid type messages */
int
PL_suppress_convmsg( mode )
int mode;
{
Supress_convmsg = mode;
return( 0 );
}

/* ============================= */
/* ZERO_CONVMSGCOUNT - zero out the conv msg counter */
int
PL_zero_convmsgcount()
{
N_convmsg = 0;
return( 0 );
}
/* ============================= */
/* REPORT_CONVMSGCOUNT - report on what the conv msg count is.. */
int
PL_report_convmsgcount()
{
return( N_convmsg );
}



/* ========================== */
/* SCALEBEENSET - return 1 if scaling has been set, 0 if not */
int
PL_scalebeenset()
{
if( EDXhi - EDXlo > 0.0000001 ) return( 1 );
else return( 0 );
}



/* ======================== */
/* DEFAULTINC - given a min and a max, estimate a reasonable default inc (linear or log numeric data)
 *
 * Improvements contributed by Dan Pelleg peldan@yahoo.com :
 *  we want to find a number that is:
 *   - the same order of magnitude of h, and greater than h
 *   - is either 1, 2, or 5, multiplied by the appropriate units
 *   - is the smallest such number
 *  For example, all numbers between 10000 and 19999 are mapped to 20000.
 *  Numbers 20000 - 49999 are mapped to 50000.
 *  Numbers 50000 - 99999 are mapped to 100000.
 *  Numbers 100000 - 199999 are mapped to 200000.   And so on.  
 *
 * Remaining scg code deleted 10/1/03...
 */

int
PL_defaultinc( min, max, inc )
double min, max, *inc;
{
double diff, h, fabs();
double ret, mult, mant;

diff = max - min;
diff = fabs( diff );
h = diff / 10.0;  /* we want to have about 10 tics on an axis.. */

/* normalize and write h = mant * mult, for 1 <= mant 10 */
mult = pow( 10.0, floor( log10(h) ) );
mant = h / mult;

/* find the next step and multiply */
if(mant < 2.0) ret = 2.0 * mult;
else if(mant < 5.0) ret = 5.0 * mult;
else ret = 10.0 * mult;
*inc = ret;
return( 0 );
}



/* ======================== */
/* REWRITENUMS - rewrite numbers, supplying a spacer (comma in US)
	every 3 zeros in large numbers.  If spacer is dot (.), European
	mode is presumed, decimal point is written as a comma, and
	large numbers are spaced using dot.   

	Numbers in scientific notation are returned unaltered.

 	Parameter num is modified.
*/

int
PL_rewritenums( num )
char *num;
{
int i, j, k, decplace, len;
char s[40], tmp[40];

if( PLS.bignumspacer == '\0' ) return( 0 ); /* do nothing */

sscanf( num, "%s", s ); /* strip off any leading spaces */

/* find any dec pt; convert it; put result into 'tmp'.. */
k = -1;
decplace = -1;
for( i = 0, len = strlen( s ); i < len; i++ ) {
	if( s[i] == 'e' ) return( 0 ); /* scientific notation detected - leave it alone - scg 11/26/02 */
	/* remember the char where number begins.. */
	if( k < 0 && beginnum( s, i ) ) k = i;
		/* ( isdigit( s[i] )  || (( s[i] == '.' || s[i] == '-' ) && isdigit( s[i+1]) ) ) ) k = i; */
	if( s[i] == '.' ) {
		decplace = i - k;
		if( PLS.bignumspacer == '.' ) tmp[i] = ',';
		else tmp[i] = '.';
		}
	else tmp[i] = s[i];
	}

tmp[i] = '\0';

if( decplace < 0 ) {
	for( ; i >= 0; i-- ) if( isdigit( (int) s[i] ) || s[i] == '.' ) { i++; break; } /* added scg 3/25/02 */
	decplace = i - k;
	}

if( decplace > PLS.bignumthres ) {   /* add spacers.. */
	/* process tmp; put result back in 's'.. */
	for( i = -1, j = 0, k = 0; i < decplace; k++ ) {
		/* just copy over any leading alpha portion.. */
		if( i < 0 && !beginnum( tmp, k ) ) {
				/* ( ! isdigit( tmp[k] ) && tmp[k] != '.' && tmp[k] != '-' ) ) { */
			s[j++] = tmp[k];
			continue; /* scg 2/28/02 */
			}

		i++;
		if( i > 0 && decplace-i > 0 && (decplace-i) % 3 == 0 ) {  /* insert 000 separator.. */
			s[j++] = PLS.bignumspacer;
			s[j++] = tmp[k]; 
			}
		else  s[j++] = tmp[k]; 
		}
	s[j] = '\0';
	if( k < strlen( tmp ) ) strcat( s, &tmp[k] ); /* append decimal point and rightward */
	}
else strcpy( s, tmp );

strcpy( num, s );
return( 0 );
} 

static int
beginnum( s, i )
char *s;
int i;
{
if( ( isdigit( (int) s[i] )  || (( s[i] == '.' || s[i] == '-' ) && isdigit( (int) s[i+1]) ) ) ) return( 1 );
else return( 0 );
}

/* ============================= */
/* CONVERTNL - change all occurrances of "\n" (backslash followed by n) to a newline character */
/* 		(Result is always shorter than input)   */
int
PL_convertnl( str )
char *str;		/* str is always updated... you can't pass a string constant to convertnl() */
{
int i, j, len;
for( i = 0, j = 0, len = strlen( str ); i < len; i++ ) {
	if( str[i] == '\\' && str[i+1] == 'n' ) {
		str[j] = '\n';
		i++;
		}
	else str[j] = str[i];
	j++;
	}
str[j] = '\0';
return( 0 );
}
/* ============================== */
/* MEASURETEXT - count the number of lines present in txt and find the length of longest line. */
int
PL_measuretext( txt, nlines, maxlen )
char *txt;
int *nlines, *maxlen;
{
int i, len, tlen;
char line[256];

i = 0;
*nlines = 0;
*maxlen = 0;
tlen = strlen( txt );
while( 1 ) {
	if( i >= tlen ) break;
	GL_getseg( line, txt, &i, "\n" );
	len = strlen( line );
	if( len > *maxlen ) *maxlen = len;
	(*nlines)++;
	}
return( 0 );
}


/* ================================= */
/* DO_X_BUTTON */
int
PL_do_x_button( label )
char *label;
{
#ifndef NOX11
double x, y;
double sx, sy;
int e;
PLG_getglobalscale( &sx, &sy );  /* turn off global scaling while we draw button.. */
PLG_setglobalscale( 1.0, 1.0 );
while( 1 ) {
	if( PL_clickmap_getdemomode() ) clickmap_show( 'x' ); /* added scg 11/23/01 */
	Elinetype( 0, 0.5, 1.0 );
	Ecblock( 0.1, 0.1, 1.0, 0.3, "yellow", 0 );
	Emov( 0.5, 0.12 );
	Etextsize( 12 );
	Ecolor( "black" );
	Ecentext( label );
	Ecblockdress( 0.1, 0.1, 1.0, 0.3, 0.06, "0.6", "0.8", 0.0, "" );
	Eflush();
	Esavewin();
        Egetkey( &x, &y, &e );
	if( e < 1000 || (y < 0.3 && x < 1.0 )) {
              	Ecblock( 0.1, 0.1, 1.0, 0.3, "black", 0 ); 
		Eflush();
		if( e == 'q' ) PLS.skipout = 1; 
                break;
                }
	else if( PLS.usingcm ) fprintf( PLS.diagfp, "%g %g\n", x*2.54, y*2.54 ); /* in cm */
        else fprintf( PLS.diagfp, "%g %g\n", x, y ); /* mouse click location in inches from lower-left */
	}
PLG_setglobalscale( sx, sy ); /* restore global scaling.. */
#endif
return( 0 );
}

#ifdef HOLD
/* ======================================================= */
/* temporary debug routine */
PLG_dump_on()
{
dump_ok = 1;
return( 0 );
}

PLG_dumpdata()
{
int i, j;
if( !dump_ok ) return( 0 );

for( i = 0; i < Nrecords; i++ ) {
	for( j = 0; j < Nfields; j++ ) printf( "[%s]", da( i, j ) );
	printf( "\n" );
	}
printf( "---------------\n" );
return( 0 );
}
#endif


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
