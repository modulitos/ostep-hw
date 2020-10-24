/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC USEDATA */

/* Note: if no attributes given, this does the same as the old 'proc originaldata' */

#include "pl.h"

int
PLP_usedata( )
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;
int npop;

TDH_errprog( "pl proc usedata" );

if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );

npop = 0;  /* pop: all */

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "pop" )== 0 ) { 
		if( strncmp( lineval, "all", 3 )==0 ) npop = 0;
		else npop = itokncpy( lineval );
		}
	else if( strncmp( attr, "original", 8 )== 0 ) npop = 0;
	else if( strcmp( attr, "fieldnames" )==0 ) definefieldnames( lineval );
	else Eerr( 1, "attribute not recognized", attr );
	}

PL_popdataset( npop );     

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
