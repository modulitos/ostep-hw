/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* routines related to interactivity, generally X11 */
#include <unistd.h>  /* for usleep() */
#include "plg.h"

/* event modes */
#define STRING 2	/* getting a \n terminated string */
#define EVENTS 3	/* getting any mouse-button or keyboard event */


/* info concerning the most recent event.. */
static double Eevx, Eevy;
static int Eeid;
/* static char semfile[128] = ""; */


/* ==================================== */
/* PLG_interactive_initstatic() - doesn't seem to be needed..  */
/* {
 * savex = 0.0; savey = 0.0;
 * savec = 0;
 * strcpy( semfile, "" );
 * return( 0 );
 * }
 */


/* ==================================== */
/* GETKEY - get mouse position (x, y) and event code.. */
/* when using postscript, this will do a showpage, endoffile, and exit. */
int
PLG_getkey( x, y, e )
double *x, *y;
int *e;
{
int stat;

if( Edev == 'p' ) { /* postscript-- eject page and return.. */
	Eshow(); 
	return( 0 );
	}
if( Edev == 'g' ) { /* gif, finish up and return */
	stat = Eendoffile(); 
	return( stat );
	}  

Eeid = 0;

/* note: the following loop has to run quick enough so as not to
   miss any keystrokes of a fast typist.. */
while( 1 ) {
	Easync();
	if( Eeid != 0 && Eeid < 1005 ) {  /* interested in all keys and mouse buttons
						but no expose, resize, etc. */
		*x = Eevx; *y = Eevy; *e = Eeid;
		break;
		}

	usleep( 20000 ); /* loop delay - resulting in 50 cycles per second */
	}

return( 0 );
}
  

/* =================================== */
/* GETCLICK wait until a key or button is hit.. */
/* when using postscript, this will do a showpage and return. */
int
PLG_getclick()
{
double x, y;
int e, stat;
if( Edev == 'p' ) { /* postscript-- eject page and return.. */
	Eshow(); 
	return(0);
	}
else if( Edev == 'g' ) {
	stat = Eendoffile();
	return( stat );
	}

return( Egetkey( &x, &y, &e ) );
}

/* ================================ */
/* HE This gets called by the X11 driver when we are waiting for an event 
   and then a key, mouse, expose, or resize event happens. Never called 
   directly by applications.  
*/
int
PLG_he( x, y, e )
double x, y;
int e;
{

/* set global vars for async processes.. */
EEvent = e; EEventx = x; EEventy = y;

/* call the application's event handler, which must be named Ehandle_events().. */
if( e == E_RESIZE ) {
	Esetwinscale( (int)x, (int)y, x/Epixelsinch, y/Epixelsinch );
	x /= Epixelsinch;
	y /= Epixelsinch;
	EWinx = x;
	EWiny = y;
	}
if( e >= 1010 ) Ehandle_events( x, y, e );

/* user is clicking any mouse button or key.. */
Eevx = x; Eevy = y; Eeid = e;
return( 1 );
}


#ifdef SUSPENDED

/* ==================== */
/* the following routines provide a place to save/retrieve an event */
int
PLG_savekey( lx, ly, c )
double lx, ly;
int c;
{
savex = lx;
savey = ly;
savec = c;
return( 0 );
}

PLG_retrievekey( lx, ly, c )
double *lx, *ly;
int *c;
{
*lx = savex;
*ly = savey;
*c = savec;
return( 0 );
}

/* =================================== */
/* Indicate that a semiphore file is to be checked and executed regularly
	when blocking for input.
   s is the full path name of the semiphore file. 
*/
/* PLG_setsemfile( s )
* char *s;
* {
* strcpy( semfile, s );
* return( 0 );
* }
*/

/* ==================================== */
/* Execute the semfile */
/* PLG_semfile()
* {
* FILE *fp;
* if( Edev == 'p' ) return(0); // postscript-- just return.. 
* // if( semfile[0] != '\0' ) Eprocess_drawfile( semfile ); 
* return( 0 );
* }
*/
#endif


/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
