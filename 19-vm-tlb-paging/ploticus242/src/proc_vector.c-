/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* windbarb features contributed by Andrew Phillips */

/* PROC VECTOR - render a display of vectors */

#include "pl.h"

#define TWOPI 6.2831854
#define HALFPI 1.5707963

int
PLP_vector()
{
char attr[NAMEMAXLEN], *line, *lineval;
int first, lvp;

int i, stat;
int xfield, yfield, dirfield, magfield, colorfield, exactcolorfield, clip;
double dirrange, lenscale, x, y, newx, newy, len, dir, ahlen, ahwid;
double sin(), cos(), basedir, constantlen, holdx, holdy;
char *acolor, *linedetails, *selex;
char legendlabel[256];
double barblimitbig, barblimitmedium, barblimitsmall, barblimittiny, barbdir, mag;
char dirunits, zeroat, clockdir, lenunits, type;
int x2field, y2field;
double taillen;

TDH_errprog( "pl proc vector" );

xfield = -1; yfield = -1; dirfield = -1; magfield = -1; colorfield = -1; exactcolorfield = -1; x2field = -1; y2field = -1;
dirrange = 360.0;
ahlen = 0.15;
ahwid = 0.4;
acolor = ""; selex = ""; linedetails = "";
strcpy( legendlabel, "" );
dirunits = 'd'; /* degrees */
zeroat = 't';   /* top */
clockdir = '+'; /* clockwise */
lenunits = 'a'; /* absolute */
type = 'a'; /* arrow */
lenscale = 1.0;
constantlen = 0.0;
taillen = 0.1;

barblimitbig = 50.0; /* Magnitude limits */
barblimitmedium = 10.0;
barblimitsmall = 5.0;
barblimittiny = 2.0;
barbdir = 120;
clip = 1;
mag = 0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "xfield" )==0 ) xfield = fref( lineval ) -1;
	else if( strcmp( attr, "yfield" )==0 ) yfield = fref( lineval ) -1;
	else if( strcmp( attr, "dirfield" )==0 ) dirfield = fref( lineval ) -1;
	else if( strcmp( attr, "x2field" )==0 ) x2field = fref( lineval ) -1;
	else if( strcmp( attr, "y2field" )==0 ) y2field = fref( lineval ) -1;
	else if( strcmp( attr, "dirrange" )==0 ) dirrange = atof( lineval );
	else if( strcmp( attr, "dirunits" )==0 ) dirunits = lineval[0];
	else if( strcmp( attr, "clockdir" )==0 ) clockdir = lineval[0];
	else if( strcmp( attr, "zeroat" )==0 ) zeroat = lineval[0];
	else if( strcmp( attr, "magfield" )==0 || strcmp( attr, "lenfield" )==0 ) magfield = fref( lineval ) -1;
	else if( strcmp( attr, "lenunits" )==0 ) lenunits = lineval[0];
	else if( strcmp( attr, "lenscale" )==0 ) lenscale = ftokncpy( lineval );
	else if( strcmp( attr, "constantlen" )==0 ) constantlen = ftokncpy( lineval );
	else if( strcmp( attr, "colorfield" )==0 ) colorfield = fref( lineval ) -1;
	else if( strcmp( attr, "exactcolorfield" )==0 ) exactcolorfield = fref( lineval ) -1;
	else if( strcmp( attr, "linedetails" )==0 ) linedetails = lineval;
	else if( strcmp( attr, "arrowheadlength" )==0 ) ahlen = ftokncpy( lineval );
	else if( strcmp( attr, "arrowheadwidth" )==0 ) ahwid = ftokncpy( lineval );
	else if( strcmp( attr, "arrowheadcolor" )==0 ) acolor = lineval;
	else if( strcmp( attr, "select" )==0 ) selex = lineval;
	else if( strcmp( attr, "legendlabel" )==0 ) tokncpy( legendlabel, lineval, 256 );
	else if( strcmp( attr, "taillen" )==0 ) taillen = ftokncpy( lineval );
	else if( strcmp( attr, "clip" )==0 ) clip = getyn( lineval );
	else if( strcmp( attr, "type" )==0 ) type = lineval[0];
	else if( strcmp( attr, "barblimits" )==0 ) { 
		sscanf( lineval, "%lf %lf %lf %lf", &barblimitbig, &barblimitmedium, &barblimitsmall, &barblimittiny );
		}	
	else if( strcmp( attr, "barbdir" )==0 ) barbdir = ftokncpy( lineval );
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
/* -------------------------- */
if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

if( type == 'e' ) type = 'i';

if( xfield < 0 || yfield < 0 ) return( Eerr( 2205, "xfield and yfield are both required", "" ));

if( dirfield < 0 && ( x2field < 0 || y2field < 0 )) return( Eerr( 2205, "dirfield or x2field&y2field required", "" ));

if( magfield < 0 && constantlen == 0.0 && (x2field < 0 || y2field < 0 )) 
	return( Eerr( 2205, "magfield, constantlen, or x2field&y2field required", "" ));

if( magfield < 0 && type == 'b' && (x2field < 0 || y2field < 0 )) 
	return( Eerr( 2205, "magfield oe x2field/y2field required when type is barb", "" ));

if( barblimitbig <= 0 || barblimitmedium <= 0 || barblimitsmall <= 0 ) return( Eerr( 2205, "barblimits must be grater then 0", ""));

if( strncmp( legendlabel, "#usefname", 9 )==0 ) getfname( dirfield+1, legendlabel ); /* legendlabel[256] */

if( !GL_member( type, "abelit" )) type = 'a';


/* now do the plotting work.. */
/* -------------------------- */
if( dirunits == 'r' ) dirrange = TWOPI;

if( zeroat == 't' ) basedir = TWOPI * 0.25; 		/* top */
else if( zeroat == 'b' ) basedir = TWOPI * 0.75; 	/* bottom */
else if( zeroat == 'l' ) basedir = TWOPI * 0.5;		/* left */


linedet( "linedetails", linedetails, 0.5 );

barbdir = (barbdir/dirrange) * TWOPI;

for( i = 0; i < Nrecords; i++ ) {

	if( selex[0] != '\0' ) { /* process against selection condition if any.. */
		int result;
                stat = do_select( selex, i, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selex ); continue; }
                if( result == 0 ) continue; /* reject */
                }

	/* get x value.. */
	x = fda( i, xfield, 'x' );
        if( Econv_error() ) { conv_msg( i, xfield, "xfield" ); continue; }

	/* get y value.. */
	y = fda( i, yfield, 'y' );
        if( Econv_error() ) { conv_msg( i, yfield, "yfield" ); continue; }

	/* if colorfield used, get color.. */
        if( colorfield >= 0 ) {
		char *ldet; /* to get it to compile.. */
                linedetails = "";
                ldet = PL_get_legent( da( i, colorfield ) );
		linedet( "colorfield", linedetails, 0.5 );
                }
	else if( exactcolorfield >= 0 ) {
                linedetails = da( i, exactcolorfield );
		linedet( "exactcolorfield", linedetails, 0.5 );
		}

	/* convert x,y to absolute units.. */
	x = Eax( x );
	y = Eay( y );

	/* added scg 12/19/03 */
	if( x2field >= 0 && y2field >=0 ) {
		newx = Eax( fda( i, x2field, 'x' ) );
		newy = Eay( fda( i, y2field, 'y' ) );
		}

	else	{
		/* dir and len.. */
		dir = atof( da( i, dirfield ) );
		if( magfield >= 0 ) mag = atof( da( i, magfield ) );
		if( constantlen > 0.0 ) len = constantlen;
		/* else len = atof( da( i, magfield ) ); */
		else len = mag;
	
		if( clockdir == '+' ) dir *= -1.0;
	
		/* normalize dir and len.. */
		dir = basedir + ((dir / dirrange) * TWOPI );
		len *= lenscale;
		if( lenunits == 'x' || lenunits == 'd' || lenunits == 'u' ) len = Eax( len ) - Eax( 0.0 );
		else if( lenunits == 'y' ) len = Eay( len ) - Eay( 0.0 );
	
		newx = x + (len * cos( dir ));
		newy = y + (len * sin( dir ));
		}

	/* skip degenerate cases.. added scg 5/24/05 */
	if( x == newx && y == newy ) continue;

	if( clip ) {
		holdx = newx; holdy = newy;
		stat = Elineclip( &x, &y, &newx, &newy, EXlo, EYlo, EXhi, EYhi );
		if( stat ) continue; /* entire vector is out of range */
		else if( ! GL_close_to( newx, holdx, 0.000001 ) || !GL_close_to( newy, holdy, 0.000001 ) ) {
			/* end is out of range, just draw line.. */
			Emov( x, y ); 
			Elin( newx, newy );
			continue;
			}
		}

	/* arrow */
	if( type == 'a' ) {
		Earrow( x, y, newx, newy, ahlen, ahwid, acolor );
		}

	/* line or error bar */
	else if( type == 'l' || type == 'i' || type == 't' ) {
		Emov( x, y );
		Elin( newx, newy );
		if( type == 'i' ) PLG_perptail( x, y, newx, newy, taillen );
		if( type == 'i' || type == 't' ) PLG_perptail( newx, newy, x, y, taillen );
		}

	/* windbarb */  
	else if( type == 'b' ) {
		/* contributed by Andrew Phillips */
	  	int bigBarbCount, mediumBarbCount, smallBarbCount, curBarb, b;
	    	double x1, x2, x3, y1, y2, y3, newMag, barbLen, barbSpace, barbAdjust;

	  	bigBarbCount = 0;
		mediumBarbCount = 0;
		smallBarbCount = 0;

		curBarb = 0;

		newMag = mag;

		/* length of barbs and the space betwen them */
		barbLen = len/3;
		barbSpace = len/6;

		/* used to make small and medium barbs come from the
		   end of them stem */
		barbAdjust = 0;

		if( newMag >= barblimittiny) {
			/* Draw the stem */
			Emov( x, y ); 
			Elin( newx, newy );
	
			/* Count how many barbs to draw.  Since barblimitbig
			   and friends arn't always going to be integers we
			   have to do this insted of some modulo fun. */
	
			while(newMag > barblimitbig) {
			  bigBarbCount++;
			  newMag -= barblimitbig;
			}
	
			while(newMag > barblimitmedium) {
			  mediumBarbCount++;
			  newMag -= barblimitmedium;
			}
	
			while(newMag > barblimitsmall) {
			  smallBarbCount++;
			  newMag -= barblimitsmall;
			}
	
			/* Draw the big (triangle) barbs */
			for (b = 0; b < bigBarbCount && curBarb < 8; b++) {
			  x1 = x + ((len - curBarb*barbSpace) * cos( dir ));
			  y1 = y + ((len - curBarb*barbSpace) * sin( dir ));
			  x2 = x + ((len - (curBarb+0.5)*barbSpace) * cos( dir )) - (barbLen * cos(dir+barbdir));
			  y2 = y + ((len - (curBarb+0.5)*barbSpace) * sin( dir )) - (barbLen * sin(dir+barbdir));
			  x3 = x + ((len - (curBarb+1)*barbSpace) * cos( dir ));
			  y3 = y + ((len - (curBarb+1)*barbSpace) * sin( dir ));
	
			  Emov(x1, y1);
			  Epath(x2, y2);
			  Epath(x3, y3);
			  Epath(x1, y1);
	
			  Ecolorfill( Ecurcolor );
			  curBarb++;
			}
	
			if (bigBarbCount > 0) barbAdjust = 0.5;
	
			/* Draw the medium barbs */
			for (b = 0; b < mediumBarbCount && curBarb < 8; b++) {
			  x1 = x + ((len - (curBarb+barbAdjust)*barbSpace) * cos( dir ));
			  y1 = y + ((len - (curBarb+barbAdjust)*barbSpace) * sin( dir ));
			  x2 = x1 - (barbLen * cos(dir+barbdir));
			  y2 = y1 - (barbLen * sin(dir+barbdir));
	
			  Emov(x1, y1);
			  Elin(x2, y2);
			  curBarb++;
			}
	
			/* Draw the small barbs */
			for (b = 0; b < smallBarbCount && curBarb < 8; b++) {
			  x1 = x + ((len - (curBarb+barbAdjust)*barbSpace) * cos( dir ));
			  y1 = y + ((len - (curBarb+barbAdjust)*barbSpace) * sin( dir ));
			  x2 = x1 - (barbLen/2 * cos(dir+barbdir));
			  y2 = y1 - (barbLen/2 * sin(dir+barbdir));
			    
			  Emov(x1, y1);
			  Elin(x2, y2);
			  curBarb++;
			}
	       }

	   else {
	  	/* Tiny barbs should just be half length stems */
                 newx = x + (len/2 * cos( dir ));
                 newy = y + (len/2 * sin( dir ));

                 /* Draw the mini stem */
                 Emov( x, y );
                 Elin( newx, newy );
               }

           } /* end of barbs */ 


	} /* end data rows loop */

if( legendlabel[0] != '\0' ) PL_add_legent( LEGEND_LINE, legendlabel, "", linedetails, "", "" );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
