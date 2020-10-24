/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* small, lowlevel routines re: scaled units */
/* see also units.c which is a layer above this. */

#include "plg.h"

struct plgc PLG;

int
PLG_set_early_defaults()
{
/* overall settings - these can be set by application program before Einit() */
strcpy( Estandard_font, "/Helvetica" );  
Estandard_textsize = 10;	 
Estandard_lwscale = 1.0;
strcpy( Estandard_color, "black" );
strcpy( Estandard_bkcolor, "white" );

/* current parameters.. */
strcpy( Ecurfont, "" );
Ecurtextsize = 0;
Ecurtextheight = 0.0;
Ecurtextwidth = 0.0;
Ecurtextdirection = 0;
Ecurpaper = -1;
Ecurlinewidth = -1.0;
Ecurlinetype = -1;
Ecurpatternfactor = 0.0;
strcpy( Ecurcolor, "" ); /* scg 6/18/04 */

EScale_x = 1; EScale_y = 1; 
Escaletype_x = E_LINEAR;	
Escaletype_y = E_LINEAR;	

strcpy( Eprogname, "" ); 
Eflip = 0;  
Eblacklines = 0; 
Eflashdelay = 150000; 

return( 0 );
}

/* ============================ */
/* SCALETYPE - select the scaling method */

int
PLG_scaletype( typ, axis )
char typ[];
char axis;
{

if( strcmp( typ, "linear" )==0 ) {
	/* Esetunits( axis, "linear" ); */
	if( axis == 'x' ) Escaletype_x = E_LINEAR;
	else if( axis == 'y' ) Escaletype_y = E_LINEAR;
	return( 0 );
	}

#ifndef NOSCALE
else if( strcmp( typ, "log" )==0 ) {
	/* Esetunits( axis, "linear" ); */ /* linear ok */
	if( axis == 'x' ) Escaletype_x = E_LOG;
	else if( axis == 'y' ) Escaletype_y = E_LOG;
	return( 0 );
	}

else if( strcmp( typ, "log+1" )==0 ) {		/* log+1 added scg 11/29/00 */
	if( axis == 'x' ) Escaletype_x = E_LOGPLUS1;
	else if( axis == 'y' ) Escaletype_y = E_LOGPLUS1;
	return( 0 );
	}

else	{
	/* stat = Esetunits( axis, typ );
	 * if( stat != 0 ) {
	 *	sprintf( buf, "Invalid scaling type for %c axis", axis );
	 *	return( Eerr( 101, buf, typ ) );
	 *	}
	 */

	/* special units always use linear as the basic units.. */
	if( axis == 'x' ) Escaletype_x = E_LINEAR;
	else Escaletype_y = E_LINEAR;
	return( 0 );
	}
#endif
}

#ifndef NOSCALE
/* =========================== */
/* SCALE_X - for setting up scaling in x */

int
PLG_scale_x( xlow, xhi, datalow, datahi )
double 	xlow, 	/* absolute x location of left side of the area */
	xhi, 	/* absolute x location of the right side of the area */
	datalow, /* data-units x at the left side */
	datahi;	 /* data-units x at the right side */
{
char msgbuf[100];

EXlo = xlow;
EXhi = xhi;
EDXlo = datalow;
EDXhi = datahi;

if( datahi-datalow <= 0 ) return( Eerr( 100, "x range is invalid .. likely culprits: xautorange, or invalid date format" , "" ) );

if( xhi-xlow <= 0 ) {
	sprintf( msgbuf, "Error in x absolute plot area dimensions (%g and %g)", xlow, xhi);
	return( Eerr( 101, msgbuf, "" ) );
	}
	
if( Escaletype_x == E_LINEAR ) EScale_x = (xhi-xlow) / (datahi-datalow) ;
else if( Escaletype_x == E_LOG ) {
	/* if( datalow <= 0.0 ) datalow = 0.01; */ /* this line commented out 9/26/03 per paul labbe */
	EScale_x = (xhi-xlow) / (log( datahi ) - log( datalow ));
	}
else if( Escaletype_x == E_LOGPLUS1 ) {
	if( (datalow) < 0.0 ) datalow = 0.0; 
	EScale_x = (xhi-xlow) / (log( datahi+1.0 ) - log( datalow+1.0 ));
	}
return( 0 );
}

/* =========================== */
/* SCALE_Y - for setting up scaling in y */

int
PLG_scale_y( ylow, yhi, datalow, datahi )
double 	ylow, 	/* absolute y location of low side of the area */
	yhi, 	/* absolute y location of high side of the area */
	datalow, /* data-units y at the low side */
	datahi;	 /* data-units y at the high side */
{
char msgbuf[100];

EYlo = ylow;
EYhi = yhi;
EDYlo = datalow;
EDYhi = datahi;

if( datahi-datalow <= 0 ) return( Eerr( 100, "y range is invalid .. likely culprit is yautorange or yrange", "" ) );

if( yhi-ylow <= 0 ) {
	sprintf( msgbuf, "Error in y absolute plot area dimensions (%g and %g)", ylow, yhi);
	return( Eerr( 101, msgbuf, "" ) );
	}

if( Escaletype_y == E_LINEAR ) EScale_y = (yhi-ylow) / (datahi-datalow) ;

else if( Escaletype_y == E_LOG ) {
	/* if( datalow <= 0.0 ) datalow = 0.01; */ /* this line commented out scg 9/26/03 per paul labbe */
	EScale_y = (yhi-ylow) / (log( datahi ) - log( datalow ));
	}
else if( Escaletype_y == E_LOGPLUS1 ) {
	if( (datalow) < 0.0 ) datalow = 0.0; 
	EScale_y = (yhi-ylow) / (log( datahi+1.0 ) - log( datalow+1.0 ));
	}
return( 0 );
}

/* =========================== */
/* A -  Returns an absolute location from a data value in xory.
   This is the preferred function to use because it handles flip. */

double 
PLG_a( xory, d )
char xory;
double d;
{
if( Eflip ) {
	if( xory == 'x' ) return( Eay( d ) );
	else if( xory == 'y' ) return( Eax( d ) );
	}
else	{
	if( xory == 'x' ) return( Eax( d ) );
	else if( xory == 'y' ) return( Eay( d ) );
	}
return( Eerr( 15, "Ea: nonsensical parameters", "" ) );
}


/* =========================== */
/* AX - returns an absolute x location from a data value */

double 
PLG_ax( d )
double d;
{

if( Escaletype_x == E_LINEAR ) 
	return( EXlo + (( d - EDXlo ) * EScale_x ));
else if( Escaletype_x == E_LOG ) {
	if( d <= 0.0 ) return( EXlo );
	else if( EDXlo <= 0.0 ) return( EXlo + (( log( d ) - log( 1.0 ) ) * EScale_x ) );
	else return( EXlo + (( log( d ) - log( EDXlo ) ) * EScale_x ) );
	}
else if( Escaletype_x == E_LOGPLUS1 ) {
	if( d <= 0.0 ) return( EXlo );
	else if( EDXlo <= 0.0 ) return( EXlo + (( log( d+1.0 ) - log( 1.0 ) ) * EScale_x ) );
	else return( EXlo + (( log( d+1.0 ) - log( EDXlo ) ) * EScale_x ) );
	}
return(0.0);
}

/* =========================== */
/* AY - returns an absolute y location from a data value */
double 
PLG_ay( d )
double d;
{
if( Escaletype_y == E_LINEAR ) return( EYlo + (( d - EDYlo ) * EScale_y ));
else if( Escaletype_y == E_LOG ) {
	if( d <= 0.0 ) return( EYlo );
	else if( EDYlo <= 0.0 ) return( EYlo + (( log( d ) - log( 1.0 ) ) * EScale_y ) );
	else return( EYlo + (( log( d ) - log( EDYlo ) ) * EScale_y ) );
	}
else if( Escaletype_y == E_LOGPLUS1 ) {
	if( d <= 0.0 ) return( EYlo );
	else if( EDYlo <= 0.0 ) return( EYlo + (( log( d+1.0 ) - log( 1.0 ) ) * EScale_y ) );
	else return( EYlo + (( log( d+1.0 ) - log( EDYlo ) ) * EScale_y ) );
	}
return(0.0);
}




/* =========================== */
/* DX - given an abs coord in X, returns a value in data space */

double 
PLG_dx( a )
double a;
{
double h;
if( Escaletype_x == E_LINEAR ) {
	h = a - EXlo;
	return( EDXlo + ( h / EScale_x ) );
	}
else if( Escaletype_x == E_LOG ) {
	if( a < EXlo ) return( EDXlo );
	h = log( a ) - log( EDXlo );
	return( EDXlo + ( h / EScale_x ) );
	}
else if( Escaletype_x == E_LOGPLUS1 ) {
	if( a < EXlo ) return( EDXlo );
	h = log( a ) - log( EDXlo );
	return( (EDXlo + ( h / EScale_x ) ) - 1.0 ); /* ??? */
	}
return(0.0);
}

/* =========================== */
/* DY - given an abs coord in Y, returns a value in data space */

double 
PLG_dy( a )
double a;
{
double h;
if( Escaletype_y == E_LINEAR ) {
	h = a - EYlo;
	return( EDYlo + ( h / EScale_y ) );
	}
else if( Escaletype_y == E_LOG ) {
	if( a < EYlo ) return( EDYlo );
	h = log( a ) - log( EDYlo );
	return( EDYlo + ( h / EScale_y ) );
	}
else if( Escaletype_y == E_LOGPLUS1 ) {
	if( a < EYlo ) return( EDYlo );
	h = log( a ) - log( EDYlo );
	return( (EDYlo + ( h / EScale_y ) ) - 1.0 ); /* ??? */
	}
return(0.0);
}


/* ====================== */
/* LIMIT - Get minima or maxima of either axis, 
   in either absolute or scaled units.. */

double 
PLG_limit( axis, end, units )
char axis;
char end; /* either 'l' == lo  or 'h' == hi */
char units; /* either 'a' == absolute or 's' == scaled */
{
if( axis == 'x' ) {
	if( end == 'l' && units == 's' ) return( EDXlo );
	else if( end == 'h' && units == 's' ) return( EDXhi );
	if( end == 'l' && units == 'a' ) return( EXlo );
	else if( end == 'h' && units == 'a' ) return( EXhi );
	}
if( axis == 'y' ) {
	if( end == 'l' && units == 's' ) return( EDYlo );
	else if( end == 'h' && units == 's' ) return( EDYhi );
	if( end == 'l' && units == 'a' ) return( EYlo );
	else if( end == 'h' && units == 'a' ) return( EYhi );
	}
Eerr( 12015, "warning, bad values passed to Elimit", "" );
return( 0.0 );
}
#endif

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
