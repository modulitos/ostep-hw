/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC LINEPLOT - draw a lineplot */

#include "pl.h"
#define MAXALT 200
#define MOVE 0
#define LINE 1
#define PATH 2

static int dblcompare( const void *a, const void *b );
static int placenum();

int
PLP_lineplot()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char buf[256], numstr[100], symcode[80];
int i, j, k, stat, align, accum;
int npoints, result, nalt, altlist[MAXALT+2], anyvalid, realrow, sortopt;
int yfield, xfield, ptlabelfield;
int stairstep, donumbers, dopoints, instancemode, gapmissing, ingap, clipping, firstpt, groupmode, relax_xrange, fillmode;

char *linedetails, *labeldetails, *shownums, *numstrfmt, *pointsym, *ptlabeldetails, *fillcolor;
char *selectex, *forcelastx, *legsamptyp, *altsym, *altwhen;
char linelabel[256], legendlabel[256];

double adjx, adjy, linestart, linestop, x, y, lastseglen, lastx, lasty, radius, ptlblstart, ptlblstop, sob, linxstart;
double f, sum, cr, firstx, firsty, typical_interval;
double fillbleed;

TDH_errprog( "pl proc lineplot" );

/* initialize */
yfield = -1; xfield = -1;
ptlabelfield = 0;
accum = 0; stairstep = 0; fillmode = 0; instancemode = 0; groupmode = 0; 
gapmissing = 0; clipping = 0; firstpt = 0; sortopt = 0; relax_xrange = 0;
labeldetails = ""; shownums = ""; ptlabeldetails = ""; linedetails = ""; pointsym = "";
selectex = ""; forcelastx = ""; altsym = ""; altwhen = "";
strcpy( linelabel, "" );
strcpy( legendlabel, "" );
numstrfmt = "%g";
fillcolor = "gray(0.8)";
legsamptyp = "symbol";
linestart = EDXlo; linestop = EDXhi;
ptlblstart = EDXlo; ptlblstop = EDXhi;
linxstart = EDXlo;
sob = 0.0;
lastseglen = 0.0;
typical_interval = -99.0;
fillbleed = 0.0;



/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "yfield" )==0 ) yfield = fref( lineval ) - 1;
	else if( strcmp( attr, "xfield" )==0 ) xfield = fref( lineval ) - 1;
	else if( strcmp( attr, "ptlabelfield" )==0 ) ptlabelfield = fref( lineval ) - 1; 
	else if( strcmp( attr, "linedetails" )==0 ) linedetails = lineval;
	else if( strcmp( attr, "label" )==0 ) { 
		tokncpy( linelabel, lineval, 256 ); 
		convertnl( linelabel );   /* linelabel[256] */
		}
	else if( strcmp( attr, "labeldetails" )==0 ) labeldetails = lineval;
	else if( strcmp( attr, "legendlabel" )==0 ) tokncpy( legendlabel, lineval, 256 );
	else if( strcmp( attr, "linerange" )==0 ) getrange( lineval, &linestart, &linestop, 'x', EDXlo, EDXhi );
	else if( strcmp( attr, "xstart" )==0 ) { linxstart = Econv( X, lineval ); if( Econv_error() ) linxstart = EDXlo; }
	else if( strcmp( attr, "firstpoint" )==0 ) {
		char xstr[80], ystr[80];
		sscanf( lineval, "%s %s", xstr, ystr );
		firstpt = 1;
		firstx = Econv( X, xstr); if( Econv_error() ) firstpt = 0;
		firsty = Econv( X, ystr ); if( Econv_error() ) firstpt = 0;
		}
	else if( strcmp( attr, "accum" )==0 ) accum = getyn( lineval );
	else if( strcmp( attr, "stairstep" )==0 ) stairstep = getyn( lineval );
	else if( strcmp( attr, "instancemode" )==0 ) instancemode = getyn( lineval );
	else if( strcmp( attr, "groupmode" )==0 ) groupmode = getyn( lineval );
	else if( strcmp( attr, "gapmissing" )==0 ) {
		if( strncmp( lineval, "y", 1 )==0 ) gapmissing = 1;
		else if( strcmp( lineval, "small" )==0 ) gapmissing = 2;
		else if( strcmp( lineval, "auto" )==0 ) gapmissing = 3;
		else if( strcmp( lineval, "autosmall" )==0 ) gapmissing = 4;
		else if( strcmp( lineval, "autozero" )==0 ) gapmissing = 5;
		else gapmissing = 0;
		}
	else if( strcmp( attr, "clip" )==0 ) clipping = getyn( lineval );
	else if( strcmp( attr, "sort" )==0 ) sortopt = getyn( lineval );
	else if( strcmp( attr, "relax_xrange" ) == 0 ) relax_xrange = getyn( lineval );
	else if( strcmp( attr, "lastseglen" )==0 ) Elenex( lineval, X, &lastseglen );
	else if( strcmp( attr, "numbers" )==0 ) shownums = lineval;
	else if( strcmp( attr, "numbersformat" )==0 ) numstrfmt = lineval;
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "altsymbol" )==0 ) altsym = lineval;
	else if( strcmp( attr, "altwhen" )==0 ) altwhen = lineval;
	else if( strcmp( attr, "lastx" )==0 ) forcelastx = lineval;
	else if( strcmp( attr, "pointsymbol" )==0 ) pointsym = lineval;
	else if( strcmp( attr, "ptlabeldetails" )==0 ) ptlabeldetails = lineval;
	else if( strcmp( attr, "ptlabelrange" )==0 ) {
		if( strcmp( lineval, "last" )==0 ) ptlblstart = EDXhi-0.1;
		else if( strcmp( lineval, "first" )==0 ) { ptlblstart = EDXlo; ptlblstop = EDXhi-0.000001; }
		else getrange( lineval, &ptlblstart, &ptlblstop, 'x', EDXlo, EDXhi );
		}
	else if( strcmp( attr, "stairoverbars" )==0 ) {
		if( strncmp( lineval, "y", 1 )==0 ) { sob = 0.5; stairstep = 1; /* implied */ }
		else sob = 0.0;
		}
	else if( strcmp( attr, "fill" )==0 ) { 
		fillcolor = lineval;
		if( fillcolor[0] != '\0' ) fillmode = 1;
		else fillmode = 0;
		}
	else if( strcmp( attr, "fillbleed" )==0 ) fillbleed = ftokncpy( lineval );
	else if( strcmp( attr, "legendsampletype" )==0 ) legsamptyp = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
/* -------------------------- */
if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() ) 
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );


if( (yfield < 0 || yfield >= Nfields ) && !instancemode ) return( Eerr( 601, "yfield out of range", "" ) );
if( xfield >= Nfields ) return( Eerr( 601, "xfield out of range", "" ) );
if( yfield >= 0 && instancemode ) {
	Eerr( 4729, "warning, turning instancemode off since yfield specified", "" );
	instancemode = 0;
	}
 
if( groupmode && ptlabelfield ) {
	Eerr( 4729, "warning, turning ptlabelfield off since groupmode specified", "" );
	ptlabelfield = 0;
	}
	
if( strncmp( legendlabel, "#usefname", 9 )==0 ) {
	if( instancemode ) getfname( xfield+1, legendlabel );  /* legendlabel[256] */
	else getfname( yfield+1, legendlabel );  /* legendlabel[256] */
	}



/* now do the plotting work.. */
/* -------------------------- */

donumbers = 1;
if( GL_slmember( shownums, "no*" ) || shownums[0] == '\0' ) donumbers = 0;

dopoints = 1;
if( GL_slmember( pointsym, "no*" ) || pointsym[0] == '\0' ) dopoints = 0;


/* put all values into PLV vector, doing accumulation if required.. */
j = 0;
f = linxstart;
nalt = 0;

for( i = 0; i < Nrecords; i++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, i, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( result == 0 ) continue; /* reject */
                }
	if( altwhen[0] != '\0' ) { /* check altwhen condition.. */
                stat = do_select( altwhen, i, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", altwhen ); continue; }
                if( result == 1 && nalt < MAXALT ) {
			/* altlist[nalt] = j/2; */
			altlist[nalt] = j/3;
			nalt++;
			}
                }
		


	/* X */
	if( xfield < 0 ) {
		PLV[j] = f + sob;
		f += 1.0;
		}
	else 	{
		PLV[j] = fda( i, xfield, X ) + sob;
		if( Econv_error() ) { 
			conv_msg( i, xfield, "xfield" ); 
			continue;
			}
		}

	j++; 

	/* Y */
	if( instancemode ) PLV[j] = 1.0;
	else 	{
		PLV[j] = fda( i, yfield, Y );
		if( Econv_error() ) { 
			conv_msg( i, yfield, "yfield" ); 
			PLV[j] = NEGHUGE;
			/* continue; removed scg 5/19/99 */
			}
		}

	j++;

	PLV[j] = (double)i;
	j++;

	if( j >= PLVsize-1 ) {
		Eerr( 3579, "Sorry, too many curve points, curve truncated (raise using -maxvector)", "" );
		break;
		}
	}

npoints = j / 3;


/* sort if required.. */  /* added 4/22/02 */
if( sortopt ) {
        if( PLS.debug ) fprintf( PLS.diagfp, "sorting points for line\n" );
        qsort( PLV, npoints, sizeof(double)*3, dblcompare );
        }



/* process for groupmode.. */
if( groupmode && xfield >= 0 ) for( i = 0; i < npoints; i++ ) {

	for( k = i+1; k < npoints; k++ ) {
                		
		if( dat3d(i,0) == dat3d(k,0) ) {
			if( instancemode ) y = 1.0;
			else y = dat3d( k, 1 );
			dat3d( k, 1 ) = NEGHUGE; /* rub out the additional instance.. */
			if( y > NEGHUGE+1 ) (dat3d( i, 1 )) += (y);
			}
		else 	{
			i = k-1;     /* back off.. */
			break;
			}
		}
	}
/* fprintf( stderr, "after grouping\n" );
 * for( i = 0; i < npoints; i++ ) fprintf( stderr, "%g %g %g\n", dat3d(i,0), dat3d(i,1), dat3d(i,2 ) );
 */


/* process for accum.. */
if( accum ) {
	sum = 0.0;
	for( i = 0; i < npoints; i++ ) {

		if( dat3d( i, 1 ) > (NEGHUGE+1) ) {
			sum += (dat3d( i, 1 )); 
			(dat3d( i, 1 )) = sum;
			}
		}
   	}

/* fprintf( stderr, "after accum\n" );
 * for( i = 0; i < npoints; i++ ) fprintf( stderr, "%g %g %g\n", dat3d(i,0), dat3d(i,1), dat3d(i,2 ) );
 */



/* draw the curve.. */
/* ---------------- */

/* set line parameters */
linedet( "linedetails", linedetails, 1.0 );

if( fillmode ) {
	Ecolor( fillcolor ); /* scg 6/18/04 */
	}
	

first = 1;
lasty = 0.0;
lastx = 0.0;
anyvalid = 0;
ingap = 0;
cr = Elimit( Y, 'l', 's' );
for( i = 0; i < npoints; i++ ) {
	if( !first && (y > (NEGHUGE+1) && x > (NEGHUGE+1) ) ) { lasty = y; lastx = x; }

	if( first && firstpt ) { 
		x = firstx; 
		y = firsty; 
		}
	else	{
		x = dat3d(i,0);
		y = dat3d(i,1);
		}
	
	if( x < (NEGHUGE+1) || y < (NEGHUGE+1) ) {
		if( gapmissing ) ingap = 1;
		continue; /* skip bad values */
		}
	
	if( x < linestart && !relax_xrange ) continue; /* X out of range - lo */ 
	if( x > linestop && !relax_xrange ) {          /* X out of range - hi */ 
		x = lastx; /* back up to last in-range point so last stairtstep is correct*/
		y = lasty;
		break; 
		}
	if( !first && ( gapmissing == 3 || gapmissing == 4 || gapmissing == 5 )) {
		if( typical_interval < 0.0 ) typical_interval = (x - lastx)*1.01;
		else if( x - lastx > typical_interval ) {
			/* if( gapmissing == 5 ) { Elinu( lastx+typical_interval, 0.0 ); Elinu( x-typical_interval, 0.0 ); } */
			if( gapmissing == 5 ) { Elinu( lastx, 0.0 ); Elinu( x, 0.0 ); Elinu( x, y ); }  /* changed, scg 11/20/07 */
			else ingap = 1;
			}
		}

	if( !anyvalid && !Ef_inr( Y, y ) ) continue; /* 1/9/03 scg - anyvalid should not become 1 if out of range in Y */

	anyvalid = 1;
	if( first ) {
		Emovu( x, y );
		setfloatvar( "XSTART", x, "%g" );
		setfloatvar( "YSTART", y, "%g" );
		first = 0;
		ingap = 0;
		continue;
		}
	if( !first && fillmode && !ingap ) {
		Emovu( x, cr ); 
		Epath( Eax( lastx )-fillbleed, Eay( cr ) );  /* -0.005 added to remove tiny gap artifact seen on EPS ... scg 1/9/08 */
		Epath( Eax( lastx )-fillbleed, Eay( lasty ) );
		/* was: Epathu( lastx, cr );  */
		/*      Epathu( lastx, lasty );  */ 

		if( stairstep ) Epathu( x, lasty );
		else Epathu( x, y );
		Ecolor( fillcolor ); /* added scg 11/2/07 - color bug related to gapmissing=2 and fill=1 */
		Efill();
		}
	if( ( gapmissing == 2 || gapmissing ==4 ) && ingap ) {        /* do a quarter-length nib at previous location */
		double nib;
		Emovu( lastx, lasty );
		nib = (x-lastx) * 0.25;
		if( fillmode ) Ecblock( Eax(lastx), Eay(cr), Eax(lastx+nib), Eay(lasty), fillcolor, 0 ); 
		else Elinu( lastx+nib, lasty );
		}
	if( ! fillmode ) { 
		if( ingap ) Emovu( x, y ); 	
		if( stairstep && x > linestart && !ingap ) Elinu( x, lasty ); 
		if( stairstep && x == linestart ) Emovu( x, y ); 
		if( clipping && !ingap && !stairstep ) {
			double cx1, cy1, cx2, cy2;
			cx1 = lastx; cy1 = lasty; cx2 = x; cy2 = y;
			stat = Elineclip( &cx1, &cy1, &cx2, &cy2, EDXlo, EDYlo, EDXhi, EDYhi );
			if( !stat ) { Emovu( cx1, cy1 ); Elinu( cx2, cy2 ); }
			}
		else Elinu( x, y );
		}
	ingap = 0;
	}

if( !anyvalid ) { /* no plottable data points.. exit */  
	/* Ecolor( oldcolor ); */ /* don't do color chg - scg 5/10/05 */
	return( 0 ); 
	} 

/* if last point was invalid, back up to most recent valid point.. */
if( x < (NEGHUGE+1) || y < (NEGHUGE+1) ) { x = lastx; y = lasty; }


/* handle last segment of stairstep.. */
/* if( stairstep ) { */ /* } changed to allow lastseglen to be used anytime, scg 12/13/01 */
if( lastseglen > 0.0 ) {
	if( x < (NEGHUGE+1) || y < (NEGHUGE+1) ) { x = lastx; y = lasty; }
	lastx = Eax( x ) + lastseglen;
	if( fillmode ) {
		Emov( Eax(x), Eay(cr) ); Epath( lastx, Eay(cr) ); 
		Epath( lastx, Eay(y) ); Epath( Eax(x), Eay(y) );
		/* Ecolorfill( fillcolor ); */ /* using Efill .. scg 6/18/04 */
		Efill();
		}
	else 	Elin( lastx, Eay( y ) );
	}
else lastx = Eax(x);

if( forcelastx[0] != '\0' ) {
	lastx = Eax( Econv( X, forcelastx ) );
	if( !Econv_error() && anyvalid ) Elin( lastx, Eay( y ) );
	}


/* set YFINAL and XFINAL */
Euprint( numstr, Y, y, numstrfmt ); /* numstr[100] */
setcharvar( "YFINAL", numstr );
Euprint( numstr, X, Edx(lastx), "" ); /* numstr[100] */
setcharvar( "XFINAL", numstr );
	





/* do points, labels, etc. */
/* ----------------------- */
for( i = 0; i < npoints; i++ ) {
	x = dat3d(i,0);
	y = dat3d(i,1);


	if( x < (NEGHUGE+1) || y < (NEGHUGE+1) ) continue; /* skip bad values */

	if( x < linestart && !relax_xrange ) continue; /* out of range - lo */
	if( x > linestop && !relax_xrange ) {          /* out of range - hi */
		break; 
		}

	if( clipping && !stairstep && !fillmode ) {
		/* if clipping, suppress points or labels that are outside the plotting area */
		if( x < EDXlo || x > EDXhi || y < EDYlo || y > EDYhi ) continue;
		}

	lasty = y;

	if( x >= ptlblstart && x <= ptlblstop ) {
		if( donumbers && stairstep ) {
			if( i == npoints-1 || GL_close_to( x, linestop, 0.01 ) ) {  
				double xls;
				xls = Edx(lastseglen) - Edx(0.0);
				placenum( shownums, x, x + xls, y, numstrfmt, linedetails );
				}
			else placenum( shownums, x, dat3d(i+1,0), y, numstrfmt, linedetails );
			}
		else if( donumbers && !stairstep ) 
			  placenum( shownums, x, x, y, numstrfmt, linedetails );
		if( donumbers && fillmode ) Ecolor( fillcolor ); /* scg 6/18/04 */
		}

	if( dopoints ) {
		int jj;
		/* see if this is one of the alternates.. if so use altsym rather
			than the regular symbol */
		for( jj = 0; jj < nalt; jj++ ) if( i == altlist[jj] ) break;
		if( jj != nalt ) symdet( "altsym", altsym, symcode, &radius );
		else symdet( "pointsymbol", pointsym, symcode, &radius );
		Emark( Eax(x), Eay(y), symcode, radius );
		}
	realrow = (int) dat3d(i,2);
	if( ptlabelfield && x >= ptlblstart && x <= ptlblstop ) {
		textdet( "ptlabeldetails", ptlabeldetails, &align, &adjx, &adjy, -4, "R", 1.0 );
		if( align == '?' ) align = 'C';
		Emov( Eax( x ) + adjx, Eay( y ) + adjy );
		Edotext( da( realrow, ptlabelfield ), align );
		}
	}


if( linelabel[0] != '\0' ) {
        GL_varsub( linelabel, "@YFINAL", numstr );  /* linelabel[256] */
	PL_do_subst( buf, linelabel, realrow, 0 );  /* also allow substitution of any @field on the last-plotted data row.. added scg 1/29/07 */
        textdet( "labeldetails", labeldetails, &align, &adjx, &adjy, -2, "R", 1.0 );
        if( align == '?' ) align = 'L';
        Emov( lastx+0.05+adjx, (Eay( lasty )-(Ecurtextheight*.35))+adjy );
        Edotext( buf, align );
        }
 
if( legendlabel[0] != '\0' ) {
	if( fillmode ) PL_add_legent( LEGEND_COLOR, legendlabel, "", fillcolor, "", "" );
	else if( pointsym[0] != '\0' && strcmp( pointsym, "none" )!= 0 ) {
		if( legsamptyp[0] == 's' && strlen( legsamptyp ) <= 6 )
			PL_add_legent( LEGEND_SYMBOL, legendlabel, "", pointsym, "", "" );
		else
			PL_add_legent( LEGEND_LINE+LEGEND_SYMBOL, legendlabel, "", linedetails, pointsym, "" );
		}
	else PL_add_legent( LEGEND_LINE, legendlabel, "", linedetails, "", "" );
	}

/* if( fillmode ) Ecolor( oldcolor ); */ /* restore */
return( 0 );
}

/* ----------------------- */
/* place one line plot number  */
static int
placenum( shownums, x1, x2, y, numstrfmt, linedetails )
char *shownums;
double x1, x2, y;
char *numstrfmt, *linedetails;
{
char numstr[100];
int align;
double adjx, adjy;

/* change to text color, size, etc.. */
textdet( "numbers", shownums, &align, &adjx, &adjy, -4, "R", 1.0 );
if( align == '?' ) align = 'C';
Emov( Eax( (x1+x2)/2.0 ) + adjx, Eay(y)+0.02+adjy );
/* sprintf( numstr, numstrfmt, y ); */
Euprint( numstr, 'y', y, numstrfmt ); /* numstr[100] */
Edotext( numstr, align );
linedet( "linedetails", linedetails, 1.0 );
Emovu( x1, y ); /* restore old position for drawing the curve.. */
return( 0 );
}

/* ------------------------- */
static int
dblcompare( a, b )
const void *a, *b;
{
double *f, *g;
f = (double *)a;
g = (double *)b;

if( *f > *g ) return( 1 );
if( *f < *g ) return( -1 );
return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
