/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */


#include "pl.h"

int
PLP_rect()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char *color, *lowbevelcolor, *hibevelcolor, *shadowcolor, *outline, *mapurl, *maplabel;
double xlo, ylo, xhi, yhi, bevelsize, shadowsize;
int gotrect, ioutline;

TDH_errprog( "pl proc rect" );


/* initialize */
outline = ""; mapurl = ""; maplabel = "";
color = "dullyellow";
lowbevelcolor = "0.6"; hibevelcolor = "0.8"; shadowcolor = "black"; 
bevelsize = 0.0; shadowsize = 0.0;
ioutline = 0; gotrect = 0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "rectangle" )==0 ) { getbox( "box", lineval, &xlo, &ylo, &xhi, &yhi ); gotrect = 1; }
	else if( strcmp( attr, "color" )==0 ) color = lineval;
	else if( strcmp( attr, "bevelsize" )==0 ) bevelsize = ftokncpy( lineval );
	else if( strcmp( attr, "shadowsize" )==0 ) shadowsize = ftokncpy( lineval );
	else if( strcmp( attr, "lowbevelcolor" )==0 ) lowbevelcolor = lineval;
	else if( strcmp( attr, "hibevelcolor" )==0 ) hibevelcolor = lineval;
	else if( strcmp( attr, "shadowcolor" )==0 ) shadowcolor = lineval;
	else if( strcmp( attr, "clickmapurl" )==0 ) mapurl = lineval;
	else if( strcmp( attr, "clickmaplabel" )==0 ) maplabel = lineval;
        else if( strcmp( attr, "clickmaplabeltext" )==0 ) maplabel = getmultiline( lineval, "get" );
	else if( strcmp( attr, "outline" )==0 ) { outline = lineval; if( GL_smember( lineval, "no none" )==0 ) ioutline = 1; }
	else Eerr( 1, "attribute not recognized", attr );
	}



/* now do the plotting work.. */

if( !gotrect ) return( Eerr( 695, "No rectangle specified", "" ));
if( strcmp( color, "none" )==0 ) strcpy( color, "" );/* "none" added scg 1/21/05 */

linedet( "outline", outline, 0.5 );
Ecblock( xlo, ylo, xhi, yhi, color, ioutline );

if( bevelsize > 0.0 || shadowsize > 0.0 ) 
	Ecblockdress( xlo, ylo, xhi, yhi, bevelsize, lowbevelcolor, hibevelcolor, shadowsize, shadowcolor);

if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) clickmap_entry( 'r', mapurl, 0, xlo, ylo, xhi, yhi, 0, 0, maplabel );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
