/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC RANGESWEEP - render a sweep */

#include "pl.h"
#define MOVE 0
#define LINE 1
#define PATH 2

int
PLP_rangesweep()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

int i, j, stat, npoints, result, xfield, lofield, hifield;
double start, stop, xstart, f; double x, lo, hi, lastx, lastlo, lasthi;
char *color, *legendlabel, *selectex;
char oldcolor[COLORLEN];

TDH_errprog( "pl proc rangesweep" );

/* initialize */
xfield = -1; lofield = -1; hifield = -1;
start = EDXlo; stop = EDXhi; xstart = EDXlo;

color = "gray(0.9)";
legendlabel = "";
selectex = "";

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "xfield" )==0 ) xfield = fref( lineval ) - 1;
	else if( strcmp( attr, "lofield" )==0 ) lofield = fref( lineval ) - 1;
	else if( strcmp( attr, "hifield" )==0 ) hifield = fref( lineval ) - 1;
	else if( strcmp( attr, "legendlabel" )==0 ) legendlabel = lineval;
	else if( strcmp( attr, "sweeprange" )==0 ) getrange( lineval, &start, &stop, 'x', EDXlo, EDXhi );
	else if( strcmp( attr, "xstart" )==0 ) { xstart = Econv( X, lineval ); if( Econv_error() ) xstart = EDXlo; }
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "color" )==0 ) color = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}

/* -------------------------- */
/* overrides and degenerate cases */
/* -------------------------- */
if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() ) 
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );


if( (lofield < 0 || lofield >= Nfields )) return( Eerr( 601, "lofield out of range", "" ) );
if( (hifield < 0 || hifield >= Nfields )) return( Eerr( 601, "hifield out of range", "" ) );
if( xfield >= Nfields ) return( Eerr( 601, "xfield out of range", "" ) );
 
/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */

/* put all values into PLV array.. */
j = 0;
f = xstart;
for( i = 0; i < Nrecords; i++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, i, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( result == 0 ) continue;
                }


	/* X */
	if( xfield < 0 ) {
		PLV[j] = f;
		f += 1.0;
		}
	else 	{
		PLV[j] = fda( i, xfield, X );
		if( Econv_error() ) { 
			conv_msg( i, xfield, "xfield" ); 
			PLV[j] = NEGHUGE;
			}
		}

	j++; 

	/* LO */
	PLV[j] = fda( i, lofield, Y );
	if( Econv_error() ) { 
		conv_msg( i, lofield, "yfield" ); 
		PLV[j] = NEGHUGE;
		/* continue; */
		}
	j++;


	/* HI */
	PLV[j] = fda( i, hifield, Y );
	if( Econv_error() ) { 
		conv_msg( i, hifield, "hifield" ); 
		PLV[j] = NEGHUGE;
		/* continue; */
		}
	j++;



	if( j >= PLVsize-3 ) {
		Eerr( 3579, "Too many points, sweep truncated (raise using -maxvector)", "" );
		break;
		}
	}

npoints = j / 3;



/* draw the sweep.. */
/* ---------------- */

first = 1;
lastlo = 0.0;
lasthi = 0.0;
lastx = 0.0;

strcpy( oldcolor, Ecurcolor );
Ecolor( color );

for( i = 0; i < npoints; i++ ) {
	if( !first && (hi > (NEGHUGE+1) && lo > (NEGHUGE+1) && 
	       x > (NEGHUGE+1) && x < (PLHUGE-1) ) )  { 
		lastlo = lo; 
		lasthi = hi; 
		lastx = x;
		}
	x = dat3d(i,0);
	lo = dat3d(i,1);
	hi = dat3d(i,2);

	/* fprintf( stderr, "[last: x=%g lo=%g hi=%g   current: x=%g lo=%g hi=%g]", lastx, lastlo, lasthi, x, lo, hi ); */

	/* skip bad values and places */
	if( x < (NEGHUGE+1) || lo < (NEGHUGE+1) || hi < (NEGHUGE+1) ) { 
		/* fprintf( stderr, "[skip]\n" );  */
		continue; 
		}

	/* if lo > hi reset so a new sweep can be started later.. */
	if( lo > hi || x > (PLHUGE-1) ) { 
		first = 1;
		/* fprintf( stderr, "[reset]\n" ); */
		continue;
		}

	if( x < start ) {
		/* fprintf( stderr, "[too lo]\n" ); */
		continue; /* out of range - lo */
		}
	if( x > stop ) {     /* out of range - hi */ 
		/* fprintf( stderr, "[too hi]\n" ); */
		break;
		}


	if( first ) {
		/* fprintf( stderr, "[First]\n" ); */
		first = 0;
		continue;
		}

	if( !first ) {
		/* fprintf( stderr, "[Draw]\n" ); */
		Emovu( x, lo ); Epathu( lastx, lastlo ); 
		Epathu( lastx, lasthi ); 
		Epathu( x, hi );
		/* Ecolorfill( color ); */ /* using Efill  scg 6/18/04 */
		Efill();
		continue;
		}
	}

Ecolor( oldcolor );

 
if( legendlabel[0] != '\0' ) {
	PL_add_legent( LEGEND_COLOR, legendlabel, "", color, "", "" );
	}

return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
