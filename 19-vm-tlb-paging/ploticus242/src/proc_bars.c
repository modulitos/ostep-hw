/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC BARS - bargraphs and histograms */

/* Improvements made by
 * Michael D. Beynon (mdb) - beynon@cs.umd.edu
 * 09/23/2001 : mdb - Bug fix: wrong position of cluster bar labels.
 *                  - Bug fix: only use selected records for automatic x value
 *                  - Changes to allow clustered segment bars.
 *
 * Note: These changes allow clustered bars to not require all data on one
 *       line, and instead use select to pick the correct portion for each
 *       clusterpos.
 */

#include "pl.h"
#define MAXSTACKFIELDS 40
#define MAXCLP 200

static int do_label();
static int do_lwl();
static char stacklist[300] = ""; /* lenfields get appended to this list so that "stackfield: *"  can be used */
static int prevclust = 1; /* attempt to be smart about resetting stacklist on cluster change */
static int labelrot = 0;	/* label rotation */


/* =========================== */
int
PLP_bars_initstatic()
{
strcpy( stacklist, "" );
prevclust = 1;
labelrot = 0;
return( 0 );
}


/* =========================== */
int
PLP_bars( )
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char *barcolor, *outline, *labeldetails, *backbox, *crossover, *selectex, *thinbarline;
char *colorlist, *mapurl, *maplabel, *labelselectex, *constantlen, *constantloc;
char *lblpos, *numstrfmt, *labelword, *expandedurl, *expandedlabel;
char legendlabel[256], labelstr[256];
int i, j, ix, ixx, nt, stat, align, lenfield, locfield, do_outline, showvals;
double adjx, adjy, halfw, x, y, y0, xleft, xright, barwidth, fval, cr, laby, taillen;
int stackf[MAXSTACKFIELDS];
int nstackf, ncluster, clusterpos, labelfld, lwl, reverse, stopfld;
int errbars, errlofld, errhifld, reflecterr; int result; int reverseorder, reversespecified, labelmaxlen;
double rlo, rhi, clustsep, ticlen, ytic, errbarmult, minlabel;
int trunc, y_endin, y0_endin, label0val, leftticfld, rightticfld, midticfld, lwl_mustfit;
int taillengiven, barwidthfield, hidezerobars, ibar, colorfield, irow, segmentflag, exactcolorfield;
char buf[256], *colorlp[MAXCLP], axis, baseax;
char rangelo[40], rangehi[40], dcolor[COLORLEN], colorbuf[COLORLEN], acolor[COLORLEN];
char thinbuf[80];

TDH_errprog( "pl proc bars" );


/* initialize */
axis = 'y';

lenfield = locfield = labelfld = errlofld = errhifld = -1;
stopfld = leftticfld = rightticfld = midticfld = -1;
barwidthfield = colorfield = exactcolorfield = -1;

barcolor = "0.7";
outline = "yes";

labeldetails = ""; crossover = ""; backbox = ""; labelword = ""; selectex = "";
labelselectex = ""; thinbarline = ""; colorlist = ""; lblpos = ""; 
mapurl = ""; constantlen = ""; constantloc = ""; maplabel = ""; 
strcpy( legendlabel, "" );

strcpy( rangelo, "" ); strcpy( rangehi, "" );

numstrfmt = "%g";

showvals = 0; nstackf = 0; lwl = 0; errbars = 0; reflecterr = 0; reverseorder = 0; reversespecified = 0;
trunc = 0; label0val = 0; taillengiven = 0; hidezerobars = 0; segmentflag = 0; 
labelrot = 0; lwl_mustfit = 0;

do_outline = 1;
barwidth = 0.0;

ncluster = 1;
clusterpos = 1;
taillen = 0.0;
clustsep = 0.0;
ticlen = 0.02;
errbarmult = 1.0;
minlabel = NEGHUGE;
labelmaxlen = 250;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "lenfield" )==0 ) lenfield = fref( lineval ) -1;
	else if( strcmp( attr, "locfield" )==0 ) locfield = fref( lineval ) -1; 
	else if( strcmp( attr, "axis" )==0 ) axis = lineval[0];
	else if( strcmp( attr, "horizontalbars" )==0 ) axis = 'x';
	else if( strcmp( attr, "color" )==0 || strcmp( attr, "barcolor" )==0 ) barcolor = lineval;
	else if( strcmp( attr, "outline" )==0 ) outline = lineval;
	else if( strcmp( attr, "barwidth" )==0 ) { barwidth = ftokncpy( lineval ); if( PLS.usingcm ) barwidth /= 2.54; }
	else if( strncmp( attr, "stackfield", 10 )==0 ) {
		for( ix = 0, j = 0; j < MAXSTACKFIELDS; j++ ) {
                        if( GL_smember( lineval, "* all" )) strcpy( buf, GL_getok( stacklist, &ix ) );
                        else strcpy( buf, GL_getok( lineval, &ix ) );
			if( buf[0] == '\0' ) break;
			stackf[j] = fref( buf );
			}
		nstackf = j;
		}
	else if( strcmp( attr, "cluster" )==0 ) {
		nt = sscanf( lineval, "%d %s %d", &clusterpos, buf, &ncluster );
		if( nt == 2 ) sscanf( lineval, "%d %d", &clusterpos, &ncluster );
		}
	else if( strcmp( attr, "clustersep" )==0 ) { clustsep = ftokncpy( lineval ); if( PLS.usingcm ) clustsep /= 2.54; }
	else if( strcmp( attr, "crossover" )==0 ) crossover = lineval;
	else if( strncmp( attr, "constantlen", 11 )==0 ) constantlen = lineval;
	else if( strncmp( attr, "constantloc", 11 )==0 ) constantloc = lineval;
	else if( strncmp( attr, "segmentfield", 12 )==0 ) {
		for( i = 0, ix = 0; ; i++ ) {
			strcpy( buf, GL_getok( lineval, &ix ));
			if( buf[0] == '\0' ) break;
			if( i == 0 ) stopfld = fref( buf );
			else if( i == 1 ) { stackf[0] = stopfld; nstackf = 1; stopfld = fref( buf ); }
			}
		segmentflag = 1;
		}
	else if( strncmp( attr, "errbarfield", 11 )==0 ) {
		char fname[2][50];
		errbars = 1;
		nt = sscanf( lineval, "%s %s", fname[0], fname[1] );
		if( strcmp( fname[0], "0" )==0 ) {  /* allow oneway error bars   scg 4/11/04 */
			if( nt == 1 ) { Eerr( 3845, "incorrect errbarfield spec", "" ); errbars = 0; }
			else errlofld = 0; 
			}
		else errlofld = fref( fname[0] ); 
		if( nt == 1 ) reflecterr = 1; /* use -val for lo, +val for hi */
		else 	{
			reflecterr = 0;
			errhifld = fref( fname[1] );
			}
		}
	else if( strncmp( attr, "errbarmult", 10 )==0 ) errbarmult = ftokncpy( lineval );
	else if( strcmp( attr, "tails" )==0 ) { taillen = ftokncpy( lineval ); taillengiven = 1; if( PLS.usingcm ) taillen /= 2.54; }
	else if( strcmp( attr, "showvalues" )==0 ) showvals = getyn( lineval );
	else if( strcmp( attr, "numbersformat" )==0 ) numstrfmt = lineval;
	else if( strcmp( attr, "labelzerovalue" )==0 ) label0val = getyn( lineval );
	else if( strcmp( attr, "minlabel" )==0 ) minlabel = ftokncpy( lineval );
	else if( strcmp( attr, "truncate" )==0 ) trunc = getyn( lineval );
	else if( strcmp( attr, "labeldetails" )==0 ) labeldetails = lineval;
	else if( strcmp( attr, "backbox" )==0 ) backbox = lineval;
	else if( strcmp( attr, "labelfield" )==0 ) labelfld = fref( lineval ) - 1;
	else if( strcmp( attr, "labelword" )==0 ) labelword = lineval;
	else if( strcmp( attr, "thinbarline" )==0 ) thinbarline = lineval;
	else if( strcmp( attr, "leftticfield" )==0 ) leftticfld = fref( lineval ) -1;
	else if( strcmp( attr, "rightticfield" )==0 ) rightticfld = fref( lineval ) -1;
	else if( strcmp( attr, "midticfield" )==0 ) midticfld = fref( lineval ) -1;
	else if( strcmp( attr, "ticlen" )==0 ) ticlen = ftokncpy( lineval );
	else if( strcmp( attr, "reverseorder" )==0 ) { reverseorder = getyn( lineval ); reversespecified = 1; } 
	else if( strcmp( attr, "hidezerobars" )==0 )  hidezerobars = getyn( lineval );

	else if( strcmp( attr, "barsrange" )==0 ) sscanf( lineval, "%s %s", rangelo, rangehi );
	else if( strcmp( attr, "colorlist" )==0 ) colorlist = lineval;
	else if( strcmp( attr, "colorfield" )==0 ) colorfield = fref( lineval ) -1;
	else if( strcmp( attr, "exactcolorfield" )==0 ) exactcolorfield = fref( lineval ) -1;
	else if( strcmp( attr, "longwayslabel" )==0 ) lwl = getyn( lineval );
	else if( strcmp( attr, "labelmustfit" )==0 ) {
		if( strcmp( lineval, "omit" )==0 ) lwl_mustfit = 1;
		else if( strcmp( lineval, "truncate" )==0 ) lwl_mustfit = 2;
		else lwl_mustfit = 0;
		}
	else if( strcmp( attr, "labelmaxlen" )==0 ) labelmaxlen = itokncpy( lineval );
	else if( strncmp( attr, "labelrot", 8 )==0 ) labelrot = itokncpy( lineval );
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "labelselect" )==0 ) labelselectex = lineval;
	else if( strcmp( attr, "legendlabel" )==0 ) { strncpy( legendlabel, lineval, 255 ); legendlabel[255] = '\0'; }
	else if( strcmp( attr, "labelpos" )==0 ) lblpos = lineval;
	else if( strcmp( attr, "barwidthfield" )==0 ) barwidthfield = fref( lineval ) -1;
	else if( strcmp( attr, "clickmapurl" )==0 ) mapurl = lineval; 
	else if( strcmp( attr, "clickmaplabel" )==0 ) maplabel = lineval;
	else if( strcmp( attr, "clickmaplabeltext" )==0 ) maplabel = getmultiline( lineval, "get" );
	else Eerr( 1, "attribute not recognized", attr );
	}


/* -------------------------- */
/* overrides and degenerate cases */
/* -------------------------- */
if( axis == 'y' ) baseax = 'x';
else baseax = 'y';

if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() ) return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

if( locfield > Nfields ) return( Eerr( 52, "locfield out of range", "" ) );
if( lenfield > Nfields ) return( Eerr( 52, "lenfield out of range", "" ) );
if( lenfield < 0 && !segmentflag && constantlen[0] == '\0' ) return( Eerr( 2805, "Either lenfield, segmentfields, or constantlen must be defined", ""));

if( stopfld >= 0 ) {
	if( nstackf > 1 )  { Eerr( 2984, "stackfield may not be used with segments", "" ); nstackf=0; }
	}

if( labelword[0] != '\0' ) showvals = 1;
if( showvals && labelword[0] == '\0' ) labelword = "@N";

if( locfield == lenfield && locfield >= 0 ) Eerr( 2479, "Warning, locfield same as lenfield", "" );

for( i = 0; i < nstackf; i++ ) {
	if( (lenfield+1) == stackf[i] ) Eerr( 2479, "Warning, lenfield same as a stackfield", "" );
	if( (locfield+1) == stackf[i] ) Eerr( 2479, "Warning, locfield same as a stackfield", "" );
	}

if( axis == 'x' && !reversespecified ) reverseorder = 1;

if( strncmp( legendlabel, "#usefname", 9 )==0 ) getfname( lenfield+1, legendlabel ); /* legendlabel[256] */

if( segmentflag ) lwl = 1;  /* when doing floating segment bars, default to use labels that are centered within the bar - scg 5/8/06 */

/* if doing error bars, and 'color' given but 'thinbarline' not given, use 'color' to build a thinbarline spec ... added scg 3/10/09 */
if( errbars && thinbarline[0] == '\0' ) { sprintf( thinbuf, "color=%s", barcolor ); thinbarline = thinbuf; }  

if( thinbarline[0] != '\0' && barwidth != 0.0 ) Eerr( 2487, "Warning, 'barwidth' not useful in this context, control bar attributes using 'thinbarline'", "" );


/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */
if( baseax == 'y' ) Eflip = 1;

if( rangelo[0] != '\0' ) rlo = Econv( baseax, rangelo );
else rlo = Elimit( baseax, 'l', 's' );
if( rangehi[0] != '\0' ) rhi = Econv( baseax, rangehi );
else rhi = Elimit( baseax, 'h', 's' );

/* maintain stacklist */
if( clusterpos != prevclust ) {
	PL_resetstacklist();
	if( !segmentflag ) nstackf = 0; /* needed for current bar */
	}
prevclust = clusterpos;
sprintf( buf, "%d ", lenfield+1 );
strcat( stacklist, buf );


if( barwidth > 0.0 ) halfw = barwidth * 0.5;
else	{
	/* automatic bar width determination... do this even with thinbarline for positioning purposes below.. */
	if( ncluster <= 1 ) halfw = ( Ea( X, 1.0 ) - Ea( X, 0.0 ) ) * 0.4;
	else if( ncluster > 1 ) halfw = (( Ea( X, 1.0 ) - Ea( X, 0.0 ) ) * 0.4)/ (double)ncluster;
	if( halfw > 0.5 ) halfw = 0.1; /* sanity  - to prevent huge bars */
	}

/* if bar width turns out to be very thin, go to line-based bars for better results....  added scg 3/10/09 */
if( halfw < 0.012 && thinbarline[0] == '\0' ) { sprintf( thinbuf, "color=%s", barcolor ); thinbarline = thinbuf; }   

if( outline[0] == '\0' || strncmp( outline, "no", 2 )==0 ) do_outline = 0;
else do_outline = 1;


if( crossover[0] == '\0' ) cr = Elimit( axis, 'l', 's' );
else cr = Econv( axis, crossover );
if( cr < Elimit( axis, 'l', 's' )) cr = Elimit( axis, 'l', 's' );  /* be sure crossover is in range .. added scg 8/25/04 */
if( cr > Elimit( axis, 'h', 's' )) cr = Elimit( axis, 'h', 's' );  /* be sure crossover is in range .. added scg 8/25/04 */


/* parse colorlist if any */
if( colorlist[0] != '\0' ) {
	/* initialize all pointers to default color.. */
	strcpy( dcolor, barcolor );
	for( i = 0; i < MAXCLP; i++ ) colorlp[i] = dcolor;
	ix = 0; ixx = 0;
	i = 0;
	while( 1 ) {
		strcpy( acolor, GL_getok( colorlist, &ix ) ); 
		if( acolor[0] == '\0' ) break;
		if( atoi( acolor ) > 0 && atoi( acolor ) < MAXCLP ) {
			colorlp[ atoi(acolor) - 1 ] = &colorlist[ix];
			GL_getok( colorlist, &ix );
			}
		else if( i < MAXCLP ) {
			colorlp[ i ] = &colorlist[ ixx ];
			i++;
			}
		ixx = ix;
		}
	}

linedet( "outline", outline, 0.5 );
PLG_forcecolorchg();


if( thinbarline[0] != '\0' && strncmp( thinbarline, "no", 2 ) != 0 ) linedet( "thinbarline", thinbarline, 0.3 );



if( errbars && !taillengiven ) taillen = 0.2; /* set a default taillen for errorbars */


/* ---------------- */
/* loop through current data set, draw bars.. */
/* ---------------- */
ibar = -1;
for( irow = 0; irow < Nrecords; irow++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, irow, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( result == 0 ) continue; /* reject */
                }
	ibar++;


	if( lenfield >= 0 ) {
		y = fda( irow, lenfield, axis );
		if( Econv_error() ) { conv_msg( irow, lenfield, "lenfield" ); continue; }
		}
	else if( constantlen[0] != '\0' ) y = Econv( axis, constantlen );

	if( constantloc[0] != '\0' ) x = Econv( baseax, constantloc );
	else if( locfield < 0 ) {
		if( reverseorder ) x = (Elimit( baseax, 'h', 's' ) - 1.0) - (double)ibar;
		else x = (double)ibar+1;
		if( x > Elimit( baseax, 'h', 's' ) ) {
			fprintf( PLS.errfp, "bars warning, skipping bar# %d, loc is out of range\n", ibar+1 );
			continue; /* out of range hi */
			}
		}	
	else 	{
		x = fda( irow, locfield, baseax );
		if( Econv_error() ) { conv_msg( irow, locfield, "locfield" ); continue; }
		if( x < rlo ) continue;  /* out of range low */
		if( x > rhi ) continue;  /* out of range high */
		}

	/* y0 = Elimit( axis, 'l', 's' ); */
	if( nstackf > 0 && !segmentflag ) y0 = 0.0; /* added scg 11/27/01 */
	else y0 = cr;


	/* if barwidthfield was specified, set bar width now.. */
	if( barwidthfield >= 0 ) {
		double bw;
		bw = fda( irow, barwidthfield, axis );
		halfw = (Ea( X, bw ) - Ea( X, 0.0 )) * 0.5;
		if( Econv_error() ) { 
			conv_msg( irow, barwidthfield, "barwidthfield" ); 
			halfw = 0.05; 
			}
		}

	if( nstackf > 0 ) /* stacking */
		for( j = 0; j < nstackf; j++ ) {
			fval = fda( irow, stackf[j]-1, axis );
			if( segmentflag ) fval -= cr; /* normalize? */
			y0 += fval;
			if( !segmentflag ) y += fval; /* condition added scg 9/26/03 .. because y is undefined at this point */
			}


	if( ncluster > 1 ) {   /* clustering - move sideways a little.. */
		xleft = Ea( X, x ) - ((halfw+clustsep) * (double)(ncluster));
		xleft += clustsep;
		if( baseax == Y ) xleft += ((halfw+clustsep) * (ncluster-clusterpos)*2.0);
		else xleft += ((halfw+clustsep) * (clusterpos-1)*2.0);
		xright = xleft + (halfw*2.0);
		}
	else	{
		xleft = Ea( X, x) - halfw;
		xright = Ea( X, x) + halfw;
		}

	y_endin = y0_endin = 1;

	if( segmentflag ) { /* set y from stopfld; bar start is done via stacking, above */
		y = fda( irow, stopfld-1, axis );
		}
	if( errbars ) { /* set y and y0 as offsets from original y */
		double eblen;
		if( errlofld == 0 ) { y0 = y; y0_endin = 0; }
		else 	{
			eblen = (fda( irow, errlofld-1, axis ) * errbarmult);
			if( y < cr ) y0 = y + eblen;
			else y0 = y - eblen;
			}
		if( reflecterr ) eblen = fda( irow, errlofld-1, axis ) * errbarmult;
		else eblen = fda( irow, errhifld-1, axis ) * errbarmult;
		if( y < cr ) y -= eblen; /* downward/leftward bar.. reverse direction */
		else y += eblen;      /* normal */
		}

	/* catch units errors for stopfld and errflds.. */
	if( segmentflag && Econv_error() ) {conv_msg( irow, stopfld, "segmentfields" );continue;}
	if( errbars && Econv_error() ){conv_msg( irow, stopfld, "errbarfields" );continue;} 

	/* null-length bars.. skip out.. scg 11/29/00 */
	if( hidezerobars && y == y0 ) continue;

	/* truncate to plotting area.. scg 5/12/99 */
	if( trunc ) {

		if( y0 <= Elimit( axis, 'l', 's' ) && y < Elimit( axis, 'l', 's' ) ) {
			fprintf( PLS.errfp, "warning, bar completely out of %c plotting area\n", axis );
			continue; /* skip entirely */
			}
		if( y0 >= Elimit( axis, 'h', 's' ) && y > Elimit( axis, 'h', 's' ) ) {
			fprintf( PLS.errfp, "warning, bar completely out of %c plotting area\n", axis );
			continue; /* skip entirely */
			}

		if( !Ef_inr( axis, y0 ) ) {
			if( y0 < y ) y0 = Elimit( axis, 'l', 's' );
			else y0 = Elimit( axis, 'h', 's' );
			y0_endin = 0;
			}
		if( !Ef_inr( axis, y ) ) {
			if( y0 < y ) y = Elimit( axis, 'h', 's' );
			else y = Elimit( axis, 'l', 's' );
			y_endin = 0;
			}
		}

	/* if colorfield used, get color.. */
	if( colorfield >= 0 )  barcolor = PL_get_legent( da( irow, colorfield ) );
	else if( exactcolorfield >= 0 ) barcolor = da( irow, exactcolorfield );


	/* if colorlist used, get color.. */
	if( colorlist[0] != '\0' && ibar < MAXCLP ) {
		sscanf( colorlp[ibar], "%s", colorbuf );
		barcolor = colorbuf;
		}

	/* now do the bar.. */

	/* allow @field substitutions into url */
	if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) {
		expandedurl = PL_bigbuf;
		expandedlabel = &PL_bigbuf[2000];
		do_subst( expandedurl, mapurl, irow, URL_ENCODED );
		do_subst( expandedlabel, maplabel, irow, NORMAL );
		}


	/* if thinbarline specified, or if doing error bars, render bar as a line */
	if( ( thinbarline[0] != '\0' && strncmp( thinbarline, "no", 2 )!= 0 ) || errbars ) { 
		Emov( xleft+halfw, Ea( Y, y0 ) ); 
		Elin( xleft+halfw, Ea( Y, y ) ); 
		}

  	/* otherwise, render bar as a rectangle */
  	else 	{
		Ecblock( xleft, Ea( Y, y0 ), xright, Ea( Y, y ), barcolor, 0 ); 

		if( do_outline ) {   /* render bar outline.. but no outline where truncated.. added scg 5/11/06 */
			Emov( xleft, Ea( Y, y0 ) );
			Elin( xleft, Ea( Y, y ) );
			if( y_endin ) Elin( xright, Ea( Y, y ) );
			else Emov( xright, Ea( Y, y ) );
			Elin( xright, Ea( Y, y0 ) );
			if( y0_endin ) Elin( xleft, Ea( Y, y0 ) );
			}

		if( PLS.clickmap && (mapurl[0] != '\0' || maplabel[0] != '\0' )) {
			if( Eflip ) clickmap_entry( 'r', expandedurl, 0, Ea( Y, y0 ), xleft, Ea( Y, y ), xright, 0, 0, expandedlabel );
			else clickmap_entry( 'r', expandedurl, 0, xleft, Ea( Y, y0 ), xright, Ea( Y, y ), 0, 0, expandedlabel );
			}

		}
  
  	/* do tics if requested */  /* don't do if trunc && outside area - scg 11/21/00 */ 
	/* Bug fix - ticks not being drawn at bars when truncating was not switched on.
         * Supplied by Michael Rausch (mr@netadair.de) date: 04 Jun 2001 */
  	if( leftticfld >= 0 ) { 
  		ytic = fda( irow, leftticfld, axis );
  		if( !Econv_error() && ( !trunc || Ef_inr( axis, ytic ) ) ) { 
  			Emov( (xleft+halfw), Ea(Y,ytic) ); 
  			Elin( (xleft+halfw)-ticlen, Ea(Y,ytic) ); 
  			}
  		}
  	if( rightticfld >= 0 ) { 
  		ytic = fda( irow, rightticfld, axis );
  		if( !Econv_error() && ( !trunc || Ef_inr( axis, ytic ) ) ) {
  			Emov( (xleft+halfw), Ea(Y,ytic) ); 
  			Elin( (xleft+halfw)+ticlen, Ea(Y,ytic) ); 
  			}
  		}
  	if( midticfld >= 0 ) { 
  		ytic = fda( irow, midticfld, axis );
  		if( !Econv_error() && ( !trunc || Ef_inr( axis, ytic ) ) ) { 
  			Emov( (xleft+halfw)-(ticlen/2.0), Ea(Y,ytic) ); 
  			Elin( (xleft+halfw)+(ticlen/2.0), Ea(Y,ytic) ); 
  			}
		}


	/* do tails if requested */
	if( taillen > 0.0 ) {
		double g, h;
		g = xleft + ((xright-xleft)  / 2.0);
		h = taillen / 2.0;
		if( y_endin ) { Emov( g-h, Ea(Y,y) ); Elin( g+h, Ea(Y,y) ); }
		if( y0_endin ) { Emov( g-h, Ea(Y,y0) ); Elin( g+h, Ea(Y,y0) ); }
		}

	}



/* ---------------- */
/* now add labels if any */
/* ---------------- */

if( showvals || labelfld >= 0 ) { 
	textdet( "labeldetails", labeldetails, &align, &adjx, &adjy, -3, "R", 1.0 );
	if( adjy == 0.0 ) adjy = 0.02; /* so label is a little above end of bar */
	if( align == '?' ) align = 'C';
	ibar = -1;
	for( i = 0; i < Nrecords; i++ ) {


		if( selectex[0] != '\0' ) { /* added 8/23/01 - process against selection condition if any.. */
                	stat = do_select( selectex, i, &result );
                	if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                	if( result == 0 ) continue; /* reject */
                	}
		ibar++;

		if( labelselectex[0] != '\0' ) { /* process against label selection condition if any.. added scg 5/11/06 */
                	stat = do_select( labelselectex, i, &result );
                	if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                	if( result == 0 ) continue; /* reject */
			}

		if( lenfield >= 0 ) {
			y = fda( i, lenfield, axis );
			if( Econv_error() ) continue; /* don't bother to label bad values */
			if( !label0val && GL_close_to( y, cr, 0.000001 )) continue; /* don't label 0 */
			if( y < minlabel ) continue;   /* suppress labels for small bars , added 5/4/04, thanks to Jessika Feustel */
			}
			
		if( constantloc[0] != '\0' ) x = Econv( baseax, constantloc );
		else if( locfield < 0 ) {
		        if( reverseorder ) x = (Elimit( baseax, 'h', 's' ) - 1.0) - (double)ibar;
			else x = (double)ibar+1;
			}	
		else 	{
			x = fda( i, locfield, baseax );
			if( Econv_error() ) continue; /* don't bother to label bad values - added scg 8/10/05 */
			if( x < rlo ) continue; /* out of range low */
			if( x > rhi ) continue; /* out of range high */
			}


		/* compose label.. */ 
		if( labelfld >= 0 ) strcpy( labelstr, da( i, labelfld ) );
		else 	{
			if( segmentflag ) y = fda( i, stopfld-1, axis ); /* get y now.. scg 9/27/04 */
			strcpy( labelstr, labelword );  /* labelstr[256] */
			}
		stat = Euprint( buf, axis, y, numstrfmt );
		GL_varsub( labelstr, "@N", buf ); /* labelstr[256] */


		/* check / truncate length.. */
		if( strlen( labelstr ) > labelmaxlen ) {
			labelstr[ labelmaxlen+2 ] = '\0';
			labelstr[ labelmaxlen+1 ] = '.';
			labelstr[ labelmaxlen ] = '.';
			}

		fval = cr; /* needed in case we want to center long text label along len of bar */
		if( nstackf > 0 ) {   /* stacking affects label placement */
			double ff;
			for( j = 0; j < nstackf; j++ ) {
				ff = fda( i, stackf[j]-1, axis ); 
				if( !segmentflag ) fval += ff;	/* scg 4/28/04 */
				else fval = ff;
				/* fval is used below to center longwise labels */
				if( segmentflag ) y += (ff - cr);  
				else y += ff;	
				}
			}

		if( segmentflag ) {  /* set y from stopfld; bar start is done via stacking, above */
			y = fda( i, stopfld-1, axis );
			if( Econv_error() ) continue;
			}

		if( errbars ) { /* set y and y0 as offsets from original y */
			if( errlofld == 0 ) y0 = y;
			else y0 = y - fda( i, errlofld+1, axis );
			if( reflecterr ) y += fda( i, errlofld-1, axis );
			else y += fda( i, errhifld+1, axis );
			if( Econv_error() ) continue;
			}


		/* truncate to plotting area.. scg 5/12/99 */
		if( trunc ) {

			/* if bar completely out of plotting area, omit  - added scg 8/10/05 */
			if( y < Elimit( axis, 'l', 's' ) ) continue; 

			if( lwl ) {  /* longways labels.. revise bar start & stop so label is properly centered - added scg 5/10/06 */
				if( fval > Elimit( axis, 'h', 's' )) continue;  /* bar completely off hi end.. omit */
				if( y > Elimit( axis, 'h', 's' )) y = Elimit( axis, 'h', 's' ); 
				if( fval < Elimit( axis, 'l', 's' )) fval = Elimit( axis, 'l', 's' ); 
				}
			else if( y > Elimit( axis, 'h', 's' ) ) continue;  /* for regular labels, if top of bar is off, don't show it */
			
			if( !Ef_inr( axis, y ) ) {
				if( y > Elimit( axis, 'h', 's' ) )  y = Elimit( axis, 'h', 's' );
				else laby = y = Elimit( axis, 'l', 's' );
				}
			}

		if( y < cr ) { 
			laby = Ea( Y, y ) + (adjy*(-1.0));
			if( !Eflip ) laby -= Ecurtextheight;
			reverse = 1;
			}
		else 	{
			laby = (Ea( Y, y )+adjy);
			reverse = 0;
			}


		/* if explicit label position given, use it.. */
		if( lblpos[0] != '\0' ) Eposex( lblpos, axis, &laby );


		if( ncluster > 1 ) { /* if clusters, move sideways a little bit.. */
			x = Ea( X, x ) - ((halfw+clustsep) * (double)(ncluster));
			x += clustsep;
		        if( baseax == Y ) x += ((halfw+clustsep) * (ncluster-clusterpos)*2.0);
			else x += ((halfw+clustsep) * (clusterpos-1)*2.0);
			x += halfw;
			if( lwl ) do_lwl( labelstr, x+adjx, Ea(Y,y)+adjy, Ea(Y,fval)+adjy, align, reverse, lwl_mustfit );
			else do_label( labelstr, x+adjx, laby, align, backbox, reverse );
			}
		else 	{
			if( lwl ) do_lwl( labelstr, Ea(X,x)+adjx, Ea(Y,y)+adjy, Ea(Y,fval)+adjy, align, reverse, lwl_mustfit );
			else do_label( labelstr, Ea(X,x)+adjx, laby, align, backbox, reverse );
			}
		}
	}



if( legendlabel[0] != '\0' ) {
	if( errbars || ( thinbarline[0] != '\0' && strncmp( thinbarline, "no", 2 )!= 0) ) 
		PL_add_legent( LEGEND_LINE, legendlabel, "", thinbarline, "", "" );

	else PL_add_legent( LEGEND_COLOR, legendlabel, "", barcolor, "", "" );
	}

if( baseax == 'y' ) Eflip = 0;

return( 0 );
}


/* ====================== */
static int
do_label( s, x, y, align, backbox, reverse )
char *s;
double x, y;
char align;
char *backbox;
int reverse;
{
double halfbox;
char tcolor[40];

strcpy( tcolor, Enextcolor ); /* remember text color that has been set; backing box could change it below..  scg 3/14/06 */

halfbox = ((strlen( s ) * Ecurtextwidth) / 2.0) + 0.01;

convertnl( s ); /* added scg 3/8 */


if( Eflip ) {
	if( reverse && align == 'L' ) {
		align = 'R';
		/* no backing necessary */
		Emov( x-0.02, y );
		}
	else if( align != 'R' ) {
		align = 'L'; 
		/* no backing box necessary, past end of bar */
		Emov( x-0.02, y+0.03 ); 
		}
	else if( reverse && align == 'R' ) {
		align = 'L';
		if( backbox[0] != '\0' )
			Ecblock( x-0.1, y+0.03,
				x+(Ecurtextheight*0.8), y+(halfbox*2)+0.03, backbox, 0 );
		Emov( x-0.02, y+0.03 );
		}
	else 	{ /* R align */
		if(  backbox[0] != '\0' )
			Ecblock( x-0.1, (y-(halfbox*2))-0.03, 
				x+(Ecurtextheight*0.8), y-0.03, backbox, 0 );
		Emov( x-0.02, y-0.03 ); 
		}
	}

else 	{
	if( backbox[0] != '\0' ) 
		Ecblock( x-halfbox, y-0.01, x+halfbox, y+(Ecurtextheight*0.8), backbox, 0 );
	Emov( x, y );
	}

Ecolor( tcolor ); /* be sure to use text color */

Edotext( s, align );

return( 0 );
}

/* ============================ */
static int
do_lwl( s, x, y, y0, align, reverse, mustfit )
char *s;
double x, y, y0;
char align;
int reverse;
int mustfit;
{
double y1, y2;
int nlines, maxlen;

if( y0 < y ) { y1 = y; y2 = y0; }
else { y1 = y0; y2 = y; }

convertnl( s );
measuretext( s, &nlines, &maxlen );


if( mustfit == 1 || ( mustfit == 2 && nlines > 1) ) { /* label too long- omit.. added scg 5/11/06 */
	if( maxlen*Ecurtextwidth > (y1-y2) ) return( 0 );  
	}
else if( mustfit == 2 && nlines == 1 ) { /* truncate label to fit.. added scg 5/11/06 */
	if( maxlen*Ecurtextwidth > (y1-y2) ) {
		int nchars;
		nchars = (int)( (y1-y2)/ Ecurtextwidth);
		if( nchars > 6 ) {
			s[ nchars ] = '\0';
			s[ nchars-1 ] = '.';
			s[ nchars-2 ] = '.';
			}
		else s[nchars] = '\0';
		measuretext( s, &nlines, &maxlen );
		}
	}

if( Eflip ) {
	x -= Ecurtextheight*0.4;
	x += (((nlines-1)*0.5)*Ecurtextheight);
	}
else 	{
	x += Ecurtextheight*0.4;
	x -= (((nlines-1)*0.5)*Ecurtextheight);
	}

if( reverse ) {
	if( align == 'L' ) align = 'R';
	else if( align == 'R' ) align = 'L';
	}

if( align == 'C' ) Emov( x, y2+((y1-y2)/2.0) );
else if( align == 'L' ) Emov( x, y2 );
else Emov( x, y1 );
if( !Eflip ) Etextdir(90+labelrot);
else if( labelrot != 0 ) Etextdir( labelrot );
Edotext( s, align );
/* if( !Eflip ) */ 
Etextdir(0);
return( 0 );
}

/* ================================ */
/* RESETSTACKLIST - called when a new areadef is done, or 
   when beginning a new member-of-cluster */

int
PL_resetstacklist()
{
strcpy( stacklist, "" );
return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
