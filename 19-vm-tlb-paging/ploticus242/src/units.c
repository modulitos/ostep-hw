/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* ROUTINES FOR WORKING WITH VARIOUS UNITS */
/* These routines may be thought of as "sitting on top of" the routines in ./src/graphic.c 

   References dmslib/dates.c and dmslib/times.c
*/

#include "pl.h"

#define LINEAR 0
/* #define YYMM 1 */
#define DATE 2
#define TIME 3
#define DATETIME 4
#define CATEGORIES 5

static char unitdesc[2][20] = { "", "" };
static int unittyp[2] = { 0, 0 };
static int conv_errflag = 0;
static int dashindate = 0;	 /* 1 if current date format contains dash(s), 0 otherwise */
static char dashindateaxis = '0';
static double catslide[2] = { 0.0, 0.0 };  /* for displaying clusters over categories */

static int pex();
static int do_pex();
static int setdatesub();
static int convdatesub();
static int evalbound();


/* ============================ */
int
PL_units_initstatic()
{
strcpy( unitdesc[0], "" ); strcpy( unitdesc[1], "" );
unittyp[0] = 0; unittyp[1] = 0;
conv_errflag = 0;
dashindate = 0;
dashindateaxis = '\0';
catslide[0] = 0.0; catslide[1] = 0.0; 

return( 0 );
}


/* ============================ */
/* SETUNITS - set up for special units on an axis */
int
PL_setunits( axis, s )
char axis;
char *s;
{
int i;
char tok[80], tok2[80];
int nt;
int stat;
int j, slen;

if( axis == 'x' ) i = 0;
else i = 1;

strcpy( unitdesc[i], "" );

strcpy( tok, "" );
strcpy( tok2, "" );
nt = sscanf( s, "%s %s", tok, tok2 );


if( strcmp( tok, "linear" )==0 ) unittyp[i] = LINEAR;

else if( strcmp( tok, "date" )==0 ) {
	unittyp[i] = DATE;
	dashindate = 0; dashindateaxis = '0';
	if( nt == 2 ) {
		/* if tok2 is a ploticus-only date type, do the setup work here.. */
		setdatesub( tok2, unitdesc[i] );
		stat = DT_setdatefmt( tok2 );
		if( stat != 0 ) return( -1 ) ;
		}
	DT_getdatefmt( tok2 );
	/* check for - in date format- this messes up arithmetic */
	for( j = 0, slen = strlen( tok2 ); j < slen; j++ ) {
		if( tok2[j] == '-' ) { dashindate = 1; dashindateaxis = axis; break; }
		}
	}

else if( strcmp( tok, "time" )==0 ) {
	unittyp[i] = TIME;
	if( nt == 2 ) {
		stat = DT_settimefmt( tok2 );
		if( stat != 0 ) return( -1 ) ;
		}
	}

else if( strncmp( tok, "datetime", 8 )==0 ) {
	unittyp[i] = DATETIME;
	dashindate = 0; dashindateaxis = '0';
	/* if( nt == 2 ) { */ /* condition removed scg 9/29/03 */
	stat = DT_setdatetimefmt( tok2, &tok[8] );
	if( stat != 0 ) return( -1 ) ;
	/* } */
	DT_getdatefmt( tok2 );
	/* check for - in date format- this messes up arithmetic */
	for( j = 0, slen = strlen( tok2 ); j < slen; j++ ) {
		if( tok2[j] == '-' ) { dashindate = 1; dashindateaxis = axis; break; }
		}
	}
else if( strcmp( tok, "categories" )==0 ) {
	unittyp[i] = CATEGORIES;
	catslide[0] = 0.0; catslide[1] = 0.0;
	}
else return( -1 ); /* unknown type */
return( 0 );
}
/* ============================ */
/* GETUNITS - report on what units are in effect on the given axis. */
int
PL_getunits( axis, result )
char axis;
char *result;
{
int i;

if( axis == 'x' ) i = 0;
else i = 1;

if( unittyp[i] == LINEAR ) strcpy( result, "linear" );
else if( unittyp[i] == DATE ) strcpy( result, "date" );
else if( unittyp[i] == TIME ) strcpy( result, "time" );
else if( unittyp[i] == DATETIME ) strcpy( result, "datetime" );
else if( unittyp[i] == CATEGORIES ) strcpy( result, "categories" );
return( 0 );
}

/* ============================ */
/* GETUNITSUBTYPE - report on subtype of units are in effect on the given axis. */
int
PL_getunitsubtype( axis, result )
char axis;
char *result;
{
int i;

if( axis == 'x' ) i = 0;
else i = 1;

strcpy( result, unitdesc[i] );
return( 0 );
}

/* ============================ */
/* SETSCALE - set scaling for an axis - special units supported */
int
PL_setscale( axis, alo, ahi, scalelo, scalehi )
char axis;
double alo, ahi;
char *scalelo, *scalehi;
{
double slo, shi;
int i;
if( axis == 'x' ) i = 0;
else i = 1;


if( unittyp[i] == CATEGORIES ) { /* exception since categories are defined later w/ axis */
	slo = atof( scalelo );
	shi = atof( scalehi );
	}
else	{
	evalbound( scalelo, axis, &slo );
	if( Econv_error() ) Eerr( 205, "range low value has invalid format", scalelo );
	evalbound( scalehi, axis, &shi );
	if( Econv_error() ) Eerr( 205, "range hi value has invalid format", scalehi );
	}

if( axis == 'x' ) return( Escale_x( alo, ahi, slo, shi ) );

else if( axis == 'y' ) return( Escale_y( alo, ahi, slo, shi ) ); 

else return( Eerr( 209, "invalid axis", "" ));
}

/* ============================ */
/* CONV - return linear equivalent of special units */
double
PL_conv( axis, s )
char axis;
char *s;
{
double f;
int i;
long l;
int stat;
char str[80];
int nt;

if( axis == 'x' ) i = 0;
else i = 1;

conv_errflag = 0;

/* fprintf( stderr, "[typ=%d ax=%c val=%s]", unittyp[i], axis, s ); */

if( unittyp[i] == LINEAR ) {
	/* made following change to allow scientific notation.. scg 5/23/00 */
	nt = sscanf( s, "%lf", &f );
	if( nt < 1 ) { conv_errflag = 1; return( 0.0 ); } 
	else return( f );
	/* if( GL_goodnum( s, &i ) ) return( atof( s ) ); */
	/* else	{ conv_errflag = 1; return( 0.0 ); } */
	}
	
else if( unittyp[i] == DATE ) {
	strcpy( str, s ); /* fallback */
	if( unitdesc[i][0] != '\0' ) {
		/* do conversion for ploticus-only date types */
		convdatesub( unitdesc[i], s, str );
		}

	stat = DT_jdate( str, &l );
	if( stat != 0 ) { conv_errflag = 1; return( 0.0 ); }
	return( (double)l );
	}
else if( unittyp[i] == TIME ) {
	stat = DT_tomin( s, &f );
	if( stat != 0 ) { conv_errflag = 1; return( 0.0 ); }
	return( f );
	}
else if( unittyp[i] == DATETIME ) {
	/* format is date.time; result is days.(sec/86400) */
	stat = DT_datetime2days( s, &f );
	if( stat != 0 ) { conv_errflag = 1; return( 0.0 ); }
	return( f );
	}

else if( unittyp[i] == CATEGORIES ) { 
	int catj;
	/* look up the category.. */
	catj = PL_findcat( axis, s );
	if( catj < 0 ) { conv_errflag = 1; return( 0.0 ); }

	if( axis == 'x' ) f = (double)( catj + 1 ) + catslide[0]; /* +1 because we will never want origin */
	else f = EDYhi-((double)( catj + 1 ) + catslide[1]);   /* cats always top down in Y */  /* 9/28/07- it seems like it should be: -catslide */
											       /* but changing this now could break lots of things*/
	return( f );
	}

else	{
	conv_errflag = 1;
	return( Eerr( 305, "Unit type error", "" ));
	}
}
/* ============================= */
/* CONV_ERROR check of Econv() error status */
int
PL_conv_error()
{
return( conv_errflag );
}

/* ============================= */
/* UPRINT - produce a string containing the external representation of a value.  Result is copied into 'result'. */
int
PL_uprint( result, axis, f, format )
char *result;
char axis;
double f;
char *format;
{
int i;
char s[80];
int stat;

if( axis == 'x' ) i = 0;
else i = 1;

strcpy( result, "???" );

if( unittyp[i] == LINEAR ) {
	/* when generating incremental axes moving from negative to positive, for zero sprintf sometimes 
	   gives -0.00 or very tiny values like -5.5579e-17.  The following is a workaround.. scg 7/5/01 */
	/* if( f < 0.0000000000001 && f > -0.0000000000001 ) f = 0.0; */  /* moved this to proc_axis() scg 10/1/03 */

	if( format[0] == '\0' ) sprintf( result, "%g", f );
	else if( strncmp( format, "autoround", 9 )==0 ) {
		if( format[9] == '\0' ) strcpy( result, GL_autoroundf( f, 0 ));
		else strcpy( result, GL_autoroundf( f, atoi( &format[9] ) ));
		}
	else sprintf( result, format, f );

	if( PLS.bignumspacer ) rewritenums( result ); /* rewrite w/various spacing, decimal pt options*/
	}


else if( unittyp[i] == DATE ) {
	stat = DT_fromjul( (long) f, s );
	if( stat != 0 ) {
		fprintf( PLS.errfp, "[invalid f value: %g stat=%d]", f, stat );
		return( Eerr( 801, "error in parsing date", "" ));
		}

	if( format[0] != '\0' )  {
		stat = DT_formatdate( s, format, result );
		if( stat != 0 ) strcpy( result, s );  /* was an error   scg 12/18/03 */
		}
	else strcpy( result, s );
	}

else if( unittyp[i] == TIME ) {
	stat = DT_frommin( f, s );
	if( stat != 0 ) return( Eerr( 803, "error in parsing time", s));
	if( format[0] != '\0' ) {
		stat = DT_formattime( s, format, result );
		if( stat != 0 ) strcpy( result, s );  /* was an 804 error  scg 12/18/03 */
		}
	else strcpy( result, s );
	}

else if( unittyp[i] == DATETIME ) {
	stat = DT_days2datetime( f, s );
	if( format[0] != '\0' ) {
		stat = DT_formatdatetime( s, format, result );
		if( stat != 0 ) strcpy( result, s );  /* was an 804 error  scg 12/18/03 */
		}
	else strcpy( result, s );
	}

else if( unittyp[i] == CATEGORIES ) { /* needed? */
	strcpy( result, "categories???" );
	}

return( 0 );	
}

/* ========================= */
/* POSEX - evaluate position expression and return absolute.
   LENEX - evaluate length expression and return absolute.  No special units
		are allowed in a length, thus for date scaling, value is in days;
		for time scaling, value is in seconds.

   Position expressions are numbers, optionally followed by '(s)' to indicate
	scaled (data) units in the given axis space, or '(a)' to indicate 
	absolute unites (inches or cm).  If no '(s)' or '(a)' is given, units are 
	presumed to be absolute units.

   Further, an offset may be added or subtracted, as in the fourth example.
	The offset is ALWAYS a length, and therefore not expressed in units
	as explained above.

	Examples:  25.2(s)   100  0.0(s)  100(a)  10:40+120

	Returns 0 if ok, 1 on error.
*/
int
PL_posex( val, axis, result )
char *val, axis;
double *result;
{
return( pex( val, axis, result, 0 ) );
}

/* ------------ */
int
PL_lenex( val, axis, result )
char *val, axis;
double *result;
{
return( pex( val, axis, result, 1 ) );
}

/* ------------- */
static int pex( in, axis, result, mode )
char *in, axis;
double *result;
int mode; /* 0 = position,  1 = length */
		/* only difference is that, when doing a length in scaled units, we
			need to normalize against minima */
{
int i;
int stat;
char *subval[2];
int nval;
double result2;
int op;
char val[255];
int neg_offset_allowed;
int slen;

if( in[0] == '\0' ) { *result = 0.0;  return( 1 ); }

conv_errflag = 0; /* scg 4/22/02 */
strcpy( val, in );
nval = 1;
op = 1;

/* check for embedded + and -  (skip first character - could be unary)*/
subval[0] = &val[0];
neg_offset_allowed = 1;
if( dashindate && dashindateaxis == axis ) neg_offset_allowed = 0;
for( i = 1, slen = strlen( val ); i < slen; i++ ) {
	/* the following condition was changed scg 10/1/03 to allow scientific notation values.. */
	if( ( val[i] == '+' || (val[i] == '-' && neg_offset_allowed) ) && tolower( val[i-1] ) != 'e' ) {  /* EMB-MIN */
		if( val[i] == '+' ) op = 1;
		else if( val[i] == '-' ) op = 2;
		val[i] = '\0';
		subval[1] = &val[i+1];
		nval = 2;
		/* fprintf( stderr, "[subvals: %s %s]", subval[0], subval[1] ); */
		}
	}

stat = do_pex( subval[0], axis, result, mode );
if( nval > 1 ) {
	stat += do_pex( subval[1], axis, &result2, 1 ); /* offset is always a length */
	if( op == 1 ) *result += result2;
	else if( op == 2 ) *result -= result2;
	}
return( stat );
}

/* -------- */
static int do_pex( val, axis, result, mode )
char *val;
char axis;
double *result;
int mode;	/* 0 = position  1 = length (no sp. units in lengths)  */
{
char buf[255];
double atof();
int nt, k, found;
double f;
int len;
char modifier[12];

/* check for min and max.. */
if( strncmp( val, "min", 3 )==0 ) { *result = Elimit( axis, 'l', 'a' ); return( 0 ); }
else if( strncmp( val, "max", 3 )==0 ) { *result = Elimit( axis, 'h', 'a' ); return( 0 ); }


strcpy( buf, val );
len = strlen( buf );
strcpy( modifier, "" );
if( buf[ len-1 ] == ')' && buf[ len-3 ] == '(' ) {
	strcpy( modifier, &buf[ len-3 ] );
	buf[ len-3 ] = '\0';
	}

/* check for proper numeric.. */
/* stat = GL_goodnum( buf, &i ); replaced scg 5/23/00 */
nt = sscanf( buf, "%lf", &f );
if( nt < 1 ) strcpy( modifier, "(s)" ); /* assume scaled */
/* if( stat != 1 ) strcpy( modifier, "(s)" ); */ /* assume scaled */
/* else *result = atof( buf );  replaced scg 5/23/00 */
else *result = f;

/* scaled units */
if( strcmp( modifier, "(s)" )==0 ) {
	if( !scalebeenset() )
		return( Eerr( 51, "scaled units specified but y scaling not set", val ) );
	if( mode == 1 ) {
		*result = Ea( axis, atof( buf ) ) - Ea( axis, 0.0 );
		return( 0 );
		}
	if( mode == 0 ) {
		*result = Ea( axis, Econv( axis, buf ) );
		if( Econv_error() ) {
			/* try again, this time converting any underscores to spaces... 
			 * allows catnames (etc) w/ embedded spaces to be usable for positioning.   added     scg 3/6/09 */
			for( k = 0, found = 0; buf[k] != '\0'; k++ ) if( buf[k] == '_' ) { buf[k] = ' '; found = 1; }
			if( found ) { /* try again only if there actually was an underscore in there.. */
				*result = Ea( axis, Econv( axis, buf ) );
				if( Econv_error() ) return( 1 );
				}
			else return( 0 );
			}
		else return( 0 );
		}
	}

/* convert absolute centimeter specs to inches */
if( PLS.usingcm && (  modifier[0] == '\0' || strcmp( modifier, "(a)" )==0 ) ) {
	*result = *result/ 2.54;
	}

return( 0 );
}

/* ============================= */
/* U - take s in special units and return absolute */
double 
PL_u( axis, s )
char axis;
char *s;
{
return( Ea( axis, Econv( axis, s ) ) );
}

/* ============================= */
/* EVALBOUND - take a units argument that may include an embedded + or -.
	Econv the first operand, add/subtract any offset and return result. 
	(offset is always numeric regardless of units).
	Example val: 02:34+5

	This cannot be a posex because scaling is not yet set as of this call.

	If date format includes embedded dashes then subtraction is disabled.
*/
static int
evalbound( val, axis, result )
char *val;
char axis;
double *result;
{
char *subval[2];
int op, nval, i, slen;

/* check for embedded + and -  (skip first character - could be unary)*/
subval[0] = &val[0];
nval = 0;
for( i = 1, slen = strlen( val ); i < slen; i++ ) {
	/* the following condition was changed scg 10/1/03 to allow scientific notation values.. */
        if( ( val[i] == '+' || (val[i] == '-' && !dashindate) ) && tolower(val[i-1]) != 'e' ) { 	/* EMB-MIN */
                if( val[i] == '+' ) op = 1;
                else if( val[i] == '-' ) op = 2;
                val[i] = '\0';
                subval[1] = &val[i+1];
                nval = 2;
                }
        }
*result = Econv( axis, subval[0] );
if( nval == 2 ) {
	if( op == 1 ) *result += atof( subval[1] ); 
	else if( op == 2 ) *result -= atof( subval[1] );
	}

return( 0 );
}


/* ======================= */
/* date types that are special to ploticus
	(generally yymm and Q type notations)
*/
static int
setdatesub( tok, desc )
char *tok, *desc;
{
if( strcmp( tok, "yymm" )==0 ) { strcpy( desc, "yymm" ); strcpy( tok, "yymmdd" ); }
else if( strcmp( tok, "yymmm" )==0 ) { strcpy( desc, "yymmm" ); strcpy( tok, "ddmmmyy" ); }
else if( GL_slmember( tok, "yy?mm" )) { strcpy( desc, "yy/mm" ); strcpy( tok, "yy/mm/dd" ); }
else if( GL_slmember( tok, "yyyy?mm" )) { strcpy( desc, "yyyy/mm" ); strcpy( tok, "yyyy/mm/dd" ); }
else if( GL_slmember( tok, "yy" )) { strcpy( desc, "yy" ); strcpy( tok, "yymmdd" ); }
else if( GL_slmember( tok, "yyyy" )) { strcpy( desc, "yyyy" ); strcpy( tok, "yyyy/mm/dd" ); }  /* added scg 2/2/05 */
else if( GL_slmember( tok, "mm?yy" )) { strcpy( desc, "mm/yy" ); strcpy( tok, "dd/mm/yy" ); }
else if( GL_slmember( tok, "mm?yyyy" )) { strcpy( desc, "mm/yyyy" ); strcpy( tok, "dd/mm/yyyy" ); }
else if( GL_slmember( tok, "yyqn" )) { strcpy( desc, "yyqn" ); strcpy( tok, "yy/mm/dd" ); }
else if( GL_slmember( tok, "yyyyqn" )) { strcpy( desc, "yyyyqn" ); strcpy( tok, "yyyy/mm/dd" ); }
else if( GL_slmember( tok, "nqyy" )) { strcpy( desc, "nqyy" ); strcpy( tok, "yy/mm/dd" ); }
else if( GL_slmember( tok, "nqyyyy" )) { strcpy( desc, "nqyyyy" ); strcpy( tok, "yyyy/mm/dd" ); }
return( 0 );
}

/* ----- */
static int
convdatesub( desc, s, result )
char *desc, *s, *result;
{
char quar;
int placeconv; /* if > 0, what position to place converted date when doing Q notations */
int slen;

slen = strlen( s );

placeconv = 0;

if( strcmp( desc, "yymm" )==0 ) { 
	if( slen == 4 ) sprintf( result, "%s01", s );  
	}
else if( strcmp( desc, "yymmm" )==0 ) { 
	if( slen == 5 ) sprintf( result, "01%s%c%c", &s[2], s[0], s[1] );
	}
else if( strcmp( desc, "yy/mm" )==0 ) {
	if( slen == 5 ) sprintf( result, "%s/01", s );  
	result[2] = '/'; result[5] = '/';
	}
else if( strcmp( desc, "yyyy/mm" )==0 ) {
	if( slen == 7 ) sprintf( result, "%s/01", s );  
	result[4] = '/'; result[7] = '/';
	}
else if( strcmp( desc, "yy" )==0 ) {
	if( slen == 2 ) sprintf( result, "%s0101", s );  
	}
else if( strcmp( desc, "yyyy" )==0 ) {  /* added scg 2/2/05 */
	if( slen == 4 ) sprintf( result, "%s/01/01", s );  
	}
else if( strcmp( desc, "mm/yy" )==0 ) {
	if( slen == 5 ) sprintf( result, "01/%s", s );  
	result[2] = '/'; result[5] = '/';
	}
else if( strcmp( desc, "mm/yyyy" )==0 ) {
	if( slen == 7 ) sprintf( result, "01/%s", s );  
	result[2] = '/'; result[5] = '/';
	}
else if( strcmp( desc, "yyqn" )==0 ) {
	if( slen == 4 ) { strcpy( result, s ); quar = s[3]; placeconv = 2; }
	}
else if( strcmp( desc, "yyyyqn" )==0 ) {
	if( slen == 6 ) { strcpy( result, s ); quar = s[5]; placeconv = 4; }
	}
else if( strcmp( desc, "nqyy" )==0 ) {
	if( slen == 4 ) { strcpy( result, &s[2] ); quar = s[0]; placeconv = 2; }
	}
else if( strcmp( desc, "nqyyyy" )==0 ) {
	if( slen == 6 ) { strcpy( result, &s[2] ); quar = s[0]; placeconv = 4; }
	}
else Eerr( 4802, "unrecognized subscale type", desc );


if( placeconv > 0 ) { /* drop in a date for Q types; these will be represented at mid-quarter */
	if( quar == '1' ) strcpy( &result[ placeconv ], "/02/15" );
	else if( quar == '2' ) strcpy( &result[ placeconv ], "/05/15" );
	else if( quar == '3' ) strcpy( &result[ placeconv ], "/08/15" );
	else if( quar == '4' ) strcpy( &result[ placeconv ], "/11/15" );
	}

return( 0 );
}

/* ======================= */
int
PL_setcatslide( axis, amount )
char axis;
double amount;
{
if( axis == 'x' ) catslide[0] = amount;
else catslide[1] = amount; 
return( 0 );
}
/* ======================= */
/* ES_INR - see if a string-based value is in range for the given axis */
int
PL_s_inr( axis, val )
char axis;
char *val;
{
double fval, flow, fhi;

fval = Econv( axis, val );
flow = Elimit( axis, 'l', 's' );
fhi = Elimit( axis, 'h', 's' );
 
if( fval >= flow && fval <= fhi ) return( 1 ) ;
else return( 0 );
}
/* ======================= */
/* EF_INR - see if a float-based value is in range for the given axis */
int
PL_f_inr( axis, val )
char axis;
double val;
{
double flow, fhi;

flow = Elimit( axis, 'l', 's' );
fhi = Elimit( axis, 'h', 's' );
 
if( val >= flow && val <= fhi ) return( 1 ) ;
else return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
