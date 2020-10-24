/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC PRINT - allow printing of variable or data field contents  */

#include "pl.h"

int
PLP_print()
{
int i, lvp, first;
char attr[NAMEMAXLEN], *line, *lineval;

char *printstring, *label, *selectex, *outfile, *outmode;
char buf[512], tok[80];
int result, dontclose, nrecords;
FILE *outfp;

TDH_errprog( "pl proc print" );

/* initialize */
printstring = ""; label = ""; selectex = ""; outfile = ""; 
outmode = "w";

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "label" )==0 ) { label = lineval; convertnl( label ); }
	else if( strcmp( attr, "print" )==0 ) { printstring = lineval; convertnl( printstring ); }
	else if( strcmp( attr, "outfile" )==0 ) outfile = lineval;
	else if( strcmp( attr, "outmode" )==0 ) outmode = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}

if( printstring[0] != '\0' && Nrecords < 1 ) return( Eerr( 17, "Warning: no data has been read yet w/ proc getdata", "" ) );


/* now do the work.. */
dontclose = 0;
if( outfile[0] != '\0' ) {
	sprintf( tok, "%c", outmode[0] );
	outfp = fopen( outfile, tok );
	if( outfp == NULL ) { Eerr( 7259, "cannot open outfile", outfile ); outfp = PLS.diagfp; dontclose = 1; }
	}
else 	{ 
	outfp = PLS.diagfp; 
	dontclose = 1;
	}

if( label[0] != '\0' ) fprintf( outfp, "%s\n", label );

nrecords = 0;
setintvar( "FOUNDROW", 0 );
for( i = 0; i < Nrecords; i++ ) {
	do_select( selectex, i, &result );
	if( result == 1 ) {
		setintvar( "FOUNDROW", i+1 );
		if( printstring[0] != '\0' ) {
			do_subst( buf, printstring, i, NORMAL );
			fprintf( outfp, "%s\n", buf );
			}
		nrecords++;
		}
	}

setintvar( "NSELECTED", nrecords );

if( !dontclose ) fclose( outfp );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
