/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC ANNOTATE - arbitrary placement of text, arrow, etc. */

#include "pl.h"

/* these statics are here to share values with calc_arrow() */
/* they need to persist only during the life of this proc. */
static double ahx, ahy, atx, aty, ahsize;
static double ah2x, ah2y, at2x, at2y;
static double boxw, boxh, ulx, uly;
static int arrowh, arrowt, arrow2h, arrow2t;
static int do_arrows(), calc_arrow();
static char *arrowdet = "";

int
PLP_annotate()
{
int lvp, first;
char attr[NAMEMAXLEN], *line, *lineval;

char *textcontent, *textdetails, *fromfile, *box, *mapurl, *maplabel;
char *backcolor, *lowbevelcolor, *hibevelcolor, *shadowcolor;
int align, fromfilemode, nlines, maxlen, verttext;
double adjx, adjy, x, y, bm, bevelsize, shadowsize;
int ioutline, do_ellipse, clip, backdim;
double bd1, bd2, bd3, bd4, cx, cy, px, py;

TDH_errprog( "pl proc annotate" );

/* initialize */
textcontent = ""; textdetails = ""; fromfile = ""; arrowdet = ""; box = ""; mapurl = ""; maplabel = "";
x = 3.0; y = 3.0;
fromfilemode = arrowh = arrowt = arrow2h = arrow2t = 0;
verttext = do_ellipse = backdim = clip = 0;
ahsize = 0.1;
bm = 0.0;
bevelsize = shadowsize = 0.0;
backcolor = "";
lowbevelcolor = "0.6";
hibevelcolor = "0.8";
shadowcolor = "black";
bd1 = bd2 = bd3 = bd4 = 0.0;


/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
        if( line == NULL ) break;
        first = 0;
        lineval = &line[lvp];

	if( strcmp( attr, "location" )==0 ) getcoords( "location", lineval, &x, &y ); 
	else if( strcmp( attr, "text" )==0 ) textcontent = getmultiline( lineval, "get" );
	else if( strcmp( attr, "textdetails" )==0 ) textdetails = lineval; 
	else if( strcmp( attr, "fromfile" )==0 ) { fromfile = lineval; fromfilemode = 1; }
	else if( strcmp( attr, "fromcommand" )==0 ) { fromfile = lineval; fromfilemode = 2; }
	else if( strcmp( attr, "arrowhead" )==0 ) { getcoords( "arrowhead", lineval, &ahx, &ahy ); arrowh = 1; }
	else if( strcmp( attr, "arrowtail" )==0 ) { getcoords( "arrowtail", lineval, &atx, &aty ); arrowt = 1; }
	else if( strcmp( attr, "arrowhead2" )==0 ) { getcoords( "arrowhead2", lineval, &ah2x, &ah2y ); arrow2h = 1; }
	else if( strcmp( attr, "arrowtail2" )==0 ) { getcoords( "arrowtail2", lineval, &at2x, &at2y ); arrow2t = 1; }
	else if( strcmp( attr, "arrowdetails" )==0 ) arrowdet = lineval; /* ok - static but persists during this proc only */
	else if( strcmp( attr, "arrowheadsize" )==0 ) ahsize = ftokncpy( lineval );
	else if( strcmp( attr, "box" )==0 || strcmp( attr, "outline" )==0 ) box = lineval; 
	else if( strcmp( attr, "ellipse" )==0 ) do_ellipse = getyn( lineval );
	else if( strcmp( attr, "clickmapurl" )==0 ) mapurl = lineval;
	else if( strcmp( attr, "clickmaplabel" )==0 ) maplabel = lineval;
        else if( strcmp( attr, "clickmaplabeltext" )==0 ) maplabel = getmultiline( lineval, "get" );
	else if( strcmp( attr, "boxmargin" )==0 ) bm = ftokncpy( lineval );
	else if( strcmp( attr, "verttext" )==0 ) verttext = getyn( lineval );
	else if( strcmp( attr, "backcolor" )==0 ) backcolor = lineval;
	else if( strcmp( attr, "bevelsize" )==0 ) bevelsize = ftokncpy( lineval ); 
        else if( strcmp( attr, "shadowsize" )==0 ) shadowsize = ftokncpy( lineval ); 
        else if( strcmp( attr, "lowbevelcolor" )==0 ) lowbevelcolor = lineval;
        else if( strcmp( attr, "hibevelcolor" )==0 ) hibevelcolor = lineval;
        else if( strcmp( attr, "shadowcolor" )==0 ) shadowcolor = lineval;
	else if( strcmp( attr, "backdim" )==0 ) { PL_getbox( "backdim", lineval, &bd1, &bd2, &bd3, &bd4 ); backdim = 1; }
	else if( strcmp( attr, "backadjust" )==0 ) { PL_getbox( "backadjust", lineval, &bd1, &bd2, &bd3, &bd4 ); backdim = 2; }
        else if( strcmp( attr, "clip" )==0 ) clip = getyn( lineval );
	else Eerr( 1, "attribute not recognized", attr );
	}

if( ahsize <= 0.0 ) ahsize = 0.0; /* no arrow */
if( PLS.usingcm ) ahsize /= 2.54;
if( PLS.usingcm ) bm /= 2.54;

if( fromfilemode > 0 ) { file_to_buf( fromfile, fromfilemode, PL_bigbuf, MAXBIGBUF ); textcontent = PL_bigbuf; }

textdet( "textdetails", textdetails, &align, &adjx, &adjy, 0, "R", 1.0 );
if( align == '?' ) align = 'C';

px = x + adjx;
py = y + adjy;

/* figure backing box */
measuretext( textcontent, &nlines, &maxlen );
boxw = (maxlen+2) * Ecurtextwidth;
boxh = (nlines*1.2) * Ecurtextheight;
uly = py + Ecurtextheight;
if( align == 'L' ) ulx = px;
else if( align == 'R' ) ulx = px - boxw;
else ulx = px - (boxw/2.0);  /* align=C */


if( bm != 0.0 ) {
	ulx -= bm;
	uly += bm;
	boxw += (bm*2);
	boxh += (bm*2);
	}

if( clip ) PLG_pcodeboundingbox( 0 ); /* clip the annotation to the cropped size (by turning off bb) */

if( backcolor[0] != '\0' || ( box[0] != '\0' && strnicmp( box, "no", 2 )!= 0 ) ) {
	if( box[0] != '\0' && strnicmp( box, "no", 2 )!= 0 ) {
		ioutline = 1;
		linedet( "box", box, 0.5 );
		}
	else ioutline = 0;
	if( do_ellipse ) {
		cx = ulx+(boxw/2.0);
		cy = uly-(boxh/2.0);
		if( backdim == 1 ) { cx = bd1; cy = bd2; boxw = bd3; boxh = bd4; }
		do_arrows(); /* do this before ellipse to get abutting edge */
		if( backdim == 2 ) PLG_ellipse( cx, cy, ((boxw/2.0)*1.3)+bd1, ((boxh/2.0)*1.3)+bd2, backcolor, ioutline );
		else PLG_ellipse( cx, cy, ((boxw/2.0)*1.3), ((boxh/2.0)*1.3), backcolor, ioutline );
		}
	else	{
		if( backdim ==1 ) { ulx = bd1; uly = bd2; boxw = bd3; boxh = bd4; bd1 = bd2 = bd3 = bd4 = 0.0; }
		do_arrows(); /* do this before fill to get abutting edge */

		Ecblock( ulx+bd1, (uly-boxh)+bd2, ulx+boxw+bd3, uly+bd4, backcolor, ioutline );
		if( bevelsize > 0.0 || shadowsize > 0.0 ) 
			Ecblockdress( ulx+bd1, (uly-boxh)+bd2, ulx+boxw+bd3, uly+bd4, bevelsize, lowbevelcolor, hibevelcolor, shadowsize, shadowcolor);
		}
	}
else do_arrows();

if( PLS.clickmap && (mapurl[0] != '\0' || maplabel[0] != '\0' )) {
	if( backdim && do_ellipse ) {
		/* need to solve back to ulx,uly in this case.. */
		ulx = (((boxw/2.0)-cx)*-1.0); 
		uly = (((boxw/2.0)-cy)*-1.0) + boxh; 
		}
	clickmap_entry( 'r', mapurl, 0, ulx, (uly-boxh), ulx+boxw, uly, 1, 0, maplabel );
	}

/* now render the text.. */
textdet( "textdetails", textdetails, &align, &adjx, &adjy, 0, "R", 1.0 ); /* need to do again */
if( align == '?' ) align = 'C';
Emov( x + adjx, y + adjy );
if( verttext ) Etextdir( 90 );
Edotext( textcontent, align );
if( verttext ) Etextdir( 0 );

if( clip ) PLG_pcodeboundingbox( 1 ); /* restore */

return( 0 );
}

/* ==================== */
/* do arrows */
static int do_arrows()
{

/* if tail location not given, try to be smart about arrow placement.. */

if( arrowh ) {
	linedet( "arrowdetails", arrowdet, 0.7 );
	if( !arrowt ) calc_arrow( ulx, uly, boxw, boxh, ahx, ahy, &atx, &aty );
	Earrow( atx, aty, ahx, ahy, ahsize, 0.4, "" );
	}

/* and render 2nd arrow.. */
if( arrow2h ) {
	if( !arrow2t ) calc_arrow( ulx, uly, boxw, boxh, ah2x, ah2y, &at2x, &at2y );
	Earrow( at2x, at2y, ah2x, ah2y, ahsize, 0.4, "" );
	}
return( 0 );
}

/* ================== */
/* figure where tail of arrow should be */
static int
calc_arrow( ulx, uly, boxw, boxh, ahx, ahy, tailx, taily )
double ulx, uly, boxw, boxh, ahx, ahy, *tailx, *taily;
{
double atx, aty;

/* ah directly above or below textbox.. straight arrow.. */
if( ahx >= ulx && ahx <= ulx+boxw ) {
	atx = ahx;
	if( ahy > uly ) aty = uly;
	else if( ahy < uly-boxh ) aty = uly-boxh;
	}

/* ah directly beside textbox.. straight arrow.. */
else if( ahy >= uly-boxh && ahy <= uly ) {
	aty = ahy;
	if( ahx < ulx ) atx = ulx;
	else if( ahx > ulx+boxw ) atx = ulx+boxw;
	} 

/* ah somewhere to left of textbox.. elbow arrow from mid-box.. */
else if( ahx < ulx ) {
	atx = ahx + ((ulx - ahx)/2.0);
	aty = uly - (boxh/2.0);
	Emov( ulx, aty ); 
	Elin( atx, aty );
	}

/* ah somewhere to right of textbox.. elbow arrow from mid-box.. */
else if( ahx > ulx+boxw ) {
	atx = ulx+boxw + ((ahx-(ulx+boxw))/2.0);
	aty = uly - (boxh/2.0);
	Emov( ulx+boxw, aty ); 
	Elin( atx, aty );
	}

else { atx = ahx; aty = ahy; } /* ? */

*tailx = atx;
*taily = aty;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
