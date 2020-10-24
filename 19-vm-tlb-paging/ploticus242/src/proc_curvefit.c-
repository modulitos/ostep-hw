/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC CURVEFIT - Fit a curve to the data.
   Result becomes the new data that plotting procedures access.  */

#include "pl.h"

#define MAXPTS 1000	/* default max # input points for movingavg curve */
#define MAXORDER 21	/* max bspline order value (no limit for movingavg order) */
#define MAXBSP_IN 100  /* max # input points for bspline curve */

#define MOVINGAVG	'm'
#define REGRESSION	'r'
#define BSPLINE		'b'
#define SIMPLEAVG	'a'
#define INTERPOLATED	'i'

/* the PLV vector is used for curve points */
static int bspline(), mavg(), plainavg(), lregress();
static int dblcompare(const void *a, const void *b);  


int
PLP_curvefit()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char *linedetails, *curvetype, *selectex;
char numstr[100];
char legendlabel[256];
int i, stat, order, xfield, yfield, npts, nresultpoints, irow, showresults, calcrangegiven;
int linerangegiven, statsonly, selectresult, xsort, maxinpoints, doclip;
double resolution, linestart, linestop, calcstart, calcstop, *inpoints, *inp, drawx, drawy, prevdrawx, prevdrawy, curveshift;

TDH_errprog( "pl proc curvefit" );

/* initialize */
order = 4;
xfield = -1;
yfield = -1;
nresultpoints = -1;
resolution = 5.0;
linedetails = "";
linestart = EDXlo;
linestop = EDXhi;
showresults = 0; calcrangegiven = 0; linerangegiven = 0; xsort = 0; doclip = 0; statsonly = 0;
curvetype = "movingavg";
selectex = "";
strcpy( legendlabel, "" ); 
maxinpoints = MAXPTS;
curveshift = 0.0;


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "xfield" )==0 ) xfield = fref( lineval ) - 1;
	else if( strcmp( attr, "yfield" )==0 ) yfield = fref( lineval ) - 1;
	else if( strcmp( attr, "order" )==0 ) order = itokncpy( lineval );
	else if( strcmp( attr, "resolution" )==0 ) resolution = ftokncpy( lineval );
	else if( strcmp( attr, "xsort" )==0 ) xsort = getyn( lineval );
	else if( strcmp( attr, "linedetails" )==0 ) linedetails = lineval;
	else if( strcmp( attr, "legendlabel" )==0 ) tokncpy( legendlabel, lineval, 256 );
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "linerange" )==0 ) {
		if( lineval[0] != '\0' ) linerangegiven = 1;
		getrange( lineval, &linestart, &linestop, 'x', EDXlo, EDXhi );
		}
	else if( strcmp( attr, "calcrange" )==0 ) {
		if( lineval[0] != '\0' ) {
			calcrangegiven = 1;
			getrange( lineval, &calcstart, &calcstop, 'x', EDXlo, EDXhi );
			}
		else calcrangegiven = 0;
		}
	else if( strcmp( attr, "curvetype" )==0 ) curvetype = lineval;
	else if( strcmp( attr, "curveshift" )==0 ) curveshift = ftokncpy( lineval ); /* added scg 6/2/06 */
	else if( strcmp( attr, "maxinpoints" )==0 ) maxinpoints = itokncpy( lineval );
	else if( strcmp( attr, "showresults" )==0 ) showresults = getyn( lineval );
	else if( strcmp( attr, "clip" )==0 ) doclip = getyn( lineval );
	else if( strcmp( attr, "statsonly" )==0 ) statsonly = getyn( lineval );
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );

if( yfield < 0 || yfield >= Nfields ) return( Eerr( 601, "yfield not specified or out of range", "" ) );
/* if( xfield < 0 || xfield >= Nfields ) return( Eerr( 601, "xfield not specified or out of range", "" ) ); */ 
/* xfield can be unspecified - scg 6/2/06 (post 2.33 release) */

if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );
 
if( strncmp( legendlabel, "#usefname", 9 )==0 ) getfname( yfield+1, legendlabel ); /* legendlabel[256] */


/* now do the computation work.. */
/* -------------------------- */

/* allocate memory for the input points list.. */
inpoints = (double *) malloc( maxinpoints * sizeof( double ) * 2 );
inp = inpoints;

/* process input data.. */
npts = 0;
for( irow = 0; irow < Nrecords; irow++ ) {

	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, irow, &selectresult );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( selectresult == 0 ) continue; /* reject */
                }

	/* in[npts][1] = fda( irow, yfield, Y ); */
	inp += 1;  /* because we're getting Y */
	*inp = fda( irow, yfield, Y );
	if( Econv_error() ) { conv_msg( irow, yfield, "yfield" ); inp -= 1; continue; }   /* bug - inp-=1 added scg 2/3/05 */

	inp -= 1;  /* now back up one to get X */
	if( xfield < 0 ) *inp = (double)irow + curveshift;   /* in[npts][0] = (int)irow; */
	else 	{
		/* in[npts][0] = fda( irow, xfield, X ); */
		*inp = fda( irow, xfield, X ) + curveshift;
		if( Econv_error() ) { conv_msg( irow, xfield, "xfield" ); continue; }
		}

	/* compute curve only for points within calcrange */
	if( calcrangegiven ) {
		/* if( in[npts][0] < calcstart ) continue; */
		/* else if( in[npts][0] > calcstop ) break; */
		if( *inp < calcstart ) continue;
		else if( *inp > calcstop ) break;
		}

	if( curvetype[0] == BSPLINE && npts >= MAXBSP_IN ) {
		Eerr( 2599, "max of 100 input points allowed for bspline exceeded; curve truncated", "" );
		break;
		}
	if( npts >= maxinpoints ) {
		Eerr( 2599, "maxinpoints exceeded, curve truncated (see maxinpoints attribute)", "" );
		break;
		}
	npts++;
	inp+=2; /* to next point slot */
	}


/* sort points on x - added scg 11/2000 */
if( curvetype[0] != INTERPOLATED || xsort ) 
	qsort( inpoints, npts, sizeof(double)*2, dblcompare );


if( curvetype[0] == MOVINGAVG ) { 
	mavg( inpoints, npts, PLV, order );
	nresultpoints = npts;
	}


else if( curvetype[0] == REGRESSION ) { 
	double rlinestart, rlinestop;

	if( linerangegiven ) {
		rlinestart = linestart;
		rlinestop = linestop;
		}
	else	{
		/* rlinestart = in[0][0]; rlinestop = in[npts-1][0]; */
		inp = inpoints; rlinestart = *inp; inp += (npts-1)*2; rlinestop = *inp;
		/* fprintf( stderr, "[rlinestart=%g   rlinestop=%g ]\n", rlinestart, rlinestop ); */
		}
	lregress( inpoints, npts, PLV, rlinestart, rlinestop );
	nresultpoints = 2;

	/* vertical line (degenerate case) suppress.. */
	if( dat2d( 0, 0 ) == dat2d( 1, 0 ) ) nresultpoints = 0;  
	
	/* clip to plotting area.. */
	stat = Elineclip( &dat2d(0,0), &dat2d(0,1), &dat2d(1,0), &dat2d(1,1), EDXlo, EDYlo, EDXhi, EDYhi );
	if( stat != 0 ) nresultpoints = 0;
	}

else if( curvetype[0] == BSPLINE ) {  
	nresultpoints = npts * resolution;
	if( nresultpoints >= PLVhalfsize ) nresultpoints = PLVhalfsize-1;

	if( order >= MAXORDER ) {
		Eerr( 2158, "Using max bspline order of 20", "" );
		order = 20;
		}
	if( npts < order ) {
		if( inpoints != NULL ) free( inpoints );
		return( Eerr( 4892, "Must have at least 'order' data points", "" ) );
		}

	/* do the computation.. */
	bspline( inpoints, npts, PLV, nresultpoints, order );
	}

else if( curvetype[0] == INTERPOLATED ) { 
	stat = PL_smoothfit( inpoints, npts, PLV, &nresultpoints );
	}

else 	{ 	/* average curve (basic) */
	plainavg( inpoints, npts, PLV, order );
	nresultpoints = npts;
	}

if( inpoints != NULL ) free( inpoints );


/* curve has been generated.. now draw the line.. */
/* ---------------------------------------------- */
linedet( "linedetails", linedetails, 1.0 );
Emovu( dat2d(0,0), dat2d(0,1) );
if( showresults ) fprintf( PLS.diagfp, "// generated curve points follow:\n%g %g\n", dat2d(0,0), dat2d(0,1) );


for( i = 1; i < nresultpoints; i++ ) {

	drawx = dat2d(i,0);
	drawy = dat2d(i,1);

	/* draw only within linerange.. */
	if( i > 0 && drawx > linestart && (dat2d(i-1,0)) < linestart ) Emovu( drawx, drawy );

	else if( drawx < linestart ) continue;
	else if( drawx > linestop ) break;

	if( doclip && !statsonly && i > 0 ) {
		prevdrawx = dat2d(i-1,0);
		prevdrawy = dat2d(i-1,1);
		stat = Elineclip( &prevdrawx, &prevdrawy, &drawx, &drawy, EDXlo, EDYlo, EDXhi, EDYhi );
		if( stat ) goto BOTTOM;
		Emovu( prevdrawx, prevdrawy );
		}
	if( !statsonly ) Elinu( drawx, drawy );  
	BOTTOM:
	if( showresults ) fprintf( PLS.diagfp, "%g %g\n", drawx, drawy );
	}


/* set YFINAL and Xfinal */
i--;
Euprint( numstr, Y, dat2d(i,1), "" ); /* numstr[100] */
setcharvar( "YFINAL", numstr );
Euprint( numstr, X, dat2d(i,0), "" ); /* numstr[100] */
setcharvar( "XFINAL", numstr );




if( legendlabel[0] != '\0' ) PL_add_legent( LEGEND_LINE, legendlabel, "", linedetails, "", "" );

return( 0 );
}



/* =========================== */
static int mavg( in, npts, out, order )
double in[][2];  /* input points */
int npts;	 /* number of input points */
double out[][2]; /* output points (same n as input points array) */
int order;	 /* # of points to average */
{
int i, j;
int avgstart;
double accum;

for( i = 0; i < npts; i++ ) {
	avgstart = i - (order - 1);
	if( avgstart < 0 ) avgstart = 0;
	accum = 0.0;
	for( j = avgstart; j <= i; j++ ) accum += in[j][1];	
	out[i][0] = in[i][0];
	out[i][1] = accum / (double)(( i - avgstart ) + 1);
	}
return( 0 );
}

/* =========================== */
/* same as movingavg but averages in points to right as well.. */
static int plainavg( in, npts, out, order )
double in[][2];  /* input points */
int npts;	 /* number of input points */
double out[][2]; /* output points (same n as input points array) */
int order;	 /* # of points to average */
{
int i, j;
int avgstart, avgstop;
double accum;


for( i = 0; i < npts; i++ ) {
	avgstart = i - (order - 1);
	avgstop = i + (order - 1);
	if( avgstart < 0 ) avgstart = 0;
	if( avgstop >= npts ) avgstop = npts-1;
	accum = 0.0;
	for( j = avgstart; j <= avgstop; j++ ) accum += in[j][1];	
	out[i][0] = in[i][0];
	out[i][1] = accum / (double)(( avgstop - avgstart ) + 1  );
	}
return( 0 );
}

/* ============================== */
/* LREGRESS - linear regression curve */
static int lregress( in, npts, out, start, stop )
double in[][2];
int npts;	 /* number of input points */
double out[][2]; /* output points (n=2 since it's a straight line) */
double start, stop; /* X values - result line start and stop */
{
int i;
double sumx, sumxx, sumy, sumyy, sumxy;
double b, m;
double numer, denom, denomleft, denomright;
char buf[128], tok[128];
double sqrt();
char *GL_autoroundf();

sumx = sumxx = sumy = sumyy = sumxy = 0.0;
for( i = 0; i < npts; i++ ) {
	sumx += in[i][0];
	sumxx += (in[i][0] * in[i][0]);
	sumy += in[i][1];
	sumyy += (in[i][1] * in[i][1]);
	sumxy += (in[i][0] * in[i][1]);
	}
/* compute x intercept (b) */
numer = (sumy * sumxx) - (sumx * sumxy);
denom = ( (double)npts * sumxx ) - (sumx * sumx);
b = numer / denom; 

/* compute slope (m) */
numer = ((double)npts * sumxy) - (sumx * sumy);
denom = ((double)npts * sumxx) - (sumx * sumx);
if( denom == 0.0 ) m = 0.0;  /* ? */
else m = numer / denom;

out[0][0] = start;
out[0][1] = (m * start) + b;
out[1][0] = stop;
out[1][1] = (m * stop) + b;

strcpy( tok, GL_autoroundf( m, 0 ) );
sprintf( buf, "Y = %s + %sX", GL_autoroundf(b,0), tok );
TDH_setvar( "REGRESSION_LINE", buf );

/* compute r (pearson correlation coeff) */
/* numer = ((double) nvalues * sumxy) - (sumx * sumy ); */
denomleft = ((double) npts * sumxx)  - (sumx * sumx);
denomright = ((double) npts * sumyy)  - (sumy * sumy);
denom = sqrt( denomleft * denomright );

if( denom == 0.0 ) strcpy( buf, "(none)" );
else sprintf( buf, "%s", GL_autoroundf( (numer/denom), 0 ) );
TDH_setvar( "CORRELATION", buf );

return( 0 );
}

/* ======================== */
/* BSPLINE curve */
static int
bspline( in, npts, out, ncv, order )
double in[][2];  /* input points */
int npts;	 /* number of input points */
double out[][2]; /* output points */
int ncv;         /* number of output points to generate */
int order;	 /* order of the curve  (2..n_in) */
{
int i, j, k, n;
int nknot; /* size of knot vector */
double t; /* parameter */
double N[MAXBSP_IN][MAXORDER]; /* weighting function */ 
double knot[MAXBSP_IN+MAXORDER]; /* knot vector */
double tmp1, tmp2, tmp3, tmp4, tmp5, tmp6;
int n_out;



/* generate the knot vector */
/* ------------------------ */
for( i = 0; i < MAXBSP_IN; i++ ) knot[i] = 0.0; /* init */
n = npts - 1;
nknot = n + 1 + order;
for( i = 0; i < order; i++ ) knot[i] = 0;
j = 1;
for( ; i < nknot-order; i++ ) knot[i] = j++;
for( ; i < nknot; i++ ) knot[i] = j;
/* printf( "Knot= [ " ); 
 *  for( i = 0; i < nknot; i++ ) printf( "%g ", knot[i] ); 
 * printf( "]\n" ); 
 */


t = 0.0;
n_out = 0;
for( j = 0; j < ncv; j++ ) { /* for each point to be generated.. */

    /* do the N(?,1) set.. */
    for( i = 0; i <= n+1; i++ ) {
	if( knot[i] <= t && t < knot[i+1] ) N[i][1] = 1.0;
	else N[i][1] = 0.0;
	}

    /* do middle N.. */
    for( k = 2; k <= order; k++ ) {
	for( i = 0; i <= npts; i++ ) {
	    tmp1 = ((t - knot[i])*N[i][k-1]);
	    tmp2 = (knot[i+k-1] - knot[i]);
	    tmp3 = ((knot[i+k] - t)*N[i+1][k-1]);
	    tmp4 = (knot[i+k] - knot[i+1]);
	    if( tmp2 == 0.0 ) { tmp5 = 0.0; }
	    else { tmp5 = tmp1 / tmp2; }
	    if( tmp4 == 0.0 ) { tmp6 = 0.0; }
	    else { tmp6 = tmp3 / tmp4; }
	    N[i][k] = tmp5 + tmp6;
	    }
	}

    if( j == ncv-1 ) N[n][order] = 1.0; /* for last point */

    tmp1 = tmp2 = tmp3 = 0;
    for( i = 0; i < npts; i++ ) {
	tmp1 += (in[i][0]*N[i][order]);
	tmp2 += (in[i][1]*N[i][order]);
	tmp3 += 0.0; /* (in[i][2]*N[i][order]); */
	}

    /* put curve into D2 */
    out[n_out][0] = tmp1;
    out[n_out][1] = tmp2;
    /* out[n_out][2] = tmp3; */
    n_out++;
    t += ( knot[nknot-1] / (double)(ncv - 1) );
    }

return( 1 );
}

/* ============================= */
static int 
dblcompare( a, b )
const void *a, *b;

/* dblcompare( f, g )
 * double *f, *g;
 */  /* changed to eliminate gcc warnings  scg 5/18/06 */

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

