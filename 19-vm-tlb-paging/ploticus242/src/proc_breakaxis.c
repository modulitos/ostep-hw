/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC BREAKAXIS - break symbol for broken axis or broken bar */

#include "pl.h"

int
PLP_breakaxis()
{
char attr[NAMEMAXLEN], *line, *lineval; 
int lvp, first;

char *loc, *breakpt, *fillcolor, *linedetails, *style;
char axis, opax;
double len, gapsize, bkpt, x, y, ofs, hlen;

TDH_errprog( "breakaxis" );

/* initialize */
axis = 'y';
loc = "axis"; 
breakpt = ""; 
fillcolor = Ecurbkcolor;
len = 0.3;
gapsize = 0.05;
linedetails = "";
style = "slant";

/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "axis" )==0 ) axis = lineval[0];
	else if( strcmp( attr, "location" )==0 ) loc = lineval;
	else if( strcmp( attr, "linelength" )==0 ) { len = itokncpy( lineval ); if( PLS.usingcm ) len /= 2.54; }
	else if( strcmp( attr, "breakpoint" )==0 ) breakpt = lineval;
	else if( strcmp( attr, "fillcolor" )==0 ) fillcolor = lineval;
	else if( strcmp( attr, "linedetails" )==0 ) linedetails = lineval;
	else if( strcmp( attr, "style" )==0 ) style = lineval;
	else if( strcmp( attr, "gapsize" )==0 ) { gapsize = itokncpy( lineval ); if( PLS.usingcm ) gapsize /= 2.54; }
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );
if( breakpt[0] == '\0' )
         return( Eerr( 9251, "The breakpoint attribute MUST be set", "" ) );


/* now do the plotting work.. */
/* -------------------------- */

bkpt = Econv( axis, breakpt );

if( axis == 'x' ) { opax = 'y'; Eflip = 1; }
else opax = 'x';


/* set line type */
linedet( "linedetails", linedetails, 0.5 );

hlen = len/2.0;
if( strcmp( loc, "axis" )==0 ) x = Elimit( opax, 'l', 'a' );   
else x = Ea( X, Econv( opax, loc ) );

y = Ea( Y, bkpt );

if( strncmp( style, "sl", 2 )==0 ) { 	/* do a 'slanted' break symbol */
	ofs = gapsize * -1.0;
	Emov( x - hlen, y + ofs );
	Epath( x + hlen, y );
	Epath( x + hlen, y - ofs );
	Epath( x - hlen, y );
	Ecolorfill( fillcolor );
	if( strncmp( linedetails, "no", 2 ) != 0 ) {
		Emov( x - hlen, y + ofs );
		Elin( x + hlen, y );
		Emov( x - hlen, y );
		Elin( x + hlen, y - ofs );
		}
	}

else	{	  /* do a straight break symbol */
	ofs = gapsize / 2.0;
	x = Ea( X, Econv( opax, loc ) );
	y = Ea( Y, bkpt );
	Ecblock( x - hlen, y - ofs, x + hlen, y + ofs, fillcolor, 0 );
	if( strncmp( linedetails, "no", 2 ) != 0 ) {
		Emov( x - hlen, y - ofs );
		Elin( x + hlen, y - ofs );
		Emov( x - hlen, y + ofs );
		Elin( x + hlen, y + ofs );
		}
	}

if( axis == 'x' ) Eflip = 0;

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
