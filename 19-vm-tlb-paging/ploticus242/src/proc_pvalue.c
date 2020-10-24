
/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC PVALUE - compute and display P values */

/* for now, this assumes X is the base axis */

#include "pl.h"

extern double GL_ttest( );

int
PLP_pvalue( )
{
int lvp, first, irow;
char attr[NAMEMAXLEN];
char *line, *lineval;

char *textdetails, *selectex, *printfmt, *signifcolor;
char fname[3][NAMEMAXLEN], str[80], printfmt2[80];
int statf[2][3]; /* field numbers for mean, sd, n, for 2 cases */
int i, j, xfield, nt, stat, select_result, align;
double lowp, signif, adjx, adjy, pval, x, y;

TDH_errprog( "pl proc pvalue" );


/* initialize */
xfield = -1;	
textdetails = ""; selectex = ""; signifcolor = "yellow";
lowp = 0.01;
signif = 0.05;
printfmt = "p=%2.2f";


/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
        first = 0;
        lineval = &line[lvp];

	if( strcmp( attr, "xfield" )==0 ) xfield = fref( lineval )-1;
	else if( strcmp( attr, "statfields1" )==0 ) {
		nt = sscanf( lineval, "%s %s %s", fname[0], fname[1], fname[2] );
		if( nt != 3 ) return( Eerr( 7295, "3 values (mean sd n) expected for statfields1", "" ) );
		statf[0][0] = fref( fname[0] )-1;
		statf[0][1] = fref( fname[1] )-1;
		statf[0][2] = fref( fname[2] )-1;
		}
	else if( strcmp( attr, "statfields2" )==0 ) {
		nt = sscanf( lineval, "%s %s %s", fname[0], fname[1], fname[2] );
		if( nt != 3 ) return( Eerr( 7295, "3 values (mean sd n) expected for statfields2", "" ) );
		statf[1][0] = fref( fname[0] )-1;
		statf[1][1] = fref( fname[1] )-1;
		statf[1][2] = fref( fname[2] )-1;
		}
        else if( strcmp( attr, "textdetails" )==0 ) textdetails = lineval; 
        else if( strcmp( attr, "lowp" )==0 ) lowp = ftokncpy( lineval );
        else if( strcmp( attr, "signif" )==0 ) signif = ftokncpy( lineval );
        else if( strcmp( attr, "signifcolor" )==0 ) signifcolor = lineval;
        else if( strcmp( attr, "select" )==0 ) selectex = lineval; 
        else if( strcmp( attr, "printformat" )==0 ) printfmt = lineval; 
	else Eerr( 1, "attribute not recognized", attr );
	}


if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() ) return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

if( xfield > Nfields || xfield < 0 ) return( Eerr( 52, "xfield invalid", "" ) );
for( i = 0; i < 2; i++ ) {
	for( j = 0; j < 3; j++ ) {
		if( statf[i][j] > Nfields || statf[i][j] < 0 ) return( Eerr( 52, "statfields spec invalid", "" ) );
		}
	}

if( strcmp( signifcolor, "none" )==0 ) signifcolor = "";
	


/* build printfmt2 (to handle cases where  p < 0.05  or whatever */
for( i = 0; printfmt[i] != '\0'; i++ ) {
	if( printfmt[i] == '=' ) printfmt2[i] = '<';
	else printfmt2[i] = printfmt[i];
	}
printfmt2[i] = '\0';

textdet( "textdetails", textdetails, &align, &adjx, &adjy, -3,"C", 1.0 );
 PLG_forcecolorchg( );

for( irow = 0; irow < Nrecords; irow++ ) {

	if( selectex ) {
		stat = do_select( selectex, irow, &select_result );
		if( select_result == 0 || stat ) continue; /* reject */
		}

	pval = GL_ttest( atof( da( irow, statf[0][0] ) ), atof( da( irow, statf[0][1] ) ), atoi( da( irow, statf[0][2] ) ), 
		      atof( da( irow, statf[1][0] ) ), atof( da( irow, statf[1][1] ) ), atoi( da( irow, statf[1][2] ) ) );

	sprintf( str, "%g", pval );
	if( strcmp( str, "NaN" )==0 ) pval = 1.0;  /* ??? */

	if( pval < lowp ) sprintf( str, printfmt2, lowp );
	else sprintf( str, printfmt, pval );

	x = Eax( fda( irow, xfield, 'x' ) ) + adjx;
	y = Eay( EDYlo ) + adjy;

	/* do highlighting of statistically significant p values... */
	if( pval <= signif && signifcolor ) {
		double hbw, bh;
		hbw = (strlen( str ) * Ecurtextwidth ) * 0.5;
		bh = Ecurtextheight*0.8;
		Ecblock( x-hbw, y, x+hbw, y+bh, signifcolor, 0 );
		}

	Emov( x, y );
	Edotext( str, 'C' );
	}
return( 0 );
}

