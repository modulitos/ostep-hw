/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* SVG Driver for Ploticus - Copyright 2001 Bill Traill (bill@traill.demon.co.uk).
 * Portions Copyright 2001, 2002 Stephen C. Grubb 
 * Covered by GPL; see the file ./Copyright for details. */

/*
   Checking for redundant calls is not done here; should be done by caller.

   special characters not delt with

	04Nov01 bt	Created svg driver based on existing postscript driver ps.c

	12Nov01 bt	Changed to relative addressing within a path. 
			Added grouping of styles and a default style in <g> elements
			Added some shorthand in the entity header
			Accuracy of some of the coords improved
			Added stoke to SVGfill to get rid of occasional ghost lines

	16Nov01 scg	Implemented <g translate> for improved alignment; implemented .svgz;
			other integration issues

	June02  scg	clickmap support added
*/
#ifndef NOSVG

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#ifdef WZ
#  include "zlib.h"
#endif

extern int TDH_err(), PLG_xrgb_to_rgb(), PLG_colorname_to_rgb(), PL_clickmap_out();
extern int atoi(), chmod(), unlink(); /* sure thing or return value not used */
extern int GL_member(), GL_goodnum();


#define Eerr(a,b,c)  TDH_err(a,b,c) 
#define stricmp(a,b) strcasecmp(a,b) 

#define MARG_X 0 
#define MARG_Y 0 

static int svg_stdout;		/* 1 if svg_fp is stdout */
static FILE *svg_fp;

static double svg_x_size;		/* width of the drawing area */
static double svg_y_size;		/* height of the drawing area */
static int  svg_path_in_prog =0;	/* flag to indicate if an svg path is in progress */
static char svg_cur_color[80] = "#000000";
static char svg_dash_style[128];
static double svg_line_width=1;

static char svg_font_name[100] = "";	/* current font name */
static int  svg_chdir;	 		/* char direction in degrees */
static int  svg_currpointsz;		/* current char size in points */
static char svg_font_weight[100];
static char svg_font_style[100];
static char svg_align[100];
static long svg_bbofs; 		   /* byte offset of viewbox statement as returned by ftell() - scg */
static int svg_pixs_inch; 	   /* number of pixels per inch - scg */
static int svg_dotag = 0;  	   /* if 1, write a suitable html <embed> tag to stdout - scg */
static char svg_filename[256] = ""; /* output file name (was local) - scg */

static char svg_style[1024] = "";	/* line,font styles etc */
static char svg_new_style[1024] = "";
static int  svg_style_in_prog = 0;	/* flag to indicate if a current <g> for attributes is open */

static double svg_last_x;
static double svg_last_y;

static char *svg_def_fill = "fill:#000000;";  	/* char * added - scg */
static char *svg_def_stroke = "stroke:#000000;"; /* char * added - scg */
static char *svg_def_font = "&ff;Helvetica;";    /* char * added - scg */

static int svg_compress = 0;			/* 1 = compress output; 0 = don't */
static char svg_compressmode[10] = "wb9";		
static char svg_tmpfilename[256] = "/unnamed";
static int svg_tmpfile_used = 0;
static char svg_encoding[80] = "iso-8859-1";
static int svg_xmldecl = 1;

static int svg_clickmap = 0;
static int svg_debug = 0;
static char svg_tagparms[80] = "";
/* static char svg_linkparms[128] = ""; */  /* discontinued 5/29/06 - use the new [target=new] syntax in clickmapurl instead */
static int svg_generic_js = 0;

static int esc_txt_svg();
static int svg_set_style();
static int svg_print_style();

static int svg_imgsetwidth = 0, svg_imgsetheight = 0;
static char svg_imgpathname[256] = "";

/* ============================= */
int
PLGS_initstatic()
{
svg_path_in_prog = 0;
svg_line_width = 1;
svg_dotag = 0;
strcpy( svg_filename, "" );
strcpy( svg_style, "" );
strcpy( svg_new_style, "" );
svg_style_in_prog = 0;
svg_compress = 0;
strcpy( svg_compressmode, "wb9" );
strcpy( svg_tmpfilename, "/unnamed" );
strcpy( svg_encoding, "iso-8859-1" );
svg_tmpfile_used = 0;
svg_xmldecl = 1;
svg_clickmap = 0;
svg_debug = 0;
strcpy( svg_tagparms, "" );
strcpy( svg_cur_color, "#000000" );
svg_imgsetwidth = 0; svg_imgsetheight = 0;
strcpy( svg_imgpathname, "" );
if( svg_font_name[0] == '\0' ) strcpy( svg_font_name, "Helvetica" );
return( 0 );
}


/* ============================= */
static int svg_set_style() 
{
char fontw[200] = "";
char fonts[200] = "";
char align[200] = "";
char font[200] = "";
char fill[200] = "";
char stroke[200] = "";

if (svg_font_weight[0] != '\0') sprintf(fontw,"&fw;%s;",svg_font_weight);
if (svg_font_style[0] != '\0') sprintf(fonts,"&fst;%s;",svg_font_style);

sprintf(font,"&ff;%s;",svg_font_name);
if (!strcmp(font,svg_def_font)) strcpy(font,"");
sprintf(fill,"fill:%s;",svg_cur_color);
if (!strcmp(fill,svg_def_fill)) strcpy(fill,"");
sprintf(stroke,"stroke:%s;",svg_cur_color);
if (!strcmp(stroke,svg_def_stroke)) strcpy(stroke,"");

if (!strcmp(svg_align, "start")) strcpy(align,"&as;");
if (!strcmp(svg_align, "middle")) strcpy(align,"&am;");
if (!strcmp(svg_align, "end")) strcpy(align,"&ae;");

sprintf (svg_new_style,"style=\"%s%s%s&sw;%3.1f;%s%s&fs;%dpt;%s\"",  /* pt added after font size integer.. scg 3/16/06 */
	fill,stroke,font,svg_line_width,fontw,fonts,svg_currpointsz,align);
return( 0 );
}

/* ============================= */

static int svg_print_style()
{
if (strcmp(svg_new_style,svg_style) ) {
	if (svg_style_in_prog) fprintf( svg_fp, "</g>");
	fprintf( svg_fp, "<g %s>\n",svg_new_style);
	strcpy(svg_style,svg_new_style);
	svg_style_in_prog = 1;
	}
return( 0 );
}

/* ======================================== */
/* SETPARMS - allow caller to pass required parms that svg driver needs - MUST be called before setup() */
int
PLGS_setparms( debug, tmpname, clickmap )
int debug;
char *tmpname;
int clickmap;
{
svg_debug = debug;
sprintf( svg_tmpfilename, "%s_V", tmpname );
svg_clickmap = clickmap;
return( 0 );
}


/* ============================= */
/* SETUP */
int
PLGS_setup( name, dev, outfile, pixs_inch, Ux, Uy, Upleftx, Uplefty )
char *name; /* arbitrary name */
char dev;  /* 'p' = monochrome   'c' = color   'e' = eps */
char *outfile;  /* file to put code in */
int pixs_inch;
double Ux;
double Uy;
int Upleftx;
int Uplefty;
{  

/* set globals */
if( dev != 's' ) dev = 's';
if( svg_font_name[0] == '\0' ) strcpy( svg_font_name, "Helvetica" );
strcpy( svg_font_weight, "" );
strcpy( svg_font_style, "" );
strcpy( svg_align, "start" );
svg_chdir = 0;
svg_currpointsz = 10;

svg_pixs_inch = pixs_inch; /* scg */
svg_path_in_prog =0;		
svg_line_width=1;
/* svg_dotag = 0;  	   */  /* this may be set to 1 (from command line) before this point, so leave svg_dotag alone.. scg 1/20/06 */
strcpy( svg_style, "" );
strcpy( svg_new_style, "" );
svg_style_in_prog = 0;	
svg_tmpfile_used = 0;


/* determine if we need to write to tmp file, and open appropriate file for write.. */
svg_stdout = 0;
if( strcmp( outfile, "stdout" )==0 || outfile[0] == '\0' ) svg_stdout = 1;
else strcpy( svg_filename, outfile );

if( svg_stdout || svg_compress ) {
	svg_fp = fopen( svg_tmpfilename, "w" ); /* output file */
	if( svg_fp == NULL ) return( Eerr( 12031, "Cannot open tmp output file", svg_tmpfilename ) );
	svg_tmpfile_used = 1;
	}
else 	{
	svg_fp = fopen( svg_filename, "w" );
	if( svg_fp == NULL ) return( Eerr( 12031, "Cannot open output file", svg_filename ) );
	}
if( svg_fp == NULL ) return( Eerr( 12031, "Cannot open output file", svg_filename ) );


svg_x_size = Ux * pixs_inch;
svg_y_size = Uy * pixs_inch;
/* print header */

if( svg_xmldecl ) fprintf( svg_fp, "<?xml version=\"1.0\" encoding=\"%s\" standalone=\"no\"?>\n", svg_encoding );

/* changing this section as a result of echlin mouseover enhancement.. scg 6/21/04 */
fprintf( svg_fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20010904//EN\" \n" );
fprintf( svg_fp, "  \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\" [ \n" );
if( svg_clickmap && !svg_generic_js ) {
	fprintf( svg_fp, "     <!ATTLIST svg \n" );
	fprintf( svg_fp, "        xmlns:a3 CDATA #IMPLIED \n" );
	fprintf( svg_fp, "        a3:scriptImplementation CDATA #IMPLIED> \n" );
	fprintf( svg_fp, "     <!ATTLIST script \n " );
	fprintf( svg_fp, "        a3:scriptImplementation CDATA #IMPLIED> \n" );
	}

/* this section was: 
 * fprintf( svg_fp, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 20001102//EN\"\n");
 * fprintf( svg_fp, "\"http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd\" [\n");
 */

fprintf( svg_fp, "<!ENTITY ff \"font-family:\">\n"); 
fprintf( svg_fp, "<!ENTITY fs \"font-size:\">\n"); 
fprintf( svg_fp, "<!ENTITY fw \"font-weight:\">\n"); 
fprintf( svg_fp, "<!ENTITY fst \"font-style:\">\n"); 
fprintf( svg_fp, "<!ENTITY sw \"stroke-width:\">\n"); 
fprintf( svg_fp, "<!ENTITY as \"text-anchor:start;\">\n"); 
fprintf( svg_fp, "<!ENTITY am \"text-anchor:middle;\">\n"); 
fprintf( svg_fp, "<!ENTITY ae \"text-anchor:end;\">\n"); 
fprintf( svg_fp, "]>\n");
fprintf( svg_fp, "<!-- Generated by ploticus (http://ploticus.sourceforge.net/)\n");
fprintf( svg_fp, "Title: %s\n",name);
fprintf( svg_fp, "SVG Driver by B.Traill\n");
fprintf( svg_fp, "-->\n");

svg_bbofs = ftell( svg_fp ); /* remember location of the viewBox line so we can update it later.. -scg */

/* <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 539.60 421.04" onload="init(evt)"
     xmlns:xlink="http://www.w3.org/1999/xlink"
     xmlns:a3="http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/"
     a3:scriptImplementation="Adobe">
<script type="text/ecmascript" a3:scriptImplementation="Adobe" xlink:href="ViewBox.js"/>
<script type="text/ecmascript" a3:scriptImplementation="Adobe" xlink:href="GraphPopups.js"/>
 */

/* these two statements will be overridden at eof when bounding box is known.. -scg  */
/* a significant amount of padding is provided */
/* xmlns=  added 10/2/03 scg */
fprintf( svg_fp, "<svg xmlns=\"http://www.w3.org/2000/svg\" \n" );
svg_bbofs = ftell( svg_fp ); /* remember location of the viewBox line so we can update it later.. -scg */

/* CAUTION! all of the following code is replicated below (search on "bbofs") */
fprintf( svg_fp, "  viewBox=\"0 0  %-5.2f %-5.2f\" \n", svg_x_size,svg_y_size); 
fprintf( svg_fp, "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" ); /* moved up from below .. scg 3/16/06 */
if( svg_clickmap && !svg_generic_js ) {
	fprintf( svg_fp, "  onload=\"init(evt)\"\n" );
	/* xmlns:xlink used to be here.. scg 3/16/06 */
	fprintf( svg_fp, "  xmlns:a3=\"http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/\" \n" );
	fprintf( svg_fp, "  a3:scriptImplementation=\"Adobe\" >\n" );
	fprintf( svg_fp, "<script type=\"text/ecmascript\" a3:scriptImplementation=\"Adobe\" xlink:href=\"ViewBox.js\"/> \n" );
	fprintf( svg_fp, "<script type=\"text/ecmascript\" a3:scriptImplementation=\"Adobe\" xlink:href=\"GraphPopups.js\"/> \n" );
	}
else fprintf( svg_fp, " > \n" );

fprintf( svg_fp, "<g tranform=\"translate(0,0)\"                                                                       \n\n" ); 
/* END of code that's replicated below */

/* print out default style */
fprintf( svg_fp, "<g style=\"%s%s%s\">\n",svg_def_fill,svg_def_stroke,svg_def_font);
strcpy(svg_style,"");
strcpy(svg_new_style,"");
svg_style_in_prog = 0;
return( 0 );
}


/* ============================= */
/* MOVETO */
int
PLGS_moveto( x, y )
double x, y;
{
double x1,y1;

x = ( x * svg_pixs_inch ) +MARG_X; 
y = svg_y_size - (( y * svg_pixs_inch ) +MARG_Y); 

if (!svg_path_in_prog) {
	svg_set_style();
	svg_print_style();
	fprintf( svg_fp, "<path d=\"");
	svg_path_in_prog =1;
	fprintf( svg_fp, "M%.4g %.4g", x, y );
	}
else 	{ 
	x1 = x - svg_last_x;
	y1 = y - svg_last_y;
	fprintf( svg_fp, "m%.4g %.4g", x1, y1 );
	}
svg_last_x = x;
svg_last_y = y;
return( 0 );
}


/* ============================= */
/* LINETO */
int
PLGS_lineto( x, y )
double x, y;
{
double x1,y1;
x = ( x * svg_pixs_inch ) +MARG_X; 
y = svg_y_size - (( y * svg_pixs_inch ) +MARG_Y); 

if (!svg_path_in_prog) {
	svg_set_style();
	svg_print_style();
	fprintf( svg_fp, "<path d=\"");
	svg_path_in_prog =1;
	fprintf( svg_fp, "L%.4g %.4g", x, y ); 
	}
else 	{ 
	x1 = x - svg_last_x;
	y1 = y - svg_last_y;
	fprintf( svg_fp, "l%.4g %.4g", x1, y1 );
	}
svg_last_x = x;
svg_last_y = y;
return( 0 );
}


/* ============================== */
/* STROKE - render a stroke (line) */
int
PLGS_stroke( )
{
char dash[50] = "";

if  (svg_dash_style[0] != '\0') sprintf(dash," stroke-dasharray=\"%s\"",svg_dash_style);
if (svg_path_in_prog) fprintf( svg_fp, "\" fill=\"none\"%s/>\n",dash); 
svg_path_in_prog = 0;
return( 0 );
}


/* ============================= */
/* PATH - add an element to a path (either new or existing) */
int
PLGS_path( x, y )
double x, y;
{

double x1,y1;

x = ( x * svg_pixs_inch ) +MARG_X; 
y = svg_y_size - (( y * svg_pixs_inch ) +MARG_Y); 

if (!svg_path_in_prog) {
	svg_set_style();
	svg_print_style();
	fprintf( svg_fp, "<path d=\"");
	svg_path_in_prog =1;
	fprintf( svg_fp, "L%.4g %.4g", x, y );
	} 
else 	{ 
	x1 = x - svg_last_x;
	y1 = y - svg_last_y;
	fprintf( svg_fp, "l%.4g %.4g", x1, y1 );
	}
svg_last_x = x;
svg_last_y = y;
return( 0 );
}

/* ============================= */
/* COLOR - set current color for text and lines */
int
PLGS_color( color )
char *color;
{
int i, n;
double r, g, b, PLG_rgb_to_gray();
int red,green,blue;
int slen;

/* color parameter can be in any of these forms:
   "rgb(R,G,B)"  where R(ed), G(reen), and B(lue) are 0.0 (none) to 1.0 (full)
   "xrgb(xxxxxx)" or "xrgb(xxxxxxxxxxxx)"
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
	r *= 255; g *= 255; b *= 255;
	red = (int)r; green = (int)g;	blue = (int)b;	
	 sprintf( svg_cur_color, "#%02x%02x%02x", red, green, blue);
	}
else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
	int gray;
	n = sscanf( color, "%*s %lf", &r );
	if( n != 1 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	gray = r * 255; /* was 256.. changed scg 6/17/04 */
	sprintf( svg_cur_color, "#%02x%02x%02x", gray, gray, gray);
	}
else if( strncmp( color, "xrgb", 4 )==0 ) {
	if (PLG_xrgb_to_rgb( &color[5], &r, &g, &b)) return(1);
	r *= 255; g *= 255; b *= 255;
	red = (int)r; green = (int)g; blue = (int)b;
	sprintf( svg_cur_color, "#%02x%02x%02x", red, green, blue);
	}
else if( color[0] == 'x' ) {  /* added scg 11/5/07 */
        /* if (PLG_xrgb_to_rgb( &color[1], &r, &g, &b)) return(1);
	 * r *= 255; g *= 255; b *= 255; 
	 * red = (int)r; green = (int)g; blue = (int)b;
	 * sprintf( svg_cur_color, "#%02x%02x%02x", red, green, blue);
	 */
	sprintf( svg_cur_color, "#%s", &color[1] );  /* handles 6 char hex representations only? (not 12 char?) */
        }
else if( GL_goodnum( color, &i ) ) {
	float no;
	int gray;
	sscanf(color,"%f",&no);

	gray = no * 255; /* was 256.. changed scg 6/17/04 */

	sprintf( svg_cur_color, "#%02x%02x%02x", gray, gray, gray);
	}
else	{	/* color names */
	PLG_colorname_to_rgb( color, &r, &g, &b );

	r *= 255; g *= 255; b *= 255;
	red = (int)r;	green = (int)g;	blue = (int)b;	
	sprintf( svg_cur_color, "#%02x%02x%02x", red, green, blue);
	}


return( 0 );
}


/* ============================== */
/* FILL - fill current path with current color */
int
PLGS_fill( )
{
if (svg_path_in_prog) fprintf( svg_fp, "z\" fill=\"%s\" stroke=\"%s\"/>\n",svg_cur_color,svg_cur_color);
svg_path_in_prog = 0;
return( 0 );
}

/* ============================== */

static int
esc_txt_svg( out, s )
char *out;
char *s;
{
int i, len;
for( i = 0, len = 0; s[i] != '\0'; i++ ) {
	if( s[i] == '<' ) { strcpy( &out[len], "&lt;" ); len += 4; }
	else if( s[i] == '>' ) { strcpy( &out[len], "&gt;" ); len += 4; }
	else if( s[i] == '&' ) {

		/* pass &#dddd and &#xHHHH constructs transparently.. */
		/* bug - (was: s[i+1] = '#' ) fixed oct 03 scg */
		if( s[i+1] == '#' && ( s[i+2] == 'x' || isdigit( (int) s[i+2] ) ) ) { 
			strcpy( &out[len], "&" ); 
			len++; 
			}

 		else 	{ 
			strcpy( &out[len], "&amp;" ); 
			len+= 5; 
			}
		}
	else out[len++] = s[i];
	}

out[ len ] = '\0';
return( 0 );
}

/* ================================= */
/* TEXT - render some text */
int
PLGS_text( com, x, y, s, w )
char com;
double x, y;
char *s;
double w;
{
char transform[200];
char esc_txt[4096];

x = (x*svg_pixs_inch)+MARG_X;  y = svg_y_size - ((y*svg_pixs_inch)+MARG_Y); w *= svg_pixs_inch;

/* set the text alignment */
if (com == 'T') strcpy(svg_align, "start");
else if (com == 'C') strcpy(svg_align, "middle");
else if (com == 'J') strcpy(svg_align, "end");

*esc_txt = '\0';
esc_txt_svg(esc_txt, s);

if (svg_chdir) sprintf(transform, " transform=\"rotate(-%d,%.2f,%.2f)\" ",svg_chdir,x,y);
else strcpy(transform,"");

svg_set_style();
svg_print_style();

fprintf(svg_fp, "<text x=\"%.2f\" y=\"%.2f\" stroke=\"none\"%s>%s</text>\n", x,y,transform,esc_txt);
return( 0 );
}


/* ================================= */
/* POINTSIZE - set text point size */
int
PLGS_pointsize( p )
int p;
{
svg_currpointsz = p;
return( 0 );
}


/* ================================== */
/* FONT - set font */
int
PLGS_font( f )
char *f;
{
if (f[0] == '/') strcpy( svg_font_name, ++f );
else strcpy( svg_font_name, f );
return( 0 );
}

/* ================================== */
/* CHARDIR - set text direction */
int
PLGS_chardir( t )
int t;
{
svg_chdir = t;
return( 0 );
}


/* ================================== */
/* LINETYPE - set line style */
int
PLGS_linetype( s, x, y )
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

strcpy(svg_dash_style,"");
svg_line_width = x ;

if(  s[0] == '\0' || strcmp( s, "0" )==0 ) strcpy(svg_dash_style,"");
else 	{
	char *p = svg_dash_style;

	if( strlen( s ) > 1 ) { 
		ltype = 0; 
		sscanf( s, "%d %d %d %d %d %d", &dash[0][0], &dash[0][1], &dash[0][2], &dash[0][3], &dash[0][4], &dash[0][5] );
		}
	else ltype = atoi( s );

	for( i = 0; i < 6; i++ ) {
		if( dash[ ltype ][ i ] > 0 ) p += sprintf( p,"%3.1f,", dash[ ltype ][ i ] * y );
		}
	p--;
	*p = '\0';
	}
return( 0 );
}

/* ================================ */
/* place secondary PNG/GIF/JPEG image within main image at absolute x, y */
int
PLGS_showimg( imgurl, x, y, align, width, height )
char *imgurl;
double x, y;
char *align;
int width, height; /* image dimemsions in SVG units */
{
x = ( x * svg_pixs_inch ) + MARG_X;
y = svg_y_size - (( y * svg_pixs_inch ) + MARG_Y);

if( strcmp( imgurl, "" )==0 ) imgurl = svg_imgpathname; /* as may have been set earlier */
if( strcmp( imgurl, "" )==0 ) imgurl = "NO_IMAGE_SPECIFIED";

if( width < 1 ) width = svg_imgsetwidth;    /* as may have been set earlier in setimgsize() */
if( width < 1 ) width = 20;		    /* last resort fallback */

if( height < 1 ) height = svg_imgsetheight;    /* as may have been set earlier in setimgsize() */
if( height < 1 ) height = 20;		    /* last resort fallback */

if( strncmp( align, "center", 6 )==0 ) { x -= width/2; y -= height/2; }

fprintf( svg_fp, "<image x=\"%f\" y=\"%f\" xlink:href=\"%s\" width=\"%dpx\" height=\"%dpx\" />\n", x, y, imgurl, width, height );
return( 0 );
}

/* =================================== */
/* set image size */
int
PLGS_setimg( pathname, width, height )
char *pathname;
int width, height;
{
strncpy( svg_imgpathname, pathname, 255 ); svg_imgpathname[255] = '\0'; 
svg_imgsetwidth = width;
svg_imgsetheight = height;
return( 0 );
}

/* =================================== */
/* TRAILER do end of file stuff */
int
PLGS_trailer( x1, y1, x2, y2 )
double x1, y1, x2, y2;
{
char *buf;
#ifdef WZ
  FILE *outfp;
#endif

if (svg_style_in_prog) fprintf( svg_fp, "</g>");
fprintf( svg_fp, "</g>\n" );   /* close default style */


#ifdef PLOTICUS
if( svg_clickmap ) {
	/* set a style for clickmap section */
	if( svg_debug ) fprintf( svg_fp, "<g style=\"fill-opacity:0;fill:red;stroke:#00ff00;stroke-width:0.4;\">\n" );
	else fprintf( svg_fp, "<g style=\"fill-opacity:0;fill:red;\">\n" );
	PL_clickmap_out( 0, 0 );  /* this will call PLGS_clickregion() (herein).. */ 
	fprintf( svg_fp, "</g>\n" ); /* close style */
	}
#endif

fprintf( svg_fp, "</g>\n" );   /* close translation transform -scg */

fprintf( svg_fp, "</svg>\n" );

/* now go back and update viewbox - scg */
fseek( svg_fp, svg_bbofs, SEEK_SET );	
/* xmlns=  added 10/2/03 scg */
fprintf( svg_fp, "  viewBox=\"0 0 %-5.2f %-5.2f\"   %s   \n", (x2-x1) * svg_pixs_inch, (y2-y1) * svg_pixs_inch, svg_tagparms );
fprintf( svg_fp, "  xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n" );
if( svg_clickmap && !svg_generic_js ) {
	/* all this is replicated below (search on "bbofs") */
	fprintf( svg_fp, "  onload=\"init(evt)\"\n" );
	/* xmlns:xlink used to be here.. scg 3/16/06 */
	fprintf( svg_fp, "  xmlns:a3=\"http://ns.adobe.com/AdobeSVGViewerExtensions/3.0/\" \n" );
	fprintf( svg_fp, "  a3:scriptImplementation=\"Adobe\" >\n" );
	fprintf( svg_fp, "<script type=\"text/ecmascript\" a3:scriptImplementation=\"Adobe\" xlink:href=\"ViewBox.js\"/> \n" );
	fprintf( svg_fp, "<script type=\"text/ecmascript\" a3:scriptImplementation=\"Adobe\" xlink:href=\"GraphPopups.js\"/> \n" );
	}
else fprintf( svg_fp, ">\n" );

fprintf( svg_fp, "<g transform=\"translate(%-5.2f,%-5.2f)\" >", 
	x1*svg_pixs_inch *-1.0, 
	(svg_y_size - (y2 * svg_pixs_inch)) *-1.0 );

if( svg_dotag ) {
	printf( "<embed src=\"%s\" name=\"svg_Embed\" width=\"%-5.2f\" height=\"%-5.2f\" \n",
		svg_filename, (x2-x1)* svg_pixs_inch, (y2-y1) * svg_pixs_inch );
	printf( "type=\"image/svg+xml\" pluginspage=\"http://www.adobe.com/svg/viewer/install/\"> \n" ); /* changed from svg- to svg+ */
	}

fclose( svg_fp );

/* if temp file used, do read/write.. -scg */
if( svg_tmpfile_used ) {
	svg_fp = fopen( svg_tmpfilename, "r" );
	if( svg_fp == NULL ) return( Eerr( 2487, "cannot reopen temp file", svg_tmpfilename ) );
#ifdef WZ
	if( svg_compress ) {
		if( svg_stdout ) outfp = (FILE *) gzdopen( 1, svg_compressmode );  /* stdout = 1 */
		else outfp = (FILE *) gzopen( svg_filename, svg_compressmode );
		if( outfp == NULL ) return( Eerr( 2488, "cannot open output file", svg_filename ) );
		} 
#endif
	buf = svg_style; /* reuse */
	while( fgets( buf, 999, svg_fp ) != NULL ) {
#ifdef WZ
		if( svg_compress ) gzprintf( outfp, "%s", buf ); 
		else 
#endif
			printf( "%s", buf );
		}
	fclose( svg_fp );
	unlink( svg_tmpfilename );
#ifdef WZ
	if( svg_compress && !svg_stdout ) {
		gzclose( outfp );
		chmod( svg_filename, 00644 );
		}
#endif
	}
else chmod( svg_filename, 00644 );
	
return( 0 );
}


/* ================================= */
/* FONTNAME - given a base name (such as Helvetica) and a modifier
   such as I (italic) B (bold) or BI (bold italic), build the 
	font style and weight strings */

int
PLGS_fontname( basename, name )
char *basename; 
char *name; /* in: B, I, or BI.  out: still the font name but statics now hold the font style and weight */
{
int i, slen;

for( i = 0, slen = strlen( name ); i < slen; i++ ) name[i] = tolower( name[i] );

strcpy (svg_font_weight, "");
strcpy (svg_font_style, "");

if( strcmp( name, "b" )==0 ) strcpy (svg_font_weight, "bold");
else if( strcmp( name, "i" )==0 ) strcpy (svg_font_style, "italic");
else if( strcmp( name, "bi" )==0 ) { strcpy (svg_font_weight, "bold"); strcpy (svg_font_style, "italic"); }

if (basename[0] == '/')  strcpy( name, basename++ );
else strcpy( name, basename );

return( 0 );
}

/* ================================ */
/* SHOWTAG - set flag to write a suitable html <embed> tag to stdout */

int
PLGS_showtag( mode )
int mode;
{
svg_dotag = mode;
return( 0 );
}

/* ================================= */
/* Z - turn on / off compression (svgz) */

int
PLGS_z( mode )
int mode;
{
#ifdef WZ
svg_compress = mode;
#else
Eerr( 4275, "svgz not available in this build; making uncompressed svg", "" );
#endif
return( 0 );
}

/* ================================== */
/* ZLEVEL - set the compression level */

int
PLGS_zlevel( level )
int level;
{
if( level < 0 || level > 9 ) level = 9;
sprintf( svg_compressmode, "wb%d", level );
return( 0 );
}

/* ================================== */
/* FMT - return the output file type, either svg or svgz */

int
PLGS_fmt( tag )
char *tag;
{
if( svg_compress ) strcpy( tag, "svgz" );
else strcpy( tag, "svg" );
return( 0 );
}

/* ================================== */
/* define a rectangular region for click hyperlink to url */
/* use <a xlink:href...>, and associate it with an invisble rectangle */

int
PLGS_clickregion( url, label, targetstr, x1, y1, x2, y2 )
char *url, *label, *targetstr;
int x1, y1, x2, y2; /* these are in absolute space x100 */
{
int i;
double xx1, yy1, xx2, yy2, fabs();


xx1 = ( ( (double)(x1) / 100.0 ) * svg_pixs_inch ) + MARG_X;
yy1 = svg_y_size - ( ( (double)(y1) / 100.0 ) * svg_pixs_inch ) + MARG_Y;
xx2 = ( ( (double)(x2) / 100.0 ) * svg_pixs_inch ) + MARG_X;
yy2 = svg_y_size - ( ( (double)(y2) / 100.0 ) * svg_pixs_inch ) + MARG_Y;

/* <a xlink for click url link.. */
if( url[0] != '\0' ) {
	fprintf( svg_fp, "<a %s xlink:href=\"", targetstr );
	/* ampersands etc. must be encoded this way to survive svg rendering pass!! */
	for( i = 0; url[i] != '\0'; i++ ) {
	   if( url[i] == '&' ) fprintf( svg_fp, "&amp;" );
	   else if( url[i] == '<' ) fprintf( svg_fp, "&lt;" );
	   else if( url[i] == '>' ) fprintf( svg_fp, "&gt;" );
	   else fprintf( svg_fp, "%c", url[i] );
	   }
	fprintf( svg_fp, "\">\n" );
	}

/* rect */
fprintf( svg_fp, "<rect x=\"%.4g\" y=\"%.4g\" width=\"%.4g\" height=\"%.4g\" ", xx1, yy1, xx2-xx1, fabs(yy2-yy1) );

/* mouseover label ... */

/* <rect x="529.9" y="252" width="5.76" height="5.76" 
 *    onmouseover='DisplayInfo(evt, "14-MAY-2004.14:45: 349" )' 
 *    onmouseout='RemoveInfo ();' />
 */
if( label[0] != '\0' ) {
	if( !svg_generic_js ) fprintf( svg_fp, "onmouseover='DisplayInfo(evt, \"" ); /* for the echlin setup */
	for( i = 0; label[i] != '\0'; i++ ) {
		if( label[i] == '&' ) fprintf( svg_fp, "&amp;" );
		else if( label[i] == '<' ) fprintf( svg_fp, "&lt;" );
		else if( label[i] == '>' ) fprintf( svg_fp, "&gt;" );
		else if( label[i] == '"' && !svg_generic_js ) fprintf( svg_fp, "&quot;" ); /* added 6/21/04 scg */
		else if( label[i] == '\'' && !svg_generic_js ) fprintf( svg_fp, " " ); /* added 10/12/05 scg */
		else fprintf( svg_fp, "%c", label[i] );
		}
	if( !svg_generic_js ) fprintf( svg_fp, "\" )' onmouseout='RemoveInfo ();'" ); /* for the echlin setup */
	}

/* close rect */
fprintf( svg_fp, " />\n" );
if( url[0] != '\0' ) fprintf( svg_fp, "</a>\n" );
return( 0 );
}



/* ======================================== */
/* SETXMLPARMS - set various xml language related parameters.. */

int
PLGS_setxmlparms( parm, value )
char *parm, *value;
{

/* set the character encoding method to be indicated at the beginning of the SVG file */
if( strcmp( parm, "encoding" )==0 ) strcpy( svg_encoding, value );

/* tell svg driver whether or not to print the <?xml ... ?> declaration line as 1st line in output */
else if( strcmp( parm, "xmldecl" )==0 ) svg_xmldecl = value[0] - '0';

/* set misc parms to be present in the <svg> tag.. */
else if( strcmp( parm, "svgparms" )==0 ) strcpy( svg_tagparms, value );

/* set misc parms to be present in the <a> tag.. */
/* else if( strcmp( parm, "linkparms" )==0 ) strcpy( svg_linkparms, value ); */  /* discontinued - use the new [target=new] syntax in clickmapurl instead */

/* indicate that we should use generic mouseover javascript */
else if( strcmp( parm, "mouseover_js" )==0 ) {
	if( strcmp( value, "generic" )==0 ) svg_generic_js = 1;
	else svg_generic_js = 0;
	}

else Eerr( 27505, "unrecognized svg xmlparm received", parm );
return( 0 );
}

#endif  /* NOSVG */

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
