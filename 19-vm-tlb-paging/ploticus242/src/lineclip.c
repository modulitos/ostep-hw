/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* Cohen-Sutherland line clip.

   Line is (x1,y1) to (x2,y2).  This routine may modify these points.
   Rectangle is (rx1,ry1) lower left to (rx2,ry2) upper right.

   Function returns 0 if line (x1,y1) (x2,y2) should be drawn.
   Returns 1 if line should not be drawn due to being completely out of the rectangle.
 */
   

#define TOP 0x1
#define BOTTOM 0x2
#define RIGHT 0x4
#define LEFT 0x8

static unsigned int boundscode();


int
PLG_lineclip( x1, y1, x2, y2, rx1, ry1, rx2, ry2 )
double *x1, *y1, *x2, *y2, rx1, ry1, rx2, ry2;
{
unsigned int c, c1, c2;
double x, y;
int i;

c1 = boundscode( *x1, *y1, rx1, ry1, rx2, ry2 );
c2 = boundscode( *x2, *y2, rx1, ry1, rx2, ry2 );

/* iterate as many as 4 times.. */
for( i = 0; i < 4; i++ ) {
	if( ( c1 | c2 ) == 0 ) return( 0 );  /* return and draw */
	if( ( c1 & c2 ) != 0 ) return( 1 );  /* return and don't draw */

	c = c1 ? c1 : c2;

	if( c & TOP ) {
		x = (*x1) + ( (*x2)-(*x1) ) * ( ry2 - (*y1) ) / ( (*y2)-(*y1) );
		y = ry2;
		}
	else if( c & BOTTOM ) {
		x = (*x1) + ( (*x2)-(*x1) ) * ( ry1-(*y1) ) / ( (*y2)-(*y1) );
		y = ry1;
		}
	else if( c & RIGHT ) {
		x = rx2;
		y = (*y1) + ( (*y2)-(*y1) ) * ( rx2-(*x1) ) / ( (*x2)-(*x1) );
		}
	else	{
		x = rx1;
		y = (*y1) + ( (*y2)-(*y1) ) * ( rx1-(*x1) ) / ( (*x2)-(*x1) );
		}

	if( c == c1 ) {
		*x1 = x; *y1 = y;
		c1 = boundscode( *x1, *y1, rx1, ry1, rx2, ry2 );
		}
	else	{
		*x2 = x; *y2 = y;
		c2 = boundscode( *x2, *y2, rx1, ry1, rx2, ry2 );
		}
	}
/* never reach here */
return( 0 );
}

/* -------------------------------------- */

static unsigned int 
boundscode( x, y, rx1, ry1, rx2, ry2 )
double x, y, rx1, ry1, rx2, ry2;
{
unsigned int code;
code = 0;
if( y > ry2 ) code |= TOP;
else if( y < ry1 ) code |= BOTTOM;
if( x > rx2 ) code |= RIGHT;
else if( x < rx1 ) code |= LEFT;
return( code );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
