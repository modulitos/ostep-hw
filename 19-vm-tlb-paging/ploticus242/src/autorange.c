/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"
#define MAXFIELDS 50

int
PLP_autorange( axis, specline, minval, maxval )
char axis;
char *specline; /* spec line from proc areadef.. */
char *minval;   /* determined plot minima.. */
char *maxval;   /* determined plot maxima.. */
{
int i, j;
int df[MAXFIELDS];
int ndf;
char nearest[80];
char buf[256];
char dfield[256];
double min, max, fmod();
char smin[80], smax[80];
double fval;
double margin;
int ix;
char lowfix[80], hifix[80];
char unittyp[80];
char floatformat[20];
/* char datepart[40], timepart[40]; */
double incmult;
char tok[80];
char selex[256]; /* added */
int selresult; /* added */
int combomode, first;
double hiaccum, loaccum;
int goodfound;
double submin, submax;
double fabs(), floor();
int mininit, maxinit;
int ffgiven;



/* see what scaletype is being used.. */
Egetunits( axis, unittyp );
if( strcmp( unittyp, "linear" )==0 ) strcpy( nearest, "auto" );
else if ( GL_smemberi( unittyp, "date time datetime" )) strcpy( nearest, "datematic" );
else strcpy( nearest, "exact" ); /* categories? */

margin = 0.0;
strcpy( dfield, "" );
ix = 0;
strcpy( lowfix, "" );
strcpy( hifix, "" );
strcpy( floatformat, "%g" );  /* changed scg 10/1/03 .. scientific formats (e+ and e-) should be ok now */
ffgiven = 0;
incmult = 1.0;
strcpy( selex, "" ); /* added */
combomode = 0;
mininit = maxinit = 0;

while( 1 ) {
        strcpy( buf, GL_getok( specline, &ix ) );
        if( buf[0] == '\0' ) break;
        if( strncmp( buf, "datafields", 10 )==0 ) strcpy( dfield, &buf[11] );
        else if( strncmp( buf, "datafield", 9 )==0 ) strcpy( dfield, &buf[10] ); 
        else if( strncmp( buf, "incmult", 7 )==0 ) incmult = atof( &buf[8] );
        else if( strncmp( buf, "nearest", 7 )==0 ) strcpy( nearest, &buf[8] );
        else if( strncmp( buf, "margin", 6 )==0 ) margin = atof( &buf[7] );
        else if( strncmp( buf, "lowfix", 6 )==0 ) strcpy( lowfix, &buf[7] );
        else if( strncmp( buf, "hifix", 5 )==0 ) strcpy( hifix, &buf[6] );
        else if( strncmp( buf, "mininit", 7 )==0 ) { strcpy( lowfix, &buf[8] ); mininit = 1; }
        else if( strncmp( buf, "maxinit", 7 )==0 ) { strcpy( hifix, &buf[8] ); maxinit = 1; }
        else if( strncmp( buf, "numformat", 9 )==0 ) { strcpy( floatformat, &buf[10] ); ffgiven = 1; }
        else if( strncmp( buf, "combomode", 9 )==0 ) {
		if( strcmp( &buf[10], "stack" )==0 ) combomode = 1;
		else if( strcmp( &buf[10], "hilo" )==0 ) combomode = 2;
		else combomode = 0;
		}
        else if( strncmp( buf, "selectrows", 10 )==0 ) {
                strcpy( selex, &buf[11] );
                strcat( selex, &specline[ix] );
                break;
                }
	else Eerr( 5702, "unrecognized autorange subattribute", buf );
	}

/* fill array of df from comma-delimited list - contributed by Paul Totten <pwtotten@nortelnetworks.com> */
ndf = 0; ix = 0; 
while( 1 ) {
	GL_getseg( tok, dfield, &ix, "," );
	if( tok[0] == '\0' ) break;
	if( ndf >= (MAXFIELDS-1) ) break;
	df[ndf] = fref( tok ) - 1;
	if( df[ndf] < 0 || df[ndf] >= Nfields ) continue;
	ndf++;
	}

/* df = fref( dfield ) - 1; */

if( Nrecords < 1 ) return( Eerr( 17, "autorange: no data set has been read/specified w/ proc getdata", "" ) );
if( axis == '\0' ) return( Eerr( 7194, "autorange: axis attribute must be specified", "" ) );
if( ndf == 0 ) return( Eerr( 7194, "autorange: datafield omitted or invalid ", dfield ) );


/* ------------------ */
/* now do the work.. */
/* ----------------- */

/* override.. */

/* find data min and max.. */


/* initialize.. */
if( mininit ) min = Econv( axis, lowfix );
else min = PLHUGE;
if( maxinit ) max = Econv( axis, hifix );
else max = NEGHUGE;

for( i = 0; i < Nrecords; i++ ) {

        if( selex[0] != '\0' ) { /* added scg 8/1/01 */
                do_select( selex, i, &selresult );
                if( selresult == 0 ) continue;
                }


	hiaccum = 0.0; loaccum = 0.0; first = 1; goodfound = 0; submin = PLHUGE; submax = NEGHUGE;

	for( j = 0; j < ndf; j++ ) {  /* for all datafields to be examined.. */
		fval = fda( i, df[j], axis );
		if( Econv_error() ) continue;
		goodfound = 1;
		if( !combomode ) {
			if( fval < submin ) submin = fval;
			if( fval > submax ) submax = fval;
			}
		else 	{
			hiaccum += fval;
			if( combomode == 2 ) { /* hilo */
				if( first ) { loaccum = fval; first = 0; }
				else loaccum -= fval;
				}
			else	{ /* stack */
				if( first ) { loaccum = fval; first = 0; }
				}
			}
		}
	if( !goodfound ) continue;

	if( combomode ) {
		if( loaccum < min ) min = loaccum;
		if( hiaccum > max ) max = hiaccum;
		}
	else	{
		if( submin < min ) min = submin;
		if( submax > max ) max = submax;
		}
	}


/* If user didn't specify "numformat", try to be "smart" about whether to use %g or %f for building result..         *
 * %g is usually best but in certain cases (high magnitude low variance data) %f should be used.  Added scg 8/10/05. */
if( !ffgiven && fabs( min ) > 100000 && fabs( max ) > 100000 && ( max - min < 1000 )) strcpy( floatformat, "%f" );


/* now convert min and max to current units and then to nearest interval.. */
if( strcmp( unittyp, "linear" )==0 ) { /* avoid using Euprint()- trouble w/ v. big or v. small #s */
	sprintf( smin, floatformat, min );
	sprintf( smax, floatformat, max );
	}
else	{
	Euprint( smin, axis, min, "" ); /* smin[80] */
	Euprint( smax, axis, max, "" ); /* smax[80] */
	}

/* save data min/max.. */
if( axis == 'x' ) {
	setcharvar( "DATAXMIN", smin );
	setcharvar( "DATAXMAX", smax );
	}
else 	{
	setcharvar( "DATAYMIN", smin );
	setcharvar( "DATAYMAX", smax );
	}


/* now adjust for margin.. */
min -= margin;
max += margin;


/* degenerate case.. all data the same (bad if it happens to lie on inc boundary, eg: 0) - added scg 9/21/01 */
if( min == max ) {
	/* min = min - 1.0; max = max + 1.0; */
	min -= fabs(min*0.2); max += fabs(max*0.2);  /* changed to work better w/ small magnitude values - scg 3/3/05 */
					             /* now uses fabs().. to handle degen case of a single neg. data value- scg 7/31/05 */
	if( min == max ) { min = min - 1.0; max = max + 1.0; } /* this kicks in if min=0 and max=0 - scg 7/6/05 */
	}


/* and do the conversion with margin.. */
if( strcmp( unittyp, "linear" )==0 ) { /* avoid using Euprint()- trouble w/ v. big or v. small #s */
	sprintf( smin, floatformat, min );
	sprintf( smax, floatformat, max );
	}
else	{
	Euprint( smin, axis, min, "" ); /* smin[80] */
	Euprint( smax, axis, max, "" ); /* smax[80] */
	}


/******* handle nearest=  ***********/

if( strcmp( nearest, "datematic" )==0 ) {
	char foo1[40], foo2[40], foo3[40], foo4[40], foo5[40], foo6[40];
	double dfoo1, dfoo2;
	/* get an automatic reasonable "nearest" value.. */
	DT_reasonable( unittyp, min, max, &dfoo1, foo1, foo2, foo3, foo4, foo5, &dfoo2, foo6, nearest );
	}


if( strcmp( nearest, "exact" )==0 ) {  /* exact */
	sprintf( minval, "%s", smin ); 
	sprintf( maxval, "%s", smax ); 
	}

else if( PLP_findnearest( smin, smax, axis, nearest, minval, maxval ) );

else 	{      
	double inc, h, fmod(), a, b;

	if( strcmp( nearest, "auto" )==0 ) PL_defaultinc( min, max, &inc );
	else inc = atof( nearest );


	h = fmod( min, inc );

	a = (min - h) - inc;
	b = a - inc;    /* include one extra inc on low end */
	if( a >= 0.0 && b < 0.0 ) b = a;  /* but don't dip below 0 */

	if( min < 0.0 ) sprintf( minval, floatformat, (min - h) - (inc * incmult) ); /* include extra inc on low end - scg 11/29/00 */
	else 	{
		a = min - h;
		b = a - (inc*(incmult-1.0)); /* include extra inc on low end - 11/29 */
		if( a >= 0.0 && b < 0.0 ) b = a;  /* but don't dip below 0  - 11/29 */
		sprintf( minval, floatformat, b );
		}

	h = fmod( max, inc );
	if( max < 0.0 ) sprintf( maxval, floatformat, (max - h) + (inc*(incmult-1.0)) ); /* include extra inc on high end - 11/29 */
	else sprintf( maxval, floatformat, (max - h) + (inc * incmult) ); /* extra inc - 11/29 */
	}



/* lowfix and hifix overrides.. */
if( lowfix[0] != '\0' && !mininit ) strcpy( minval, lowfix );
if( hifix[0] != '\0' && !maxinit ) strcpy( maxval, hifix );

/* be sure result makes sense.. for instance, if lowfix=0 specified but data are all negative,
	min will be 0 but max will be eg. -3.... patch that up here to avoid areadef crash&burn 
	(automated situations) - scg 3/25/04 */
if( GL_slmember( unittyp, "linear log*" ) && atof( maxval ) <= atof( minval ) ) {
	Eerr( 5709, "autorange: all data out of range", "" );
	strcpy( minval, "0" );
	strcpy( maxval, "1" );
	}

if( PLS.debug )
  fprintf( PLS.diagfp, "Autorange on %c: min=%s to max=%s\n", axis, minval, maxval);

DT_suppress_twin_warn( 1 ); /* suppress complaints about datetime outside of window 
				until after areadef */
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
