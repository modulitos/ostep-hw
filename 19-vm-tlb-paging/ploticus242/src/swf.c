/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* SWF Driver for Ploticus - Copyright 2003 Bill Traill (bill@traill.demon.co.uk).
 * Portions Copyright 2001, 2002 Stephen C. Grubb 
 * Covered by GPL; see the file ./Copyright for details. */

/*
   Checking for redundant calls is not done here; should be done by caller.
   special characters not delt with

	Jan03 bt	Created swf driver based on existing svg driver svg.c

	15 May 03 scg	a couple changes to make font loading more graceful when
			font dir not set or default font not available

	15 May 03 scg	disabled swf_LS call

	12 May 04 scg   This driver wasn't designed to handle multiple "jobs" per process..
			swf_movie probably needs to be released at end of each job, and Ming_init()
			only called first time (?)

*/
#ifndef NOSWF

/* #define MAX_D_ROWS 1000 */

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <ming.h>

extern int TDH_err(), GL_member(), PLG_xrgb_to_rgb(), GL_goodnum(), PLG_colorname_to_rgb();
extern int unlink(); /* return value not used */
int PLGF_linetype(), PLGF_chardir();

#define Eerr(a,b,c)  TDH_err(a,b,c) 
#define stricmp(a,b) strcasecmp(a,b) 

extern double atof();

#define swf_scale_x(x)   ((x * swf_pixs_inch ) + MARG_X - swf_offset_x) 
#define swf_scale_y(y)   (swf_y_size - (( y * swf_pixs_inch ) + MARG_Y - swf_offset_y)) 

#define MARG_X 0 
#define MARG_Y 0 

char *getenv();

static int	swf_stdout;		/* 1 if swf_fp is stdout */
static FILE	*swf_fp;

static double	swf_x_size;		/* width of the drawing area */
static double	swf_y_size;		/* height of the drawing area */
static int	swf_path_in_prog =0;	/* flag to indicate if an swf path is in progress */

static int	swf_cur_color_r = 0, swf_cur_color_g = 0, swf_cur_color_b = 0;

static SWFFont	swf_font;
/* changed, scg 5/4/04...
 * static double	path_x[MAX_D_ROWS];
 * static double	path_y[MAX_D_ROWS];
 */
static double *path_x = NULL, *path_y = NULL;
static int max_pts;		

static int	path_count;

static char	swf_dash_style[1024];
static double	swf_line_width=1;

static char	swf_font_name[100] = "";	/* current font name */
static int	swf_chdir;	 		/* char direction in degrees */
static int	swf_currpointsz;		/* current char size in points */
static char	swf_font_weight[100];
static char	swf_font_style[100];

static int	swf_pixs_inch; 	   		/* number of pixels per inch - scg */
static char	swf_filename[256] = ""; 	/* output file name (was local) - scg */

static char	swf_style[1024] = "";	/* line,font styles etc */

static double	swf_offset_x;
static double	swf_offset_y;

static char	swf_tmpfilename[256] = "/unnamed";
static int	swf_tmpfile_used = 0;

static int	swf_clickmap = 0;
static int	swf_debug = 0;

static SWFMovie swf_movie;

static void swf_MV(char *s);
static void swf_LL(char *s);
static void swf_LE(char *s);
static void swf_CO(char *s);
static void swf_PL(char *s);
static void swf_PE(char *s);
static void swf_TX(char *s);
/* static void swf_LS(char *s); */
static void swf_PO(char *s);
static int swf_FT(char *s); /* was: static void swf_FT(char *s); scg 7/28/03 */
static void swf_TR(char *s);

/* ============================= */
int
PLGF_initstatic()
{
swf_path_in_prog =0;	
/* swf_cur_color_r = 256; swf_cur_color_g = 256; swf_cur_color_b = 256; */
swf_cur_color_r = 0; swf_cur_color_g = 0; swf_cur_color_b = 0;
swf_line_width = 1;
strcpy( swf_font_name, "" );	
strcpy( swf_filename,"" ); 	
strcpy( swf_style, "" );
strcpy( swf_tmpfilename, "/unnamed" );
swf_tmpfile_used = 0;
swf_clickmap = 0;
swf_debug = 0;

return( 0 );
}

/* ============================================================================ */
/* SETPARMS - allow caller to pass required parms that swf driver needs - MUST be called before setup() */
/* ============================================================================ */
int
PLGF_setparms( debug, tmpname, font )
int debug;
char *tmpname;
char *font;	/* user-selected font, or "" */
/* int clickmap; */
{
swf_debug = debug;
sprintf( swf_tmpfilename, "%s_V", tmpname );

/* get an early idea of default font.. allows -font to control first load.. scg 5/15/03 */
/* this will be /Helvetica by default.. */
strcpy( swf_font_name, font );

/* swf_clickmap = clickmap; */    /* clickmap not supported */
return( 0 );
}


/* ============================================================================ */
/* SETUP */
/* ============================================================================ */
int
PLGF_setup( name, dev, outfile, pixs_inch, Ux, Uy, Upleftx, Uplefty, maxdrivervect )
char *name; /* arbitrary name */
char dev;  /* 'p' = monochrome   'c' = color   'e' = eps */
char *outfile;  /* file to put code in */
int pixs_inch;
double Ux;
double Uy;
int Upleftx;
int Uplefty;
int maxdrivervect;
{  

/* set globals */
if( dev != 'f' ) dev = 'f';
strcpy( swf_font_weight, "" );
strcpy( swf_font_style, "" );
swf_chdir = 0;
swf_currpointsz = 10;

swf_pixs_inch = pixs_inch; /* scg */
swf_path_in_prog =0;		
swf_line_width=1;
strcpy( swf_style, "" );
swf_tmpfile_used = 0;

max_pts = maxdrivervect;
if( path_x != NULL ) free( path_x );
if( path_y != NULL ) free( path_y );
path_x = (double *) malloc( max_pts * sizeof( double ));
path_y = (double *) malloc( max_pts * sizeof( double ));

/* determine if we need to write to tmp file, and open appropriate file for write.. */
swf_stdout = 0;
if( stricmp( outfile, "stdout" )==0 || outfile[0] == '\0' ) swf_stdout = 1;
else strcpy( swf_filename, outfile );

swf_x_size = Ux * pixs_inch;
swf_y_size = Uy * pixs_inch;
strcpy(swf_style,"");

swf_fp = fopen( swf_tmpfilename, "w" ); 
/* swf_fp = fopen( "temp_dump.txt", "w" );  */
fprintf(swf_fp,"FT:%s :%s :%s \n",swf_font_name,swf_font_weight,swf_font_style);
fprintf(swf_fp,"PO:%d\n",swf_currpointsz);
PLGF_linetype( "0", 1.0, 1.0);
PLGF_chardir( swf_chdir );
return( 0 );
}

/* ============================================================================ */
/* FMT - return the output file type  */
/* ============================================================================ */
int
PLGF_fmt( tag )
char *tag;
{
strcpy( tag, "swf" );
return( 0 );
}

/* ============================================================================ */
/* MOVETO */
/* ============================================================================ */
int
PLGF_moveto( x, y )
double x, y;
{
if (!swf_path_in_prog) { fprintf(swf_fp,"MV:%lf %lf\n",x,y); } 
return( 0 );
}


/* ============================================================================ */
/* LINETO */
/* ============================================================================ */
int
PLGF_lineto( x, y )
double x, y;
{
if (!swf_path_in_prog) { swf_path_in_prog =1; } 
fprintf(swf_fp,"LL:%lf %lf\n",x,y); 
return( 0 );
}


/* ============================================================================ */
/* STROKE - render a stroke (line) */
/* ============================================================================ */
int
PLGF_stroke( )
{
if (swf_path_in_prog) { fprintf(swf_fp,"LE:\n"); }
swf_path_in_prog = 0;
return( 0 );
}


/* ============================================================================ */
/* PATH - add an element to a path (either new or existing) */
/* ============================================================================ */
int
PLGF_path( x, y )
double x, y;
{
if (!swf_path_in_prog) { swf_path_in_prog =1; }
fprintf(swf_fp,"PL:%lf %lf\n",x,y); 
return( 0 );
}

/* ============================================================================ */
/* FILL - fill current path with current color */
/* ============================================================================ */
int
PLGF_fill( )
{
if (swf_path_in_prog) { fprintf(swf_fp,"PE:\n"); }
swf_path_in_prog = 0;
return( 0 );
}

/* ============================================================================ */
/* COLOR - set current color for text and lines */
/* ============================================================================ */
int
PLGF_color( color )
char *color;
{
int i, n;
double r, g, b, PLG_rgb_to_gray();
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
	}
else if( strncmp( color, "gray", 4 )==0 || strncmp( color, "grey", 4 )==0 ) {
	n = sscanf( color, "%*s %lf", &r );
	if( n != 1 ) { Eerr( 12031, "Invalid color", color ); return(1); }
	g = b = r;
	}
else if( strncmp( color, "xrgb", 4 )==0 ) {
	if (PLG_xrgb_to_rgb( color+5, &r, &g, &b)) return(1);
	}
else if( color[0] == 'x' ) {  /* added scg 5/31/07 */
        if (PLG_xrgb_to_rgb( &color[1], &r, &g, &b)) return(1);
        }
else if( GL_goodnum( color, &i ) ) {
	r = atof( color );
	g = b = r;
	}
else PLG_colorname_to_rgb( color, &r, &g, &b );

swf_cur_color_r = r * 255;
swf_cur_color_g = g * 255;
swf_cur_color_b = b * 255;

fprintf(swf_fp,"CO:%d:%d:%d\n",swf_cur_color_r,swf_cur_color_g,swf_cur_color_b);
return( 0 );
}




/* ============================================================================ */
/* TEXT - render some text */
/* ============================================================================ */
int
PLGF_text( com, x, y, s, w )
char com;
double x, y;
char *s;
double w;
{
fprintf(swf_fp,"TX:%lf %lf %lf %c:%s\n",x,y,w,com, s);
return( 0 );
}


/* ============================================================================ */
/* POINTSIZE - set text point size */
/* ============================================================================ */
int
PLGF_pointsize( p )
int p;
{
swf_currpointsz = p;
fprintf(swf_fp,"PO:%d\n",swf_currpointsz);
return( 0 );
}


/* ============================================================================ */
/* FONT - set font */
/* ============================================================================ */
int
PLGF_font( f )
char *f;
{
if (f[0] == '/') strcpy( swf_font_name, ++f );
else strcpy( swf_font_name, f );
fprintf(swf_fp,"FT:%s :%s :%s \n",swf_font_name,swf_font_weight,swf_font_style);
return( 0 );
}

/* ============================================================================ */
/* CHARDIR - set text direction */
/* ============================================================================ */
int
PLGF_chardir( t )
int t;
{
fprintf(swf_fp,"TR:%d\n",t);
return( 0 );
}


/* ============================================================================ */
/* LINETYPE - set line style */
/* ============================================================================ */
int
PLGF_linetype( s, x, y )
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

strcpy(swf_dash_style,"");
swf_line_width = x ;

if(  s[0] == '\0' || strcmp( s, "0" )==0 ) strcpy(swf_dash_style,"");
else 	{
	char *p = swf_dash_style;

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

fprintf(swf_fp,"LS:%lf:%s\n",swf_line_width, swf_dash_style);
return( 0 );
}
	

/* ============================================================================ */
/* write a char to stdout */
/* ============================================================================ */
static void putC(byte b, void *data) {
	putchar(b);
}

/* ============================================================================ */
/* TRAILER do end of file stuff */
/* ============================================================================ */
int
PLGF_trailer( x1, y1, x2, y2 )
double x1, y1, x2, y2;
{
char *buf;
int len;
char  ptype[5];
char  *ptype_s;

/* Close the temp file we have been writing to */
fclose(swf_fp);

/* clickmap not yet supported */
/* if( swf_clickmap ) {
 *	PL_clickmap_out( 0, 0 );
 * }
 */



buf = swf_style; /* reuse */
swf_fp = fopen( swf_tmpfilename, "r" ); 
if( swf_fp == NULL ) return( Eerr( 75205, "cannot reopen tmpfile", swf_tmpfilename ) );


/* swf_fp = fopen( "temp_dump.txt", "r" );  */

/* initialise the ming library */
Ming_init();
swf_movie =  newSWFMovie();


/* Calculate the size of the drawing area and
   set the size of the swf_movie */
swf_x_size = (x2-x1) *swf_pixs_inch;
swf_y_size = (y2-y1) *swf_pixs_inch;
SWFMovie_setDimension(swf_movie, swf_x_size, swf_y_size); 


/* work out the x and y offset into the drawing area */
swf_offset_x = x1 *swf_pixs_inch;
swf_offset_y = y1 *swf_pixs_inch;

swf_cur_color_r = 0; swf_cur_color_g = 0; swf_cur_color_b = 0; /* bug - axes sometimes same color as plot content..
								  but this didn't fix it.. scg 1/19/05 */

while (fgets(buf,999,swf_fp)) {
/* fprintf( stderr, "---->%s", buf ); */
	sscanf(buf,"%2s",ptype);
	ptype_s = buf + 3;

	/* Strip the newline off the end */
	len = strlen(ptype_s);
	ptype_s[len-1] = '\0';

	if (!strcmp(ptype,"FT")) {
		if( swf_FT(ptype_s) != 0 ) return( 1 );  /* allow return of err code for font load error.. scg */

	} else if (!strcmp(ptype,"MV")) {
		swf_MV(ptype_s); 
	} else if (!strcmp(ptype,"LL")) {
		swf_LL(ptype_s); 
	} else if (!strcmp(ptype,"LE")) {
		swf_LE(ptype_s); 
	} else if (!strcmp(ptype,"CO")) {
		swf_CO(ptype_s); 
	} else if (!strcmp(ptype,"PL")) {
		swf_PL(ptype_s); 
	} else if (!strcmp(ptype,"PE")) {
		swf_PE(ptype_s); 
	} else if (!strcmp(ptype,"TX")) {
		swf_TX(ptype_s); 
	} else if (!strcmp(ptype,"LS")) {
		/* swf_LS(ptype_s); */ /* scg 5/15/03 commented out - causes seg fault on solaris */ ;

	} else if (!strcmp(ptype,"PO")) {
		swf_PO(ptype_s); 
	} else if (!strcmp(ptype,"TR")) {
		swf_TR(ptype_s); 
	}
}


if (swf_stdout) {
	SWFByteOutputMethod  put_char;
	put_char = putC;
	SWFMovie_output(swf_movie,put_char,0); 
	/* for ming 0.3a+ use SWFMovie_output( swf_movie );  ??? */
	}
else SWFMovie_save(swf_movie,swf_filename);

fclose( swf_fp );

unlink( swf_tmpfilename ); 

return( 0 );
}


/* ================================= */
/* FONTNAME - given a base name (such as Helvetica) and a modifier
   such as I (italic) B (bold) or BI (bold italic), build the 
	font style and weight strings */

int
PLGF_fontname( basename, name )
char *basename; 
char *name; /* in: B, I, or BI.  out: still the font name but statics now hold the font style and weight */
{
	int i, slen;

	for( i = 0, slen = strlen( name ); i < slen; i++ ) name[i] = tolower( name[i] );

	strcpy (swf_font_weight, "");
	strcpy (swf_font_style, "");

	if( strcmp( name, "b" )==0 ) strcpy (swf_font_weight, "bold");
	else if( strcmp( name, "i" )==0 ) strcpy (swf_font_style, "italic");
	else if( strcmp( name, "bi" )==0 ) { strcpy (swf_font_weight, "bold"); strcpy (swf_font_style, "italic"); }

	if (basename[0] == '/')  strcpy( name, basename++ );
	else strcpy( name, basename );

	return( 0 );
}



/* ============================================================================ */
/* Line MOVETO */
/* ============================================================================ */
static void swf_MV( s )
char *s;
{
	double x, y;

	sscanf(s,"%lf %lf",&x,&y);

	path_count = 0;
	path_x[path_count] = x;
	path_y[path_count] = y;
	path_count++ ;

	swf_path_in_prog =1;

	return;
}

/* ============================================================================ */
/* LINETO */
/* ============================================================================ */
static void swf_LL(s)
char *s;
{
	double x,y;

	sscanf(s,"%lf %lf",&x,&y);

	if (!swf_path_in_prog) swf_path_in_prog =1;

	/* if( (path_count-2) > max_pts ) return; */ /* scg 5/4/04 */
	if (path_count >= max_pts ) return; /* scg 5/4/04 and crt 10-Jun-06 */
 
	path_x[path_count] = x;
	path_y[path_count] = y;
	path_count++;

	return;
}

/* ============================================================================ */
/* STROKE - render a stroke (line) */
/* ============================================================================ */
static void swf_LE(s)
char *s;
{
	int i;
	SWFShape swf_shape;

	swf_shape = newSWFShape();
	SWFShape_setLine(swf_shape, swf_line_width,  swf_cur_color_r,swf_cur_color_g,swf_cur_color_b,0xff); 
	/* SWFShape_setLine(swf_shape,1,  swf_cur_color_r,swf_cur_color_g,swf_cur_color_b,0xff);  */
	SWFShape_movePenTo(swf_shape,swf_scale_x(path_x[0]),swf_scale_y(path_y[0])); 

	for (i=1;i < path_count;i++) {
		SWFShape_drawLineTo(swf_shape, swf_scale_x(path_x[i]),swf_scale_y(path_y[i]));
	}
	SWFMovie_add(swf_movie,swf_shape);

	swf_path_in_prog = 0;
	return;
}

/* ============================================================================ */
/* swf_PL - path line segment */
/* ============================================================================ */
static void swf_PL(s)
char *s;
{
	double x,y;

	sscanf(s,"%lf %lf",&x,&y);

	if (!swf_path_in_prog) {
		swf_path_in_prog =1;
	} 

	path_x[path_count] = x;
	path_y[path_count] = y;
	path_count++;

	return;
}


/* ============================================================================ */
/* swf_PE -  Path end - fill current path with current color */
/* ============================================================================ */
static void swf_PE(s)
char *s;
{
	int i;
	SWFFill swf_fill;
	SWFShape swf_shape;

	if (swf_path_in_prog) {

		swf_shape = newSWFShape();
		swf_fill=SWFShape_addSolidFill(swf_shape,swf_cur_color_r,swf_cur_color_g,swf_cur_color_b,0xff);
		SWFShape_setRightFill(swf_shape, swf_fill);

		SWFShape_movePenTo(swf_shape,swf_scale_x(path_x[0]),swf_scale_y(path_y[0])); 
		for (i=1;i < path_count;i++) {
			SWFShape_drawLineTo(swf_shape,swf_scale_x(path_x[i]),swf_scale_y(path_y[i])); 
		}
		SWFShape_drawLineTo(swf_shape,swf_scale_x(path_x[0]),swf_scale_y(path_y[0])); 
		SWFMovie_add(swf_movie,swf_shape);
	}
	swf_path_in_prog = 0;
	return;
}


/* ============================================================================ */
/* Colour - set the current colour */
/* ============================================================================ */
static void swf_CO(s)
char *s;
{
	sscanf(s,"%d:%d:%d",&swf_cur_color_r,&swf_cur_color_g,&swf_cur_color_b);
	/* printf("Colour %d,%d,%d\n",swf_cur_color_r,swf_cur_color_g,swf_cur_color_b); */
	return;
}

/* ============================================================================ */
/* TEXT - render some text */
/* ============================================================================ */
static void swf_TX(s)
char *s;
{
	float width;
	char com;
	double x, y,w;
	int no_chars;
	char *str;
	SWFText swf_text;
	SWFDisplayItem swf_item;

fprintf(swf_fp,"TX:%lf %lf %lf %c:%s\n",x,y,w,com, s);

	sscanf(s,"%lf %lf %lf %c:%n\n",&x,&y,&w,&com,&no_chars);
	str = s + no_chars;

/* printf("TX align:%c text:%s\n",com,str);   */

	x = swf_scale_x(x);
	y = swf_scale_y(y);



        swf_text =  newSWFText();
        SWFText_moveTo(swf_text,0,0);
        SWFText_setFont(swf_text, swf_font);
        SWFText_setHeight(swf_text, swf_currpointsz);
        SWFText_setColor(swf_text, swf_cur_color_r,swf_cur_color_g,swf_cur_color_b, 1);


        SWFText_addString(swf_text,str,0);
        swf_item = SWFMovie_add(swf_movie,swf_text);

        /* SWFDisplayItem_rotateTo(swf_item, - swf_chdir); */
        SWFDisplayItem_rotateTo(swf_item, swf_chdir);

        /* set the text alignment */
        width = SWFText_getStringWidth(swf_text, str);

        if (com == 'T') {}
        else if (com == 'C') {
                x = x - (cos((swf_chdir * 3.141592653)/180.0)*(width/2));
                y = y + (sin((swf_chdir * 3.141592653)/180.0)*(width/2)); 
        }
        else if (com == 'J') {
                x = x - (cos((swf_chdir * 3.141592653)/180.0)*(width));
                y = y + (sin((swf_chdir * 3.141592653)/180.0)*(width)); 
        }

        SWFDisplayItem_moveTo(swf_item, x, y);

	return;
}

/* 
PI = 3.141592653589793238462643;
DegToRad = Degrees * 3.1416 / 180   
RadToDeg = Radians * 180 / 3.1416   
*/

/* ============================================================================ */
/* POINTSIZE - set text point size */
/* ============================================================================ */
static void swf_PO(s)
char *s;
{
	sscanf(s,"%d\n",&swf_currpointsz);
/* printf ("PO:%d\n",swf_currpointsz); */
	return;
}

/* ============================================================================ */
/* Line style */
/* ============================================================================ */
#ifdef CUT
static void swf_LS(s)
char *s;
{
/* X = line width;  Y = dash pattern magnification (0.1 to 10)
 *  S indicates dash pattern.  If S is "0", an unbroken (normal) line is produced.
 *  If S is "1" through "8", a preset dash pattern is used.  Otherwise, S is
 *  assumed to hold the dash pattern string "[ n1 n2 n3.. ]".	
 */
	int no_chars;

	sscanf( s, "%lf:",&swf_line_width, &no_chars);
	s = s + no_chars;

	strcpy(swf_dash_style,s);
/* printf("Width %lf Style:%s\n",swf_line_width,swf_dash_style); */
	return;
}
#endif
/* ============================================================================ */
/* Font */
/* ============================================================================ */
static int swf_FT(s)
/* was static void swf_FT(s) .. changed scg 7/28/03 */
char *s;
{

	FILE *font;
	char filename[512];
	char *swf_fonts_dir;

	sscanf(s,"%s :%s :%s \n",swf_font_name,swf_font_weight,swf_font_style);


        swf_fonts_dir = getenv( "SWF_FONTS_DIR");

        if( swf_fonts_dir == NULL ) { 
		Eerr( 13001, "SWF_FONTS_DIR environment variable not found", "" );
		sprintf( filename, "%s.fdb", swf_font_name);
	} else {
		sprintf( filename, "%s/%s.fdb", swf_fonts_dir, swf_font_name);
	}

	font = fopen(filename,"r");
	if (font == NULL) {
		Eerr( 13001, "Unable to open font file ", filename );
		if( swf_fonts_dir == NULL ) return(1);	/* don't try to build path.. abort .. scg 5/15/03 */
		sprintf( filename, "%s/Arial.fdb", swf_fonts_dir); /* 2nd try */
		font = fopen(filename,"r");
		if (font == NULL) {
			Eerr( 13002, "Unable to load default font", filename );
			return( 1 );
		} else {
			Eerr( 13003, "Using default font test.fdb", "" );
		}
	}
	swf_font = loadSWFFontFromFile(font);
	fclose (font);
	return( 0 );
}

/* ============================================================================ */
/* Test Rotation */
/* ============================================================================ */
static void swf_TR(s)
char *s;
{
	sscanf(s,"%d\n",&swf_chdir);
	return;
}



#endif /* NOSWF */

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
