/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC LEGENDENTRY - proc to define one legend entry */

#include "pl.h"

int
PLP_legendentry( )
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char *label, *sampletype, *spec1, *spec2, *spec3, *tag;
int  samptyp;

TDH_errprog( "pl proc legendentry" );


/* initialize */
label = "";
sampletype = "none";
spec1 = "";
spec2 = "";
spec3 = "";
tag = "";

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "label" )==0 ) label = lineval;
	else if( strcmp( attr, "tag" )==0 ) tag = lineval; 
	else if( strcmp( attr, "sampletype" )==0 ) sampletype = lineval;
	else if( strcmp( attr, "details" )==0 ) spec1 = lineval;
	else if( strcmp( attr, "details2" )==0 ) spec2 = lineval;
	else if( strcmp( attr, "details3" )==0 ) spec3 = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}

samptyp = 0; /* for 'none' */
if( strcmp( sampletype, "line" )==0 ) samptyp = LEGEND_LINE;
else if( strcmp( sampletype, "color" )==0 ) samptyp = LEGEND_COLOR;
else if( strcmp( sampletype, "symbol" )==0 ) samptyp = LEGEND_SYMBOL;
else if( strcmp( sampletype, "text" )==0 ) samptyp = LEGEND_TEXT;
else if( strcmp( sampletype, "line+symbol" )==0 ) samptyp = LEGEND_LINE + LEGEND_SYMBOL;
else if( strcmp( sampletype, "text+symbol" )==0 ) samptyp = LEGEND_TEXT + LEGEND_SYMBOL;

PL_add_legent( samptyp, label, tag, spec1, spec2, spec3 );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
