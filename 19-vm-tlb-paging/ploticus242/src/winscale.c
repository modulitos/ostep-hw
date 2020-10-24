/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include <stdio.h>

static double xscale, yscale;
static int window_height, window_width;

/* ====================== set up user coords */
int
PLG_setwinscale( width, height, x_max, y_max )
int width, height;
double x_max, y_max;
{
xscale = (double)(width) / (x_max);
yscale = (double)(height) / (y_max);
window_height = height;
window_width = width;
return( 0 );
}
/* ==================== scale in x to window size */
int
PLG_xsca( f )
double f;
{
int iout;
iout = (int) (f * xscale);
return( iout );
}

/* ==================== inverse of xsca */
double 
PLG_xsca_inv( i )
int i;
{
double out;
out = (double) i / xscale;
return( out );
}

/* ==================== scale in y to window size */
int
PLG_ysca( f )
double f;
{
int iout;
iout = window_height - (int) (f * yscale);
return( iout );
}

/* ===================== inverse of ysca */
double 
PLG_ysca_inv( i )
int i;
{
double out;
out = (double)(window_height-i) / yscale;
return( out );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
