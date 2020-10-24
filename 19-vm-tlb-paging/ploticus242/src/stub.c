/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */


/* Low level elib drawing stub subroutines.. */
/* not to be confused with axis "stubs".. which are a completely different thing */


#include <ctype.h>
#include "plg.h"
extern int GL_getseg(), GL_member();


static int doing_sup = 0;  /* 1 if superscript/subscript enabled */
static double textslantparm = 0.3;  /* how much to move vertically for every 1.0 horizontal */

/* ======================================== */
int
PLG_stub_initstatic()
{
doing_sup = 0;
textslantparm = 0.3;
return( 0 );
}


/* ======================================== */
/* CLR - clear the display */
int
PLG_clr()
{
double atof();
Epcode( 'z', 0.0, 0.0, Ecurbkcolor );
return(0);
}
/* ======================================== */
/* MOV - move to x, y absolute */
int
PLG_mov( x , y )
double x, y;
{
if( Eflip ) Epcode( 'M', (double)y , (double)x, "" );
else Epcode( 'M', (double)x , (double)y, "" );
return(0);
}
/* ======================================== */
/* LIN - line to x, y absolute */
int
PLG_lin( x , y )
double x, y;
{
if( Eflip ) Epcode( 'L', (double)y , (double)x, "" );
else Epcode( 'L', (double)x , (double)y, "" );
return(0);
}
/* ======================================== */
/* PATH - path to x, y absolute (form a polygon to be shaded later) */
int
PLG_path( x, y )
double x, y;
{
if( Eflip ) Epcode( 'P', (double)y , (double)x, "" );
else Epcode( 'P', (double)x , (double)y, "" );
return(0);
}
 

/* ======================================== */
/* DOTEXT -  handle multi-line text. */
int
PLG_dotext( s, op )
char *s;
char op;
{
int i, slen;
char chunk[256], chunk2[256], supchunk[256], subchunk[256];
double x, y;
int j, k, insup, insub, supfound, subfound;
double ofs;
double a, b;


x = Ex1; y = Ey1;

/* convert op */
if( tolower(op) == 'l' ) op = 'T';
else if( tolower(op) == 'r' ) op = 'J';
else op = toupper(op);
if( !GL_member( op, "TCJDU" ) ) op = 'C';

slen = strlen( s );
for( i = 0; ;  ) {
	GL_getseg( chunk, s, &i, "\n" ); 

	/* superscripts and subscripts - limited implementation - scg 8/23/04 */
	supfound = 0; subfound = 0;
	if( doing_sup ) for( j = 0; chunk[j] != '\0'; j++ ) {
		if( chunk[j] == '^' ) supfound++;
		else if( chunk[j] == '`' ) subfound++; 
		}
	if( (supfound > 0 && supfound % 2 == 0) || (subfound > 0 && subfound % 2 == 0 )) {
		for( j = 0, insup = 0, insub = 0, k = 0; chunk[j] != '\0'; j++ ) {
			if( !insup && chunk[j] == '^' ) { insup = 1; continue; }
			else if( insup && chunk[j] == '^' ) { insup = 0; continue; }
			else if( insup ) { chunk2[k] = ' '; supchunk[k] = chunk[j]; subchunk[k] = ' '; k++; }
			else if( !insub && chunk[j] == '`' ) { insub = 1; continue; }
			else if( insub && chunk[j] == '`' ) { insub = 0; continue; }
			else if( insub ) { chunk2[k] = ' '; subchunk[k] = chunk[j]; supchunk[k] = ' '; k++; }
			else { chunk2[k] = chunk[j]; supchunk[k] = ' '; subchunk[k] = ' '; k++; }
			}
		chunk2[k] = '\0'; supchunk[k] = '\0'; subchunk[k] = '\0';

		Epcode( op, 0.0, 0.0, chunk2 ); /* do regular portion of string */

		if( supfound ) {
			ofs = Ecurtextheight * 0.3;

			/* if( Ecurtextdirection == 0 ) Emov( x, y+ofs ); 
			 * else if( Ecurtextdirection == 90 ) Emov( x-ofs, y ); 
			 * else if( Ecurtextdirection == 270 ) Emov( x+ofs, y ); 
			 ***** changed - didn't work w/ Eflip   scg 8/5/05 */

			if( Ecurtextdirection == 0 ) { a = x; b = y+ofs; } 
			else if( Ecurtextdirection == 90 ) { a = x-ofs; b = y; } 
			else if( Ecurtextdirection == 270 ) { a = x+ofs; b = y; } 
			if( Eflip ) Emov( b, a );
			else Emov( a, b );
			Epcode( op, 0.0, 0.0, supchunk );  /* overlay the superscript - same pt size */
			}
		if( subfound ) {
			ofs = Ecurtextheight * 0.3;

			/* if( Ecurtextdirection == 0 ) Emov( x, y-ofs ); 
			 * else if( Ecurtextdirection == 90 ) Emov( x+ofs, y ); 
			 * else if( Ecurtextdirection == 270 ) Emov( x-ofs, y ); 
			 ***** changed - didn't work w/ Eflip   scg 8/5/05 */

			if( Ecurtextdirection == 0 ) { a = x; b = y-ofs; }         
			else if( Ecurtextdirection == 90 ) { a = x+ofs; b = y; }   
			else if( Ecurtextdirection == 270 ) { a = x-ofs; b = y; }  
			if( Eflip ) Emov( b, a );
			else Emov( a, b );
			Epcode( op, 0.0, 0.0, subchunk );  /* overlay the subscript - same pt size */
			}
		}

	else if( op == 'D' || op == 'U' ) {  /* diagonal text (D=left-justified  U=right justified)- added scg 8/8/07 */
		double xx, yy, xofs, yofs;
		int len;
		char tc[5];
		xofs = Ecurtextwidth;
		yofs = Ecurtextheight * textslantparm;
		if( op == 'D' ) xx = x + 0.03;
		else if( op == 'U' ) xx = x;
		yy = y + 0.09;
		len = strlen( chunk );
		for( j = 0; j < len; j++ ) {
			if( op == 'U' ) k = (len-j)-1;
			else k = j;
			sprintf( tc, "%c", chunk[k] );
			if( Eflip ) Emov( yy, xx );
			else Emov( xx, yy ); 
			Epcode( 'C', 0.0, 0.0, tc );
			if( op == 'D' ) xx += xofs;
			else xx -= xofs;  
			yy -= yofs;
			}
		}

	else Epcode( op, 0.0, 0.0, chunk );

	if( i >= slen ) break;

	/* position for next line */
	if( Ecurtextdirection == 0 ) y -= Ecurtextheight;
	else if( Ecurtextdirection == 90 ) x += Ecurtextheight;
	else if( Ecurtextdirection == 270 ) x -= Ecurtextheight;
	if( Eflip ) Emov( y, x );
	else Emov( x, y );
	}
return(0);
}
/* ======================================== */
/* TEXTSUPMODE - set doing_sup .. */
int
PLG_textsupmode( mode )
int mode;
{
doing_sup = mode;
return( 0 );
}
/* ======================================== */
/* TEXTSLANT - set textslantparm .. */
int 
PLG_textslant( val )
double val;
{
textslantparm = val;
return( 0 );
}

/* ======================================== */
/* FONT - Set font to s.  If s is "" use standard font. */
int
PLG_font( s )      
char *s;
{ 
if( strlen( s ) < 1 ) {
        Epcode( 'F', 0.0, 0.0, Estandard_font ); 
	strcpy( Ecurfont, Estandard_font ); 
	}
else if( strcmp( s, Ecurfont )!= 0 ) {
	Epcode( 'F', 0.0, 0.0, s ); 
	strcpy( Ecurfont, s ); 
	} 
return(0);
}
/* ======================================== */
/* TEXTSIZE - Set textsize to x.  If x is 0 use standard textsize.
   In any case, the size is scaled by the standard text scaling factor. */
int
PLG_textsize( x )          
int x;
{ 
double p;

/* scale text */
if( x == 0 ) p = Estandard_textsize;
else p = (double)x * (double)(Estandard_textsize/10.0);


Ecurtextheight = (p+2.0)/72.0;  /* moved up here scg 8/10/05 so textdet() can reliably alter curtextheight */

if( (int)p != Ecurtextsize ) {
	Epcode( 'I', p, 0.0, "" );
	Ecurtextsize = (int)p; 
	/* Ecurtextheight = (p+2.0)/72.0;  */
	if( p >= 14 ) Ecurtextheight = p / 72.0;
	if( Edev == 'g' && Ecurfont[0] == '\0' ) {  /* "ascii" does not use preset sizes */
		/* get exact dimensions of one of the 5 gif text sizes available .. */
		if( p <= 6 ) { Ecurtextwidth = 0.05; Ecurtextheight = 9 / 72.0; } /* 7 */
		else if( p >= 7 && p <= 9 ) 
			{ Ecurtextwidth = 0.06; Ecurtextheight = 12 / 72.0; } /* 10 */
		else if( p >= 10 && p <= 12 ) 
			{ Ecurtextwidth = 0.07; Ecurtextheight = 14 / 72.0; } /* 12 */
		else if( p >= 13 && p <= 15 ) 
			{ Ecurtextwidth = 0.08; Ecurtextheight = 17 / 72.0; } /* 15 */
		else if( p >= 15 ) { Ecurtextwidth = 0.09; Ecurtextheight = 20 / 72.0; }
		}
	else if( Edev != 'x' ) Ecurtextwidth = Ecurtextheight * 0.4; 

	}

return(0);
}
/* ======================================== */
int
PLG_textdir( x )
int x;
{ 
if( x != Ecurtextdirection ) {
	Epcode( 'D', (double)x , 0.0, "" ); 
	Ecurtextdirection = x ; 
	}
return(0);
}
/* ======================================== */
int
PLG_paper( x )
int x;
{
Epcode( 'O', (double)x , 0.0, "" ); 
Ecurpaper = x; 
return(0);
}
/* ======================================== */
/* LINETYPE - Set line parameters.  If linewidth is 0 use standard linescale.
   If pattern density is 0 use standard linescale. */
int
PLG_linetype( pattern, linewidth, pat_dens )    
int pattern;
double linewidth, pat_dens;
{ 
char buf[12];

/* scale linewidth  */
if( linewidth == 0.0 ) linewidth = Estandard_lwscale;
else linewidth = linewidth * Estandard_lwscale;

if( pat_dens == 0.0 ) pat_dens = Estandard_lwscale;

if( linewidth != Ecurlinewidth || 
	pattern != Ecurlinetype || 
	pat_dens != Ecurpatternfactor ) {

	sprintf( buf, "%d", pattern );
	Epcode( 'Y', linewidth, pat_dens, buf );
	Ecurlinewidth = linewidth / Estandard_lwscale;   /* Estandard_lwscale added scg 4/20/07 */
	Ecurlinetype = pattern; 
	Ecurpatternfactor = pat_dens;
	}
return(0);
}

/* ======================================== */
int
PLG_normline()             
{ 
Epcode( 'Y', Estandard_lwscale, 1.0, "0" ); 
Ecurlinewidth = Estandard_lwscale;
Ecurlinetype = 0; 
Ecurpatternfactor = 1; 
return(0);
}

/* ======================================== */
/* set current color for lines and text to s.  If s is "", use
   standard color.  */
int
PLG_color( s )
char *s;
{

if( s[0] == '\0' ) strcpy( Ecurcolor, Estandard_color );
Epcode( 'r', 0.0, 0.0, s );
return(0);
}

/* ======================================== */
/* set background color.
   If background color is "" use standard background color. */
int
PLG_backcolor( color )
char *color;
{
if( color[0] != '\0' )strcpy( Ecurbkcolor, color );
else strcpy( Ecurbkcolor, Estandard_bkcolor );
return(0);
}

/* ======================================== */
/* fill currently defined rectangle/polygon with color c */
int
PLG_colorfill( c )
char *c;
{
char oldcolor[30];
/* strcpy( oldcolor, Ecurcolor ); */
strcpy( oldcolor, Enextcolor );  /* changed scg 3/15/06 */
Ecolor( c );
Efill();
Ecolor( oldcolor ); /* go back to color as it existed before.. */
return(0);
}

#ifdef SUSPENDED
/* ======================================== */
/* (Old) do shading, within the previously defined polygon path.. the shade can be 0 to 1 */
PLG_shade( s )
double s;
{
char str[20];
sprintf( str, "%g", s );
Ecolorfill( str );
return( 0 )
}
#endif

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
