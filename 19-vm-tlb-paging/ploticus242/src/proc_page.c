/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC PAGE - set page-wide attributes, and do a "page" break for pp 2 and up */

#include "pl.h"

extern int PLGS_setparms(), PLGG_setimpixsize();

int
PLP_page( )
{
int lvp, first;
char attr[NAMEMAXLEN], *line, *lineval;

char buf[512], devval[20];
char *outfilename, *mapfilename, *titledet, *pagetitle, *url;
int stat, nt, align, nlines, maxlen, landscapemode, dobackground, dopagebox, pagesizegiven, clickmap_enabled_here, tight, map;
double adjx, adjy, scalex, scaley, sx, sy;

TDH_errprog( "pl proc page" );

/* initialize */
landscapemode = PLS.landscape; /* from command line */
titledet = "";
outfilename = "";
mapfilename = "";
pagetitle = "";
dobackground = 1;
dopagebox = 1;
if( GL_member( PLS.device, "gesf" )) dopagebox = 0; /* bounding box shouldn't include entire page for gif , eps */
if( PLS.device == 'e' ) dobackground = 0; 
pagesizegiven = 0;
strcpy( devval, "" );
scalex = scaley = 1.0;
clickmap_enabled_here = 0;

/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
        if( line == NULL ) break;
        first = 0;
        lineval = &line[lvp];


	/* if an attribute is given on command line, it overrides anything here.. */
	if( GL_slmember( attr, PLS.cmdlineparms )) continue;
	if( strcmp( attr, "landscape" )==0 && GL_slmember( "portrait", PLS.cmdlineparms )) continue;
	if( strcmp( attr, "outfilename" )==0 && GL_slmember( "o", PLS.cmdlineparms )) continue;

	if( strcmp( attr, "landscape" )==0 ) landscapemode = getyn( lineval );
	else if( strcmp( attr, "title" )==0 ) pagetitle = getmultiline( lineval, "get" ); 
	else if( strcmp( attr, "titledetails" )==0 ) titledet = lineval;
	else if( strcmp( attr, "color" )==0 ) tokncpy( Estandard_color, lineval, COLORLEN );
	else if( strcmp( attr, "scale" )==0 ) { 
		nt = sscanf( lineval, "%lf %lf", &scalex, &scaley ); 
		if( nt == 1 ) scaley = scalex; 
		}
	else if( strcmp( attr, "backgroundcolor" )==0 ) {
		tokncpy( Estandard_bkcolor, lineval, COLORLEN );
		Ebackcolor( Estandard_bkcolor );
		dobackground = 1; /* added scg 9/27/99 */
		}
	else if( strcmp( attr, "linewidth" )==0 ) Estandard_lwscale = ftokncpy( lineval );
	else if( strcmp( attr, "textsize" )==0 ) Estandard_textsize = itokncpy( lineval );
	else if( strcmp( attr, "font" )==0 ) tokncpy( Estandard_font, lineval, FONTLEN );
	else if( strcmp( attr, "dobackground" )==0 ) dobackground = getyn( lineval );
	else if( strcmp( attr, "dopagebox" )==0 ) dopagebox = getyn( lineval );
	else if( strcmp( attr, "tightcrop" )==0 ) { tight = getyn( lineval ); Etightbb( tight ); }
	else if( strncmp( attr, "crop", 4 )==0 ) {
		double cropx1, cropy1, cropx2, cropy2;
		nt = sscanf( lineval, "%lf %lf %lf %lf", &cropx1, &cropy1, &cropx2, &cropy2 );
		if( nt != 4 ) Eerr( 2707, "usage: crop x1 y1 x2 y2 OR croprel left bottom right top", "" );
		else {
			if( PLS.usingcm ) { cropx1 /= 2.54; cropy1 /= 2.54; cropx2 /= 2.54; cropy2 /= 2.54; }
			if( strcmp( attr, "croprel" )==0 ) Especifycrop( 2, cropx1, cropy1, cropx2, cropy2 ); /* relative to tight */
			else Especifycrop( 1, cropx1, cropy1, cropx2, cropy2 ); /* absolute */
			}
		}

	else if( strcmp( attr, "pixsize" ) ==0 ) {  /* added scg 1/9/08 */
		int reqwidth, reqheight;
		nt = sscanf( lineval, "%d %d", &reqwidth, &reqheight );
		if( nt != 2 ) Eerr( 57233, "pixsize ignored.. it requires width and height (in pixels)", "" );
#ifndef NOGD
        	PLGG_setimpixsize( reqwidth, reqheight );
#endif
        	if( PLS.device != 'g' ) Eerr( 24795, "pixsize ignored.. it's only applicable when generating png/gif/jpeg images", "" ); 
		}

	else if( strcmp( attr, "pagesize" )==0 ) {
		getcoords( "pagesize", lineval, &(PLS.winw), &(PLS.winh) );
		pagesizegiven = 1;
		}
	else if( strcmp( attr, "outfilename" )==0 ) {
		outfilename = lineval;
		if( strlen( outfilename ) > MAXPATH-1 ) { PLS.skipout = 1; return( Eerr( 57932, "outfilename too long", "" ) ); }  
		}
	else if( strncmp( attr, "mapfile", 7 )==0 ) {
		mapfilename = lineval;
		if( strlen( mapfilename ) > MAXPATH-1 ) { Eerr( 57932, "mapfile name too long", "" ); mapfilename = ""; }
		}

	else if( strcmp( attr, "clickmapdefault" )==0 ) { 
		url = lineval;
		if( strlen( url ) > MAXURL-1 ) Eerr( 57933, "clickmapdefault url too long", "" );
		else clickmap_setdefaulturl( url ); 
		}

	else if( strcmp( attr, "map" )==0 ) { map = getyn( lineval ); if( map ) { PLS.clickmap = 1; clickmap_enabled_here = 1; }}
	else if( strcmp( attr, "csmap" )==0 ){ map = getyn( lineval ); if( map ) { PLS.clickmap = 2; clickmap_enabled_here = 1; }} 
	else if( strcmp( attr, "outlabel" )==0 ) Esetoutlabel( lineval );
	else Eerr( 1, "page attribute not recognized", attr );
	}



/* -------------------------- */
/* Page break logic.. */
/* -------------------------- */
if( PLS.npages == 0 ) {

	/* following 3 lines moved here from above - also replicated below.  scg 10/31/00 */
	if( scalex != 1.0 || scaley != 1.0 ) Esetglobalscale( scalex, scaley );
	Egetglobalscale( &sx, &sy );
	if( pagesizegiven ) Esetsize( PLS.winw * sx, PLS.winh * sy, PLS.winx, PLS.winy );
	else if( landscapemode && !PLS.winsizegiven ) Esetsize( 11.0, 8.5, PLS.winx, PLS.winy ); /* landscape */

	/* clickmap (must come before init for eg. svg - scg 2/7/05) */
	if( clickmap_enabled_here ) {
		if( mapfilename[0] == '\0' ) {
        		if( PLS.clickmap == 2 ) strcpy( PLS.mapfile, "stdout" );  /* csmap defaults to stdout..  scg 8/26/04  */
        		else if( PLS.outfile[0] != '\0' ) makeoutfilename( PLS.outfile, PLS.mapfile, 'm', 1);
        		else strcpy( PLS.mapfile, "unnamed.map" );
        		}
		PL_clickmap_init();
#ifndef NOSVG
		/* must update this now too.. scg 2/7/05  */
		if( PLS.device == 's' ) PLGS_setparms( PLS.debug, PLS.tmpname, PLS.clickmap );
#endif
		}
	else if( mapfilename[0] != '\0' ) strcpy( PLS.mapfile, mapfilename ); /* PPP */

	/* initialize and give specified output file name .. */
	if( outfilename[0] != '\0' ) Esetoutfilename( outfilename );
	stat = Einit( PLS.device );
	if( stat ) { PLS.skipout = 1; return( stat ); }

	/* set paper orientation */
	if( landscapemode ) Epaper( 1 );

	}


else if( PLS.npages > 0 ) {

	if( GL_member( PLS.device, "gesf" )) {

		/* finish up current page before moving on to next one.. */
		Eshow();
		stat = Eendoffile();
		if( stat ) return( stat );

		/* now set file name for next page.. */
		if( outfilename[0] != '\0' ) Esetoutfilename( outfilename );
		else	{
			makeoutfilename( PLS.outfile, buf, PLS.device, (PLS.npages)+1 );
			if( PLS.debug ) fprintf( PLS.diagfp, "Setting output file name to %s\n", PLS.outfile );
			Esetoutfilename( buf );
			}

		if( PLS.clickmap ) {
			/* initialize a new click map file.. */
			if( mapfilename[0] != '\0' ) strcpy( PLS.mapfile, mapfilename );
			else makeoutfilename( PLS.outfile, PLS.mapfile, 'm', (PLS.npages)+1 );
			PL_clickmap_init();

			}


		/* perhaps set global scaling and/or page size for next page.. */
		/* following 3 lines copied here from above - scg 10/31/00 */
		if( scalex != 1.0 || scaley != 1.0 ) Esetglobalscale( scalex, scaley );
		Egetglobalscale( &sx, &sy ); 
		if( pagesizegiven ) Esetsize( PLS.winw * sx, PLS.winh * sy, PLS.winx, PLS.winy );
		else if( landscapemode && !PLS.winsizegiven ) Esetsize( 11.0, 8.5, PLS.winx, PLS.winy ); /* landscape */

		/* initialize next page.. */
		stat = Einit( PLS.device );
		if( stat ) return( stat );
		}

	else if ( PLS.device == 'x' ) PL_do_x_button( "More.." );

	else if ( GL_member( PLS.device, "pc" ) ) {
		Eprint();
		if( landscapemode ) Epaper( 1 ); /* added scg 2/29/00 */
		Elinetype( 0, 0.6, 1.0 );   /* added scg 9/20/99 */
		}
	}
(PLS.npages)++;


/* -------------------------- */
/* now do other work.. */
/* -------------------------- */


/* do background.. */
/* if( dopagebox ) Ecblock( 0.0, 0.0, EWinx, EWiny, Ecurbkcolor, 0 ); */ /* does update bb */
if( dopagebox ) Ecblock( 0.0, 0.0, PLS.winw, PLS.winh, Ecurbkcolor, 0 ); /* does update bb */
else if( dobackground ) {
	/* EPS color=transparent - best to do nothing.. */
        if( PLS.device == 'e' && strcmp( Ecurbkcolor, "transparent" )==0 ) ;

        else Eclr(); /* doesn't update bb */
	}

if( pagetitle[0] != '\0' ) {
	textdet( "titledetails", titledet, &align, &adjx, &adjy, 3, "B", 1.0 );
	if( align == '?' ) align = 'C';
	measuretext( pagetitle, &nlines, &maxlen );
	if( align == 'L' ) Emov( 1.0 + adjx, (PLS.winh-0.8) + adjy );
	else if ( align == 'C' ) Emov( (PLS.winw / 2.0 ) + adjx, (PLS.winh-0.8) + adjy );
	else if( align == 'R' ) Emov( (PLS.winw-1.0) + adjx, (PLS.winh-0.8) + adjy );
	Edotext( pagetitle, align );
	}

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
