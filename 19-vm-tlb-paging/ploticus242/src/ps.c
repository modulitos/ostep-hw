/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* Postscript driver.	(c) 1989-96 Steve Grubb, Marvin Newhouse

   Checking for redundant calls is not done here; should be done by caller.

	13 Nov 95 scg	upgraded to EPSF-3.0 guidelines, including:
			bounding box and page # information; and
			using "userdict begin" and "end" to ensure
			that dictionary stack is undisturbed.

			Added trailer generation.

	16 Apr 96 scg, mmn  Incorporated the special characters feature 
			written by Marvin Newhouse.

	24 Apr 96 scg	Added color.

	27 Sep 01 scg   Added ISO char set encoding, as contributed by 
			Johan Hedin <johan@eCare.se>  May be turned off
			by setting ps_latin1 = 0.

			
*/
#ifndef NOPS

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef PLOTICUS
#include "special_chars.h"
#endif

extern int TDH_err(), PLG_xrgb_to_rgb(), PLG_colorname_to_rgb();
extern int GL_member(), GL_goodnum();
extern int atoi(), fchmod(); /* sure thing or return value not used */

#define Eerr(a,b,c)  TDH_err(a,b,c)

#define YES 1
#define NO 0
#define PORTRAIT 0
#define LANDSCAPE 1
#define MARG_X 14 
#define MARG_Y 8 
#define PAGWIDTH 600;

static int ps_device;		/* 'p' = monochrome, 'c' = color, 'e' = eps (color) */
static int ps_orient;		/* paper orientation (-1 = not done) */
static int ps_orgx, ps_orgy;	/* location of origin on page */
static int ps_theta;		/* current rotation for page orientation */
static char ps_font[60];		/* current font name */
static int ps_chdir;	 	/* char direction in degrees */
static int ps_curpointsz;		/* current char size in points */
static int ps_specialpointsz;		/* current char size in pts for special chars */
static int ps_stdout;		/* 1 if Epf is stdout */
static int ps_paginate = 1;
static long ps_bbloc;
static FILE *ps_fp;
static int ps_latin1 = 1;		/* use latin1 character encoding */
static double ps_darken = 1.0;		/* factor to darken all colors (1.0 = none) 
				           Used when printing graphics that were originally light lines on dark (lxlogo renderd) */

#ifdef PLOTICUS
static int ps_print_special();
static int ps_set_specialsz();
#endif

extern double atof();

/* ============================= */
int
PLGP_initstatic()
{
ps_paginate = 1;
ps_latin1 = 1;
ps_darken = 1.0;

return( 0 );
}


/* ============================= */
int
PLGP_setup( name, dev, f )    
char *name; /* arbitrary name */
char dev;  /* 'p' = monochrome   'c' = color   'e' = eps */
char *f;  /* file to put code in */
{  
char filename[256]; 

/* initialize.. */
if( dev != 'e' && dev != 'p' && dev != 'c' ) dev = 'p';
ps_device = dev;
ps_orient = -1;
strcpy( ps_font, "/Helvetica" );
ps_theta = 0;
ps_chdir = 0;
ps_curpointsz = 10;
if( dev == 'e' ) ps_paginate = 0;


ps_stdout = 0;
if( strcmp( f, "stdout" )==0 ) { ps_fp = stdout; ps_stdout = 1; }
else if(  f[0] == '\0' ) {
	if( dev == 'e' ) strcpy( filename, "out.eps" );
	else	{ ps_fp = stdout; ps_stdout = 1; }
	}
else strcpy( filename, f );


if( !ps_stdout ) {
	ps_fp = fopen( filename, "w" ); /* output file */
	if( ps_fp == NULL ) return( Eerr( 12030, "Cannot open postscript output file", filename ) );
#ifdef UNIX
	fchmod( fileno( ps_fp ), 00644 );
#endif
	}

/* print header */
fprintf( ps_fp,  "%%!PS-Adobe-3.0 EPSF-3.0\n" );
fprintf( ps_fp,  "%%%%Title: %s\n", name );
fprintf( ps_fp,  "%%%%Creator: ploticus (http://ploticus.sourceforge.net)\n" );
if( ps_paginate ) fprintf( ps_fp,  "%%%%Pages: (atend)\n" );
if( ps_device == 'e' ) {
	if( !ps_stdout ) ps_bbloc = ftell( ps_fp );
	fprintf( ps_fp,  "%%%%BoundingBox: (atend)                                                            \n" );
	}
	
fprintf( ps_fp,  "%%%%EndComments\n\n\n" );
fprintf( ps_fp,  "%%%%BeginProlog\n" );

	
/* set up macros to reduce output verbosity */
fprintf( ps_fp,  "userdict begin\n" );
fprintf( ps_fp,  "/sset\n" );
fprintf( ps_fp,  "{ translate rotate } def \n" );
fprintf( ps_fp,  "/sclr\n" );
fprintf( ps_fp,  "{ rotate translate } def \n" );
fprintf( ps_fp,  "/mv { moveto } def\n" );
fprintf( ps_fp,  "/np { newpath } def\n" );
fprintf( ps_fp,  "/ln { lineto } def\n" );
fprintf( ps_fp,  "/st { stroke } def\n" ); 
fprintf( ps_fp,  "/sh { show } def\n" );
fprintf( ps_fp,  "/cent { stringwidth pop sub 2 div 0 rmoveto } def\n" );
fprintf( ps_fp,  "/rjust { stringwidth pop sub 0 rmoveto } def\n" );

if( ps_latin1 ) {
  /* Iso latin 1 encoding from gnuplot, as contributed by Johan Hedin <johan@eCare.se>  - added 9/27/01 */
  fprintf( ps_fp,  "/reencodeISO {\n");
  fprintf( ps_fp,  "dup dup findfont dup length dict begin\n");
  fprintf( ps_fp,  "{ 1 index /FID ne { def }{ pop pop } ifelse } forall\n");
  fprintf( ps_fp,  "currentdict /CharStrings known {\n");
  fprintf( ps_fp,  "        CharStrings /Idieresis known {\n");
  fprintf( ps_fp,  "                /Encoding ISOLatin1Encoding def } if\n");
  fprintf( ps_fp,  "} if\n");
  fprintf( ps_fp,  "currentdict end definefont\n");
  fprintf( ps_fp,  "} def\n");
  fprintf( ps_fp,  "/ISOLatin1Encoding [\n");
  fprintf( ps_fp,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( ps_fp,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( ps_fp,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( ps_fp,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( ps_fp,  "/space/exclam/quotedbl/numbersign/dollar/percent/ampersand/quoteright\n");
  fprintf( ps_fp,  "/parenleft/parenright/asterisk/plus/comma/minus/period/slash\n");
  fprintf( ps_fp,  "/zero/one/two/three/four/five/six/seven/eight/nine/colon/semicolon\n");
  fprintf( ps_fp,  "/less/equal/greater/question/at/A/B/C/D/E/F/G/H/I/J/K/L/M/N\n");
  fprintf( ps_fp,  "/O/P/Q/R/S/T/U/V/W/X/Y/Z/bracketleft/backslash/bracketright\n");
  fprintf( ps_fp,  "/asciicircum/underscore/quoteleft/a/b/c/d/e/f/g/h/i/j/k/l/m\n");
  fprintf( ps_fp,  "/n/o/p/q/r/s/t/u/v/w/x/y/z/braceleft/bar/braceright/asciitilde\n");
  fprintf( ps_fp,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( ps_fp,  "/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef/.notdef\n");
  fprintf( ps_fp,  "/.notdef/dotlessi/grave/acute/circumflex/tilde/macron/breve\n");
  fprintf( ps_fp,  "/dotaccent/dieresis/.notdef/ring/cedilla/.notdef/hungarumlaut\n");
  fprintf( ps_fp,  "/ogonek/caron/space/exclamdown/cent/sterling/currency/yen/brokenbar\n");
  fprintf( ps_fp,  "/section/dieresis/copyright/ordfeminine/guillemotleft/logicalnot\n");
  fprintf( ps_fp,  "/hyphen/registered/macron/degree/plusminus/twosuperior/threesuperior\n");
  fprintf( ps_fp,  "/acute/mu/paragraph/periodcentered/cedilla/onesuperior/ordmasculine\n");
  fprintf( ps_fp,  "/guillemotright/onequarter/onehalf/threequarters/questiondown\n");
  fprintf( ps_fp,  "/Agrave/Aacute/Acircumflex/Atilde/Adieresis/Aring/AE/Ccedilla\n");
  fprintf( ps_fp,  "/Egrave/Eacute/Ecircumflex/Edieresis/Igrave/Iacute/Icircumflex\n");
  fprintf( ps_fp,  "/Idieresis/Eth/Ntilde/Ograve/Oacute/Ocircumflex/Otilde/Odieresis\n");
  fprintf( ps_fp,  "/multiply/Oslash/Ugrave/Uacute/Ucircumflex/Udieresis/Yacute\n");
  fprintf( ps_fp,  "/Thorn/germandbls/agrave/aacute/acircumflex/atilde/adieresis\n");
  fprintf( ps_fp,  "/aring/ae/ccedilla/egrave/eacute/ecircumflex/edieresis/igrave\n");
  fprintf( ps_fp,  "/iacute/icircumflex/idieresis/eth/ntilde/ograve/oacute/ocircumflex\n");
  fprintf( ps_fp,  "/otilde/odieresis/divide/oslash/ugrave/uacute/ucircumflex/udieresis\n");
  fprintf( ps_fp,  "/yacute/thorn/ydieresis\n");
  fprintf( ps_fp,  "] def\n");
  }

/* load default font */
if( ps_latin1 ) fprintf( ps_fp,  "%s reencodeISO\n", ps_font ); /* scg 9/27/01 */
else fprintf( ps_fp,  "%s findfont\n", ps_font );

fprintf( ps_fp,  "%d scalefont setfont\n", (int) ps_curpointsz );
fprintf( ps_fp,  "%%%%EndProlog\n" ); 

return( 0 );
}


/* ============================= */
int
PLGP_moveto( x, y )
double x, y;
{

/* convert to p.s. units (1/72 inch) */
x = ( x * 72.0 ) +MARG_X; y = ( y * 72.0 ) + MARG_Y; 
fprintf( ps_fp,  "np\n%-5.2f %-5.2f mv\n", x, y ); 
return( 0 );
}


/* ============================= */
int
PLGP_lineto( x, y )
double x, y;
{
/* convert to p.s. units */
x = ( x * 72 ) +MARG_X; 
y = ( y * 72 ) +MARG_Y; 
fprintf( ps_fp,  "%-5.2f %-5.2f ln\n", x, y );
return( 0 );
}

/* ============================== */
int
PLGP_closepath()
{
fprintf( ps_fp, "closepath\n" );
return( 0 );
}

/* ============================== */
int
PLGP_stroke( )
{
fprintf( ps_fp, "st\n" );
return( 0 );
}


/* ============================= */
int
PLGP_path( x, y )
double x, y;
{
/* convert to p.s. units */
x = ( x * 72 ) +MARG_X; y = ( y * 72 ) + MARG_Y; 
fprintf( ps_fp,  "%-5.2f %-5.2f ln\n",  x, y );
return( 0 );
}

/* ============================= */
/* set current color for text and lines */
int
PLGP_color( color )
char *color;
{
int i, n;
double r, g, b, PLG_rgb_to_gray();
int slen;

/* color parameter can be in any of these forms:
   "rgb(R,G,B)"  where R(ed), G(reen), and B(lue) are 0.0 (none) to 1.0 (full)
   "xrgb(xxxxxx)" or "xrgb(xxxxxxxxxxxx)"
   "hsb(H,S,B)"  where H(ue), S(aturation), and B(rightness) range from 0.0 to 1.0.
   "cmyk(C,M,Y,K)"  where values are 0.0 to 1.0
   "gray(S)"	 where S is 0.0 (black) to 1.0 (white)
   "S"		 same as above
    or, a color name such as "blue" (see color.c)
*/ 
for( i = 0, slen = strlen( color ); i < slen; i++ ) {
	if( GL_member( color[i], "(),/:|-" ) ) color[i] = ' ';
	else color[i] = tolower( color[i] );
	}
if( strncmp( color, "rgb", 3 )==0 ) {
	n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
	if( n != 3 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	if( ps_device == 'p' ) fprintf( ps_fp, "%g setgray\n", PLG_rgb_to_gray( r, g, b ) * ps_darken );
	else fprintf( ps_fp, "%g %g %g setrgbcolor\n", r*ps_darken, g*ps_darken, b*ps_darken );
	}
else if( strncmp( color, "xrgb", 4 )==0 ) {
	if (PLG_xrgb_to_rgb( &color[5], &r, &g, &b)) return(1);
	if( ps_device == 'p' ) fprintf( ps_fp, "%g setgray\n", PLG_rgb_to_gray( r, g, b )*ps_darken );
	else fprintf( ps_fp, "%g %g %g setrgbcolor\n", r*ps_darken, g*ps_darken, b*ps_darken );
	}
else if( color[0] == 'x' ) {  /* added scg 11/5/07 */
        if (PLG_xrgb_to_rgb( &color[1], &r, &g, &b)) return(1);
	if( ps_device == 'p' ) fprintf( ps_fp, "%g setgray\n", PLG_rgb_to_gray( r, g, b )*ps_darken );
	else fprintf( ps_fp, "%g %g %g setrgbcolor\n", r*ps_darken, g*ps_darken, b*ps_darken );
        }

else if( strncmp( color, "hsb", 3 )==0 ) {
	n = sscanf( color, "%*s %lf %lf %lf", &r, &g, &b );
	if( n != 3 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	if( ps_device == 'p' ) fprintf( ps_fp, "%g setgray\n", PLG_rgb_to_gray( r, g, b ) );
	else fprintf( ps_fp, "%g %g %g sethsbcolor\n", r, g, b );
	}

else if( strncmp( color, "cmyk", 4 )==0 ) { /* added scg 10/26/00 */
	double c, m, y, k;
	n = sscanf( color, "%*s %lf %lf %lf %lf", &c, &m, &y, &k );
	if( n != 4 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	if( ps_device == 'p' ) fprintf( ps_fp, "%g setgray\n", PLG_rgb_to_gray( r, g, b ) );
	else fprintf( ps_fp, "%g %g %g %g setcmykcolor\n", c, m, y, k );
	}

else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
	n = sscanf( color, "%*s %lf", &r );
	if( n != 1 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	fprintf( ps_fp, "%g setgray\n", r*ps_darken );
	}
else if( GL_goodnum( color, &i ) ) {
	fprintf( ps_fp, "%g setgray\n", atof(color) *ps_darken );
	}
else	{	/* color names */
	PLG_colorname_to_rgb( color, &r, &g, &b );
	if( ps_device == 'p' ) fprintf( ps_fp, "%g setgray\n", PLG_rgb_to_gray( r, g, b )*ps_darken );
	else fprintf( ps_fp, "%g %g %g setrgbcolor\n", r*ps_darken, g*ps_darken, b*ps_darken );
	}
return( 0 );
}


/* ============================== */
/* fill current path with current color */
int
PLGP_fill( )
{
fprintf( ps_fp,  "closepath\nfill\n" );
return( 0 );
}

/* ============================== */
int
PLGP_paper( i )
int i;
{

if( ps_device == 'e' ) return( 1 ); /* paper orientation has no meaning with EPS */

if( i == 1 ) { /* handle landscape mode-- it's put into portrait mode before beginning each page */
	ps_orgx = PAGWIDTH; 
	ps_orgy = 0; 
	ps_theta = 90; 
	fprintf( ps_fp,  "%d %d %d sset\n", ps_theta, ps_orgx, ps_orgy );
	} 
ps_orient = (int) i;
return( 0 );
}


/* ================================= */
int
PLGP_text( com, x, y, s, w )
char com;
double x, y;
char *s;
double w;
{
char str[400];
int i, j, slen;

x = (x*72)+MARG_X;  y = (y*72)+MARG_Y; w *= 72;

/* if text direction is not normal do a rotate then move.. */
if( ps_chdir != 0 ) fprintf( ps_fp,  "%d %-5.2f %-5.2f sset 0 0 mv\n", ps_chdir, x, y ); 
/* normal direction-- do a move.. */
else fprintf( ps_fp,  "%-5.2f %-5.2f mv ", x, y );

if( GL_member( com, "CJ" )) {
	/* GL_strip_ws( s ); */ /* removed scg 8/23/04 (suscript) */

	/* escape out parens and substitute special char requests with 'm' for centering */
	for( i = 0, j = 0, slen = strlen( s ); i < slen; i++ ) {
		if( s[i] == '(' || s[i] == ')' ) { str[j++] = '\\'; str[j++] = s[i]; }
		else if( s[i] == '\\' && s[i+1] == '(' ) { str[j++] = 'm'; i += 3; }
	        else if( s[i] == '\\' && s[i+1] == 's' ) { str[j++] = 'm'; i += 3; }
	        else if( s[i] == '\\' ) { str[j++] = s[i]; str[j++] = s[i]; i += 3; }
		else str[j++] = s[i];
		}
	str[j] = '\0';
	
	/* centered text */
	if( com == 'C' ) fprintf( ps_fp,  "%-5.2f (%s) cent ", w, str ); 
	else if( com == 'J' ) fprintf( ps_fp,  "%-5.2f (%s) rjust ", w, str );
	}

ps_specialpointsz = ps_curpointsz;

/* print the string */
fprintf( ps_fp, "\n(" );
for( i = 0, slen = strlen( s ); i < slen; i++ ) {
	if( s[i] == '(' || s[i] == ')' ) { fprintf( ps_fp, "\\%c", s[i]); }
#ifdef PLOTICUS
	else if( s[i] == '\\' && s[i+1] == 's' ) { ps_set_specialsz( &s[i+2] ); i += 3; }
	else if( s[i] == '\\' && s[i+1] == '(' ) { ps_print_special( &s[i+2] ); i += 3; }
#endif
	else if( s[i] == '\\' ) fprintf( ps_fp, "\\%c", s[i] ); /* scg 1-6-97 */
	else { fprintf( ps_fp, "%c", s[i] ); }
	}
fprintf( ps_fp, ") sh\n" );

if( ps_chdir != 0 ) fprintf( ps_fp,  "%-5.2f %-5.2f %d sclr\n", -x, -y, -ps_chdir ); /* restore */
return( 0 );
}


/* ================================= */
int
PLGP_pointsize( p )
int p;
{
ps_curpointsz = p;
if( ps_latin1 ) fprintf( ps_fp,  "%s reencodeISO\n", ps_font ); /* scg 09/27/01 */
else fprintf( ps_fp,  "%s findfont\n", ps_font ); 

fprintf( ps_fp,  "%d scalefont\nsetfont\n", p );
return( 0 );
}


/* ================================== */
int
PLGP_font( f )
char *f;
{
if( strcmp( f, ps_font ) != 0 ) {
	strcpy( ps_font, f );

	if( ps_latin1 ) fprintf( ps_fp,  "%s reencodeISO\n", ps_font ); /* scg 9/27/01 */
	else fprintf( ps_fp,  "%s findfont\n", ps_font ); 

	fprintf( ps_fp,  "%d scalefont setfont\n", ps_curpointsz );
	}
return( 0 );
}

/* ================================== */
int
PLGP_chardir( t )
int t;
{
ps_chdir = t;
return( 0 );
}


/* ================================== */
int
PLGP_linetype( s, x, y )
char *s;
double x, y;
{
/* X = line width;  Y = dash pattern magnification (0.1 to 10)
 *  S indicates dash pattern.  If S is "0", an unbroken (normal) line is produced.
 *  If S is "1" through "8", a preset dash pattern is used.  Otherwise, S is
 *  assumed to hold the dash pattern string "[ n1 n2 n3.. ]".	
 */
static int dash[10][6]= { {0,0,0,0,0,0}, {1,1}, {3,1}, {5,1}, {2,1,1,1}, {4,1,1,1}, {6,1,1,1}, 
			  {2,1,1,1,1,1}, {4,1,1,1,1,1}, {6,1,1,1,1,1} };
int ltype, i;

fprintf( ps_fp,  "%3.1f setlinewidth\n", x );
if(  s[0] == '\0' || strcmp( s, "0" )==0 ) fprintf( ps_fp,  "[] 0 setdash\n" );
else 	{
	if( strlen( s ) > 1 ) { 
		ltype = 0; 
		sscanf( s, "%d %d %d %d %d %d", &dash[0][0], &dash[0][1], &dash[0][2], 
			&dash[0][3], &dash[0][4], &dash[0][5] );
		}
	else ltype = atoi( s );
	fprintf( ps_fp,  "[" );
	for( i = 0; i < 6; i++ ) 
		if( dash[ ltype ][ i ] > 0 ) fprintf( ps_fp,  "%3.1f ", dash[ ltype ][ i ] * y );
	fprintf( ps_fp,  "] 0 setdash\n" );
	}
return( 0 );
}
	

/* =================================== */
int
PLGP_show()
{
if( ps_orient == 1 )fprintf( ps_fp,  "%d %d %d sclr\n", -ps_orgx, -ps_orgy, -ps_theta ); /* restore rotation */
ps_orient = -1; 
if( ps_device != 'e' ) fprintf( ps_fp, "showpage\n" ); /* condition added scg 9/26/03 */
return( 0 );
}

/* =================================== */
int
PLGP_newpage( p )
int p;
{
if( ps_paginate )fprintf( ps_fp, "\n\n\n%%%%Page: %d %d\n", p, p );
return( 0 );
}

/* =================================== */
int
PLGP_trailer( pp, x1, y1, x2, y2 )
int pp; 
double x1, y1, x2, y2;
{
/* if( x1 < 0.0 ) x1 = 0.0;
 * if( y1 < 0.0 ) y1 = 0.0;
 * if( x2 < 0.0 ) x2 = 0.0;
 * if( y2 < 0.0 ) y2 = 0.0;
 */ /* negative values in bounding box should be ok - scg 1/22/01 */

fprintf( ps_fp, "end\n" );
if( ps_paginate ) {
	fprintf( ps_fp, "\n\n\n%%%%Trailer\n" ); /* condition added scg 9/26/03 */
	fprintf( ps_fp, "%%%%Pages: %d\n", pp );
	}
if( ps_device == 'e' ) {
	if( !ps_stdout ) {
		/* fprintf( ps_fp, "%%%%EOF\n" ); */ /* commented out scg 9/26/03 */
		fseek( ps_fp, ps_bbloc, SEEK_SET ); /* go back to write BoundingBox near top of file.. */
		}
	fprintf( ps_fp, "%%%%BoundingBox: %.0f %.0f %.0f %.0f\n", 
  	    ( x1 * 72 ) + MARG_X, ( y1 * 72 ) + MARG_Y, ( x2 * 72 ) + MARG_X, ( y2 * 72 ) + MARG_Y );
	}

if( ps_device != 'e' || ps_stdout ) fprintf( ps_fp, "%%%%EOF\n" );
if( ps_stdout ) return( 0 ); /* we don't want to close stdout */
if( ps_fp != NULL ) fclose( ps_fp );
return( 0 );
}

#ifdef PLOTICUS
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */
/* ================================= */

static int
ps_set_specialsz( s )
char *s;
{
char tmp[5];
/*
 * Check for two characters to set the point size for special chars
 * This size will remain in effect for all special chars in this string
 * or until another \s is encountered
 */
if( GL_member(s[0],"+-0123456789") && GL_member(s[1],"0123456789") )
	{
	tmp[0] = s[0]; tmp[1] = s[1]; tmp[2] = '\0';
	if( GL_member(s[0], "+-") ) ps_specialpointsz = ps_curpointsz + atoi( tmp );
	else ps_specialpointsz = atoi( tmp );
	}
else 	{
	char sbuf[8];
	sprintf( sbuf, "\\s%c%c", s[0], s[1] );
	Eerr( 12033, "Invalid \\s operator", sbuf );
	}
return( 0 );
}

/* ================================= */
/*
 * Lookup and generate PostScript for printing special chars
 *	using the  troff  conventions as input where they exist.
 * e.g. Input: \(bu	Outputs a bullet
 *      Input: \(dg	Outputs a dagger
 *      Input: \(<=	Outputs a less than or equal sign
 * MMN 072292
 */
static int
ps_print_special( s )
char *s;
{
int i;
char sbuf[8];

/* Check for two letter abbreviations which corresppond to a standard font encoding */
for( i = 0; strcmp( ps_special_std[i][0],"@@") != 0 ; i++ ) {
	if( strncmp( ps_special_std[i][0], s, 2 ) != 0 ) continue;

	/* end the current string and save the current font on the stack */
	fprintf( ps_fp, ") sh\ncurrentfont\n");

	/* Reset the font to the currentfont at the special point size  */
	if( ps_latin1 == 1 ) fprintf( ps_fp, "%s reencodeISO %d scalefont setfont", ps_font, ps_specialpointsz ); 
	else /* fprintf( ps_fp, "%s findfont %d scalefont setfont", ps_font, ps_specialpointsz ); */

	/* Show the character and reset the font from what's left on the stack */
	fprintf( ps_fp, " (\\%s) sh\nsetfont\n(", ps_special_std[i][1]);
	return ( 1 );
	}
/* If not found ... */
/* Check for symbols in the symbol font encoding vector */
for( i = 0; strcmp(ps_symbol[i][0],"@@") != 0 ; i++ ) {
	if( strncmp(ps_symbol[i][0], s, 2 ) != 0 ) continue;

	/* end the current string and save the current font on the stack */
	fprintf( ps_fp, ") sh\ncurrentfont\n");

	/* Set the font to Symbol */
	fprintf( ps_fp, "/Symbol findfont %d scalefont setfont", ps_specialpointsz ); /* don't reencode Symbol font */

	/* Show the character and reset the font from what's left on the stack */
	fprintf( ps_fp, " (\\%s) sh\nsetfont\n(", ps_symbol[i][1]);
	return ( 1 );
	}
/* symbol abbrev. not found */
sprintf( sbuf, "\\(%c%c", s[0], s[1] );
Eerr( 12035, "warning: special symbol not found", sbuf );
fprintf( ps_fp, "??" );
return( -1 );
}
#endif

/* ========== */
/* fontname - given a base name (such as /Helvetica) and a modifier
   such as I (italic) B (bold) or BI (bold italic), build the postscript
   font name and copy it into name. */
int
PLGP_fontname( basename, name )
char *basename; 
char *name; /* in: B, I, or BI.  out: full postscript font name */
{
int i, slen;

for( i = 0, slen = strlen( name ); i < slen; i++ ) name[i] = tolower( name[i] );

if( strcmp( name, "" )== 0 || strcmp( name, "r" )==0 ) {
	strcpy( name, basename );
	return( 0 );
	}

if( strcmp( name, "b" )==0 ) {
	if( strcmp( basename, "/Helvetica" )==0 ) strcpy( name, "/Helvetica-Bold" );
	else if( strcmp( basename, "/Helvetica-Oblique" )==0 ) 
		strcpy( name, "/Helvetica-BoldOblique" );
	else if( strcmp( basename, "/Times-Roman" )==0 ) strcpy( name, "/Times-Bold" );
	else if( strcmp( basename, "/Times-Italic" )==0 ) strcpy( name, "/Times-BoldItalic" );
	else if( strcmp( basename, "/Courier" )==0 ) strcpy( name, "/Courier-Bold" );
	else if( strcmp( basename, "/Courier-Oblique" )==0 ) strcpy( name, "/Courier-BoldOblique" );
	else if( strcmp( basename, "/Palatino-Roman" )==0 ) strcpy( name, "/Palatino-Bold" );
	else sprintf( name, "%s-Bold", basename );
	}
if( strcmp( name, "i" )==0 ) {
	if( strcmp( basename, "/Helvetica" )==0 ) strcpy( name, "/Helvetica-Oblique" );
	else if( strcmp( basename, "/Times-Roman" )==0 ) strcpy( name, "/Times-Italic" );
	else if( strcmp( basename, "/Times-Bold" )==0 ) strcpy( name, "/Times-BoldItalic" );
	else if( strcmp( basename, "/Courier" )==0 ) strcpy( name, "/Courier-Oblique" );
	else if( strcmp( basename, "/Courier-Bold" )==0 ) strcpy( name, "/Courier-BoldOblique" );
	else sprintf( name, "%s-Italic", basename );
	}
if( strcmp( name, "bi" )==0 ) {
	if( strncmp( basename, "/Helvetica", 10 )==0 ) strcpy( name, "/Helvetica-BoldOblique" );
	else if( strncmp( basename, "/Times", 6 )==0 ) strcpy( name, "/Times-BoldItalic" );
	else if( strncmp( basename, "/Courier", 8 )==0 ) strcpy( name, "/Courier-BoldOblique" );
	else sprintf( name, "%s-BoldItalic", basename );
	}
return( 0 );
}

/* =========================== */
/* SETTINGS */
int
PLGP_settings( attr, val )
char *attr, *val;
{
double atof();
if( strcmp( attr, "ps_latin1_encoding") == 0 ) ps_latin1 = atoi( val );
else if( strcmp( attr, "darken") == 0 ) ps_darken = atof( val );

return( 0 );
}

#endif  /* NOPS */

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
