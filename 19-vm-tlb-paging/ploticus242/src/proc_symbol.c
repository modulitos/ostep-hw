/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC SYMBOL - render one symbol */
#include "pl.h"

int
PLP_symbol()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char symcode[80];
char *symbol;
double x, y, radius;

TDH_errprog( "pl proc symbol" );

/* initialize */
symbol = "";
x = 3.0;
y = 3.0;
radius = 0.2;

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "symbol" )==0 ) symbol = lineval;
	else if( strcmp( attr, "location" )==0 ) getcoords( "location", lineval, &x, &y );
	else Eerr( 1, "attribute not recognized", attr );
	}

symdet( "symbol", symbol, symcode, &radius );
Emark( x, y, symcode, radius );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
