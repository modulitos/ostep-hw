/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PCODE - Draw operations.
	All draw operations use pcode() except initialization (done in init.c), 
	and doing filled GD rectangles which use gifrect().

	Bounding box, overall output scaling, and other low level operations should
	be done here if possible rather than in the individual device interfaces.

	Many E function calls such as Emov and Elin are actually macros defined in
	plg.h, which call pcode with a single character op code and perhaps some 
	parameters.  Others go thru a routine in stub.c.

   ==========================================================================
   Device codes (Edev):
	p		postscript
	x		X11
	g		GD (png, gif, jpeg, etc.)
	s		SVG
 	f		swf (flash format)

   Op codes:
	L		lineto
	M		moveto
	P		path to
	T, C, J		left-adusted Text, Centered text, right Justified text
	r		set pending current color - color change actually occurs on next line/text or 'v'
	v		force color change immediately (can be done by app code)
	  k		   execute set current color (recursive call from pcode - never done by app code)
	
	s		Fill (color)
	I		set text size
	F		set text font 
	D		set text direction 
	Y		set line properties
	.		direct pixel data point (gd and x11 only)
	z		clear screen / set background color
	H		squelch display (s = "on" or s = "off" )		added 8/5/04 scg
	Q		end of file (tells drivers to finish up)

   These codes are used by paginated postscript only (paper control):
	O		paper orientation (ps only)
	Z		print/eject (ps only)

   These codes are used by x11 only (interactive context):
	U		flush (x11 only)
	W		wait for a key or mouse click (x11 only)
	w		cycle notifier from within a loop (x11 only)
	b		save window to backing store (x11 only)
	B		restore window from backing store (x11 only)
	d		make window disappear (x11 only)
	a		make window re-appear (x11 only)
	e		set text scale factor (x11 only, when user resizes window)

   ==========================================================================

*/

#include "plg.h"

extern int PL_clickmap_inprogress(), PL_clickmap_out();
extern int GL_member(), GL_member_nullmode();
#ifndef NOX11
  extern int PLGX_pointsize(), PLGX_stroke(), PLGX_moveto(), PLGX_lineto(), PLGX_path(), PLGX_text(), PLGX_color(), PLGX_fill(); 
  extern int PLGX_savewin(),  PLGX_restorewin(),  PLGX_centext(),  PLGX_rightjust(),  PLGX_wait(),  PLGX_linetype(),  PLGX_async();
  extern int PLGX_disappear(), PLGX_appear(), PLGX_scaletext(), PLGX_flush(), PLGX_dot(), PLGX_pixpt();
#endif
#ifndef NOGD
  extern int PLGG_lineto(), PLGG_moveto(), PLGG_pathto(), PLGG_text(), PLGG_centext(), PLGG_font(), PLGG_rightjust();
  extern int PLGG_fill(), PLGG_color(), PLGG_textsize(), PLGG_chardir(), PLGG_linetype(), PLGG_eof(), PLGG_rect();
  extern int PLGG_imload(), PLGG_implace(), PLGG_pixpt();
#endif
#ifndef NOSVG
  extern int PLGS_stroke(), PLGS_moveto(), PLGS_lineto(), PLGS_path(), PLGS_text(), PLGS_fill(), PLGS_color();
  extern int PLGS_pointsize(), PLGS_font(), PLGS_chardir(), PLGS_linetype(), PLGS_trailer();
  extern int PLGS_showimg(), PLGS_setimg();
#endif
#ifndef NOSWF
  extern int PLGF_stroke(), PLGF_moveto(), PLGF_lineto(), PLGF_path(), PLGF_text(), PLGF_fill(), PLGF_color();
  extern int PLGF_pointsize(), PLGF_font(), PLGF_chardir(), PLGF_linetype(), PLGF_trailer();
#endif
#ifndef NOPS
  extern int PLGP_stroke(), PLGP_moveto(), PLGP_lineto(), PLGP_path(), PLGP_text(), PLGP_fill(), PLGP_color();
  extern int PLGP_pointsize(), PLGP_font(), PLGP_chardir(), PLGP_linetype(), PLGP_show(), PLGP_paper(), PLGP_trailer(), PLGP_newpage();
#endif


static int line_new = 0, line_drawing = 0;		/* used by postscript section */
static int vertchar = 0; 			/* true if character direction is vertical..*/
						/* ..used only by x11 */
#ifndef NOX11
static double txt_curx;				/* real starting place of centered &rt justified text */
#endif
static double txt_width;				/* width of most recently drawn text line. */
static char prev_op;				/* previous op */
static int squelched = 0;			/* is output being squelched? 1=yes, 0=no */
static int pagenum = 1;				/* current page # */
static int virginpage = 1;		/* 1 when nothing has yet been drawn on current page */

static int keeping_bb = 1;
					
static double bb_x1 = 999;		/* coords of bounding box for entire run */
static double bb_y1 = 999;
static double bb_x2 = -999;
static double bb_y2 = -999;

static double bb2_x1 = 999;		/* coords of "sub" bounding box available to app */
static double bb2_y1 = 999;
static double bb2_x2 = -999;
static double bb2_y2 = -999;
static int keep_bb2 = 0;
static int tightbb = 0;
static double scx1, scy1, scx2, scy2;   /* specified crop zone */
static int specifycrop = 0; /* 0 = no  1 = absolute values   2 = values relative to bounding box */

static double globalscale = 1.0;
static double globalscaley = 1.0;
static double posterxo = 0.0;
static double posteryo = 0.0;
static int postermode = 0;
static double lmlx = 0.0, lmly = 0.0;  /* last move local x and y  
	(saves location of last 'MOVE', including any effects of globalscale and poster ofs */
static int pcodedebug = 0;
static FILE *pcodedebugfp = NULL;
static int in_obj = 0;
static int colorchg_pending = 0;
static FILE *dumpfp = NULL;
static int dumpfp_closable = 0;
static int dumpmax = 0;
static int ndumplines = 0;
static int susp_dump = 0;
/* static double xsanemax, ysanemax;
 * static int sanezone = 0;
 */

static int verttextsim();

/* ======================= */
int
PLG_pcode_initstatic()
{
line_new = 0; line_drawing = 0;
vertchar = 0;
squelched = 0;
pagenum = 1;
virginpage = 1;
keeping_bb = 1;
bb_x1 = 999; bb_y1 = 999; bb_x2 = -999; bb_y2 = -999;
bb2_x1 = 999; bb2_y1 = 999; bb2_x2 = -999; bb2_y2 = -999;
keep_bb2 = 0;
tightbb = 0;
specifycrop = 0;
globalscale = 1.0;
globalscaley = 1.0;
posterxo = posteryo = 0.0;
postermode = 0;
lmlx = lmly = 0.0;
pcodedebug = 0;
in_obj = 0;
colorchg_pending = 0;
pcodedebugfp = NULL;
dumpfp = NULL;
dumpfp_closable = 0;
dumpmax = 0;
ndumplines = 0;
susp_dump = 0;
/* sanezone = 0; */
return( 0 );
}


/* ======================= */
int
PLG_pcode( op, x, y, s )
char op; /* op code */
double x, y;  /* coordinates */
char s[];     /* optional character string */
{
char buf[512];
int stat;

/* postscript: inform driver we're beginning a new page.. */
if( Edev == 'p' && virginpage && !GL_member( op, "Q" ) ) {
#ifndef NOPS
	PLGP_newpage( pagenum );
#endif
	virginpage = 0;
	}

if( pcodedebug == 2 ) fprintf( stderr, "%c %g %g %s\n", op, x, y, s );

/* use this chunk to step thru one op code at a time - press any key to continue */
/*  // if( op != 'w' ) 
 * fprintf( stderr, "%c %g %g %s\n", op, x, y, s );
 * PLGX_flush(); 
 * PLGX_wait(); 
 */



/* lazy color set .. only execute a color change when line, text, or fill is imminent.. scg 6/18/04 */
if( op == 'r' ) {
	strcpy( Enextcolor, s );
	if( strcmp( Enextcolor, Ecurcolor ) !=0 ) colorchg_pending = 1;
	else colorchg_pending = 0;
	return( 0 );
	}
else if( colorchg_pending && GL_member( op, "LTCJszv." )) {  /* v added scg 5/10/05 */
	PLG_pcode( 'k', 0.0, 0.0, Enextcolor );
	colorchg_pending = 0;
	if( op == 'v' ) return( 0 ); /* added scg 5/10/05 */
	}

if( op == 'k' ) strcpy( Ecurcolor, s );


/* For clear op: do it then return; don't include in bounding box calculations */
if( op == 'z' ) {
	keeping_bb = 0;
	susp_dump = 1;
	/* compensate EWinw and EWiny by globalscale because this op will
		be doubly affected by globalscale.. */
	Ecblock( 0.0, 0.0, EWinx/globalscale, EWiny/globalscaley, s, 0 );  
	susp_dump = 0;
	keeping_bb = 1;
	if( dumpfp != NULL ) fprintf( dumpfp, "z 0.0 0.0 %s\n", s );
	return( 0 );
	}



if( op == 'M' ) { Ex1 = x; Ey1 = y; } /* remember most recent 'move', untainted 
					by globalscale or posterofs.. */

/* scale x, y according to global scale, for all move, draw and text size ops */
/* if( globalscale != 1.0 ) {  ... changed scg 3/28/06 */
if( globalscale != 1.0 || globalscaley != 1.0 ) {
	if( GL_member( op, "LMPI." )) { x *= globalscale; y *= globalscaley; }
	if( op == 'Y' ) y *= globalscale; /* dash scale */
	}

/* if poster offset specified, translate */
if( postermode ) {
	if( GL_member( Edev, "pcx" ) && GL_member( op, "LMP." )) { 
		x += posterxo; 
		y += posteryo; 
		}
	}

if( op == 'Q' ) {
#ifdef PLOTICUS
	/* allow clickmap to be generated on any device.. */
	if( Edev != 'g' && Edev != 's' && PL_clickmap_inprogress() ) PL_clickmap_out( 0, 0 ); /* GD & SWF handled in drivers.. */
#endif
	if( pcodedebug ) fprintf( pcodedebugfp, "Done with page.  Bounding box is: %-5.2f, %-5.2f to %-5.2f, %-5.2f\n", 
					bb_x1, bb_y1, bb_x2, bb_y2 );
	}
	

/* reject endobj's without companion beginobj */
if( op == '<' ) in_obj = 1;
else if( op == '>' ) {
	if( ! in_obj ) return( 0 );
	else in_obj = 0;
	}


/* dump file */
if( dumpfp != NULL && !susp_dump ) {
	if( GL_member( op, "UWwbBdae" )) ; /* do nothing for these interactive-context ops.. */
	else if( dumpmax > 0 && ndumplines > dumpmax ) ; /* # output lines exceeded */
	else if( op == 'Q' && dumpfp_closable ) fclose( dumpfp );
	else if( op == 'k' ) fprintf( dumpfp, "r 0.0 0.0 %s\n", s ); /* write an r instead.. (color change optim) */
	else fprintf( dumpfp, "%c %.3f %.3f %s\n", op, x, y, s );
	}


if( op == 'H' ) {  /* moved up - scg 8/12/05 */
	if( strcmp( s, "on" )==0 ) squelched = 1;
	else squelched = 0;
	return( 0 );
	}



/*********** squelch mode: suppress all operations that actually draw something.. */
if( squelched && GL_member( op, "LMPTCJs." )) ;   /* scg 10/16/05 */
	

/*********** interface to X11 xlib driver.. */
else if( Edev == 'x' ) {
#ifndef NOX11

	/* this section changed scg 5/4/04 to improve line dashing on lineplots */
	if( op != 'L' ) { 
		if( line_drawing ) PLGX_stroke(); 
		line_drawing = 0;
		}

	switch (op) {

                case 'L' : if( line_new ) PLGX_moveto( lmlx, lmly );
                            PLGX_lineto( x, y );
                            line_new = 0;
                            line_drawing = 1;
                            break;
                case 'M' : line_new = 1; break;
                case 'P' : if( line_new ) PLGX_moveto( lmlx, lmly );
                           PLGX_path( x, y );
                           line_new = 0;
                           break;
		case 'T' : if( vertchar ) break;
			   PLGX_moveto( lmlx, lmly );
			   PLGX_text( s, &txt_width );
			   break;

		case 'k' : PLGX_color( s ); return( 0 );
		case 's' : PLGX_fill( ); return( 0 );
		case 'U' : PLGX_flush(); return( 0 );
		case 'b' : PLGX_savewin(); return( 0 );
		case 'B' : PLGX_restorewin(); return( 0 );
		case 'I' : PLGX_pointsize( (int)(x), &Ecurtextwidth ); return( 0 );
		case 'C' : if( !vertchar ) { 
				PLGX_moveto( lmlx-6.0, lmly ); 
				PLGX_centext( s, 12.0, &txt_curx, &txt_width ); 
				}
			   break;
		case 'J' : if( !vertchar ) {
				PLGX_moveto( lmlx-12.0, lmly ); 
				PLGX_rightjust( s, 12.0, &txt_curx, &txt_width );
				}
			   break;
		case 'W' : PLGX_wait(); return( 0 );
		case 'Y' : PLGX_linetype( s, x, y ); return( 0 );
		case 'D' : if( x == 90 || x == 270 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
			
		case 'w' : PLGX_async(); return( 0 );
		case '.' : PLGX_pixpt( x, y, s ); break;   /* scg 5/25/06 */
		case 'd' : PLGX_disappear(); return( 0 );
		case 'a' : PLGX_appear(); return( 0 );
		case 'e' : PLGX_scaletext( x ); return( 0 );
		}
#endif
	}


/*************** interface to GD driver.. */
else if( Edev == 'g' ) {
#ifndef NOGD
	switch (op) {
		case 'L' : PLGG_lineto( x, y ); break;
		case 'M' : PLGG_moveto( x, y ); break;
		case 'P' : PLGG_pathto( x, y ); break;
		case '.' : PLGG_pixpt( x, y, s ); break;   /* scg 5/25/06 */
		case 'T' : PLGG_text( s ); break; 
		case 'C' : PLGG_centext( s ); break; 
		case 'F' : PLGG_font( s ); break;
		case 'J' : PLGG_rightjust( s ); break; 
		case 's' : PLGG_fill(); return( 0 );
		case 'k' : PLGG_color( s ); return( 0 );
		case 'I' : PLGG_textsize( (int)x ); return( 0 );
		case 'D' : PLGG_chardir( (int)x ); 
			   if( (int)x != 0 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
		case 'Y' : PLGG_linetype( s, x, y ); return( 0 );
		case 'Q' : PLG_getoutfilename( buf );
			   if( buf[0] == '\0' ) strcpy( buf, "unnamed_result_image" ); /* fallback */

			   /* see if anything has been drawn, if not, return */
			   if ( bb_x2 < -998 && bb_y2 < -998 ) return( 0 );

			   if( tightbb ) stat = PLGG_eof( buf, bb_x1,  bb_y1, bb_x2, bb_y2 ); 
			   else if( specifycrop == 1 )
			     stat = PLGG_eof( buf, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 )
			     stat = PLGG_eof( buf, (bb_x1)-scx1, (bb_y1)-scy1, (bb_x2)+scx2, (bb_y2)+scy2 );
			   else stat = PLGG_eof( buf, bb_x1-0.2,  bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 ); 
			   Eresetbb();
			   return( stat );
		}
#endif
	}


/*************** interface to svg driver */
else if( Edev == 's'  ) {
#ifndef NOSVG
	if( op != 'L' ) { 
		if( line_drawing ) PLGS_stroke(); 
		line_drawing = 0;
		}

	switch( op ) {
		case 'L' : if( line_new ) PLGS_moveto( lmlx, lmly ); 
			    PLGS_lineto( x, y );
			    line_new = 0;
			    line_drawing = 1;
			    break;
		case 'M' : line_new = 1; break;
		case 'P' : if( line_new ) PLGS_moveto( lmlx, lmly ); 
			   PLGS_path( x, y ); 
			   line_new = 0;
			   break;
	
		case 'T' : PLGS_text( op, lmlx, lmly, s, 0.0 ); break;
	
		case 'C' : if( !vertchar ) PLGS_text( op, lmlx , lmly, s, 0.0 );
			   else if( vertchar ) PLGS_text( op, lmlx, lmly , s, 0.0 );
			   break;
		case 'J' : if( !vertchar ) PLGS_text( op, lmlx, lmly, s, 0.0 );
			   else if( vertchar ) PLGS_text( op, lmlx, lmly , s, 0.0 );
			   break;

		case 's' : PLGS_fill( ); return( 0 );
		case 'k' : PLGS_color( s ); return( 0 );
		case 'I' : PLGS_pointsize( (int)x ); return( 0 );
		case 'F' : PLGS_font( s ); return( 0 );
		case 'D' : PLGS_chardir( (int)x );
			   if( x == 90 || x == 270 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
		case 'Y' : PLGS_linetype( s, x, y ); return( 0 );
		/* case '<' : PLGS_objbegin( s ); return( 0 );
		 * case '>' : PLGS_objend(); return( 0 );
		 */
		case 'Z' : pagenum++; virginpage = 1; return( 0 );
		case 'Q' : if( !virginpage ) pagenum++; 
			   if( tightbb ) 
			     stat = PLGS_trailer( bb_x1-0.05, bb_y1-0.05, bb_x2+0.05, bb_y2+0.05 );
			   else if( specifycrop == 1 )
			     stat = PLGS_trailer( scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 ) {
			     stat = PLGS_trailer( (bb_x1-0.05)-scx1, (bb_y1-0.05)-scy1, (bb_x2+0.05)+scx2, (bb_y2+0.05)+scy2 );
				}
			   else 
			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
			     stat = PLGS_trailer( bb_x1-0.2, bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 );
			   Eresetbb();
			   return( stat );
		}
#endif
	}

/*************** interface to swf driver */
else if( Edev == 'f'  ) {
#ifndef NOSWF
 	if( op != 'L' ) { 
 		if( line_drawing ) PLGF_stroke(); 
 		line_drawing = 0;
 		}
 
 	switch( op ) {
 		case 'L' : if( line_new ) PLGF_moveto( lmlx, lmly ); 
 			    PLGF_lineto( x, y );
 			    line_new = 0;
 			    line_drawing = 1;
 			    break;
 		case 'M' : line_new = 1; break;
 		case 'P' : if( line_new ) PLGF_moveto( lmlx, lmly ); 
 			   PLGF_path( x, y ); 
 			   line_new = 0;
 			   break;
 	
 		case 'T' : PLGF_text( op, lmlx, lmly, s, 0.0 ); break;
 	
 		case 'C' : if( !vertchar ) PLGF_text( op, lmlx , lmly, s, 0.0 );
 			   else if( vertchar ) PLGF_text( op, lmlx, lmly , s, 0.0 );
 			   break;
 		case 'J' : if( !vertchar ) PLGF_text( op, lmlx, lmly, s, 0.0 );
 			   else if( vertchar ) PLGF_text( op, lmlx, lmly , s, 0.0 );
 			   break;
 
 		case 's' : PLGF_fill( ); return( 0 );
 		case 'k' : PLGF_color( s ); return( 0 );
 		case 'I' : PLGF_pointsize( (int)x ); return( 0 );
 		case 'F' : PLGF_font( s ); return( 0 );
 		case 'D' : PLGF_chardir( (int)x );
 			   if( x == 90 || x == 270 ) vertchar = 1;
 			   else vertchar = 0;
 			   return( 0 );
 		case 'Y' : PLGF_linetype( s, x, y ); return( 0 );
		/* case '<' : PLGF_objbegin( s ); return( 0 );
 		 * case '>' : PLGF_objend(); return( 0 );
		 */
 		case 'Z' : pagenum++; virginpage = 1; return( 0 );
 		case 'Q' : if( !virginpage ) pagenum++; 
 			   if( tightbb ) 
 			     stat = PLGF_trailer( bb_x1-0.05, bb_y1-0.05, bb_x2+0.05, bb_y2+0.05 );
 			   else if( specifycrop == 1 )
 			     stat = PLGF_trailer( scx1, scy1, scx2, scy2 );
 			   else if( specifycrop == 2 ) {
 			     stat = PLGF_trailer( (bb_x1-0.05)-scx1, (bb_y1-0.05)-scy1, (bb_x2+0.05)+scx2, (bb_y2+0.05)+scy2 );
 				}
 			   else 
 			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
 			     stat = PLGF_trailer( bb_x1-0.2, bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 );
 			   Eresetbb();
 			   return( stat );
 		}
#endif
 	}



/* interface to postscript driver */
else if( Edev == 'p'  ) {
#ifndef NOPS

	if( op != 'L' ) { 
		if( line_drawing ) PLGP_stroke(); 
		line_drawing = 0;
		}

	switch( op ) {
		case 'L' : if( line_new ) PLGP_moveto( lmlx, lmly ); 
			    PLGP_lineto( x, y );
			    line_new = 0;
			    line_drawing = 1;
			    break;
		case 'M' : line_new = 1; break;
		case 'P' : if( line_new ) PLGP_moveto( lmlx, lmly ); 
			   PLGP_path( x, y ); 
			   line_new = 0;
			   break;
		case 'T' : PLGP_text( op, lmlx, lmly, s, 0.0 ); break;
	
		case 'C' : if( !vertchar ) PLGP_text( op, lmlx - 6.0, lmly, s, 12.0 );
			   else if( vertchar ) PLGP_text( op, lmlx, lmly - 6.0, s, 12.0 );
			   break;
		case 'J' : if( !vertchar ) PLGP_text( op, lmlx-12.0, lmly, s, 12.0 );
			   else if( vertchar ) PLGP_text( op, lmlx, lmly - 12.0, s, 12.0 );
			   break;
	
		case 's' : PLGP_fill( ); return( 0 );
		case 'k' : PLGP_color( s ); return( 0 );
		/* case 'c' : PLGP_closepath(); return( 0 ); */
		case 'I' : PLGP_pointsize( (int)x ); return( 0 );
		case 'F' : PLGP_font( s ); return( 0 );
		case 'D' : PLGP_chardir( (int)x );
			   if( x == 90 || x == 270 ) vertchar = 1;
			   else vertchar = 0;
			   return( 0 );
		case 'Y' : PLGP_linetype( s, x, y ); return( 0 );
		case 'Z' : PLGP_show(); pagenum++; virginpage = 1; return( 0 );
		case 'O' : PLGP_paper( (int)x ); return( 0 );
		case 'Q' : if( !virginpage ) { PLGP_show(); pagenum++; }
			   if( tightbb ) 
			     PLGP_trailer( pagenum - 1, bb_x1-0.05, bb_y1-0.05, bb_x2+0.05, bb_y2+0.05 );
			   else if( specifycrop == 1 )
			     PLGP_trailer( pagenum - 1, scx1, scy1, scx2, scy2 );
			   else if( specifycrop == 2 ) {
			     PLGP_trailer( pagenum - 1, (bb_x1-0.05)-scx1, (bb_y1-0.05)-scy1, 
							(bb_x2+0.05)+scx2, (bb_y2+0.05)+scy2 );
				}
			   else 
			     /* add 0.2" margin to be generous in cases of fat lines, etc. */
			     PLGP_trailer( pagenum - 1, bb_x1-0.2, bb_y1-0.2, bb_x2+0.2, bb_y2+0.2 );
			   Eresetbb();
			   return( 0 );
		}
#endif
	}

else if( Edev == 'n' ) ; /* null device.. do nothing */

else 	{ 
	if( Edev == '\0' ) return( Eerr( 12021, "Graphics subsystem never initialized", "" ) );
	else 	{
		char sdev[8];
		sprintf( sdev, "%c", Edev );
		return( Eerr( 12022, "Unrecognized graphic device code", sdev ) );
		}
	}




if( op == 'M' ) { lmlx = x; lmly = y; } /* remember most recent 'move' */



/* figure approximate text dimensions */
/* if( Edev != 'x' && GL_member( op, "TCJ" )) { */  /* why not x?  scg 8/12/05 */
if( GL_member( op, "TCJ" )) {   
	txt_width = strlen( s ) * Ecurtextwidth;
	txt_width *= globalscale;
	}

if( keeping_bb ) {
	/* keep bounding box info (minima and maxima) */
	if( GL_member( op, "LP." ) ) {
		if( prev_op == 'M' ) Ebb( lmlx, lmly );
		Ebb( x, y );
		}
	/* normal (horizontal) text operations.  (vertical text below) */
	else if( op == 'T' && !vertchar ) {
		if( prev_op == 'M' ) Ebb( lmlx, lmly );
		Ebb( lmlx + txt_width+0.05, lmly + (Ecurtextheight*globalscale) );
		}
	else if( op == 'C' && !vertchar ) { 
		Ebb( lmlx - ((txt_width/2.0)+0.05), lmly );
		Ebb( lmlx + ((txt_width/2.0)+0.05), lmly + (Ecurtextheight*globalscale) );
		}
	else if( op == 'J' && !vertchar ) { 
		Ebb( lmlx - (txt_width+0.05), lmly );
		Ebb( lmlx, lmly + (Ecurtextheight*globalscale) );
		}
	}

prev_op = op;


/* handle vertical text .. must be simulated for x windows;
   also gets bounding box for vertical text operations (all devices) */

if( vertchar && GL_member( op, "TCJ" )) verttextsim( op, s );

return( 0 );
}

#ifdef NOX11
int
PLG_getclick()
{
if( Edev == 'p' ) {
	Eshow(); /* eject page and return.. */
	return(0); 
	}
else if( Edev == 'g' ) {
	Eendoffile();
	return( 0 );
	}
return( 0 );
}  
#endif


/* ============================================= */
/* BB - keep an overall bounding box for the entire image.
	 Also call Echeckbb() to maintain nested object bounding boxes.. */
int
PLG_bb( x, y )
double x, y;
{
if( keeping_bb ) {
 	/* if( ( x < bb_x1 && x < 0.0 ) || (x > bb_x2 && x > 8.0 ) ) fprintf( pcodedebugfp, "draw out X = %g\n", x ); 
	 * if( ( y < bb_y1 && y < 0.0 ) || (y > bb_y2 && y > 8.0 ) ) fprintf( pcodedebugfp, "draw out Y = %g\n", y );
	 *	} 
	 */
	if( x < bb_x1 ) { bb_x1 = x; if( pcodedebug == 2 ) { fprintf( pcodedebugfp, "(new x min %g)\n", x ); }}
	if( x > bb_x2 ) { bb_x2 = x; if( pcodedebug == 2 ) { fprintf( pcodedebugfp, "(new x max %g)\n", x ); }}
	if( y < bb_y1 ) { bb_y1 = y; if( pcodedebug == 2 ) { fprintf( pcodedebugfp, "(new y min %g)\n", y ); }}
	if( y > bb_y2 ) { bb_y2 = y; if( pcodedebug == 2 ) { fprintf( pcodedebugfp, "(new y max %g)\n", y ); }}
	}
if( keep_bb2 ) {
	if( x < bb2_x1 ) bb2_x1 = x;
	if( x > bb2_x2 ) bb2_x2 = x;
	if( y < bb2_y1 ) bb2_y1 = y;
	if( y > bb2_y2 ) bb2_y2 = y;
	}


return( 0 );
}

/* ============================================== */
/* RESETBB - needed for multiple pages */
int
PLG_resetbb()
{
bb_x1 = 999;
bb_y1 = 999;
bb_x2 = -999;
bb_y2 = -999;
return( 0 );
}

/* ============================================= */
/* GETBB - get current bounding box.. */
int
PLG_getbb( xlo, ylo, xhi, yhi )
double *xlo, *ylo, *xhi, *yhi;
{
*xlo = bb_x1 / globalscale;
*ylo = bb_y1 / globalscaley;
*xhi = bb_x2 / globalscale;
*yhi = bb_y2 / globalscaley;
return( 0 );
}


/* ============================================== */
/* GETTEXTSIZE - get width and height of last text item.. */

int
PLG_gettextsize( w, h )
  double *w, *h;
{
*w = txt_width;
*h = Ecurtextheight;
return( 0 );
}

/* ================================================ */
/* VERTTEXTSIM - vertical text bounding box, also simulation for X11 displays */
static int
verttextsim( op, s )
char op, s[];
{
double dist, y1, y2, x, y;
int len;

len = strlen( s );

if( Edev == 'x' ) dist = len * (Ecurtextheight * globalscale);
else dist = len * (Ecurtextwidth * globalscale);

if( op == 'T' ) { y1 = lmly; y2 = lmly + dist; }
else if( op == 'C' ) { y1 = lmly - (dist/2); y2 = lmly + (dist/2); }
else if( op == 'J' ) { y1 = lmly - dist; y2 = lmly; }
if( Edev == 'x' ) x = lmlx - (Ecurtextwidth * globalscale);
else x = lmlx;
y = y2;
#ifndef NOX11
if( Edev == 'x' ) {
	int i;
	char let[4];
	double w;
	for( i = 0; i < len; i++ ) {
		sprintf( let, "%c", s[i] );
		PLGX_moveto( x, y ); 
		PLGX_text( let, &w ); 
		y -= (Ecurtextheight * globalscale);
		}
	}
#endif
Ebb( x-(Ecurtextheight*globalscale), y1 );
Ebb( x-(Ecurtextheight*globalscale), y2 );
return( 0 );
}


#ifdef SUSPENDED
/* ==================================================== */
/* SQUELCH_DISPLAY - 1 = squelch all display activity, 0 restore to normal */
/* Used to calculate bounding box without displaying */
/* handles nested calls. */
int
PLG_squelch_display( mode )
int mode;
{
static int snest = 0;
if( mode == 1 ) {
	snest++;
	squelched = 1;
	}
else if( mode == 0 ) { 
	if( snest > 0 ) snest--;
	if( snest == 0 )squelched = 0;
	}
}

#endif
/* ==================================================== */
/* INIT_BB2 - a second bounding box available to app  */
int
PLG_init_bb2( )
{
keep_bb2 = 1;
bb2_x1 = 999.0; bb2_y1 = 999.0; bb2_x2 = -999.0; bb2_y2 = -999.0; 
return( 0 );
}
/* ==================================================== */
/* GET_BB2 - get the second bounding box */
int
PLG_get_bb2( x1, y1, x2, y2 )
double *x1, *y1, *x2, *y2;
{
*x1 = bb2_x1/globalscale; *y1 = bb2_y1/globalscaley; *x2 = bb2_x2/globalscale; *y2 = bb2_y2/globalscaley;
/* keep_bb2 = 0; */
return( 0 );
}

/* ==================================================== */
/* TIGHTBB - switch ON=don't add margin when doing final BB crop */
int
PLG_tightbb( mode )
int mode;
{
tightbb = mode;
return( 0 );
}
/* ==================================================== */
/* SPECIFYCROP - application may use this to specify crop rectangle */
int
PLG_specifycrop( mode, x1, y1, x2, y2 )
int mode; /* 0=off   1=absolute values   2=relative to tightcrop values  */
double x1, y1, x2, y2;
{
specifycrop = mode;
if( specifycrop ) { scx1 = x1; scy1 = y1; scx2 = x2; scy2 = y2; }
return( 0 );
}

#ifdef HOLD
/* ==================================================== */
/* SANEZONE - application may use this to specify a sane zone.  If pcode gets
	a request to draw outside of the sane zone, the draw operation is cancelled
	before calling the device drivers.  This addresses the issue where a wild
	draw request gets thru to GD which in turn creates an enormous image.  Added scg 5/8/06 */
int 
PLG_sanezone( x1, y1, x2, y2 )
double x1, y1, x2, y2;
{
/* currently only keeping x2,y2, since they seem to be the main problem (in GD anyway..) */
xsanemax = x2; ysanemax = y2; 
sanezone = 1;
return( 0 );
}
#endif
	

/* ==================================================== */
/* GIFRECT - direct interface to GD driver for better efficiency on rectangles */
int
PLG_gifrect( xlo, yhi, xhi, ylo, color )
double xlo, yhi, xhi, ylo;
char *color;
{
#ifndef NOGD
char oldcolor[COLORLEN];
strcpy( oldcolor, Ecurcolor );
strcpy( Ecurcolor, color );   /* so that Ecolor() knows to change back below.. scg 6/18/04 */
/* if( globalscale != 1.0 ) { .... changed scg 3/28/06 */
if( globalscale != 1.0 || globalscaley != 1.0 ) { 
	xlo *= globalscale; ylo *= globalscaley; 
	xhi *= globalscale; yhi *= globalscaley;
	}
PLGG_rect( xlo, yhi, xhi, ylo, color );
Ebb( xlo, ylo );
Ebb( xhi, yhi );
Ecolor( oldcolor );
#endif
return( 0 );
}
/* ==================================================== */
/* IMLOAD - for GD this loads the named image file... 
 *	  - for SVG this tells the svg driver to remember image filename for later 
 */
int
PLG_imload( filename, width, height )
char *filename;
int width, height;  /* optional, may be given if known here but not known at time of implace() (eg. symboldetails)... otherwise 0, 0 */
{
int stat;
stat = 1;

if( globalscale != 1.0 || globalscaley != 1.0 ) {
	width = (int) (width * globalscale); height = (int) (height * globalscaley); 
	}

if( Edev == 'g' ) {
#ifndef NOGD
  	stat = PLGG_imload( filename, width, height );
#endif
	}
else if( Edev == 's' && width != 0 && height != 0 ) { 
#ifndef NOSVG
	stat = PLGS_setimg( filename, width, height );
#endif
	}
return( stat );
}

/* ==================================================== */
/* IMPLACE - for GD this places the currently loaded image file at x,y (filename not used).
 *	   - for SVG this adds an <image> tag to the svg output file (image filename 
 */
int
PLG_implace( x, y, filename, align, width, height )
double x, y;
char *filename; /* image file, used by svg only .. but not required if filename was set earlier using imload() */
char *align;    /* alignment, used by gd and svg */
int width, height;  /* render the image in this width and height (in pixels) */
{
int stat;
stat = 1;
if( globalscale != 1.0 || globalscaley != 1.0 ) {
	x *= globalscale; y *= globalscaley;
	width = (int) (width * globalscale); height = (int) (height * globalscaley); 
	}
if( Edev == 'g' ) {
#ifndef NOGD
	stat = PLGG_implace( x, y, align, width, height );
#endif
	}
else if( Edev == 's' ) {
#ifndef NOSVG
	if( line_drawing ) PLGS_stroke();
        line_drawing = 0;
	width = (int) (width * 0.72 );  /* svg wants image dimensions in svg pixels (72 per inch) */
	height = (int) (height * 0.72 );
	stat = PLGS_showimg( filename, x, y, align, width, height );
#endif
	}
return( stat );
}


/* ===================================================== */
/* SETGLOBALSCALE - set global scale factor */
int
PLG_setglobalscale( sx, sy )
double sx, sy;
{
if( sx < 0.01 || sx > 20.0 ) return( Eerr( 20815, "Invalid global scaling", "" ) );
if( sy < 0.01 || sy > 20.0 ) return( Eerr( 20815, "Invalid global scaling", "" ) );
globalscale = sx;
globalscaley = sy;
Estandard_lwscale = 1.0 * sx;
return( 0 );
}
/* ===================================================== */
/* GETGLOBALSCALE - get global scale factor */
int
PLG_getglobalscale( sx, sy )
double *sx, *sy;
{
*sx = globalscale;
*sy = globalscaley;
return( 0 );
}

/* ======================================= */
/* SETPOSTEROFS - set poster offset (paginated postscript only).
   x, y are in absolute units, and are where the lower-left of the page will be.
   So if I have a poster made of 4 8.5x11 sheets held portrait style,
	the lowerleft would use 0,0
	the lowerright would use 8,0
	the upperleft would use 0,10.5
	the lowerright would use 8,10.5
   (8 and 10.5 are used because of print margins).
   The four pages can then be trimmed w/ a paper cutter and butted up against 
   one another to create a poster.
*/
int
PLG_setposterofs( x, y )
double x, y;
{
postermode = 1;
posterxo = x * (-1.0);
posteryo = y * (-1.0);
return( 0 );
}

/* ========================================== */
/* PCODEDEBUG - turn on/off local debugging */
int
PLG_pcodedebug( mode, fp )
int mode;
FILE *fp;  /* stream for diagnostic output */
{
pcodedebug = mode;
pcodedebugfp = fp;
return( 0 );
}

/* =========================================== */
int
PLG_pcodeboundingbox( mode )
int mode;
{
keeping_bb = mode;
return( 0 );
}

/* =========================================== */
int
PLG_setdumpfile( dumpfile, filemode )
char *dumpfile;
char *filemode;
{
dumpfp_closable = 0;
if( strcmp( dumpfile, "stdout" )==0 ) dumpfp = stdout;
else 	{
	dumpfp = fopen( dumpfile, filemode );
	if( dumpfp == NULL ) return( Eerr( 57202, "cannot open dump file", dumpfile ) );
	dumpfp_closable = 1;
	fprintf( dumpfp, "A 0 0 init-graphics\n" );  /* added scg 5/24/07 */
	}
ndumplines = 0;
return( 0 );
}

/* ========================================= */
int
PLG_closedumpfile()
{
if( dumpfp_closable && dumpfp != NULL ) fclose( dumpfp );
dumpfp = NULL;  /* can't assume on linux */
return( 0 );
}
/* ========================================= */
int
PLG_setdumpmaxlines( nlines )
int nlines;
{
dumpmax = nlines;
return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
