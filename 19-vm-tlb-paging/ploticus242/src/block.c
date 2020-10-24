/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* do a rectangle, with shading and/or outline */

#include "plg.h"
extern int atoi();

static double box_x1 = 0.0, box_y1 = 0.0, box_x2 = 0.0, box_y2 = 0.0;

/* ========================================== */
int
PLG_cblock_initstatic( )
{
box_x1 = 0.0;
box_y1 = 0.0;
box_x2 = 0.0;
box_y2 = 0.0;
return( 0 );
}

/* ========================================== */
/* CBLOCK - all coords in abs space, color fill */
int
PLG_cblock( xlo, ylo, xhi, yhi, color, outline )
double xlo, ylo, xhi, yhi;
char *color;
int outline;
{
double gxlo, gylo, gxhi, gyhi;
double lw;

/* if hatch pattern.. */
if( strnicmp( color, "hatch", 5 )==0 ) {
	double oldwidth, oldpatfact; 
	int oldlt, hnum;
	double h, start, stop, inc;
	char oldcolor[COLORLEN];
	char dir;

	/* save current line width, dash style.. */
	oldlt = Ecurlinetype;
	oldwidth = Ecurlinewidth;
	oldpatfact = Ecurpatternfactor;
	strcpy( oldcolor, Ecurcolor );

	hnum = atoi( &color[5] );
	if( hnum < 1 ) hnum = 1;
	if( hnum > 9 ) hnum = 9;
	/* 
	   1 = vl dotted ver
	   2 = l dotted hor
	   3 = m dotted 
	   4 = l vert
	   5 = l hor
	   6 = m vert
	   7 = m hor
           8 = vl vert & hor
           9 = l vert & hor
	 */
	Ecolor( "black" );

	if( Edev == 's' ) lw = 0.5; /* svg needs thicker lines here.. */
	else lw = 0.1;

	if( hnum <= 3 ) Elinetype( 1, lw, 1.0 );
	else Elinetype( 0, lw, 1.0 );

	if( hnum == 1 || hnum == 8 ) inc = 0.04; /* v light */
	else if( hnum == 2 || hnum == 4 || hnum == 5 || hnum == 9 ) inc = 0.03; /* light */
	else inc = 0.018; /* medium */

	if( hnum != 2 && hnum != 5 && hnum != 7 ) dir = 'v';
	if( hnum == 2 || hnum == 5 || hnum == 7 ) dir = 'h';
	if( hnum == 8 || hnum == 9 ) dir = 'b';
	
	/* if Eflip is in effect flip horizontal v. vertical */
	if( Eflip && dir == 'v' ) dir = 'h';
	else if( Eflip && dir == 'h' ) dir = 'v';

	if( dir == 'v' || dir == 'b' ) {   /* vertical lines */
		if( xlo < xhi ) { start = xlo; stop = xhi; }
		else { start = xhi; stop = xlo; }
		h = start + (inc/2.0);
		while( h < stop ) {
			Emov( h, ylo ); Elin( h, yhi );
			h += inc;
			}
		}

	if( dir == 'h' || dir == 'b' ) {   /* horizontal lines */
		if( ylo < yhi ) { start = ylo; stop = yhi; }
		else { start = yhi; stop = ylo; }
		h = start + (inc/2.0);
		while( h < stop ) {
			Emov( xlo, h ); Elin( xhi, h );
			h += inc;
			}
		}

	/* restore line type */
	Elinetype( oldlt, oldwidth, oldpatfact );
	Ecolor( oldcolor );
	}

/* if solid color.. */
else if( color[0] != '\0' ) {
	if( Edev == 'g' ) {
		double tmp;
		char gcolor[COLORLEN];
		/* this is a code exception made because gif driver 
		   gives much better performance on filled rectangles
		   than on filled polygons */
		strcpy( gcolor, color );

		if( Eflip ) { gxlo = ylo; gylo = xlo; gxhi = yhi; gyhi = xhi; }
		else	{ gxlo = xlo; gylo = ylo; gxhi = xhi; gyhi = yhi; }

		/* gif driver requires that low be low and high be high.. */
		if( gxhi < gxlo ) { tmp = gxhi; gxhi = gxlo; gxlo = tmp; }
		if( gyhi < gylo ) { tmp = gyhi; gyhi = gylo; gylo = tmp; }

		Egifrect( gxlo, gyhi, gxhi, gylo, gcolor ); 
		}
	else	{
		Emov( xlo, ylo ); 
		Epath( xlo, yhi );
		Epath( xhi, yhi ); 
		Epath( xhi, ylo ); 
		Ecolorfill( color );
		}
	}

/* do an outline if requested.. */
if( outline ) {
	Emov( xlo, ylo );
	Elin( xlo, yhi );
	Elin( xhi, yhi );
	Elin( xhi, ylo );
	Elin( xlo, ylo ); /* close.. */
	Elin( xlo, yhi ); /* and go one more to get last corner mitre right.. */
	}

return( 0 );
}
/* ============================================== */
/* SETLASTBOX - set last box */
int
PLG_setlastbox( x1, y1, x2, y2 )
double x1, y1, x2, y2;
{
box_x1 = x1; box_y1 = y1; box_x2 = x2; box_y2 = y2;
return( 0 );
}
/* ============================================== */
/* GETLASTBOX - get dimensions of most recently generated box.. */
int
PLG_getlastbox( x1, y1, x2, y2 )
double *x1, *y1, *x2, *y2;
{
*x1 = box_x1; *y1 = box_y1; *x2 = box_x2; *y2 = box_y2;
return( 0 );
}

/* ============================================== */
/*  BLOCKDRESS - color shadowing/3-d effect for rectangles */
int
PLG_cblockdress( xlow, ylow, xhi, yhi, 
	bevelsize, lowbevelcolor, hibevelcolor, shadowsize, shadowcolor)
double xlow, ylow, xhi, yhi;
char *lowbevelcolor, *hibevelcolor, *shadowcolor;
double bevelsize, shadowsize;
{
double x1, y1, x2, y2;
x1 = xlow-bevelsize; y1 = ylow-bevelsize; x2 = xhi + bevelsize; y2 =  yhi+bevelsize;

if( bevelsize > 0.0 ) {
	if( lowbevelcolor[0] != '\0' ) {
		Emov( x1, y1 ); Epath( x1+bevelsize, y1+bevelsize ); 
		Epath( x2-bevelsize, y1+bevelsize ); 
		Epath( x2, y1 ); 
		Ecolorfill( lowbevelcolor );
	
		Emov( x2, y2 );  /* wierd bug with gif driver when done old way.. */
		Epath( x2, y1 ); 
		Epath( x2-bevelsize, y1+bevelsize ); 
		Epath( x2-bevelsize, y2-bevelsize ); 
		Ecolorfill( lowbevelcolor );
		}
	if( hibevelcolor[0] != '\0' ) {
		Emov( x1, y1 ); Epath( x1+bevelsize, y1+bevelsize ); 
		Epath( x1+bevelsize, y2-bevelsize );
		Epath( x2-bevelsize, y2-bevelsize ); Epath( x2, y2 ); Epath( x1, y2 ); 
			Ecolorfill( hibevelcolor );
		}
	}

if( shadowsize > 0.0 && shadowcolor[0] != '\0' ) {
        Ecblock( xlow+shadowsize, ylow-shadowsize, xhi+shadowsize, ylow, shadowcolor, 0 );
        Ecblock( xhi, ylow-shadowsize, xhi+shadowsize, yhi-shadowsize, shadowcolor, 0 );
	}

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
