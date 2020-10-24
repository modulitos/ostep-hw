/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC BOXPLOT - render boxplots */

/* 10/31/07 scg - in version 2.34 proc rangebar has been renamed and pared down.  It now requires precomputed stats (always)
 *		Stats can be precomputed in 2.34+ using proc processdata (action: summaryplus).
 *		Options for log transform, display of outlier data points, #missing annotation, 
 *		1.5 IQR tails, and output of computed stats, are no longer supported.
 *		Ploticus variables such as RANGEBARMIN are no longer set.
 */

#include "pl.h"

#define MEDIANBASED 0
#define MEANBASED 1

#define LOWTAIL 0
#define LOWBOX 1
#define MIDBOX 2
#define HIBOX 3
#define HITAIL 4

#define N_OBS 0
#define MEAN 1

#define SD 2
#define MIN 3
#define MAX 4

#define PCTL5 2
#define PCTL25 3
#define MEDIAN 4
#define PCTL75 5
#define PCTL95 6

#define COLOR 0
#define SYM 1

/* ========================= */
int
PLP_boxplot()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char baseax, axis; 
char *statfields, *tailmode, *nlocation, *taildet, *outlinedet, *barcolor, *textdet;
char *medsym, *selectex, *meansym, *legendlabel;
char buf[256], symcode[50], sfbuf[80];
int i, stat, align, ix, locfield, printn, baroutline;
int pf[10], fnftics, trunc, result, npf, imeth, irow, legendtype;  /* bug fix 3/20/09, was pf[6] */
double stats[20], h[5];
double adjx, adjy, nloc, barwidth, hb, r, ticsize, ticlen, radius, barloc;

TDH_errprog( "pl proc boxplot" );

/* initialize */
axis = 'y';
tailmode = "5/95";
nlocation = ""; textdet = ""; taildet = ""; outlinedet = ""; selectex = ""; meansym = ""; statfields = ""; legendlabel = "";
barcolor = "gray(0.8)";
medsym = "line";
printn = 1;
barwidth = 0.12;
baroutline = 1;
ticsize = 0.7; /* % of box width */
fnftics = 1;
trunc = 1;
locfield = -1;
imeth = MEDIANBASED;
npf = 0;
legendtype = COLOR;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "orientation" )==0 ) {
		if( lineval[0] == 'h' ) axis = 'x';
		else if( lineval[0] == 'v' ) axis = 'y';
		}
	else if( strcmp( attr, "barwidth" )==0 ) { barwidth = ftokncpy( lineval ); if( PLS.usingcm ) barwidth /= 2.54; }
	else if( strcmp( attr, "basis" )==0 ) { 
		if( strncmp( lineval, "mean", 4 )==0 ) imeth = MEANBASED; 
		else if( strncmp( lineval, "median", 6 )==0 ) imeth = MEDIANBASED; 
		}
	else if( strcmp( attr, "statfields" )==0 ) statfields = lineval;
	else if( strcmp( attr, "locfield" )==0 ) locfield = fref( lineval ) -1;  /* added 10/31/07 scg */
	else if( strcmp( attr, "tailmode" )==0 ) tailmode = lineval;
	else if( strcmp( attr, "95tics" )==0 ) fnftics = getyn( lineval );
	else if( strcmp( attr, "mediansym" )==0 ) medsym = lineval;
	else if( strcmp( attr, "taildetails" )==0 ) taildet = lineval;
	else if( strcmp( attr, "outlinedetails" )==0 ) { outlinedet = lineval; baroutline = 1; }
	else if( strcmp( attr, "color" )==0 ) barcolor = lineval;
	else if( strcmp( attr, "outline" )==0 ) baroutline = getyn( lineval );
	else if( strcmp( attr, "printn" )==0 ) printn = getyn( lineval );
	else if( strcmp( attr, "nlocation" )==0 ) nlocation = lineval;
	else if( strcmp( attr, "textdetails" )==0 ) textdet = lineval;
	else if( strcmp( attr, "truncate" )==0 ) trunc = getyn( lineval );
	else if( strcmp( attr, "ticsize" )==0 ) ticsize = ftokncpy( lineval ); /* a % of bar width */
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "meansym" )==0 ) meansym = lineval;
	else if( strcmp( attr, "legendlabel" )==0 ) legendlabel = lineval;
	else if( strcmp( attr, "legendtype" )==0 ) {
		if( lineval[0] == 'c' ) legendtype = COLOR;
		else if( lineval[0] == 's' ) legendtype = SYM;
		}
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
/* -------------------------- */
if( axis == 'x' ) baseax = 'y';
else baseax = 'x';

if( ! GL_smember( tailmode, "5/95 min/max" )) { Eerr( 582, "tailmode must be '5/95' or 'min/max'", tailmode ); tailmode = "5/95"; }

/* statfields.. */
if( imeth == MEDIANBASED && statfields[0] == '\0' ) {
	if( tailmode[0] == '5' ) strcpy( sfbuf, "n_obs pctl5th pctl25th median pctl75th pctl95th " ); 
	else if( tailmode[0] == 'm' ) strcpy( sfbuf, "n_obs min pctl25th median pctl75th max " ); 
	if( meansym[0] != '\0' ) strcat( sfbuf, "mean" );
	statfields = sfbuf;
	}

else if( imeth == MEANBASED && statfields[0] == '\0' ) statfields = "n_obs mean sd min max";

	
for( ix = 0, i = 0; i < 7; i++ ) { /* fill pf */
	strcpy( buf, GL_getok( statfields, &ix ));
	if( buf[0] == '\0' ) break;
	pf[i] = fref( buf ) - 1;
	if( PL_fref_error() ) return( Eerr( 583, "invalid statfield", buf ) );
	}
npf = i;

if( imeth == MEDIANBASED ) {
	if( meansym[0] != '\0' && npf != 7 ) return( Eerr( 2749, "expecting 7 statfields for median-based boxplots including mean", "" ));
	else if( meansym[0] == '\0' && npf != 6 ) return( Eerr( 2749, "expecting 6 statfields for median-based boxplots", "" ));  /* bug fix 3/19/09 */
	}
else if( imeth == MEANBASED ) {
	if( npf != 5 ) return( Eerr( 2749, "expecting 5 statfields for mean-based boxplots", "" ));
	}

if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );

if( !scalebeenset() ) return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

if( nlocation[0] == '\0' ) nloc = Elimit( axis, 'l', 'a' ) + 0.05;
else Eposex( nlocation, axis, &nloc );


if( strcmp( medsym, "dot" )==0 || strcmp( medsym, "yes" )==0 ) medsym = "shape=pixcircle style=filled fillcolor=black radius=0.05";



/* render the boxplots */
/* ------------------- */

if( baseax == 'y' ) Eflip = 1;
hb = barwidth / 2.0;
ticlen = barwidth * ticsize * 0.5;
textdet( "textdetails", textdet, &align, &adjx, &adjy, -2, "R", 1.0 );

for( irow = 0; irow < Nrecords; irow++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, irow, &result );
                if( result == 0 || stat ) continue; /* reject */
                }


	if( imeth == MEANBASED ) {
		stats[N_OBS] = fda( irow, pf[0], axis );
		stats[MEAN] = fda( irow, pf[1], axis );
		stats[SD] = fda( irow, pf[2], axis );
		stats[MIN] = fda( irow, pf[3], axis );
		stats[MAX] = fda( irow, pf[4], axis );
		}
	else	{
		stats[N_OBS] = fda( irow, pf[0], axis );
		stats[PCTL25] = fda( irow, pf[2], axis );
		stats[MEDIAN] = fda( irow, pf[3], axis );
		stats[PCTL75] = fda( irow, pf[4], axis );
		if( tailmode[0] == '5' ) { stats[PCTL5] = fda( irow, pf[1], axis ); stats[PCTL95] = fda( irow, pf[5], axis ); }
		else if( tailmode[0] == 'm' ) { stats[MIN] = fda( irow, pf[1], axis ); stats[MAX] = fda( irow, pf[5], axis ); }
		if (meansym[0] != '\0') stats[MEAN] = fda( irow, pf[6], axis ); /* bug fix 3/19/09 */
		}


	/* bar location */
	if( locfield >= 0 ) barloc = fda( irow, locfield, baseax );
	else barloc = irow+1; /* as they fall */
	if( !Ef_inr( baseax, barloc ) ) { fprintf( PLS.errfp, "warning, boxplot location out of %c plotting area\n", baseax ); continue; }
	barloc = Ea(X,barloc); /* convert barloc to absolute units */

	if( printn ) {  /* print N= ... it appears even for degenerate or out of range cases */
		sprintf( buf, "N=%g", stats[N_OBS] );
		if( baseax == 'y' ) Emov( (barloc-(Ecurtextheight*0.3)) + adjx, nloc + adjy );
		else Emov( barloc + adjx, nloc + adjy );
		Ecentext( buf );
		}

	/* set drawing array h[] */
	if( imeth == MEANBASED ) {
		h[LOWTAIL] = stats[MIN];
		h[LOWBOX] = stats[MEAN] - stats[SD];
		h[MIDBOX] = stats[MEAN];
		h[HIBOX] = stats[MEAN] + stats[SD];
		h[HITAIL] = stats[MAX];
		}
	else 	{
		if( tailmode[0] == '5' ) { h[LOWTAIL] = stats[PCTL5]; h[HITAIL] = stats[PCTL95]; }
		else if( tailmode[0] == 'm' ) { h[LOWTAIL] = stats[MIN]; h[HITAIL] = stats[MAX]; }
		h[LOWBOX] = stats[PCTL25];
		h[MIDBOX] = stats[MEDIAN];
		h[HIBOX] = stats[PCTL75];
		}


	/* if N=0, skip out (degenerate case) */
	if( stats[N_OBS] <= 0.0 ) continue;

	/* if entire bar is out of plotting area, skip out.. */
	if( h[LOWTAIL] < Elimit( axis, 'l', 's' ) && h[HITAIL] < Elimit( axis, 'l', 's' ) ) 
		{ fprintf( PLS.errfp, "warning, entire boxplot out of %c plotting area (under)\n", axis ); continue; }
	
	if( h[LOWTAIL] > Elimit( axis, 'h', 's' ) && h[HITAIL] > Elimit( axis, 'h', 's' ) ) 
		{ fprintf( PLS.errfp, "warning, entire boxplot out of %c plotting area (over)\n", axis ); continue; }
	


	/* truncate values to be within range.. */
	if( trunc && !Ef_inr( axis, h[LOWTAIL] )) h[LOWTAIL] = Elimit( axis, 'l', 's' );
	if( trunc && !Ef_inr( axis, h[LOWBOX] )) h[LOWBOX] = Elimit( axis, 'l', 's' );
	if( trunc && !Ef_inr( axis, h[HIBOX] )) h[HIBOX] = Elimit( axis, 'h', 's' );
	if( trunc && !Ef_inr( axis, h[HITAIL] )) h[HITAIL] = Elimit( axis, 'h', 's' );

	linedet( "taildetails", taildet, 0.5 );
	Emov( barloc, Ea( Y, h[LOWTAIL] ) ); Elin( barloc, Ea( Y, h[LOWBOX] ) ); 
	Emov( barloc, Ea( Y, h[HIBOX] ) ); Elin( barloc, Ea( Y, h[HITAIL] ) ); 

	if( !trunc || Ef_inr( axis, h[LOWTAIL] ) ) { /* bottom tic */
		if( h[LOWTAIL] < h[LOWBOX] ) { /* only do if below bottom of box - added scg 5/23/00 */
			Emov( barloc-ticlen, Ea( Y, h[LOWTAIL] ) ); 
			Elin( barloc+ticlen, Ea( Y, h[LOWTAIL] ) ); 
			}
		}

	if( !trunc || Ef_inr( axis, h[HITAIL] ) ) { /* top tic */
		if( h[HITAIL] > h[HIBOX] ) { /* only do if above top of box - added scg 5/23/00 */
			Emov( barloc-ticlen, Ea( Y, h[HITAIL] ) );  /* slop added to hide line ending glitch */
			Elin( barloc+ticlen, Ea( Y, h[HITAIL] ) ); 
			}
		}

	if( tailmode[0] != '5' && fnftics && imeth == MEDIANBASED ) { /* add 5/95 ticks to min/max tails */
		if( !trunc || Ef_inr( axis, stats[PCTL5] )) {
			Emov( barloc-(ticlen*0.8), Ea( Y, stats[PCTL5] ) );
			Elin( barloc+(ticlen*0.8), Ea( Y, stats[PCTL5] ) );
			}
		if( !trunc || Ef_inr( axis, stats[PCTL95] )) {
			Emov( barloc-(ticlen*0.8), Ea( Y, stats[PCTL95] ) );
			Elin( barloc+(ticlen*0.8), Ea( Y, stats[PCTL95] ) );
			}
		}

	linedet( "outlinedetails", outlinedet, 0.5 );
	Ecblock( barloc-hb, Ea(Y,h[LOWBOX]), barloc+hb, Ea(Y,h[HIBOX]), barcolor, baroutline );

	/* do midbox */
	if( strcmp( medsym, "line" ) ==0 && (!trunc || Ef_inr( axis, h[MIDBOX] ))) {
		Emov( barloc-hb, Ea(Y,h[MIDBOX]) );
		Elin( barloc+hb, Ea(Y,h[MIDBOX]) );
		}
	else if( medsym[0] != '\0' && (!trunc || Ef_inr( axis, h[MIDBOX] ))) {
		symdet( "mediansym", medsym, buf, &r );
		if( Eflip ) Emark( Ea(Y,h[MIDBOX]), barloc, buf, r );
		else Emark( barloc, Ea(Y,h[MIDBOX]), buf, r );
		}


	/* display mean symbol on a median-based rangebar.. */
	if( imeth == MEDIANBASED && meansym[0] != '\0' ) {
		if( strcmp( meansym, "yes" )==0 || strcmp( meansym, "dot" )==0 ) meansym = "shape=circle style=filled fillcolor=black radius=0.02";
		symdet( "meansym", meansym, symcode, &radius );
		Emark( barloc, Ea(Y,stats[MEAN]), symcode, radius );
		}

	}

if( baseax == 'y' ) Eflip = 0;

if( legendlabel[0] != '\0' ) {
	if( medsym[0] != '\0' && legendtype == SYM ) PL_add_legent( LEGEND_SYMBOL, legendlabel, "", medsym, "", "" );
	else PL_add_legent( LEGEND_COLOR, legendlabel, "", barcolor, "", "" );
        }

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
