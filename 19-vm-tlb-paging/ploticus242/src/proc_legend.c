/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC LEGEND - Render an automatic legend. Plotting procs supply legend entries */

#include "pl.h"

#define MAXLEGENT 100           /* max # of legend entries */
#define MAXLEGTEXT 5000         /* max amount of legend text, including labels
                                        and details attributes */

#define DOWN 0
#define ACROSS 1

extern int PLG_init_bb2(), PLG_get_bb2();
 
static int NLE = 0;
static int LEavail = 0;
static int LEtype[MAXLEGENT];
static int LEparm1[MAXLEGENT];
static int LEparm2[MAXLEGENT];
static int LEparm3[MAXLEGENT];
static int LElabel[MAXLEGENT];
static int LEtag[MAXLEGENT];
static char Ltext[MAXLEGTEXT];

/* ================================ */
int
PLP_legend_initstatic()
{
NLE = 0;
LEavail = 0;
return(0);
}

/* ================================ */
int
PLP_legend()
{
int i;
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char buf[256], symcode[80], holdstdcolor[COLORLEN], url[MAXURL], color[COLORLEN], format;
char *textdetails, *frame, *backcolor, *title, *titledetails, *s, *specifyorder;
int j, k, ix, ixx, nt, align, outline, nlines, maxlen, reverseorder, do_outline, colortext;
int buflen, noclear, nlinesym, maxtwidth, maxthi, wraplen, dobox, bstate;
double adjx, adjy, x, y, yy, seglen, hsep, msep, swatchsize, radius, extent;
double startx, starty, sampwidth, colchunksep, colbreak, rowchunksep, orig_x, orig_y, bx1, by1, bx2, by2, bmx1, bmy1, bmx2, bmy2, titx, tity;

TDH_errprog( "pl proc legend" );

/* initialize */
textdetails = "";
frame = "no";
title = "";
titledetails = "";
specifyorder = "";
backcolor = Ecurbkcolor;
x = -9999.0;
y = -9999.0;
format = DOWN;
seglen = 0.5;
msep = 0.0;
hsep = 0.3; 
reverseorder = 0; do_outline = 0; colortext = 0; noclear = 0; wraplen = 0; dobox = 0;
swatchsize = 0.1;
extent = -1.0;
colchunksep = 0.35;
rowchunksep = 0.1;
nlinesym = 2;
bmx1 = bmy1 = bmx2 = bmy2 = 0.08;

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "location" )==0 ) getcoords( "location", lineval, &x, &y );
	else if( strcmp( attr, "format" )==0 ) {
		format = lineval[0];
		if( format == 'd' || format == 'm' ) format = DOWN;
		else if( format == 'a' || format == 's' ) format = ACROSS;
		}
	else if( strcmp( attr, "textdetails" )==0 ) textdetails = lineval;
	else if( strcmp( attr, "seglen" )==0 ) seglen = ftokncpy( lineval );
	else if( strcmp( attr, "sep" )==0 ) ; /* superseded by the 'separation' attribute */
	else if( strcmp( attr, "separation" )==0 ) { 
		hsep = ftokncpy( lineval ); msep = ftokncpy( lineval );
		if( PLS.usingcm ) { hsep /= 2.54; msep /= 2.54; }
		}
	else if( strcmp( attr, "colortext" )==0 ) colortext = getyn( lineval );
	else if( strcmp( attr, "outlinecolors" )==0 ) do_outline = getyn( lineval ); 
	else if( strcmp( attr, "noclear" )==0 )  noclear = getyn( lineval ); 
	else if( strcmp( attr, "specifyorder" )==0 ) specifyorder = getmultiline( lineval, "get" );
	else if( strcmp( attr, "specifyorder1" )==0 ) { specifyorder = lineval; convertnl( specifyorder ); }

	else if( strcmp( attr, "swatchsize" )==0 ) { swatchsize = ftokncpy( lineval ); if( PLS.usingcm ) swatchsize /= 2.54; }

	else if( strcmp( attr, "reverseorder" )==0 )  reverseorder = getyn( lineval );
	else if( strcmp( attr, "reset" )==0 ) { NLE = 0; return( 0 ); }

	/* the following attributes added for 2.32 - scg 8/12/05 ....... */

	else if( strcmp( attr, "extent" )==0 ) {  extent = ftokncpy( lineval ); if( PLS.usingcm ) extent /= 2.54; }
	else if( strcmp( attr, "chunksep" )==0 ) {  /* additional amount of separation between column chunks or row chunks */
		colchunksep = ftokncpy( lineval );
		rowchunksep = ftokncpy( lineval );
		if( PLS.usingcm ) { colchunksep /= 2.54; rowchunksep /= 2.54; }
		}
	else if( strcmp( attr, "nlinesym" )== 0 ) {  nlinesym = itokncpy( lineval ); if( nlinesym > 2 ) nlinesym = 2; }
	else if( strcmp( attr, "wraplen" )==0 ) wraplen = itokncpy( lineval );
	else if( strcmp( attr, "frame" )==0 ) { frame = lineval; dobox = 1; }
	else if( strcmp( attr, "backcolor" )==0 ) { backcolor = lineval; dobox = 1; }
	else if( strcmp( attr, "boxmargin" )==0 ) {
		char foo1[40], foo2[40];
		nt = sscanf( lineval, "%s %s", foo1, foo2 );
		if( nt == 1 ) { if( PLS.usingcm ) bmx1 /= 2.54; bmy1 = bmx2 = bmy2 = bmx1; }
		else 	{
			PL_getbox( "boxmargin", lineval, &bmx1, &bmy1, &bmx2, &bmy2 );
			if( PLS.usingcm ) { bmx1 /= 2.54; bmy1 /= 2.54; bmx2 /= 2.54; bmy2 /= 2.54; }
			}
		}
	else if( strcmp( attr, "title" )==0 ) { title = lineval; dobox = 1; convertnl( title ); }
	else if( strcmp( attr, "titledetails" )==0 ) titledetails = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}


if( NLE < 1 ) return( 0 ); /* silent is better here ... scg 5/5/04 */



/*********** now do the plotting work.. **********/


/* If we're doing a legend title and/or a bounding box around the legend... some tricks required....
   What we'll do is go into squelch mode (suppressing all graphic device output calls), and
   render the legend invisibly while keeping a bounding box.  Then (see "goto RENDER"), we'll
   get out of squelch mode, render the box (since we know it's coordinates now) then go back
   and render the legend for real.  [Note added scg 3/18/2011]
 */

/* box init stuff.. */
if( dobox ) { bstate = 0; PLG_init_bb2(); Esquelch( "on" ); }

/* default location */
if( x < -9000.0 ) { 
	if( scalebeenset() ) { x = EXhi - 1.5; y = EYhi - 0.1; }
	else { x = 6.0; y = 2.0; }
	}

orig_x = x; orig_y = y;

RENDER:

ix = 0;
buflen = strlen( specifyorder );

startx = x; starty = y;
if( format == DOWN ) {
	if( extent < 0.0 ) colbreak = 0.0; /* always at y = 0 */
	else colbreak = starty - extent;  /* extent was specified.. do it */
	}
maxtwidth = 0; /* keep track of the max text width in column */
maxthi = 0;  /* keep track of the max text #lines in column (since individual entries can wrap with "\n" */


textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2, "R", 1.0 );
y -= Ecurtextheight; 
for( i = 0; i < NLE; i++ ) {


	/* fprintf( stderr, "%d|%s|%s|%s\n", LEtype[i], &Ltext[LElabel[i]], &Ltext[LEparm1[i]], &Ltext[LEparm2[i]] ); */

	if( specifyorder[0] != '\0' ) {
		/* get next line in orderspec.. */
		NEXTORDERLINE:
		GL_getchunk( buf, specifyorder, &ix, "\n" );
		if( ix >= buflen ) break;

		/* now search for matching entry.. */
		for( k = 0; k < NLE; k++ ) {
			if( strncmp( buf, &Ltext[LElabel[k]], strlen(buf) )==0) { 
				j = k;
				break;
				}
			}
		if( k == NLE ) {
			/* following changed, scg 2/27/02 */
			/* Eerr( 2894, "No legend entry matched", buf ); */ 
			goto NEXTORDERLINE;
			/* continue; */
			}
		}
	else if( reverseorder ) j = ((NLE-1)-i);
	else j = i;
		
	yy = y+(Ecurtextheight*0.4);

	/* draw swatch(es), depending on type */
	if( LEtype[j] == LEGEND_COLOR ) {
		sampwidth = swatchsize+0.1;
		if( format == ACROSS && x > startx ) x += sampwidth;
		sscanf( &Ltext[LEparm1[j]], "%s", color );
		if( strcmp( color, backcolor ) ==0 ) outline = 1;
		else outline = do_outline;
		if( outline ) { Elinetype( 0, 0.5, 1.0 ); Ecolor( Estandard_color ); PLG_forcecolorchg(); }  
		Ecblock( x-(swatchsize+0.1), y, x-0.1, y+swatchsize, color, outline );
		}
	else if( LEtype[j] == LEGEND_LINE ) {
		sampwidth = seglen+0.1;
		if( format == ACROSS && x > startx ) x += sampwidth;
		linedet( &Ltext[LElabel[j]], &Ltext[LEparm1[j]], 1.0 );
		Emov( x-(seglen+0.1), yy );
		Elin( x-0.1, yy );
		}
	else if( LEtype[j] == LEGEND_LINEMARK ) {  /* tiny line marks that can be used in scatterplots */
		sampwidth = 0.2;
		if( format == ACROSS && x > startx ) x += sampwidth;
		linedet( &Ltext[LElabel[j]], &Ltext[LEparm1[j]], 1.0 );
		if( strcmp( &Ltext[LEparm2[j]], "v" )==0 ) { Emov( x-0.15, yy+0.05 ); Elin( x-0.15, yy-0.05 ); } /* vertical */
		else	{  Emov( x-0.2, yy ); Elin( x-0.1, yy ); } /* horizontal */
		}
	else if( LEtype[j] == LEGEND_SYMBOL ) {
		symdet( "symbol", &Ltext[LEparm1[j]], symcode, &radius );
		sampwidth = 0.17 + radius;
		if( format == ACROSS && x > startx ) x += sampwidth;
		Emark( x-0.17, yy, symcode, radius );
		}
	else if( LEtype[j] == LEGEND_TEXT ) {
		sampwidth = 0.8; /* just a guess */
		if( format == ACROSS && x > startx ) x += sampwidth;
		/* parm1 is text, parm2 is textdetails */
		textdet( &Ltext[LElabel[j]], &Ltext[LEparm2[j]], &align, &adjx, &adjy, -2, "R", 1.0 );
		Emov( x-0.1, y );
		Erightjust( &Ltext[LEparm1[j]] ); 
		}
	else if( LEtype[j] == (LEGEND_LINE + LEGEND_SYMBOL) ) {
		sampwidth = seglen+0.1;
		if( format == ACROSS && x > startx ) x += sampwidth;
		/* parm1 is linedetails, parm2 is symboldetails */
		linedet( &Ltext[LElabel[j]], &Ltext[LEparm1[j]], 1.0 );
		Emov( x-(seglen+0.1), yy );
		Elin( x-0.1, yy );
		symdet( "symbol", &Ltext[LEparm2[j]], symcode, &radius );
		if( nlinesym == 1 ) Emark( x-((seglen/2.0)+0.1), yy, symcode, radius );
		else if ( nlinesym == 2 ) {
			Emark( x-(seglen), yy, symcode, radius );
			Emark( x-0.18, yy, symcode, radius );
			}
		}
	else if( LEtype[j] == (LEGEND_TEXT + LEGEND_SYMBOL) ) {
		sampwidth = 0.8; /* just a guess */
		if( format == ACROSS && x > startx ) x += sampwidth;
		/* parm1 is text, parm2 is textdetails, parm3 is symboldetails */
		symdet( "symbol", &Ltext[LEparm3[j]], symcode, &radius );
		Emark( x-0.17, yy, symcode, radius );
		textdet( &Ltext[LElabel[j]], &Ltext[LEparm2[j]], &align, &adjx, &adjy, -2, "R", 1.0 );
		Emov( x-0.17, y+(Ecurtextheight*0.2) );
		Ecentext( &Ltext[LEparm1[j]] ); 
		}

	s = &Ltext[ LElabel[ j ]];

	/* check for embedded url.. */
	strcpy( url, "" );
	if( strncmp( s, "url:", 4 )==0 ) {
		ixx = 0;
		strcpy( url, GL_getok( &s[4], &ixx ) );
		s = &Ltext[ LElabel[j] + ixx + 4 + 1 ];
		}

	if( wraplen ) GL_wraptext( s, wraplen );

	/* get #lines and maxlen of label */
	measuretext( s, &nlines, &maxlen );
	if( maxlen  > maxtwidth ) maxtwidth = maxlen;
	if( nlines > maxthi ) maxthi = nlines;

	/* render label */
	if( colortext ) {
		if( LEtype[j] == LEGEND_COLOR ) Ecolor( color );
		strcpy( holdstdcolor, Estandard_color );
		strcpy( Estandard_color, "" ); /* this prevents textdet() from changing the color 7/12/01 */
		}
	textdet( "textdetails", textdetails, &align, &adjx, &adjy, -2, "R", 1.0 );

	Emov( x + adjx, y + adjy );
	Etext( s );
	if( colortext ) strcpy( Estandard_color, holdstdcolor );

	if( PLS.clickmap && url[0] ) {
		clickmap_entry( 'r', url, 0, x+adjx, y+adjy, 
			x+adjx+(maxlen*Ecurtextwidth), y+adjy+(nlines*Ecurtextheight), 1, 0, "" );
		}


	/* determine position for next legend entry.. */
	if( format == DOWN ) { 
		y -= ((Ecurtextheight * nlines) + 0.03 ) + msep;

		if(  y < colbreak ) { /* start a new column - added scg 8/12/05 */
			/* x = x + ((double)maxtwidth*Ecurtextwidth) + sampwidth + colchunksep;  */
			x = x + ((double)maxtwidth*Ecurtextwidth) + sampwidth + colchunksep;  
			y = starty - Ecurtextheight;
			maxtwidth = 0;
			}
		}

	else if( format == ACROSS ) { /* single line */
		/* if( hsep > 0.0 ) */ /* changed, scg 11/1/07 */
		x += ((Ecurtextwidth * maxlen ) + hsep);  
		/* else x += ((Ecurtextwidth * maxlen ) + 1.2); */
		if( x < startx ) x = startx; /* 11/1/07 */

		if( extent > 0.0 && ((x-startx) > extent) ) { /* start a new row - added scg 8/12/05 */
			y -= ((Ecurtextheight * maxthi) + rowchunksep);
			x = startx;
			maxthi = 0;
			}
		}
	}


if( dobox ) {
	PLG_get_bb2( &bx1, &by1, &bx2, &by2 ); /* get bb so we know where to place the title.. */
	if( title[0] != '\0' ) { /* do invisible title 1st time thru to influence bb, then really draw it 2nd time thru.. */
		textdet( "titledetails", titledetails, &align, &adjx, &adjy, 0, "R", 1.0 );
		if( align == '?' ) align = 'C';
		measuretext( title, &nlines, &maxlen );
		if( bstate == 0 ) { titx = bx1+((bx2-bx1)/2.0)+adjx;  tity = by2+(nlines*Ecurtextheight)+adjy; }
		Emov( titx, tity );
		Edotext( title, align );
		}
	Esquelch( "off" );
	if( bstate == 0 ) {
		/* now that we know the extent of the legend, do box now; 
		 * then we'll go back and draw the legend on top of it */
		PLG_get_bb2( &bx1, &by1, &bx2, &by2 ); /* may have changed if there was a title.. */
		if( strcmp( frame, "bevel" )==0 ) {
			Ecblock( bx1-bmx1, by1-bmy1, bx2+bmx2, by2+bmy2, backcolor, 0 );
			Ecblockdress( bx1-bmx1, by1-bmy1, bx2+bmx2, by2+bmy2, 0.07, "0.6", "0.8", 0.0, "" );
			}
		else if( strcmp( frame, "no" ) == 0 ) Ecblock( bx1-bmx1, by1-bmy1, bx2+bmx2, by2+bmy2, backcolor, 0 );
		else	{
			linedet( "frame", frame, 1.0 );
			Ecblock( bx1-bmx1, by1-bmy1, bx2+bmx2, by2+bmy2, backcolor, 1 );
			}
		bstate = 1;
		x = orig_x; y = orig_y;
		goto RENDER;
		}
	}


/* now reset so a new legend can be accumulated.. */
if( !noclear ) { NLE = 0; LEavail = 0; }  /* LEavail init added here, scg 10/3/06 */
return( 0 );
}


/* ===================================== */
/* ADD_LEGENT - used by procs to add a legend entry */
int
PL_add_legent( typ, label, tag, parm1, parm2, parm3 )
int typ; 
char *label, *tag, *parm1, *parm2, *parm3;
{
char *errmsg;

errmsg = "Sorry, too much legend content";

LEtype[ NLE ] = typ;

if( typ == 0 ) label = "\n\n";  /* type=none is supposed to create extra space.. */
else if( label[0] != '\0' ) convertnl( label );
	
LElabel[ NLE ] = LEavail;
if( LEavail + strlen( label ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
strcpy( &Ltext[ LEavail ], label );
LEavail += (strlen( label ) + 1);

LEtag[ NLE ] = LEavail;
if( LEavail + strlen( tag ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
strcpy( &Ltext[ LEavail ], tag );
LEavail += (strlen( tag ) + 1);

if( LEavail + strlen( parm1 ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
LEparm1[ NLE ] = LEavail;
strcpy( &Ltext[ LEavail ], parm1 );
LEavail += (strlen( parm1 ) + 1);

if( LEavail + strlen( parm2 ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
LEparm2[ NLE ] = LEavail;
strcpy( &Ltext[ LEavail ], parm2 );
LEavail += (strlen( parm2 ) + 1);
	
if( LEavail + strlen( parm3 ) >= MAXLEGTEXT ) 
	return( Eerr( 8478, errmsg, "" ) );
LEparm3[ NLE ] = LEavail;
strcpy( &Ltext[ LEavail ], parm3 );
LEavail += (strlen( parm3 ) + 1);

NLE++;
return( 0 );
}

/* ========================================= */
/* GET_LEGENT - get parm1 of a legend entry, based on tag ... used for "legend-driven" plot element selection */
char *
PL_get_legent( tag )
char *tag;
{
int i;
for( i = 0; i < NLE; i++ ) if( strcmp( tag, &Ltext[ LEtag[i] ] )==0 ) break;
if( i == NLE ) return( "" ); /* tag not found.. */

return( &Ltext[ LEparm1[i] ] );

/* if( parm1 != NULL ) strcpy( parm1, &Ltext[ LEparm1[i] ] ); */
/* if( parm2 != NULL ) strcpy( parm2, &Ltext[ LEparm2[i] ] ); */
/* if( parm3 != NULL ) strcpy( parm3, &Ltext[ LEparm3[i] ] ); */
/* return( 0 ); */
}
/* ========================================= */
/* GET_LEGENT_RG - get a legend entry based numeric comparison with tag ... used for "legend-driven" plot element selection */
char *
PL_get_legent_rg( val )
double val;
{
int i;
double atof();

for( i = 0; i < NLE; i++ ) if( val >= atof( &Ltext[ LEtag[i] ] ) ) break;
	
if( i == NLE ) return( "" ); /* tag not found.. */

return( &Ltext[ LEparm1[i] ] );

/* if( parm1 != NULL ) strcpy( parm1, &Ltext[ LEparm1[i] ] ); */
/* if( parm2 != NULL ) strcpy( parm2, &Ltext[ LEparm2[i] ] ); */
/* if( parm3 != NULL ) strcpy( parm3, &Ltext[ LEparm3[i] ] ); */
/* return( 0 ); */
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
