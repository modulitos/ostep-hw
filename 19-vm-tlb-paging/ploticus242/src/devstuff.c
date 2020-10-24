/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"
extern int PLGS_z(), PLGG_setimfmt(), PLGG_getimfmt(), PLGS_fmt();

/* ========================== */
/* DEVAVAIL - returns 1 if specified device driver or resource is available
	in this build, or 0 if not. */
int
PL_devavail( dev )
char *dev;
{

if( dev[0] == 'x' ) {  /* x11 */
#ifdef NOX11
	return( 0 );
#endif
	return( 1 );
	}

else if( strcmp( dev, "gif" )==0 ) { 
#if GD16 || GD18 || NOGD
	return( 0 );
#endif
	return( 1 );
	}

else if( strcmp( dev, "png" )==0 ) {
#if GD13 || NOGD
	return( 0 );
#endif
	return( 1 );
	}

else if( strcmp( dev, "jpeg" )==0 ) {
#if GD18
	return( 1 );
#endif
	return( 0 );
	}

else if( strcmp( dev, "wbmp" )==0 ) {
#if GD18
	return( 1 );
#endif
	return( 0 );
	}

else if( GL_smember( dev, "ps eps svg" )) return( 1 );

else if( strcmp( dev, "svgz" )==0 ) {
#if GD16 || GD18 || WZ
       	return( 1 );
#else
	return( 0 );
#endif
	}

else if( strcmp( dev, "swf" )==0 ) {
#ifdef NOSWF
      return( 0 );
#endif
      return( 1 );
      }


return( 0 );
}

/* ========================== */
/* DEVSTRING - creates a string listing the output format options available with this build. */
int
PL_devstring( s )
char *s;
{
/* Added svg option - BT 05/11/01 */
strcpy( s, "This build can produce: PS EPS " );
#ifndef NOSVG
 strcat( s, "SVG " );
 #if WZ || GD16 || GD18
       	strcat( s, "SVGZ " );
 #endif
#endif
#ifndef NOX11
 strcat( s, "X11 " );
#endif
#ifdef GD13
 strcat( s, "GIF " );
#endif
#ifdef GD16
 strcat( s, "PNG " );
#endif
#ifdef GD18
 strcat( s, "PNG JPEG WBMP " );
#endif
#ifdef GDFREETYPE
 strcat( s, "FreeType2 " );
#endif
/* Added swf option - BT 13/01/03 */
#ifndef NOSWF
 strcat( s, "SWF " );
#endif

return( 0 );
}

/* ========================== */
/* DEVNAMEMAP - translate between internal and external 
	representation of output device code */
int
PL_devnamemap( s, t, mode )
char *s; /* internal (PLS.device) */
char *t; /* external (command line opt[1] or DEVICE */
int mode; /* 1 = set s given t    2 = set t given s */
{

if( mode == 1 ) {

	/* old monochrome ps */
	if( strcmp( t, "bwps" )==0 ) *s = 'p';

	/* postscript */
	else if( strcmp( t, "ps" )==0 ) *s = 'c';

	/* eps */
	else if( strcmp( t, "eps" )==0 ) *s = 'e';

	/* svg  - BT 05/11/01 */
	else if( strcmp( t, "svg" )==0 ) *s = 's';
	else if( strcmp( t, "svgz" )==0 ) {
		*s = 's';
#ifndef NOSVG
		PLGS_z( 1 );
#endif
		}

        else if( strcmp( t, "swf" )==0 ) {
		*s = 'f';
		if( !devavail( t ) ) return( Eerr( 5975, "SWF not supported in this build", "" ) );
		}


	/* x11 */
	else if( strcmp( t, "x11" )==0 ) {
		if( !devavail( "x" ) ) return( Eerr( 5973, "X11 not supported in this build", "" ) );
		*s = 'x';
		}

	/* GD image formats */
	else if( GL_smember( t, "gif png jpeg wbmp" ) ) { 
		*s = 'g'; 
		if( !devavail( t )) return( Eerr( 5974, "requested output format not supported in this build", t  ));
		/* Png = 1;  */
#ifndef NOGD
		PLGG_setimfmt( t );
#endif
		}


	else Eerr( 8026, "unrecognized device code", t );
	return( 0 );
	}
else if( mode == 2 ) {
	if( *s == 'p' ) strcpy( t, "bwps" );
	else if( *s == 'c' ) strcpy( t, "ps" );
	else if( *s == 'e' ) strcpy( t, "eps" );
#ifndef NOGD
	else if( *s == 'g' ) PLGG_getimfmt( t );
#endif
#ifndef NOSVG
	else if( *s == 's' ) PLGS_fmt( t ); /* svg  - BT 05/11/01 */
#endif
	else if( *s == 'f' ) strcpy( t, "swf" );
	else if( *s == 'x' ) strcpy( t, "x11" );
	else if( *s == 'n' ) strcpy( t, "nodevice" );
	else return( Eerr( 8025, "unrecognized internal device rep", "" ) );
	return( 0 );
	}
else return( Eerr( 8027, "invalid devnamemap mode", "" ) );
}

/* ============================== */
/* MAKEOUTFILENAME - given script file name, make a default output file
   for when no output file is specified by user.

   Returns 0 if ok; 1 if usage error 
*/
int
PL_makeoutfilename( scriptfn, outfn, dev, page )
char *scriptfn; /* script name (or if -o and page > 1, may be the output file name given on command line) */
char *outfn;
char dev;
int page;
{
int len, j;
char *ext;
char imfmt[20];

len = strlen( scriptfn );

for( j = len-1; j >= 0; j-- ) if( scriptfn[j] == '.' ) break;
if( j < 0 ) ext = "";
else ext = &scriptfn[ j ];

/* svg added - BT 05/11/01 */
if( GL_smember( ext, ".p .pl .plo .pls .htm .html .gif .png .eps .ps .jpg .jpeg .bmp .svg .svgz .swf" )) {
	strcpy( outfn, scriptfn );
	len -= strlen( ext );
	}
else strcpy( outfn, "out" );

if( dev == 'e' ) strcpy( imfmt, "eps" );
#ifndef NOGD
else if( dev == 'g' ) PLGG_getimfmt( imfmt );
#endif
#ifndef NOSVG
else if( dev == 's' ) PLGS_fmt( imfmt ); /* svg or svgz  - BT 05/11/01 */
#endif

else if( dev == 'f' ) strcpy( imfmt, "swf" );

else if( dev == 'm' ) strcpy( imfmt, "map" ); /* for click map name */

if( page > 1 ) sprintf( &outfn[ len ], ".p%d.%s", page, imfmt );
else sprintf( &outfn[ len ], ".%s", imfmt );
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
