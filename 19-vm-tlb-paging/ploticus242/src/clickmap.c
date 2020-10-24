/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* clickmap.c - generate a clickable imagemap.  All related bookkeeping is here. 

   Feb    2002 scg	client-side HTML clickmap supported

   Mar 27 2002 scg  	SVG clickable map support added (PLS.device == 's')  Not implemented here
			but rather via SVG_beginobj and SVG_endobj.

   Feb  4 2009 jqn      added encodeurl config parameter so that clickmap URL targets with embedded spaces
                        will use '_' instead of '+' (because certain operating contexts need this)
 */

#include "pl.h"
#include <string.h>
extern int chmod();

#define MAXENTRIES 500
#define SERVERSIDE 1
#define CLIENTSIDE 2

#define PLUS_TO_UNDERSCORE 1

static int imap;
static char *urls[MAXENTRIES];
static char *titles[MAXENTRIES];
static struct {
	char typ;
	int pmode;
	int x1; int y1; int x2; int y2;
	} box[MAXENTRIES];
static char defaulturl[MAXURL] = "";
static char tpurl[MAXURL] = ""; /* a url template */
static int mapstatus = 0; /* 1 if we are in the process of doing a map; 0 otherwise */
static int demomode = 0;  /* 1 if we are in demo mode */
static int intersect = 0;
static double adjx = 0.0, adjy = 0.0;

extern int PLGS_clickregion();
static int get_targetstr();


/* ========================= */
/* INIT - initialize clickmap facility & make various settings */
int
PL_clickmap_init()
{

PL_clickmap_free(); /* free any previously alloc'ed stores.. */

strcpy( defaulturl, "" );
strcpy( tpurl, "" );
/* debugmode = PLS.debug; */
imap = 0;
intersect = 0;
mapstatus = 1;
adjx = 0.0; adjy = 0.0;
return( 0 );
}

/* ========================= */
/* ENTRY - add one map entry */
/* typs: 'r' = rectangle, lower left x,y and upper right x,y;   'p' = point */

int
PL_clickmap_entry( typ, url, pmode, x1, y1, x2, y2, textpad, clipmode, title )
/* mapentry */
char typ;
char *url;
int pmode; /* processing mode; 0 = none, 1 = sub into tpurl as x, 2 = sub into tpurl as y, 
		3 = intersect w/another then sub into tpurl as x, 4 = intersect then sub as y  */
double x1, y1, x2, y2;
int textpad; /* add extra padding to text */
int clipmode;  /* 0 = no clip;  1 = clip X to plotting area  2 = clip Y to plotting area */
char *title; /* client-side only.. this will be supplied as the title= attribute - can be large (MAXTT) */
{
int i;
double sx, sy;


if( imap >= MAXENTRIES-1 ) return( Eerr( 2706, "too many clickmap regions, ignoring", url ) );
if( !mapstatus ) return( Eerr( 2707, "-map or -csmap must be specified on command line", "" ) );

if( url[0] == '\0' && title[0] == '\0' ) return( 0 ); /* degenerate case */
/* if( PLS.device == 's' && url[0] == '\0' ) return( 0 ); */ /* mouseover not yet supported for svg */

/* do character conversions within url.. */
for( i = 0; url[i] != '\0'; i++ ) {
	if( url[i] == ' ' || url[i] == '\n' ) url[i] = '_';
	if( url[i] == '\\' && url[i+1] == 'n' ) { url[i++] = '_'; url[i] = '_'; }
	}

urls[ imap ] = (char *) malloc( strlen( url ) + 1 );
strcpy( urls[ imap ], url );

titles[ imap ] = NULL;
if( title[0] != '\0' ) {
	int tlen;
	tlen = strlen( title );
	if( tlen > 0 ) {
		titles[ imap ] = (char *) malloc( strlen( title ) + 1 ); 
		strcpy( titles[ imap ], title );
		}
	}

if( pmode > 2 ) intersect = 1;
box[ imap ].pmode = pmode;
box[ imap ].typ = typ;
if( typ == 'p' ) { x2 = x1; y2 = y1; }
if( pmode ) { x1 += adjx; x2 += adjx; y1 += adjy; y2 += adjy; }  /* for grid regions, apply adjustment if any */

/* adjust for textpad, clip, and global scaling, if any.. */
if( textpad ) { x1 -= 0.15; x2 += 0.1; y1 -= 0.04; }
if( clipmode == 1 ) { if( x1 < EXlo ) x1 = EXlo; if( x2 >  EXhi ) x2 = EXhi; }
if( clipmode == 2 ) { if( y1 < EYlo ) y1 = EYlo; if( y2 >  EYhi ) y2 = EYhi; }
Egetglobalscale( &sx, &sy );
x1 *= sx; y1 *= sy; x2 *= sx; y2 *= sy;

if( PLS.device == 's' ) {
	box[ imap ].x1 = (int)(x1*100); box[ imap ].y1 = (int)(y2*100);
	box[ imap ].x2 = (int)(x2*100); box[ imap ].y2 = (int)(y1*100);
	}
else	{
	box[ imap ].x1 = Exsca( x1 ); box[ imap ].y1 = PLG_ysca( y2 );
	box[ imap ].x2 = Exsca( x2 ); box[ imap ].y2 = PLG_ysca( y1 );
	}

imap++;
return( 0 );
}

/* ========================= */
/* OUT - write out the clickmap info */
/* Note: PLS.clickmap == 1 for server-side,  2 for client-side */

/* In gif/png/jpg image maps the "top" elements (from an overlap standpoint) need to be near the top of the list.
   In SVG this is the opposite.  Hence for SVG things have to be done in reverse order (see the gotos). */


int
PL_clickmap_out( tx, ty )
int tx, ty; /* translate vector - compensates for image cropping, etc.  (pass as 0, 0 for svg) */
{
int i, j;
FILE *fp;
char buf[1024], targetstr[1024]; /* raised from 256 scg 11/5/07 */
int ox1, oy1, ox2, oy2;
int loopstart, loopend, loopinc;

if( imap < 1 ) return( Eerr( 2795, "Warning, no map regions were assigned", PLS.mapfile ) );

if( PLS.device != 's' ) {
	if( strcmp( PLS.mapfile, "stdout" )==0 ) fp = stdout;
	else if( strcmp( PLS.mapfile, "stderr" )==0 ) fp = stderr;
	else fp = fopen( PLS.mapfile, "w" );
	if( fp == NULL ) return( Eerr( 2705, "Cannot open mapfile", PLS.mapfile ));

	if( PLS.debug ) fprintf( PLS.diagfp, "writing clickmap file %s, coords translated by %d,%d\n", PLS.mapfile, tx, ty );

	if( PLS.clickmap == SERVERSIDE && defaulturl[0] != '\0' ) fprintf( fp, "default %s\n", defaulturl ); /*svg equivalent?*/
	if( demomode && PLS.clickmap == CLIENTSIDE ) fprintf( fp, "<map name=\"demo1\">\n" );
	}

/* control output ordering (varies according to device) */
if( PLS.device == 's' ) { loopstart = 0; loopend = imap; loopinc = 1; goto DO_THE_REST; }	/* svg: "top" is last */
else { loopstart = imap-1; loopend = -1; loopinc = -1; }		/* images: "top" is first */


/* do any plot elements (these sit on top of any grid).. split out from "do_the_rest" scg 10/30/06 */
DO_ELEMENTS:
for( i = loopstart; i != loopend; i += loopinc ) { 

	if( box[ i ].pmode > 0 ) continue;
	strcpy( buf, urls[i] );
	strcpy( targetstr, "" ); get_targetstr( buf, targetstr );

#ifndef NOSVG
	if( PLS.device == 's' ) {
		if( titles[i] == NULL ) PLGS_clickregion( buf, "", targetstr, box[ i ].x1, box[ i ].y1, box[ i ].x2, box[ i ].y2 );
		else PLGS_clickregion( buf, titles[i], targetstr, box[ i ].x1, box[ i ].y1, box[ i ].x2, box[ i ].y2 );
		}
	else 
#endif
	if( PLS.clickmap == SERVERSIDE ) fprintf( fp, "rect %s	%d,%d	%d,%d\n", 
				buf, box[ i ].x1 - tx, box[ i ].y1 - ty, box[ i ].x2 - tx, box[ i ].y2 - ty );

	else if( PLS.clickmap == CLIENTSIDE ) {
		fprintf( fp, "<area shape=\"rect\" %s%s%c coords=\"%d,%d,%d,%d\" ",
			(buf[0] == '\0') ? "nohref" : "href=\""   , buf,    (buf[0] == '\0' ) ? ' ' : '"',
			box[ i ].x1 - tx, box[ i ].y1 - ty, box[ i ].x2 - tx, box[ i ].y2 - ty );
		if( titles[i] != NULL ) fprintf( fp, "title=\"%s\" alt=\"%s\" %s />\n", titles[i], titles[i], targetstr );
		else fprintf( fp, "alt=\"\" %s />\n", targetstr );
		}
	}

if( PLS.device == 's' ) return( 0 ); 

/* do any xygrid regions (intersections) before any xonly or yonly grid regions.. */
DO_INTERSECTS:
if( intersect ) for( i = loopstart; i != loopend; i += loopinc ) { 

	if( box[i].pmode == 3 ) {
		/* find all '4' entries.. write out an entry for each one found.. */
		/* assume all '4' entries overlap all '3' entries.. */
		for( j = imap-1; j >= 0; j-- ) {
			if( box[j].pmode == 4 ) {

				PL_setcharvar( "XVAL", urls[i] );
				PL_setcharvar( "YVAL", urls[j] );
				PL_fref_showerr( 0 );
				PL_value_subst( buf, tpurl, NULL, URL_ENCODED );
				PL_fref_showerr( 1 );

				strcpy( targetstr, "" );
				get_targetstr( buf, targetstr );

				/* take greatest x1 */
				if( box[ i ].x1 > box[j].x1 ) ox1 = box[i].x1;
				else ox1 = box[j].x1;

				/* take smallest x2 */
				if( box[ i ].x2 < box[j].x2 ) ox2 = box[i].x2;
				else ox2 = box[j].x2;

				/* take greatest y1 (smallest for svg) */
				if( PLS.device == 's' ) {
					if( box[ i ].y1 < box[j].y1 ) oy1 = box[i].y1;
					else oy1 = box[j].y1;
					}
				else	{
					if( box[ i ].y1 > box[j].y1 ) oy1 = box[i].y1;
					else oy1 = box[j].y1;
					}

				/* take smallest y2 (biggest for svg) */
				if( PLS.device == 's' ) {
					if( box[ i ].y2 > box[j].y2 ) oy2 = box[i].y2;
					else oy2 = box[j].y2;
					}
				else	{
					if( box[ i ].y2 < box[j].y2 ) oy2 = box[i].y2;
					else oy2 = box[j].y2;
					}

#ifndef NOSVG
				if( PLS.device == 's' ) PLGS_clickregion( buf, "", targetstr, ox1, oy1, ox2, oy2 );
				else
#endif
				if( PLS.clickmap == SERVERSIDE ) 
					fprintf( fp, "rect %s	%d,%d	%d,%d\n", buf, ox1-tx, oy1-ty, ox2-tx, oy2-ty );
				else if( PLS.clickmap == CLIENTSIDE ) 
					fprintf( fp, "<area shape=\"rect\" href=\"%s\" coords=\"%d,%d,%d,%d\" %s >\n", 
						buf, ox1-tx, oy1-ty, ox2-tx, oy2-ty, targetstr );
				}
			continue;
			}
		}
	}

if( PLS.device == 's' ) goto DO_ELEMENTS;

DO_THE_REST:
/* now do the rest.. this section is basically the same as DO_ELEMENTS, above.. */
for( i = loopstart; i != loopend; i += loopinc ) { 
	if( box[i].pmode == 0 ) continue;
	strcpy( buf, "" );
	if( box[ i ].pmode > 0 ) {
		PL_fref_showerr( 0 );  
		if( box[ i ].pmode == 1 || box[i].pmode == 3 ) {
			PL_setcharvar( "XVAL", urls[i] );
			PL_value_subst( buf, tpurl, NULL, URL_ENCODED );
			}
		else if( box[ i ].pmode == 2 || box[i].pmode == 4 ) {
			PL_setcharvar( "YVAL", urls[i] );
			PL_value_subst( buf, tpurl, NULL, URL_ENCODED );
			}
		PL_fref_showerr( 1 );
		}
	strcpy( targetstr, "" ); get_targetstr( buf, targetstr );


#ifndef NOSVG
	if( PLS.device == 's' ) {
		if( titles[i] == NULL ) PLGS_clickregion( buf, "", targetstr, box[ i ].x1, box[ i ].y1, box[ i ].x2, box[ i ].y2 );
		else PLGS_clickregion( buf, titles[i], targetstr, box[ i ].x1, box[ i ].y1, box[ i ].x2, box[ i ].y2 );
		}
	else 
#endif

	if( PLS.clickmap == SERVERSIDE ) fprintf( fp, "rect %s	%d,%d	%d,%d\n", 
				buf, box[ i ].x1 - tx, box[ i ].y1 - ty, box[ i ].x2 - tx, box[ i ].y2 - ty );

	else if( PLS.clickmap == CLIENTSIDE ) {
		fprintf( fp, "<area shape=\"rect\" %s%s%c coords=\"%d,%d,%d,%d\" ",
			(buf[0] == '\0') ? "nohref" : "href=\""   , buf,    (buf[0] == '\0' ) ? ' ' : '"',
			box[ i ].x1 - tx, box[ i ].y1 - ty, box[ i ].x2 - tx, box[ i ].y2 - ty );
		if( titles[i] != NULL ) fprintf( fp, "title=\"%s\" alt=\"%s\" %s />\n", titles[i], titles[i], targetstr );
		else fprintf( fp, "alt=\"\" %s />\n", targetstr );
		}
	}

if( PLS.device == 's' ) goto DO_INTERSECTS;

if( PLS.clickmap == CLIENTSIDE && PLS.device != 's' ) { /* do default (last, so it's on the "bottom") */
	strcpy( buf, defaulturl );
	strcpy( targetstr, "" );
	get_targetstr( buf, targetstr );
	if( defaulturl[0] != '\0' ) fprintf( fp, "<area shape=\"default\" href=\"%s\" %s >\n", buf, targetstr );
	if( demomode ) fprintf( fp, "</map>\n<img src=\"%s\" usemap=\"#demo1\">\n", PLS.outfile );
	}

if( PLS.device != 's' && !GL_smember( PLS.mapfile, "stderr stdout" )) {

	fclose( fp );

#ifndef WIN32
	chmod( PLS.mapfile, 00644 );
#endif
	}

return( 0 );
}

/* =========================== */
/* SHOW - display map regions using a green outline - for svg this is done in svg.c */

int
PL_clickmap_show( dev )
char dev;
{
int i;
extern int PLGX_color(), PLGX_linetype(), PLGX_rawline(), PLGG_color(), PLGG_linetype(), PLGG_rawline();

if( PLS.device == 's' ) return( 0 );
if( imap < 1 ) return( Eerr( 2937, "Warning, no map regions were assigned", PLS.mapfile ) );
if( dev == 'x' ) {
#ifndef NOX11
	PLGX_color( "brightgreen" );
	PLGX_linetype( "0", 0.5, 1.0 );
	for( i = 0; i < imap; i++ ) {
		if( box[i].typ == 'p' ) { box[i].x2 += 1; box[i].y2 += 1; }
		PLGX_rawline( box[i].x1, box[i].y1, box[i].x2, box[i].y1 );
		PLGX_rawline( box[i].x2, box[i].y1, box[i].x2, box[i].y2 );
		PLGX_rawline( box[i].x2, box[i].y2, box[i].x1, box[i].y2 );
		PLGX_rawline( box[i].x1, box[i].y2, box[i].x1, box[i].y1 );
		}
#endif
	;
	}

else if( dev == 'g' ) {
#ifndef NOGD
	PLGG_color( "brightgreen" );
	PLGG_linetype( "1", 0.5, 1.0 );
	for( i = 0; i < imap; i++ ) {
		if( box[i].typ == 'p' ) { box[i].x2 += 1; box[i].y2 += 1; }
		PLGG_rawline( box[i].x1, box[i].y1, box[i].x2, box[i].y1 );
		PLGG_rawline( box[i].x2, box[i].y1, box[i].x2, box[i].y2 );
		PLGG_rawline( box[i].x2, box[i].y2, box[i].x1, box[i].y2 );
		PLGG_rawline( box[i].x1, box[i].y2, box[i].x1, box[i].y1 );
		}
#endif
	;
	}
return( 0 );
}

/* ========================== */
int
PL_clickmap_free()
{
int i;
if( imap < 1 ) return( 0 );
for( i = 0; i < imap; i++ ) {
	free( urls[i] );
	if( titles[i] != NULL ) free( titles[i] );
	}
imap = 0;
mapstatus = 0;
return( 0 );
}

/* ========================= */
/* INPROGRESS - return 1 if we are doing a clickmap, 0 otherwise */
int
PL_clickmap_inprogress()
{
return( mapstatus );
}

/* ========================= */
/* DEMOMODE - set demo mode */
int
PL_clickmap_demomode( mode )
int mode;
{
demomode = mode;
return( 0 );
}


/* ========================= */
/* GETDEMOMODE - return 1 if we are in demo mode, 0 otherwise */
int
PL_clickmap_getdemomode()
{
return( demomode );
}

/* ========================= */
/* SETDEFAULTURL - set the "default" url */
int
PL_clickmap_setdefaulturl( url )
char *url;
{
strcpy( defaulturl, url );
return( 0 );
}
/* ========================= */
/* ADJUST - for grid clickmaps, this allows a horizontal and vertical adjustment to be set. */
int
PL_clickmap_adjust( x, y )
double x, y;
{
adjx = x;
adjy = y;
return( 0 );
}

/* ========================== */
/* SETURLT - set the url template to be used for plot area grid mapping */
int
PL_clickmap_seturlt( url )
char *url;
{
strcpy( tpurl, url );
return( 0 );
}



/* =========================== */
static int
get_targetstr( buf, targetstr )
char *buf, *targetstr;
{
int j;
if( buf[0] == '[' ) {
	for( j = 0; buf[j] != '\0'; j++ ) {
		if( buf[j] == '[' ) buf[j] = ' ';
		if( buf[j] == ']' ) {
			buf[j] = ' ';
			sscanf( buf, "%s", targetstr );
			strcpy( buf, &buf[j+1] );
			break;
			}
		}
	} 
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
