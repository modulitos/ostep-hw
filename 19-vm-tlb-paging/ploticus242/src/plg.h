/* 
 * Graphics notes:
 *   - Origin is in lower left corner of "paper", regardless of orientation of paper.
 *   - Format of i-code will be: "a x y s\n", where a is an op code, x and y
 *     are coordinates in inches, and s is a variable length string (may be null).
 */

#ifndef PLGHEAD
  #define PLGHEAD 1

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#define YES 1
#define NO 0
#define MAXPATH 256
#define FONTLEN 60
#define COLORLEN 40

#define E_LINEAR 0
#define E_LOG 1
#define E_LOGPLUS1 2
#define E_RADIAL 3

#ifdef LOCALE
 #define stricmp( s, t )         stricoll( s, t )
 #define strnicmp( s, t, n )     strnicoll( s, t, n )
 extern int stricoll(), strnicoll();   /* added scg 5/31/06 gcc4 */
#else
 #define stricmp( s, t )         strcasecmp( s, t )
 #define strnicmp( s, t, n )     strncasecmp( s, t, n )
#endif

/* output devices (Edev) */
#define X11 'x'
#define GD  'g'
#define SVG 's'
#define POSTSCRIPT 'p'
#define SWF 'f'


struct plgc {
	/* overall settings */
	char standard_font[FONTLEN] ;
	int standard_textsize;
	double standard_lwscale;
	char standard_color[COLORLEN];
	char standard_bkcolor[COLORLEN];
	/* window size.. */
	double winx, winy;
	double winx_0, winy_0;
	
	/* graphics parameters.. */
	char dev;
	int  pixelsinch;
	char standardfont[FONTLEN];
	char curfont[FONTLEN];
	int curtextsize;
	int standardtextsize;
	double curtextheight;
	double curtextwidth;
	int curtextdirection;
	int curpaper;
	double standardlinewidth;
	double curlinewidth;
	int curlinetype;
	double curpatternfactor;
	int curpen; 
	char curcolor[COLORLEN];
	char curbkcolor[COLORLEN];
	char nextcolor[COLORLEN]; 
	char sparecolor[COLORLEN];  
	
	/* event information */
	int event;
	double eventx, eventy;
	
	/* scaling.. */
	double xlo, xhi, ylo, yhi;		/* graphic area bounds, absolute coords */
	double dxlo, dxhi, dylo, dyhi;		/* graphic area bounds, data coords */
	double scale_x, scale_y; 		/* linear scaling factors in x and y */
	int scaletype_x;			/* either LINEAR or LOG */
	int scaletype_y;			/* either LINEAR or LOG */
	
	/* last moveto and lineto */
	double x1, y1, x2, y2;
	char progname[FONTLEN];
	int flip;
	int blacklines;
	long flashdelay;
	FILE *errfp;
	};

/* ========== global vars ================= */
extern struct plgc PLG;




/* ========== function mappings - map E functions (used in most code) to PLG_ names  ================ */
#define Emovu( x , y )		PLG_pcode( 'M', Eax((double) x ) , Eay((double) y ), "" )
#define Elinu( x , y )		PLG_pcode( 'L', Eax((double) x ) , Eay((double) y ), "" )
#define Epathu( x , y )		PLG_pcode( 'P', Eax((double) x ) , Eay((double) y ), "" ) 
/* #define Eclosepath()		PLG_pcode( 'c', 0.0, 0.0, "" ) */
#define Efill( )		PLG_pcode( 's', 0.0, 0.0, "" )
#define Etext( s )		PLG_dotext( s, 'T' )
#define Ecentext( s )		PLG_dotext( s, 'C' )
#define Erightjust( s )		PLG_dotext( s, 'J' )
#define Esavewin( )		PLG_pcode( 'b', 0.0, 0.0, "" );
#define PLG_savewin( )		PLG_pcode( 'b', 0.0, 0.0, "" );
#define Erestorewin( )		PLG_pcode( 'B', 0.0, 0.0, "" );
#define PLG_restorewin( )	PLG_pcode( 'B', 0.0, 0.0, "" );
#define Escaletext( x )		PLG_pcode( 'e', x, 0.0, "" )
#define Eprint()		PLG_getclick()
#define Eshow()			PLG_pcode( 'Z', 0.0, 0.0, "" )
#define Esit()		 	PLG_pcode( 'W', 0.0, 0.0, "" )
#define Easync()	 	PLG_pcode( 'w', 0.0, 0.0, "" )
#define Eflush()		PLG_pcode( 'U', 0.0, 0.0, "" )
#define PLG_flush()		PLG_pcode( 'U', 0.0, 0.0, "" )
#define Ewinappear()		PLG_pcode( 'a', 0.0, 0.0, "" )
#define PLG_winappear()		PLG_pcode( 'a', 0.0, 0.0, "" )
#define Ewindisappear()		PLG_pcode( 'd', 0.0, 0.0, "" )
#define Eendoffile()		PLG_pcode( 'Q', 0.0, 0.0, "" )
#define Esquelch( s )		PLG_pcode( 'H', 0.0, 0.0, s )
#define PLG_squelch( s )	PLG_pcode( 'H', 0.0, 0.0, s )
#define PLG_forcecolorchg( )	PLG_pcode( 'v', 0.0, 0.0, "" )
#define Eerr( n, s, p )		TDH_err( n, s, p )
#define Epixpt( x, y, s )	PLG_pcode( '.', x, y, s )           /* direct pixel data point 5/25/06 */


#define Earrow( x1, y1, x2, y2, r, w, color )		PLG_arrow( x1, y1, x2, y2, r, w, color )
#define Ecblock( xlo, ylo, xhi, yhi, color, outline )	PLG_cblock( xlo, ylo, xhi, yhi, color, outline )
#define Ecblockdress( x1, y1, x2, y2, bs, lbc, hbc, ss, sc ) PLG_cblockdress( x1, y1, x2, y2, bs, lbc, hbc, ss, sc ) 
#define Esetlastbox( x1, y1, x2, y2 )			PLG_setlastbox( x1, y1, x2, y2 )
#define Egetlastbox( x1, y1, x2, y2 )			PLG_getlastbox( x1, y1, x2, y2 )
#define Escaletype( typ, axis )				PLG_scaletype( typ, axis )
#define Escale_x( xlo, xhi, datalow, datahi )		PLG_scale_x( xlo, xhi, datalow, datahi )
#define Escale_y( ylo, yhi, datalow, datahi )		PLG_scale_y( ylo, yhi, datalow, datahi )
#define	Ea( xory, d )					PLG_a( xory, d )
#define Eax( d )					PLG_ax( d )
#define Eay( d )					PLG_ay( d )
#define Edx( a )					PLG_dx( a )
#define Edy( a )					PLG_dy( a )
#define Elimit( axis, end, units )			PLG_limit( axis, end, units )
#define Einit( dev )					PLG_init( dev )
#define Eset_early_defaults()				PLG_set_early_defaults()
#define Esetsize( ux, uy, upleftx, uplefty )		PLG_setsize( ux, uy, upleftx, uplefty )
#define Esetdefaults()					PLG_setdefaults()
#define Esetoutfilename( name )				PLG_setoutfilename( name )
#define Egetoutfilename( name )				PLG_getoutfilename( name )
#define Esetoutlabel( name )				PLG_setoutlabel( name )
#define Ehandle_events( x, y, e )			PLG_handle_events( x, y, e )
#define Egetkey( x, y, e )				PLG_getkey( x, y, e )
#define Egetclick()					PLG_getclick()
#define Ehe( x, y, e )					PLG_he( x, y, e )
#define E_savekey( lx, ly, x )				PLG_savekey( lx, ly, c )
#define Eretreivekey( lx, ly, c )			PLG_retrievekey( lx, ly, c )
#define Esetsemfile( s )				PLG_setsemfile( s )
#define Esemfile()					PLG_semfile()
#define Emark( x, y, code, r )				PLG_mark( x, y, code, r )
#define Epcode( op, x, y, s )				PLG_pcode( op, x, y, s )
#define Ebb( x, y )					PLG_bb( x, y )
#define Eresetbb()					PLG_resetbb()
#define Egetbb( xlo, ylo, xhi, yhi )			PLG_getbb( xlo, ylo, xhi, yhi )
#define Egettextsize( w, h )				PLG_gettextsize( w, h )
#define Everttextsim( op, s )				PLG_verttextsim( op, s )
#define Etightbb( mode )				PLG_tightbb( mode )
#define Especifycrop( mode, x1, y1, x2, y2 )		PLG_specifycrop( mode, x1, y1, x2, y2 )
#define Egifrect( xlo, yhi, xhi, ylo, color )		PLG_gifrect( xlo, yhi, xhi, ylo, color )
#define Eimload( filename, scalex, scaley )		PLG_imload( filename, scalex, scaley )
#define Eimplace( x, y, filename, imalign, xscale, yscale )	PLG_implace( x, y, filename, imalign, xscale, yscale )
#define Esetglobalscale( sx, sy )			PLG_setglobalscale( sx, sy )
#define Egetglobalscale( sx, sy )			PLG_getglobalscale( sx, sy )
#define Esetposterofs( x, y )				PLG_setposterofs( x, y )
#define Epcodedebug( mode, fp )				PLG_pcodedebug( mode, fp )
#define Ecolorname_to_rgb( color, r, g, b )		PLG_colorname_to_rgb( color, r, g, b )
#define Ergb_to_gray( r, g, b )				PLG_rgb_to_gray( r, g, b )
#define Eicolor( i )					PLG_icolor( i )
#define Eclr()						PLG_clr()
#define Emov( x, y )					PLG_mov( x , y )
#define Elin( x, y )					PLG_lin( x , y )
#define Epath( x, y )					PLG_path( x, y )
#define Edotext( s, op )				PLG_dotext( s, op )
#define Efont( s )					PLG_font( s )      
#define Etextsize( x ) 					PLG_textsize( x )          
#define Etextdir( x )					PLG_textdir( x )
#define Epaper( x )					PLG_paper( x )
#define Elinetype( pat, wid, dens )			PLG_linetype( pat, wid, dens )    
#define Enormline()					PLG_normline()             
#define Ecolor( s )					PLG_color( s )
#define Ebackcolor( s )					PLG_backcolor( s )
#define Ecolorfill( c )					PLG_colorfill( c )
/* #define Eshade( s )					PLG_shade( s ) */
#define Esetwinscale( width, height, x_max, y_max )	PLG_setwinscale( width, height, x_max, y_max )
#define Exsca( f )					PLG_xsca( f )
#define Exsca_inv( i )					PLG_xsca_inv( i )
#define E_ysca( f )					PLG_ysca( f )
#define E_ysca_inv( i )					PLG_ysca_inv( i )
#define Elineclip( x1, y1, x2, y2, rx1, ry1, rx2, ry2)	PLG_lineclip( x1, y1, x2, y2, rx1, ry1, rx2, ry2 )


/* =========== global vars mappings - map the 'E' name to the PLG member name ======= */
#define Estandard_font		PLG.standard_font
#define Estandard_textsize 	PLG.standard_textsize
#define Estandard_lwscale	PLG.standard_lwscale
#define Estandard_color		PLG.standard_color
#define Estandard_bkcolor	PLG.standard_bkcolor
#define EWinx			PLG.winx
#define EWiny			PLG.winy
#define EWinx_0			PLG.winx_0
#define EWiny_0			PLG.winy_0
#define Edev			PLG.dev
#define Epixelsinch		PLG.pixelsinch
#define Estandardfont		PLG.standardfont
#define Ecurfont		PLG.curfont
#define Ecurtextsize		PLG.curtextsize
#define Estandardtextsize	PLG.standardtextsize
#define Ecurtextheight		PLG.curtextheight
#define Ecurtextwidth		PLG.curtextwidth
#define Ecurtextdirection	PLG.curtextdirection
#define Ecurpaper		PLG.curpaper
#define Estandardlinewidth	PLG.standardlinewidth
#define Ecurlinewidth		PLG.curlinewidth
#define Ecurlinetype		PLG.curlinetype
#define Ecurpatternfactor	PLG.curpatternfactor
#define Ecurpen			PLG.curpen
#define Ecurcolor		PLG.curcolor
#define Ecurbkcolor		PLG.curbkcolor
#define Enextcolor		PLG.nextcolor
#define EEvent			PLG.event
#define EEventx			PLG.eventx
#define EEventy			PLG.eventy
#define EXlo			PLG.xlo
#define EXhi 			PLG.xhi
#define EYlo			PLG.ylo
#define EYhi			PLG.yhi
#define EDXlo			PLG.dxlo
#define EDXhi 			PLG.dxhi
#define EDYlo 			PLG.dylo
#define EDYhi			PLG.dyhi
#define EScale_x 		PLG.scale_x
#define EScale_y		PLG.scale_y
#define Escaletype_x		PLG.scaletype_x
#define Escaletype_y		PLG.scaletype_y
#define Ex1			PLG.x1
#define Ey1			PLG.y1
#define Ex2 			PLG.x2
#define Ey2			PLG.y2
#define Eprogname		PLG.progname
#define Eflip			PLG.flip
#define Eblacklines		PLG.blacklines
#define Eflashdelay		PLG.flashdelay
#define Errfp			PLG.errfp

/* ===========  more defines ================ */
/* mouse buttons */
#define E_MOUSE_LEFT 1001
#define E_MOUSE_MIDDLE 1002
#define E_MOUSE_RIGHT 1003

/* events */
#define E_EXPOSE 1010  /* window has been exposed, unable to restore contents */ 
#define E_RESIZE 1011  /* window has been resized */
#define E_MESSAGE 2000
#define E_COLORMAP_FULL 1100 /* no more colors, drop back to simple defaults */
#define E_MONODISPLAY 1101  /* monochrome display alert */

/* arrow keys */
#define E_LEFT 551
#define E_UP 552
#define E_RIGHT 553
#define E_DOWN 554
#define E_PAGEUP 555
#define E_PAGEDOWN 556

#define E_WAITFOR_WM 200000	/* in certain situations such as after remapping the window or after 
				resizing, a delay seems to be necessary before the window manager 
				responds to subsequent instructions.. This is in microseconds.  */

/* ========== non-int function defines ================= */
double atof(), sqrt(), log();
extern double PLG_a(), PLG_ax(), PLG_ay(), PLG_dx(), PLG_dy(), PLG_limit(), PLG_conv(), PLG_u();
extern double  PLG_xsca_inv(), PLG_ysca_inv();
extern char *PLG_icolor();
extern char *GL_getok(), *GL_autoround(), *GL_autoroundf(); 


/* ========== int function declares =================== */
extern int PLG_pcode();
extern int PLG_arrow();
extern int PLG_cblock();
extern int PLG_cblockdress();
extern int PLG_setlastbox();
extern int PLG_getlastbox();
extern int PLG_scaletype();
extern int PLG_scale_x();
extern int PLG_scale_y();
extern int PLG_init();
extern int PLG_set_early_defaults();
extern int PLG_setsize();
extern int PLG_setdefaults();
extern int PLG_setoutfilename();
extern int PLG_getoutfilename();
extern int PLG_setoutlabel();
extern int PLG_handle_events();
extern int PLG_getkey();
extern int PLG_getclick();
extern int PLG_he();
extern int PLG_savekey();
extern int PLG_retrievekey();
extern int PLG_setsemfile();
extern int PLG_semfile();
extern int PLG_mark();
extern int PLG_pcode();
extern int PLG_bb();
extern int PLG_resetbb();
extern int PLG_getbb();
extern int PLG_gettextsize();
extern int PLG_verttextsim();
extern int PLG_tightbb();
extern int PLG_specifycrop();
extern int PLG_gifrect();
extern int PLG_imload();
extern int PLG_implace();
extern int PLG_setglobalscale();
extern int PLG_getglobalscale();
extern int PLG_setposterofs();
extern int PLG_pcodedebug();
extern int PLG_colorname_to_rgb();
extern int PLG_rgb_to_gray();
extern int PLG_clr();
extern int PLG_mov();
extern int PLG_lin();
extern int PLG_path();
extern int PLG_dotext();
extern int PLG_font();
extern int PLG_textsize();
extern int PLG_textdir();
extern int PLG_paper();
extern int PLG_linetype();
extern int PLG_normline();
extern int PLG_color();
extern int PLG_backcolor();
extern int PLG_colorfill();
extern int PLG_setwinscale();
extern int PLG_xsca();
extern int PLG_ysca();
extern int PLG_lineclip();
extern int TDH_err();

extern int PLG_cblock_initstatic();
extern int PLG_colorname_to_rgb();
extern int PLG_ellipse();
extern int PLG_he();
extern int PLG_init_initstatic();
extern int PLG_mark_initstatic();
extern int PLG_pcode_initstatic();
extern int PLG_pcodeboundingbox();
extern int PLG_perptail();
extern int PLG_setdumpfile();
extern int PLG_setmaxdrivervect();
extern int PLG_stub_initstatic();
extern int PLG_textsupmode();
extern int PLG_xrgb_to_rgb();
extern int PLG_xsca();
extern int PLG_ysca();



#endif 	/* PLGHEAD */

