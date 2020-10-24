/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC DRAWCOMMANDS - execute a list of draw commands */

#include "pl.h"

extern int unlink();
static int do_drawcommands();


int
PLP_drawcommands()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char buf[1024], op[80];
char *dcfile, *dumpfile, *commands;
int ix, len;
double x, y;
FILE *fp;

TDH_errprog( "pl proc drawcommands" );

/* initialize */
dcfile = "";
dumpfile = "";
commands = "";

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "file" )==0 ) dcfile = lineval;
	else if( strcmp( attr, "dumpfile" )==0 ) dumpfile = lineval;
	else if( strcmp( attr, "commands" )==0 ) commands = getmultiline( lineval, "get" );
	else Eerr( 1, "attribute not recognized", attr );
	}



/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */
if( commands[0] != '\0' ) {
	sprintf( buf, "%s_Z", PLS.tmpname );
	fp = fopen( buf, "w" ); /* temp file, unlinked below */
	if( fp == NULL ) return( Eerr( 522, "Cannot open draw commands file", buf ));
	fprintf( fp, "%s\n", commands );
	fclose( fp );
	do_drawcommands( buf );
	unlink( buf );
	}

else if( dcfile[0] != '\0' ) do_drawcommands( dcfile );

else if( dumpfile[0] != '\0' ) {
	fp = fopen( dumpfile, "r" );
	if( fp == NULL ) return( Eerr( 523, "Cannot open dump file", dumpfile ) );
	while( fgets( buf, 1023, fp ) != NULL ) {
		len = strlen( buf );
		buf[ len-1 ] = '\0';
		ix = 0;
		strcpy( op, GL_getok( buf, &ix ));
		if( op[0] == 'A' ) { PLG_setdefaults(); continue; }  /* added scg 5/24/07 */
		x = atof( GL_getok( buf, &ix ));
		y = atof( GL_getok( buf, &ix ));
		if( buf[ix] == ' ' ) ix++;
		PLG_pcode( op[0], x, y, &buf[ix] );
		}
	fclose( fp );
	}
return( 0 );
}

/* ===================================== */
/* DO_DRAWCOMMANDS - draw commands interpreter.
	The commands follow the structure of the API,
	except that movs lins and paths are used instead of movu, linu, and pathu
*/
static int
do_drawcommands( filename )
char *filename;
{
FILE *fp;
char buf[512];
int nt;
char op[80];
double x, y;
int oldtextsize;
char oldcolor[COLORLEN];
int oldlinetype;
double oldlinewidth, oldpatfact;
char sx[80], sy[80];

if( filename[0] == '\0' ) return( 0 );

if( strcmp( filename, "stdin" )==0 ) fp = stdin;
else fp = fopen( filename, "r" );
if( fp == NULL ) return( Eerr( 5737, "cannot open draw commands file", filename ) );
               
oldtextsize = Ecurtextsize;
strcpy( oldcolor, Ecurcolor );
oldlinetype = Ecurlinetype;
oldlinewidth = Ecurlinewidth;
oldpatfact = Ecurpatternfactor;
Etextsize( 12 );
Ecolor( "black" );
Elinetype( 0, 1.0, 1.0 );

while( fgets( buf, 511, fp ) != NULL ) {
	buf[ strlen( buf ) - 1 ] = '\0';

	nt = sscanf( buf, "%s %lf %lf", op, &x, &y );	
	if( PLS.usingcm ) { x /= 2.54; y /= 2.54; } /* added scg 8/8/05 */

	if( nt < 1 ) continue; /* blank line */
	if( op[0] == '/' && op[1] == '/' ) continue; /* comment */

	if( GL_smember( op, "mov lin" ) && nt != 3 ) { 
		if( nt != 6 ) Eerr( 2582, "error in drawcommands", buf );
		continue;
		}

	if( strcmp( op, "mov" )==0 ) Emov( x, y );
	else if( strcmp( op, "lin" )==0 ) Elin( x, y );
	else if( strcmp( op, "text" )==0 ) {
		convertnl( &buf[5] );
		Etext( &buf[5] );
		}	
	else if( strcmp( op, "path" )==0 ) Epath( x, y );
	else if( GL_smember( op, "movs lins paths" )) { /* scaled space operators */
		sscanf( buf, "%*s %s %s", sx, sy );
		if( op[0] == 'm' ) Emov( PL_u(X, sx ), PL_u(Y, sy ) );
		else if( op[0] == 'l' ) Elin( PL_u(X, sx ), PL_u(Y, sy ) );
		else if( op[0] == 'p' ) Epath( PL_u(X, sx ), PL_u(Y, sy ) );
		}
	else if( GL_smember( op, "movp linp pathp" )) { /* posex operators */
		sscanf( buf, "%*s %s %s", sx, sy );
		Eposex( sx, X, &x );
		Eposex( sy, Y, &y );
		/* posex() handles usingcm.. */
		if( op[0] == 'm' ) Emov( x, y );
		else if( op[0] == 'l' ) Elin( x, y );
		else if( op[0] == 'p' ) Epath( x, y );
		}
	else if( strcmp( op, "fill" )==0 ) Efill();
	else if( strcmp( op, "centext" )==0 ) {
		convertnl( &buf[8] );
		Ecentext( &buf[8] );
		}
	else if( strcmp( op, "rightjust" )==0 ) {
		convertnl( &buf[10] );
		Erightjust( &buf[10] );
		}	
	else if( strcmp( op, "cblock" )==0 ) {
		double x2, y2;
		char color[COLORLEN], sx2[40], sy2[40];
		int outline;
		nt = sscanf( buf, "%*s %s %s %s %s %s %d", sx, sy, sx2, sy2, color, &outline );
		Eposex( sx, X, &x ); /* changed to use posex - scg 8/8/05 */
		Eposex( sy, Y, &y );
		Eposex( sx2, X, &x2 );
		Eposex( sy2, Y, &y2 );
		if( nt != 6 ) Eerr( 2849, "drawfile error on this line", buf );
		else Ecblock( x, y, x2, y2, color, outline );
		}
	else if( strcmp( op, "color" )==0 ) Ecolor( &buf[6] );
	else if( strcmp( op, "textsize" )==0 ) Etextsize( atoi( &buf[9] ) );
	else if( strcmp( op, "linetype" )==0 ) {
		double z;
		sscanf( buf, "%*s %*s %*s %lf", &z );
		Elinetype( (int)x, y, z );
		}
	else if( strcmp( op, "mark" )==0 ) {
		char sym[80];
		double r;
		sscanf( buf, "%*s %s %s %s %lf", sx, sy, sym, &r );
		Eposex( sx, X, &x );
		Eposex( sy, Y, &y );
		Emark( x, y, sym, r );
		}
	else if( strcmp( op, "clr" )==0 ) Eclr(); /* allow? */
	else if( strcmp( op, "bkcolor" )==0 ) Ebackcolor( &buf[8] ); /* persist? */
	}
if( strcmp( filename, "stdin" )!= 0 ) fclose( fp );
Eflush();
/* restore previous settings.. */
Ecolor( oldcolor );
Etextsize( oldtextsize );
Elinetype( oldlinetype, oldlinewidth, oldpatfact );
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
