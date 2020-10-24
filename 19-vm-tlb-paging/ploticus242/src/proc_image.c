/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"

extern int PLGG_getimfmt();

int
PLP_image()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first, stat;

char *filename, *align, *mapurl, *maplabel;
double x, y, aw, ah;
int width, height;

TDH_errprog( "pl proc image" );

/* initialize */
filename = "";
align = "topleft";
x = 3.0; y = 3.0;
width = height = 0;
mapurl = "";
maplabel = "";

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strncmp( attr, "file", 4 )==0 ) filename = lineval;
	else if( strcmp( attr, "location" )==0 ) { if( lineval[0] != '\0' ) getcoords( "location", lineval, &x, &y ); } 
	else if( strcmp( attr, "height" )==0 ) height = itokncpy( lineval );  /* new */
	else if( strcmp( attr, "width" )==0 ) width = itokncpy( lineval );    /* new */
	else if( strcmp( attr, "align" )==0 ) align = lineval;  /* center  topcenter  topleft  bottomleft */
	else if( strcmp( attr, "clickmapurl" )==0 ) mapurl = lineval;
	else if( strcmp( attr, "clickmaplabel" )==0 ) maplabel = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}

/* for GD, image alignment (centering, etc) is done in gdgr.c (where image size is automatically available).
 * for SVG, image alignment (centering, etc) is done here, and image width and height must be given in order to do alignment.
 */

if( filename[0] == '\0' ) return( Eerr( 72509, "image file not specified", "" ));

if( height == 0 || width == 0 ) {
	Eerr( 75935, "warning, image height and width (in pixels) expected but not specified", "" );
	if( Edev != GD ) { height = 100; width = 100; }
	} 

if( Edev == GD ) {
	stat = Eimload( filename, 0, 0 );
	if( stat != 0 ) return( Eerr( 5892, "Error on image load", filename ) );
	stat = Eimplace( x, y, "", align, width, height );
	}

aw = width / 100.0;  /* images have 100 pixels per inch */
ah = height / 100.0; 
if( strcmp( align, "center" )==0 ) { x-= (aw/2.0); y+= (ah/2.0); }
else if( strcmp( align, "topcenter" )==0 ) { x-= (aw/2.0); }
else if( strcmp( align, "bottomleft" )==0 ) { y-= ah; }
if( x < 0.0 ) x = 0.0;
if( y < 0.0 ) y = 0.0;
if( Edev == SVG ) {
	/* do some invisible drawing to update bounding box.. */
	Ecolor( Ecurbkcolor ); Elinetype( 0, 0.0, 1.0 );
	Emov( x, y ); Elin( x+0.001, y ); Emov( x+aw, y-ah ); Elin( x+aw+0.001, y-ah ); 
	stat = Eimplace( x, y, filename, "", width, height );
	}
else if( Edev != GD ) {
	Ecolor( "gray(0.8)" ); Elinetype( 0, 0.5, 1.0 );
        Emov( x, y ); Elin( x+aw, y ); Elin( x+aw, y-ah );  Elin( x, y-ah ); Elin( x, y );
	}

if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) clickmap_entry( 'r', mapurl, 0, x, y-ah, x+aw, y, 0, 0, maplabel );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
