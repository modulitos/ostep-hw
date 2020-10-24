/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* 
 ploticus interface to Thomas Boutell's GD library (www.boutell.com)

 Notes: 

 * For development, "make devgrgd" can be used

 * This module supports these #defines: 
	GD13  (create GIF only using GD 1.3; no import)
	GD16  (create PNG only using GD 1.6; also can import PNG)
	GD18   (use GD 1.8+ to create PNG, JPEG, or WBMP; also can import these formats)
	GDFREETYPE (use FreeType font rendering; may be used only when GD18 is in effect)

 * GD renders text such that the TOP of the character box is at x, y

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef WIN32
#include <fcntl.h>  /* for _O_BINARY */
#endif

#include "pixpt.h"  /* circle ptlists */

extern double PLG_xsca_inv(), PLG_ysca_inv();
extern int TDH_err(), GL_member(), GL_goodnum();
extern int PLGG_color(), PLG_xsca(), PLG_ysca(), PLGG_linestyle(), PLGG_fill(), PLG_xrgb_to_rgb(), PLG_colorname_to_rgb();
extern int PLG_bb(), PL_clickmap_getdemomode(), PL_clickmap_show(), PL_clickmap_inprogress(), PL_clickmap_out();
extern int atoi(), chmod();

#define Exsca( h )      PLG_xsca( h )
#define Eysca( h )      PLG_ysca( h )
#define Exsca_inv( h )  PLG_xsca_inv( h )
#define Eysca_inv( h )  PLG_ysca_inv( h )
#define Eerr(a,b,c)  TDH_err(a,b,c)

#define VERT 1.570796  /* 90 degrees, expressed in radians */

static char g_fmt[20] = "";

/* ================================= */
/* SETIMFMT - set the image format that we will be creating.. 
   allowable values for fmt are "gif", "png", "jpeg", etc.  */
int
PLGG_setimfmt( fmt )
char *fmt;
{
strcpy( g_fmt, fmt );
return( 0 );
}
/* ================================= */
/* GETIMFMT - allow other modules to find out the image format.
   Format name is copied into fmt. */
int
PLGG_getimfmt( fmt )
char *fmt;
{
strcpy( fmt, g_fmt );
return( 0 );
}


/* =============================================================================== */
#ifndef NOGD

#include "gd.h"
#include "gdfontg.h"
#include "gdfontl.h"
#include "gdfontmb.h"
#include "gdfonts.h"
#include "gdfontt.h"

/* #define MAX_D_ROWS 1000 */
#define NBRUSH 8
#define CHARHW 1.75 /* 2.0 */
#define MINTEXTSIZE 4 /* anything less than this is rendered as a line for thumbnails */
#define stricmp(a,b) strcasecmp(a,b)

static gdImagePtr Gm; /* image */
static gdImagePtr Gm2 = NULL; /* secondary image */
static int Gm2height = 0, Gm2width = 0;
static gdFontPtr Gfont;  /* current font */
static int Gxmax, Gymax; /* drawing area image size */
static double Goldx = 0.0, Goldy = 0.0; /* last passed x,y */
static double Gcharwidth;
static int Gvertchar = 0;
static int Gtextsize;
static char Gcurcolorname[40];
static int Gcurcolor;

/* static gdPoint Gptlist[MAX_D_ROWS]; */
static gdPoint *Gptlist = NULL;
static int Gmax_pts;
static int Gnpts = 0;

static int Gcurlinestyle = 0;
static double Gcurlinewidth = 1.0;
static double Gcurdashscale = 1.0;
static gdImagePtr Gbrush[NBRUSH]; /* brush images */
static int Gdash[10][6]= { {1}, {1,1}, {3,1}, {5,1}, {2,1,1,1}, {4,1,1,1}, {6,1,1,1},
                          {2,1,1,1,1,1}, {4,1,1,1,1,1}, {6,1,1,1,1,1} };  /* constants */
static int Gndash[10] = { 1, 2, 2, 2, 4, 4, 4, 6, 6, 6 }; /* constants */
static int Gdashpat[1000];
static int Ginitialized = 0;
static int Gtransparent_color = -1; 
static int Gblack = 0;		    
static char GFTfont[80] = "";
#ifdef GDFREETYPE
  static int GFTbox[8];
  static double GFTsize;
#endif

static int Gpixelsinch;
static int Greqwidth = 0, Greqheight = 0;



/* ================================= */
int
PLGG_initstatic() 
{
strcpy( g_fmt, "" );
Gm2 = NULL;
Gm2height = 0; Gm2width = 0;
Goldx = 0.0, Goldy = 0.0;
Gvertchar = 0;
Gnpts = 0;
Gcurlinestyle = 0;
Gcurlinewidth = 1.0;
Gcurdashscale = 1.0;
Ginitialized = 0;
Gtransparent_color = -1;
Gblack = 0;
strcpy( GFTfont, "" );
strcpy( Gcurcolorname, "" );
Greqwidth = 0, Greqheight = 0;
if( Gptlist != NULL ) { free( Gptlist ); Gptlist = NULL; }  /* added scg 2/17/09 , joel reported Gptlist as a memory leak */

return( 0 );
}


/* ================================= */
/* GETIMG - allow API access to the image and its size */
gdImagePtr
PLGG_getimg( width, height )
int *width, *height; /* pixels */
{
if( !Ginitialized ) return( NULL );
*width = Gxmax;
*height = Gymax;
return( Gm );
}

/* =================================== */
/* SETIMPIXSIZE - set an exact pixel height and width for result (cropped) image */
int
PLGG_setimpixsize( width, height )
int width, height;
{
Greqwidth = width;
Greqheight = height;
return( 0 );
}


/* ================================ */
int
PLGG_setup( name, pixelsinch, ux, uy, upleftx, uplefty, maxdrivervect )
char *name;
int pixelsinch;
double ux, uy; /* size of image in inches x y */
int upleftx, uplefty; /* position of window - not used by this driver */
int maxdrivervect;
{
int i;

if( Ginitialized ) { /* could be if a late setsize was issued.. */
	gdImageDestroy( Gm );
	for( i = 0; i < NBRUSH; i++ ) gdImageDestroy( Gbrush[i] );
	}
Ginitialized = 1;

Gpixelsinch = pixelsinch;
Gxmax = (int)(ux * pixelsinch );
Gymax = (int)(uy * pixelsinch );

/* Allocate pixels.. */
Gm = gdImageCreate( Gxmax, Gymax );
if( Gm == NULL ) return( Eerr( 12003, "Cannot create working image", "" ) );
for( i = 0; i < NBRUSH; i++ ) {
	Gbrush[i] = gdImageCreate( i+1, i+1 ); 
	if( Gbrush[i] == NULL ) return( Eerr( 12004, "Cannot create brush image", "" ) );
	}


PLGG_color( "white" );
PLGG_color( "black" );
gdImageSetBrush( Gm, Gbrush[0] );


Gmax_pts = maxdrivervect;
if( Gptlist != NULL ) free( Gptlist );
Gptlist = (gdPoint *) malloc( Gmax_pts * sizeof( gdPoint ) );

return( 0 );
}

/* ================================ */
int
PLGG_moveto( x, y )
double x, y;
{
Goldx = x;
Goldy = y;
return( 0 );
}
/* ================================ */
int
PLGG_lineto( x, y )
double x, y;
{
int a, b, c, d;
a = Exsca( Goldx ); b = Eysca( Goldy ); c = Exsca( x ); d = Eysca( y );
/* gdImageLine( Gm, a, b, c, d, gdStyled ); */
gdImageLine( Gm, a, b, c, d, gdStyledBrushed ); 
Goldx = x; 
Goldy = y;
return( 0 );
}

/* ================================ */
int
PLGG_rawline( a, b, c, d )
int a, b, c, d;
{
gdImageLine( Gm, a, b, c, d, gdStyledBrushed ); 
return( 0 );
}

/* ================================ */
int
PLGG_linetype( s, x, y )
char *s;
double x, y;
{
int style;
style = atoi( s );
return( PLGG_linestyle( style, x, y ) );
}

/* ================================ */
int
PLGG_linestyle( style, linewidth, dashscale )
int style;
double linewidth, dashscale;
{
int i, j, k, np, state, ds;

style  = style % 9;

ds = (int)(dashscale*2.0);
if( ds < 1 ) { style = 0; ds = 1; }
np = 0;

/* build array p to indicate dash pattern, and set the dash style.. */
state = 1;
for( i = 0; i < Gndash[style]; i++ ) {
	for( j = 0; j < Gdash[style][i]; j++ ) {
		for( k = 0; k < ds; k++ ) {
			if( np >= 1000 ) {
				Eerr( 12005, "img dashscale out of range", "" );
				return( 0 );
				}
			Gdashpat[np++] = state;
			}
		}
	if( state == 1 ) state = 0;
	else state = 1;
	}
gdImageSetStyle( Gm, Gdashpat, np );
Gcurlinestyle = style;
Gcurdashscale = dashscale;

/* set the line width by setting pixels in the brush image.. */
i = (int) linewidth;
if( i > (NBRUSH-1) ) i = NBRUSH - 1;
gdImageSetBrush( Gm, Gbrush[i] );
Gcurlinewidth = linewidth;


return( 0 );
}
/* ================================ */
int
PLGG_pathto( px, py )
double px, py;
{

if( (Gnpts+2) > Gmax_pts ) PLGG_fill(); /* fill what we have so far, then start again.  scg 5/4/04 */
	
if( Gnpts == 0 ) {
	Gptlist[ Gnpts ].x = Exsca( Goldx );
	Gptlist[ Gnpts ].y = Eysca( Goldy );
	Gnpts++;
	}
Gptlist[ Gnpts ].x = Exsca( px );
Gptlist[ Gnpts ].y = Eysca( py );
Gnpts++;
return( 0 );
}
/* ================================ */
int
PLGG_fill()
{
if( Gnpts < 3 ) { 
	Eerr( 12007, "warning, not enough points", "" );
	return( 0 ); 
	}
Gptlist[ Gnpts ].x = Gptlist[0].x;
Gptlist[ Gnpts ].y = Gptlist[0].y;
Gnpts++;
gdImageFilledPolygon( Gm, Gptlist, Gnpts, Gcurcolor );
Gnpts = 0;
return( 0 );
}
/* ================================ */
/* note: caller must restore previous color after this routine returns. */
int
PLGG_rect( x1, y1, x2, y2, color )
double x1, y1, x2, y2;
char *color;
{
int a, b, c, d;
a = Exsca( x1 );
b = Eysca( y1 );
c = Exsca( x2 );
d = Eysca( y2 );
PLGG_color( color );
gdImageFilledRectangle( Gm, a, b, c, d, Gcurcolor );
return( 0 );
}

/* ================================ */
/* set a freetype font */
int
PLGG_font( s )
char *s;
{
#ifdef GDFREETYPE
char *fontpath;
  if( s[0] == '/' ) return( 0 ); /* ignore postscript fonts */
  if( strcmp( s, "ascii" )==0 ) strcpy( GFTfont, "" );
  else 	{
	fontpath = getenv( "GDFONTPATH" );
	if( fontpath == NULL ) Eerr( 12358, "warning: environment var GDFONTPATH not found. See ploticus fonts docs.", "" );
	if( strcmp( &s[ strlen(s) - 4 ], ".ttf" )==0 ) s[ strlen( s)-4 ] = '\0'; /* strip off .ttf ending - scg 1/26/05 */
	strcpy( GFTfont, s );
	}
#endif
return( 0 );
}

/* ================================ */
int
PLGG_textsize( p )
int p;
{

#ifdef GDFREETYPE
if( GFTfont[0] ) {
	GFTsize = (double)p;
	Gtextsize = p;
	Gcharwidth = 0.0; /* no top/bottom adjustment needed with FT */
	return( 0 );
	}
#endif

/* this logic is replicated in Etextsize() */
if( p <= 6 ) { Gfont = gdFontTiny; Gcharwidth = 0.05; }
else if( p >= 7 && p <= 9 ) { Gfont = gdFontSmall; Gcharwidth = 0.06; } /* was 0.0615384 */
else if( p >= 10 && p <= 12 ) { Gfont = gdFontMediumBold; Gcharwidth = 0.070; } /* was 0.0727272 */
else if( p >= 13 && p <= 15 ) { Gfont = gdFontLarge; Gcharwidth = 0.08; }
else if( p >= 15 ) { Gfont = gdFontGiant; Gcharwidth = 0.09; } /* was 0.0930232 */
Gtextsize = p;

return( 0 );
}
/* ================================ */
int
PLGG_chardir( d )
int d;
{
if( d == 90 ) Gvertchar = 1;
else Gvertchar = 0;
return( 0 );
}
/* ================================ */
int
PLGG_text( s )
char *s;
{
int a, b, c, d;
double x, y;
#ifdef GDFREETYPE
  char *err;
#endif

a = Exsca( Goldx ); b = Eysca( Goldy );
if( Gvertchar ) {
	if( Gtextsize < MINTEXTSIZE ) {
		x = Goldx - (Gtextsize/90.0);
		a = Exsca( x ); b = Eysca( Goldy ); 
		c = Exsca( x ); d = Eysca( Goldy + (((double)Gtextsize/100.0)*strlen(s)));
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
		x = Goldx - (Gcharwidth*CHARHW);  /* adjust for top loc */
		a = Exsca( x ); b = Eysca( Goldy );
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, VERT, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) gdImageStringUp( Gm, Gfont, a, b, (unsigned char *)s, Gcurcolor );
		}
	}
else 	{
	if( Gtextsize < MINTEXTSIZE ) {
		a = Exsca( Goldx ); b = Eysca( Goldy ); 
		c = Exsca( Goldx + (((double)Gtextsize/100.0) * strlen(s)) ); d = Eysca( Goldy );
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
		y = Goldy +  (Gcharwidth*CHARHW);  /* adjust for top loc */
		a = Exsca( Goldx ); b = Eysca( y );
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) gdImageString( Gm, Gfont, a, b, (unsigned char *)s, Gcurcolor );
		}
	}
Goldx = x;
Goldy = y;

return( 0 );
}
/* ================================ */
int
PLGG_centext( s )
char *s;
{
double halflen, x, y;
int a, b, c, d;
#ifdef GDFREETYPE
  char *err;
#endif

halflen = (Gcharwidth * (double)(strlen( s ))) / 2.0;
if( Gvertchar ) {
	if( Gtextsize < MINTEXTSIZE ) {
		halflen = (double)(strlen(s))/2.0 * ((double)Gtextsize/100.0);
		x = Goldx - (Gtextsize/90.0);  /* adjust for top loc */
		a = Exsca( x ); b = Eysca( Goldy - halflen );
		c = Exsca( x ); d = Eysca( Goldy + halflen );
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy );
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			b += (GFTbox[4] - GFTbox[0])/2;
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, VERT, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {
			x = Goldx - (Gcharwidth*CHARHW);  /* adjust for top loc */
			y = Goldy - halflen;
			a = Exsca( x ); b = Eysca( y ); 
			gdImageStringUp( Gm, Gfont, a, b, (unsigned char *)s, Gcurcolor );
			}
		}
	}
else	{
	if( Gtextsize < MINTEXTSIZE ) {
		halflen = (double)(strlen(s))/2.0 * ((double)Gtextsize/100.0);
		a = Exsca( Goldx - halflen ); b = Eysca( Goldy ); 
		c = Exsca( Goldx + halflen ); d = Eysca( Goldy ); 
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy ); 
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			a -= (GFTbox[4] - GFTbox[0])/2;
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {    /* ascii font */
			x = (Goldx - halflen) + (Gcharwidth/4.0); /* not sure why the extra is needed..*/
			y = Goldy + (Gcharwidth*CHARHW);  /* adjust for top loc */
			a = Exsca( x ); b = Eysca( y ); 
			gdImageString( Gm, Gfont, a, b, (unsigned char *)s, Gcurcolor );
			}
		}
	}
Goldx = x;
Goldy = y;
return( 0 );
}
/* ================================ */
int
PLGG_rightjust( s )
char *s;
{
double len, x, y;
int a, b, c, d;
#ifdef GDFREETYPE
  char *err;
#endif

len = Gcharwidth * strlen( s );
if( Gvertchar ) {
	if( Gtextsize < MINTEXTSIZE ) {
		x = Goldx - (Gtextsize/90.0);
		a = Exsca(x); b = Eysca( Goldy - (((double)Gtextsize/100.0)*strlen(s)));
		c = Exsca(x); d = Eysca( Goldy );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy );
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			b += (GFTbox[4] - GFTbox[0]);
			/* err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s ); */
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, VERT, a, b, s );  /* fixed 9/17/02 - Artur Zaprzala */
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {
			x = Goldx - (Gcharwidth*CHARHW);  /* adjust for top loc */
			y = Goldy - len;
			a = Exsca( x ); b = Eysca( y ); 
			gdImageStringUp( Gm, Gfont, a, b, (unsigned char *)s, Gcurcolor );
			}
		}
	}
else	{
	if( Gtextsize < MINTEXTSIZE ) {
		a = Exsca( Goldx - ((strlen(s))*(Gtextsize/100.0))); b = Eysca( Goldy ); 
		c = Exsca( Goldx ); d = Eysca( Goldy ); 
		gdImageLine( Gm, a, b, c, d, gdStyledBrushed );
		}
	else	{
#ifdef GDFREETYPE
		if( GFTfont[0] ) {
			a = Exsca( Goldx ); b = Eysca( Goldy );
			err = gdImageStringFT( NULL, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, GFTfont ); return( 1 ); }
			a -= (GFTbox[4] - GFTbox[0]);
			err = gdImageStringFT( Gm, GFTbox, Gcurcolor, GFTfont, GFTsize, 0.0, a, b, s );
			if( err ) { fprintf( stderr, "%s (%s)\n", err, GFTfont ); return( 1 ); }
			}
#endif
		if( GFTfont[0] == '\0' ) {
			x = Goldx - len;
			y = Goldy + (Gcharwidth*CHARHW);  /* adjust for top loc */
			a = Exsca( x ); b = Eysca( y ); 
			gdImageString( Gm, Gfont, a, b, (unsigned char *)s, Gcurcolor );
			}
		}
	}
Goldx = x;
Goldy = y;
return( 0 );
}

/* ================================ */
/* find the width of the given txt using given freetype font and size */
/* added 8/5/05 - sugg by Erik Zachte */
int
PLGG_freetype_twidth( txt, font, size, twidth )
char *txt, *font;
double size;
double *twidth;
{

*twidth = 0.0;

#ifdef GDFREETYPE 
if( font[0] ) {
	char *err;
	err = gdImageStringFT( NULL, GFTbox, 0, font, size, 0.0, 0, 0, txt );
	if( err ) { fprintf( stderr, "%s (%s) (width calc)\n", err, font ); return( 0 ); }
	*twidth = (GFTbox[2] - GFTbox[0]) / 100.0;
	}
#endif
return(0);
}


/* ================================ */
int
PLGG_color( color )
char *color;
{
int i, n;
double r, g, b;
int ir, ig, ib, len;
int bc; /* brush color */
double atof();


/* request to load the color we currently have.. ignore - scg 6/18/04 */
/* this is necessary even with pcode lazy color change, because of rectangles */
if( strcmp( color, Gcurcolorname ) ==0 ) return( 0 );
else strcpy( Gcurcolorname, color );


/* parse graphcore color spec.. */
for( i = 0, len = strlen( color ); i < len; i++ ) {
        if( GL_member( color[i], "(),/:|-" ) ) color[i] = ' ';
        }
 
if( strncmp( color, "rgb", 3 )==0 ) {
        n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
        if( n != 3 ) { Eerr( 12008, "Invalid color", color ); return(1); }
        }
else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
        n = sscanf( color, "%*s %lf", &r );
        if( n != 1 ) { Eerr( 12008, "Invalid color", color ); return(1); }
        g = b = r;
        }
else if( strcmp( color, "transparent" )==0 ) {  /* added scg 12/29/99 */
	if( Gtransparent_color < 0 ) {
		/* allocate transparent color.. */
		Gtransparent_color = gdImageColorAllocate( Gm, 254, 254, 254 ); /* white fallbk */
		/* gdImageColorTransparent( Gm, Gtransparent_color ); */
		/* also keep brushes in sync.. */
		for( i = 0; i < NBRUSH; i++ ) gdImageColorAllocate( Gbrush[i], 254, 254, 254 );
		}
	if( Gtransparent_color >= 0 ) Gcurcolor = Gtransparent_color;
	/* Don't set brushes, etc, because we will never need to draw 
	   transparent lines.  Brushes remain set to previous color. */
	return( 0 );
	}
else if( strncmp( color, "xrgb", 4 )==0 ) {
        if (PLG_xrgb_to_rgb( &color[5], &r, &g, &b)) return(1);
        }
else if( color[0] == 'x' ) {  /* added scg 5/31/07 */
        if (PLG_xrgb_to_rgb( &color[1], &r, &g, &b)) return(1);
        }
else if( GL_goodnum( color, &i ) ) {
        r = atof( color );
        g = b = r;
        }
else PLG_colorname_to_rgb( color, &r, &g, &b );

ir = (int)(r * 255);
ig = (int)(g * 255);
ib = (int)(b * 255);


Gcurcolor = gdImageColorExact( Gm, ir, ig, ib );
if( Gcurcolor < 0 ) {
	Gcurcolor = gdImageColorAllocate( Gm, ir, ig, ib );
	if( Gcurcolor < 0 ) {
		Gcurcolor = gdImageColorClosest( Gm, ir, ig, ib );
		if( Gcurcolor < 0 ) return( 1 ); /* Eerr( 12009, "Error on img color allocation", color ); exit(1); */
		}
	}

if( ir + ig + ib == 0 ) Gblack = Gcurcolor; /* for wbmp */


/* also set all brushes to new color.. */
for( i = 0; i < NBRUSH; i++ ) {
	/* first find color index or allocate new one.. */
	bc = gdImageColorExact( Gbrush[i], ir, ig, ib );
	if( bc < 0 ) {
		bc = gdImageColorAllocate( Gbrush[i], ir, ig, ib );
		if( bc < 0 ) {
			bc = gdImageColorClosest( Gbrush[i], ir, ig, ib );
			if( bc < 0 ) {
				Eerr( 12010, "Error on img brush color alloc", color );
				bc = 0;
				}
			}
		}
	/* next set color of each brush.. */
	gdImageFilledRectangle( Gbrush[i], 0, 0, i+1, i+1, bc ); /* bc should == Gcurcolor */
	}

/* This message occurs all the time with JPEG.. perhaps not needed..
 * if( bc != Gcurcolor ) {
 *	Eerr( 12011, "warning, img brush color does not match current color", "" );
 * 	}
 */

/* now, need to update brush image to new color using current linewidth */
i = (int) Gcurlinewidth;
if( i > (NBRUSH-1) ) i = NBRUSH - 1;
gdImageSetBrush( Gm, Gbrush[i] );

return( 0 );
}

/* ================================ */
/* PIXPT - pixel data point - clean data points rendered by setting GD pixels directly */
/* added 5/29/06 scg */
/* note: color already set by symboldet() */
int
PLGG_pixpt( x, y, symcode )
double x, y;
char *symcode;
{
int a, b;
int i, j, irow, icol, radius, iw, omode, scanend;
double atof();

/* Note - this code is essentially replicated in x11.c */

if( symcode[0] == 'o' ) { omode = 1; symcode[0] = 'p'; }
else omode = 0;

a = Exsca( x ); b = Eysca( y ); /* convert to pixel coordinates */

if( strncmp( symcode, "pixsquare", 9 )==0 ) {
	radius = (int) (atof( &symcode[9] ) * Gpixelsinch);
	if( radius < 1 ) radius = 3;
	if( omode ) { /* do top and bottom lines */
		irow = b-radius; for( icol = a-radius; icol < a+radius; icol++ ) gdImageSetPixel( Gm, icol, irow, gdStyledBrushed );
		irow = b+radius; for( icol = a-radius; icol <= a+radius; icol++ ) gdImageSetPixel( Gm, icol, irow, gdStyledBrushed );
		}
	for( irow = b-radius; irow < b+radius; irow++ ) {
		if( omode ) { gdImageSetPixel( Gm, a-radius, irow, gdStyledBrushed ); gdImageSetPixel( Gm, a+radius, irow, gdStyledBrushed ); }
		else { for( icol = a-radius; icol < a+radius; icol++ ) gdImageSetPixel( Gm, icol, irow, gdStyledBrushed ); }
		}
	}
else if( strncmp( symcode, "pixcircle", 9 )==0 ) {
	radius = (int) (atof( &symcode[9] ) * Gpixelsinch);
	if( radius <= 2 ) goto DO_DIAMOND;
	else if( radius > 9 ) radius = 9;
	for( i = circliststart[radius]; ; i+= 2 ) {
		scanend = circpt[i+1];
		if( circpt[i] == 0 && scanend == 0 ) break;
		for( j = 0; j <= scanend; j++ ) {
			if( omode && !( j == scanend )) continue;
			gdImageSetPixel( Gm, a-j, b+circpt[i], gdStyledBrushed ); 
			if( j > 0 ) gdImageSetPixel( Gm, a+j, b+circpt[i], gdStyledBrushed );
			gdImageSetPixel( Gm, a-j, b-circpt[i], gdStyledBrushed ); 
			if( j > 0 ) gdImageSetPixel( Gm, a+j, b-circpt[i], gdStyledBrushed );
			if( omode ) {
				gdImageSetPixel( Gm, a+circpt[i], b-j, gdStyledBrushed ); 
				if( j > 0 ) gdImageSetPixel( Gm, a+circpt[i], b+j, gdStyledBrushed );
				gdImageSetPixel( Gm, a-circpt[i], b-j, gdStyledBrushed ); 
				if( j > 0 ) gdImageSetPixel( Gm, a-circpt[i], b+j, gdStyledBrushed );
				}
			}
		}
	}
else if( strncmp( symcode, "pixdiamond", 10 )==0 ) {
	radius = (int) (atof( &symcode[10] ) * Gpixelsinch);
	DO_DIAMOND:
	if( radius < 1 ) radius = 3;
	radius++;  /* improves consistency w/ other shapes */
	for( irow = b-radius, iw = 0; irow <= (b+radius); irow++, iw++ ) {
	    scanend = a+abs(iw);
	    for( icol = a-abs(iw), j = 0; icol <= scanend; icol++, j++ ) {
		if( omode && !( j == 0 || icol == scanend )) ;
		else gdImageSetPixel( Gm, icol, irow, gdStyledBrushed );
		}
	    if( irow == b ) iw = (-radius);
	    }
	}
else if( strncmp( symcode, "pixtriangle", 11 )==0 ) {
	radius = (int) (atof( &symcode[11] ) * Gpixelsinch);
	if( radius < 1 ) radius = 3;
	for( irow = b-radius, iw = 0; irow <= b+radius; irow++, iw++ ) {
	    scanend = a+abs(iw/2);
	    for( icol = a-abs(iw/2), j = 0; icol <= scanend; icol++, j++ ) {
		if( omode && irow == b+radius ) gdImageSetPixel( Gm, icol, irow-1, gdStyledBrushed );
		else if( omode && !( j == 0 || icol == scanend ));
		else gdImageSetPixel( Gm, icol, irow-1, gdStyledBrushed );
		}
	    }
	}
else if( strncmp( symcode, "pixdowntriangle", 15 )==0 ) {
	radius = (int) (atof( &symcode[15] ) * Gpixelsinch);
	if( radius < 1 ) radius = 3;
	for( irow = b+radius, iw = 0; irow >= (b-radius); irow--, iw++ ) {
	    scanend = a+abs(iw/2);
	    for( icol = a-abs(iw/2), j = 0; icol <= scanend; icol++, j++ ) {
		if( omode && irow == b-radius ) gdImageSetPixel( Gm, icol, irow-1, gdStyledBrushed );
		else if( omode && !(j == 0 || icol == scanend ));
		else gdImageSetPixel( Gm, icol, irow, gdStyledBrushed );
		}
	    }
	}
return( 0 );
}

/* ============================== */
int
PLGG_imload( imgname, width, height )
char *imgname;
int width, height;
{
FILE *fp;

if( Gm2 != NULL ) {
	gdImageDestroy( Gm2 );
	Gm2 = NULL;
	}

fp = fopen( imgname, "rb" );
if( fp == NULL ) return( -1 );
	
#ifdef GD13
  Gm2 = gdImageCreateFromGif( fp );
#endif
#ifdef GD16
  Gm2 = gdImageCreateFromPng( fp );
#endif
#ifdef GD18
  if( strcmp( g_fmt, "png" )==0 ) Gm2 = gdImageCreateFromPng( fp );
  else if( strcmp( g_fmt, "jpeg" )==0 ) Gm2 = gdImageCreateFromJpeg( fp );
  else if( strcmp( g_fmt, "wbmp" )==0 ) Gm2 = gdImageCreateFromWBMP( fp );
#endif

if( width != 0 ) Gm2width = width;
if( height != 0 ) Gm2height = height;

fclose( fp );
return( 0 );
}

/* ================================ */
/* place secondary GIF image within main image at absolute x, y 
 *	align may be one of: topleft   topcenter  center  bottomleft 
 */
int
PLGG_implace( x, y, align, width, height )
double x, y;
char *align;
int width, height;  /* render the image using this width and height in pixels... if 0 0 then use natural size */
{
int gx, gy;
double PLG_xsca_inv(), PLG_ysca_inv();

fprintf( stderr, "in gd imcopy..\n" );
if( Gm2 == NULL ) return( -1 );

if( width < 1 ) width = Gm2width;  /* as it may have been set in imload */
if( width < 1 ) width = Gm2->sx;   /* fallback to image's natural size */

if( height < 1 ) height = Gm2height;  /* as it may have been set in imload */
if( height < 1 ) height = Gm2->sy;    /* fallback to image's natural size */

if( strncmp( align, "center", 6 )==0 ) { gx = Exsca( x ) - (width/2); gy = Eysca( y ) - (height/2); }
else if( strcmp( align, "topcenter" )==0 ) { gx = Exsca( x ) - (width/2); gy = Eysca( y ); }
else if( strcmp( align, "bottomleft" )==0 ) { gx = Exsca( x ); gy = Eysca( y ) - height; }
else { gx = Exsca( x ); gy = Eysca( y ); } /* default to top left */

if( gx < 0 ) gx = 0;
if( gy < 0 ) gy = 0;

gdImageCopyResized( Gm, Gm2, gx, gy, 0, 0, width, height, Gm2->sx, Gm2->sy );

/* add to app bounding box */
PLG_bb( Exsca_inv( gx ), Eysca_inv( gy ) );
PLG_bb( Exsca_inv( gx + width ), Eysca_inv( gy + height ) );

return( 0 );
}


/* ================================ */
/* EOF - crop image to bounding box size, and create output file */
/* scg 11/23/01 added click map support */

int
PLGG_eof( filename, x1, y1, x2, y2 )
char *filename;
double x1, y1, x2, y2; /* rectangle of the image that we will be copying into to do the final cropping */
{
int i, width, height, ux, uy;
gdImagePtr outim;
FILE *outfp;
int t;


if( x1 < 0.0 ) x1 = 0.0;
if( y1 < 0.0 ) y1 = 0.0;
/* if( x2 < 0.0 ) x2 = 0.0; */ 
/* if( y2 < 0.0 ) y2 = 0.0; */

/* final area may not be larger than originally defined 'pagesize' - added scg 5/8/06 */
if( x2 > (Gxmax/(double)Gpixelsinch) || x2 < 0.0 ) x2 = Gxmax/(double)Gpixelsinch ;
if( y2 > (Gymax/(double)Gpixelsinch) || y2 < 0.0 ) y2 = Gymax/(double)Gpixelsinch ;

#ifdef PLOTICUS
if( PL_clickmap_getdemomode() ) PL_clickmap_show( 'g' ); /* 11/23/01 */
#endif

if( Greqwidth > 0 && Greqheight > 0 ) {
	width = Greqwidth;
	height = Greqheight;
	}
else	{
	width = Exsca( x2 ) - Exsca( x1 );
	height = Eysca( y1 ) - Eysca( y2 );
	}
if( height < 10 || width < 10 ) return( Eerr( 12012, "Result image is too small - not created", "" ) );

ux = Exsca( x1 );
uy = Eysca( y2 );


/* copy to smaller img sized by bounding box.. */
outim = gdImageCreate( width, height );
if( outim == NULL ) return( Eerr( 12013, "Error on creation of image output", "" ) );
gdImageCopy( outim, Gm, 0, 0, ux, uy, width, height );
/* fprintf( stderr, "new im w:%d h:%d   ux:%d uy:%d\n", width, height, ux, uy ); */

/* if a transparent color was used, set it now in outim.. */
if( Gtransparent_color >= 0 ) {
	t = gdImageColorExact( outim, 254, 254, 254 );
	gdImageColorTransparent( outim, t );
	}

/* Open a file for writing. "wb" means "write binary", important under MSDOS, harmless under Unix. */
if( strcmp( filename, "stdout" )==0 ) {
	fflush( stdout );
#ifdef WIN32
	_setmode( _fileno( stdout ), _O_BINARY ); /* use binary mode stdout */
#endif
	outfp = stdout;
	}
else outfp = fopen( filename, "wb");
if( outfp == NULL ) return( Eerr( 12014, "Cannot open for write", filename ) );

/* Output the image to the disk file. */
#ifdef GD13
gdImageGif( outim, outfp );    
#endif
#ifdef GD16
gdImagePng( outim, outfp );    
#endif
#ifdef GD18
if( strcmp( g_fmt, "png" )==0 ) gdImagePng( outim, outfp );
else if( strcmp( g_fmt, "jpeg" )==0 ) gdImageJpeg( outim, outfp, 75 );
else if( strcmp( g_fmt, "wbmp" )==0 ) gdImageWBMP( outim, Gblack, outfp );
#endif

if( strcmp( filename, "stdout" )!=0 ) {
	fclose( outfp );
#ifndef WIN32
	chmod( filename, 00644 );
#endif
	}
else	{
	fflush( stdout );
#ifdef WIN32
	_setmode( _fileno( stdout ), _O_TEXT ); /* if using stdout, restore stdout to text mode */
#endif
	}

/* free memory (other ims freed in EGSetup() for subsequent pages) */
gdImageDestroy( Gm );
for( i = 0; i < NBRUSH; i++ ) gdImageDestroy( Gbrush[i] );
gdImageDestroy( outim );
Ginitialized = 0;

#ifdef PLOTICUS
/* write map file */
if( PL_clickmap_inprogress() ) PL_clickmap_out( ux, uy ); 
#endif

return( 0 );
}




#endif /* NOGD */

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
