/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC AXIS - Draw an X or Y axis.  astart parameter allows areadef xaxis.* or yaxis.* parameters */

/* Coded for Y axis.. axes are logically flipped when drawing an X axis */

#include "pl.h"

#define INCREMENTAL 1
#define HERE 2
#define FROMFILE 3
#define FROMDATA 4
#define FROMCATS 5
#define MONTHS 100


int
PLP_axis( xory, astart )
char xory; /* either 'x' or 'y' */
int astart;
{
int lvp, first;
char attr[NAMEMAXLEN];
char *line, *lineval;

char *tics, *mtics, *axisline, *stubdetails, *axislabeldet, *griddet, *labelurl, *labelinfo;
char *axislabel, *stubomit, *filename, *stubs, *stubround;
char *gridskip, *firststub, *laststub, *stubsubpat, *stubsubnew, *cmvalfmt;
char opax, clickmap;
char buf[256], txt[256], tok[80];
char stubformat[82], incunits[50], ticincunits[50], minorticunits[50];
char scaleunits[30], scalesubtype[20]; 
char autoyears[20], autodays[40], automonths[40], nearest[30];
char glemins[40], glemaxs[40], cmemins[40], cmemaxs[40];
char gbcolor1[COLORLEN], gbcolor2[COLORLEN];
char cmtxt[100]; 
int i, j, ix, stat, nt, align, stubreverse, stubvert, irow, mon, day, yr, stubdf1, stubdf2, nstubdf, selfloc, isrc;
int ibb, doingtics, stubrangegiven, stubreverse_given, specialunits, doinggrid, txtlen;
int firsttime, revsign, gbstate, stubevery, forlen, logx, logy, stubexp, stublen, sanecount, stubhide, curyr, curday, curmon;
int circuit_breaker_disable, axis_arrow, prec;
double adjx, adjy, y, f, pos, stubstart, stubstop, inc, ticin, ticout, minorticinc, mticin, mticout, axlinestart, axlinestop, max, min;
double axislabelofs, ofsx, ofsy, incamount, ticincamount, overrun, stubslide, ticslide, glemin, glemax, gbylast, stubcull, prevstub;
double cmylast, cmemin, cmemax, stubmult, stubmininc, arrowheadsize;
FILE *stubfp;


TDH_errprog( "pl proc axis" );

if( xory == 'x' ) opax = 'y';
else if( xory == 'y' ) opax = 'x';
else { Eerr( 301, "axis: bad xory", "" ); xory = 'x'; opax = 'y'; }



/* initialize */
min = Elimit( xory, 'l', 's' );
max = Elimit( xory, 'h', 's' );
pos = Elimit( opax, 'l', 'a' ); /* location will be at minima of other axis */
stubstart = axlinestart = min; 
stubstop = axlinestop = max; 
axisline = ""; tics = ""; stubdetails = ""; axislabel = ""; axislabeldet = ""; 
mtics = "none"; griddet = "none"; stubs = ""; stubround = ""; stubomit = "";
gridskip = ""; firststub = ""; laststub = ""; stubsubpat = ""; stubsubnew = ""; cmvalfmt = "";
ticout = 0.07;
minorticinc = 0.0;
mticout = 0.03;
axislabelofs = 0.4;
incamount = 0.0;
ticincamount = 0.0;
stubslide = 0.0;
ticslide = 0.0;
stubcull = 0.0;
stubevery = 1;
ticin = mticin = isrc = stubvert = nstubdf = txtlen = 0;
doingtics = 1;
selfloc = stubrangegiven = stubreverse = stubreverse_given = doinggrid = 0;
revsign = clickmap = stubhide = 0;
strcpy( autoyears, "" ); strcpy( autodays, "" ); strcpy( automonths, "" ); strcpy( nearest, "" );
strcpy( stubformat, "" );
strcpy( glemins, "min" ); strcpy( glemaxs, "max" );
strcpy( cmemins, "min" ); strcpy( cmemaxs, "max" );
strcpy( gbcolor1, "" ); strcpy( gbcolor2, "" );
stubexp = stublen = axis_arrow = 0;
stubmult = 1.0; stubmininc = 0.0;
curyr = curmon = curday = 0;
circuit_breaker_disable = 0;
arrowheadsize = 0.15;
sanecount = 0;
labelurl = ""; labelinfo = "";

Egetunits( xory, scaleunits );  /* moved from below - scg 1/27/05 */

/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];


	/* screen out areadef attributes for the other axis.. */
	if( GL_slmember( attr, "axisline tic* minortic*" )) ;
	else if( astart > 0 && strncmp( &attr[1], "axis.", 5 )!=0 ) continue;
	else if( astart > 0 && attr[0] != xory ) continue; 

	if( strcmp( &attr[astart], "label" )==0 ) { axislabel = lineval; convertnl( axislabel ); }

	else if( strcmp( &attr[astart], "stubs" )==0 || strcmp( &attr[astart], "selflocatingstubs" )==0 ) {

		if( strcmp( &attr[astart], "selflocatingstubs" )==0 ) selfloc = 1;	
		else selfloc = 0;

		tokncpy( tok, lineval, 80 );
		if( strncmp( tok, "inc", 3 )==0 || strcmp( tok, "datematic" )==0 ) {
			isrc = INCREMENTAL;
			strcpy( incunits, "" );
			nt = sscanf( lineval, "%*s %lf %s", &incamount, incunits );
			if( nt < 1 || incamount == 0.0 ) {
				if( strncmp( scaleunits, "date", 4 )== 0 || strcmp( scaleunits, "time" )==0 ) {
					DT_reasonable( scaleunits, min, max, &incamount, incunits, stubformat, 
						automonths, autoyears, autodays, &minorticinc, minorticunits, nearest );
					}
				else incamount = 0.0;
				}
			}
		else if( strcmp( tok, "minmaxonly" )==0 ) { isrc = INCREMENTAL; incamount = max - min; }
		else if( strcmp( tok, "minonly" )==0 ) { isrc = INCREMENTAL; incamount = (max - min) * 2.0; }
		else if( strcmp( tok, "maxonly" )==0 ) { isrc = INCREMENTAL; stubstart = max; incamount = (max - min) * 2.0; }
		else if( strcmp( tok, "file" )==0 ) {
			ix = 0; 
			GL_getok( lineval, &ix );
			for( ; lineval[ix] != '\0'; ix++ ) if( !isspace( (int)lineval[ix] ) ) break;
			filename = &lineval[ix]; 
			isrc = FROMFILE;
			}
		else if( strncmp( tok, "datafield", 9 ) ==0 )  {
			char fnames[2][50];
			for( j = 0, forlen = strlen( lineval ); j < forlen; j++ ) /* allow = and , */
				if( GL_member( lineval[j], "=," )) lineval[j] = ' ';
			nstubdf = sscanf( lineval, "%*s %s %s", fnames[0], fnames[1] );
			if( nstubdf > 0 ) stubdf1 = fref( fnames[0] );
			if( nstubdf == 2 ) stubdf2 = fref( fnames[1] );
			isrc = FROMDATA;
			}
		else if( strcmp( tok, "categories" )==0 || strcmp( tok, "usecategories" )==0 ) isrc = FROMCATS;
		else if( strcmp( tok, "list" )==0 ) { isrc = HERE; stubs = lineval; convertnl( stubs ); }
		else if( strcmp( tok, "none" )==0 ) isrc = 0;
		else 	{ isrc = HERE; stubs = getmultiline( lineval, "get" ); }
		}
			
	else if( strcmp( &attr[astart], "stubformat" )==0 ) { strncpy( stubformat, lineval, 80 ); stubformat[80] = '\0'; } /* (can be multi-token) */
	else if( strcmp( &attr[astart], "stubdetails" )==0 ) stubdetails = lineval;
	else if( strcmp( &attr[astart], "stubrange" )==0 ) { getrange( lineval, &stubstart, &stubstop, xory, min, max ); stubrangegiven = 1; }
	else if( strcmp( &attr[astart], "stubround" )==0 ) stubround = lineval;
	else if( strcmp( &attr[astart], "stubreverse" )==0 ) { stubreverse = getyn( lineval ); stubreverse_given =1; }
	else if( strcmp( &attr[astart], "stubvert" )==0 ) { 
		if( atoi( lineval ) != 0 ) stubvert = atoi( lineval );
		else if( strncmp( lineval, "y", 1 )==0 ) stubvert = 90; 
		else stubvert = 0;
		if( stubvert != 0 && Edev == 'x' ) stubvert = 90; /* should also do same thing here if we know we're rendering to non-freetype gd */
		}
	else if( strcmp( &attr[astart], "stubomit" )==0 ) stubomit = lineval;
	else if( strcmp( &attr[astart], "stubslide" )==0 ) Elenex( lineval, xory, &stubslide );
	else if( strcmp( &attr[astart], "stubexp" )==0 ) { if( strcmp( lineval, "exp-1")==0 ) stubexp = 2; else stubexp = getyn( lineval ); }
	else if( strcmp( &attr[astart], "stublen" )==0 ) stublen = itokncpy( lineval );
	else if( strcmp( &attr[astart], "stubmult" )==0 ) stubmult = ftokncpy( lineval );
	else if( strcmp( &attr[astart], "stubmininc" )==0 ) stubmininc = ftokncpy( lineval );
	else if( strcmp( &attr[astart], "stubhide" )==0 ) stubhide = getyn( lineval );
	else if( strcmp( &attr[astart], "ticslide" )==0 )  Elenex( lineval, xory, &ticslide );
	else if( strcmp( &attr[astart], "labeldetails" )==0 ) axislabeldet = lineval;
	else if( strcmp( &attr[astart], "labeldistance" )==0 ) { axislabelofs = ftokncpy( lineval ); if( PLS.usingcm ) axislabelofs /= 2.54; }
	else if( strcmp( &attr[astart], "location" )==0 ) { Eposex( lineval, opax, &f ); pos = f; }
	else if( strcmp( &attr[astart], "axisline" )==0  ) axisline = lineval;
	else if( strcmp( &attr[astart], "axislinerange" )==0 ) getrange( lineval, &axlinestart, &axlinestop, xory, min, max );
	else if( strcmp( &attr[astart], "tics" )==0  ) {
		tics = lineval;
		if( strncmp( tics, "no", 2 ) != 0 ) doingtics = 1;
		else doingtics = 0;
		}
	else if( strcmp( &attr[astart], "ticincrement" )==0  ) { 
		strcpy( ticincunits, "" ); 
		sscanf( lineval, "%lf %s", &ticincamount, ticincunits );
		}
	else if( strcmp( &attr[astart], "ticlen" )==0  ) {
		sscanf( lineval, "%lf %lf", &ticout, &ticin );
		if( PLS.usingcm ) { ticout /= 2.54; ticin /= 2.54; }
		}
	else if( strcmp( &attr[astart], "minortics" )==0 ) {
		if( strncmp( lineval, "no", 2 )==0 ) { mtics = "none"; minorticinc = 0.0; }   
		else mtics = lineval;
		}
	else if( strcmp( &attr[astart], "minorticinc" )==0 ) {
		strcpy( minorticunits, "" );
		nt = sscanf( lineval, "%lf %s", &minorticinc, minorticunits );
		}

	else if( strcmp( &attr[astart], "minorticlen" )==0 ) {
		sscanf( lineval, "%lf %lf", &mticout, &mticin );
		if( PLS.usingcm ) { mticout /= 2.54; mticin /= 2.54; }
		}

	else if( strcmp( &attr[astart], "grid" )==0 ) { griddet = lineval; if( strncmp( griddet, "no", 2 )!= 0 ) doinggrid = 1; }
	else if( strcmp( &attr[astart], "gridskip" )==0 ) gridskip = lineval;
	else if( strcmp( &attr[astart], "gridlineextent" )==0 ) nt = sscanf( lineval, "%s %s", glemins, glemaxs );
	else if( strcmp( &attr[astart], "gridblocks" )==0 ) {
		sscanf( lineval, "%s %s", gbcolor1, gbcolor2 );
		if( strcmp( gbcolor1, "no" )==0 || strcmp( gbcolor1, "none" )==0 ) strcpy( gbcolor1, "" );
		else doinggrid = 1;
		}

	else if( strcmp( &attr[astart], "stubcull" )==0 ) {
		if( getyn( lineval ) ) stubcull = 0.1;
		else if( ftokncpy( lineval ) > 0.0 ) stubcull = ftokncpy( lineval );
                else stubcull = 0.0;
		}

	else if( strcmp( &attr[astart], "autoyears" )==0 ) {
		tokncpy( autoyears, lineval, 20 );
		if( strcmp( autoyears, "yes" )==0 || strcmp( autoyears, "y" )==0 ) strcpy( autoyears, "'yy" );
		else if( strcmp( autoyears, "no" )==0 ) strcpy( autoyears, "" );
		}

	else if( strcmp( &attr[astart], "autodays" )==0 ) {
		tokncpy( autodays, lineval, 40 );
		if( strcmp( autodays, "yes" )==0 || strcmp( autodays, "y" )==0 ) strcpy( autodays, "Mmmdd" );
		else if( strcmp( autodays, "no" )==0 ) strcpy( autodays, "" );
		}

	else if( strcmp( &attr[astart], "automonths" )==0 ) {
		tokncpy( automonths, lineval, 40 );
		if( strcmp( automonths, "yes" )==0 || strcmp( automonths, "y" )==0 ) strcpy( automonths, "Mmm" );
		else if( strcmp( automonths, "no" )==0 ) strcpy( automonths, "" );
		}

	else if( strcmp( &attr[astart], "signreverse" )==0 ) revsign = getyn( lineval );
	else if( strcmp( &attr[astart], "stubevery" )==0 ) stubevery = itokncpy( lineval );
	else if( strcmp( &attr[astart], "firststub" )==0 ) { firststub = lineval; if( lineval[0] == '"' && lineval[1] == '"' ) firststub = " "; }
	else if( strcmp( &attr[astart], "laststub" )==0 ) { laststub = lineval; if( lineval[0] == '"' && lineval[1] == '"' ) laststub = " "; }
	else if( strcmp( &attr[astart], "stubsubpat" )==0 ) stubsubpat = lineval;
	else if( strcmp( &attr[astart], "stubsubnew" )==0 ) { stubsubnew = lineval; if( lineval[0] == '"' && lineval[1] == '"' ) stubsubnew = " "; }

	else if( strcmp( &attr[astart], "clickmap" )==0 ) {
		if( strncmp( lineval, "xy", 2 )==0 ) { if ( xory == 'x' ) clickmap=3; else clickmap=4; }
		else { if( xory == 'x' ) clickmap=1; else clickmap=2; }
		}
	else if( strcmp( &attr[astart], "clickmapextent" )==0 ) nt = sscanf( lineval, "%s %s", cmemins, cmemaxs );
	else if( strcmp( &attr[astart], "clickmapvalformat" )==0 ) cmvalfmt = lineval;
	else if( strcmp( &attr[astart], "labelurl" )==0 ) labelurl = lineval;
	else if( strcmp( &attr[astart], "labelinfo" )==0 ) labelinfo = lineval;
        else if( strcmp( &attr[astart], "labelinfotext" )==0 ) labelinfo = getmultiline( lineval, "get" ); 
	else if( strcmp( &attr[astart], "nolimit" )==0 ) circuit_breaker_disable = getyn( lineval );
	else if( strcmp( &attr[astart], "arrow" )==0 ) axis_arrow = getyn( lineval );
	else if( strcmp( &attr[astart], "arrowheadsize" )==0 ) { arrowheadsize = ftokncpy( lineval ); if (PLS.usingcm) arrowheadsize /= 2.54; }
	else if( astart == 0 || strncmp( attr, "xaxis.", 6 )==0 || strncmp( attr, "yaxis.", 6 )==0 ) 
		Eerr( 301, "axis attribute not recognized", attr );
	}



/* check for degenerate cases and control overrides.. */
if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

if( stubstart < min || stubstart > max ) stubstart = min;
if( stubstop > max || stubstop < min ) stubstop = max;
if( axlinestart < min || axlinestart > max ) axlinestart = min;
if( axlinestop > max || axlinestop < min ) axlinestop = max;
if( stubevery == 0 ) stubevery = 1;



/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */

/* Egetunits( xory, scaleunits ); */ /* moved up - scg 1/27/05 */


/* do the label.. easier if we do it before the Eflip below.. */
if( axislabel[0] != '\0' ) {
	double xpos, ypos, txtsize;
	textdet( "labeldetails", axislabeldet, &align, &adjx, &adjy, 0, "R", 1.0 );
	txtsize = (double)(strlen( axislabel )) * Ecurtextwidth;

	if( xory == 'x' ) {
		xpos =  ( EXlo+ (( EXhi - EXlo ) / 2.0 )) + adjx;
		ypos = (pos - axislabelofs) + adjy;
		Emov( xpos, ypos );
		Ecentext( axislabel );
		if( PLS.clickmap && ( labelurl[0] != '\0' || labelinfo[0] != '\0' ) )
			clickmap_entry( 'r', labelurl, 0, xpos - (txtsize/2.0), ypos, 
				xpos + (txtsize/2.0), ypos+Ecurtextheight, 1, 0, labelinfo );
		}
	else if( xory == 'y' ) {
		xpos = (pos - axislabelofs) + adjx;
		ypos = (EYlo + (( EYhi-EYlo ) / 2.0 )) + adjy;
		Emov( xpos, ypos );
		Etextdir( 90 );
		Ecentext( axislabel );
		Etextdir( 0 );
		if( PLS.clickmap && ( labelurl[0] != '\0' || labelinfo[0] != '\0' ) )
			clickmap_entry( 'r', labelurl, 0, xpos-Ecurtextheight, ypos - (txtsize*0.6), 
				xpos, ypos + (txtsize*0.6), 0, 0, labelinfo );
		}
	}


if( xory == 'x' ) Eflip = 1; /* reverse sense of x and y for draw operations */



/* avoid "circuit breaker" message when just doing label and/or line..  added scg 5/24/06 */
if( isrc == 0 && !doingtics && !doinggrid ) goto SKIPLOOP;   


/* --------------------- */
/* tics and axis preliminaries.. */
/* --------------------- */

inc = 1.0;
overrun = inc / 10.0; 
specialunits = 0;



/* ----------------- */
/* helpful overrides */
/* ----------------- */


/* if user didn't specify stub range, and 
   if isrc indicates text stubs, and not doing stubreverse, start at 1.0 */
if( !stubrangegiven && !stubreverse && !selfloc &&
	( isrc == HERE || isrc == FROMFILE || isrc == FROMDATA ) ) stubstart = 1.0;


/* if doing Y axis with text stubs, and user didn't specify stubreverse: no, 
	reverse the stubs */
if( xory == 'y' && !stubreverse_given && stubreverse == 0 &&
	( isrc == HERE || isrc == FROMFILE || isrc == FROMDATA || isrc == FROMCATS ) && !selfloc ) {  /* && !selfloc added scg 3/19/03 */
													/* FROMCATS added scg 3/10/09 */
	stubreverse = 1;
	if( !stubrangegiven ) stubstop = max - 1.0;
	}

if( stubformat[0] != '\0' && isrc == 0 )
	Eerr( 2749, "warning, stubformat but no stubs specification", "" );

/* if( strncmp( scaleunits, "date", 4 )==0 && stubformat[0] == '\0' ) 
 *	Eerr( 319, "warning, no stubformat specified.. using current notation.. for better results try 'stubs: datematic'", "" );
 */
	

if( minorticinc > 0.0 && strncmp( mtics, "no", 2 ) == 0 ) mtics = "yes";



/* compute abs grid line extent - added scg 11/22/00 */
if( doinggrid ) { 
	Eposex( glemins, opax, &glemin );
	Eposex( glemaxs, opax, &glemax );
	/* Eposex( "min", Y, &gbylast ); changed to below 5/17/01 */
	Eposex( "min", xory, &gbylast );
	gbstate = 0;
 	} 

if( PLS.clickmap && clickmap ) {
	Eposex( cmemins, opax, &cmemin ); 
	Eposex( cmemaxs, opax, &cmemax );
	Eposex( "min", xory, &cmylast );
	}


/* stubround stuff..   added scg 5/29/06 */
if( stubround[0] != '\0' ) {    /* stubround is useful when user requires min and max to be at exact oddball locations, 
				 * but want the stubs to fall on round locations.  Shouldn't come into play with 'datematic' 
				 * since dm sets min and max to round (nearest) locations. */
	char minval[40], maxval[40];
	double ninc;
	if( GL_smember( scaleunits, "date datetime time" )) {
		Euprint( buf, xory, stubstart, "" ); /* buf[256] */
		PLP_findnearest( buf, buf, xory, stubround, minval, maxval );  
		stubstart = Econv( xory, maxval );
		}
	else if( strcmp( stubround, "useinc" )==0 ) {   /* base it on whatever the axis increment value is/will be.. */
		if( incamount > 0.0 ) stubstart = GL_numgroup( stubstart, incamount, "high" );
		else	{
			double newstubstart;
			PL_defaultinc( min, max, &ninc );
			newstubstart = GL_numgroup( stubstart, ninc, "high" );
			if( newstubstart - ninc == stubstart );  /* already on a round boundary.. do nothing.. */
			else stubstart = newstubstart;
			}
		}
	else if( GL_goodnum( stubround, &prec )) stubstart = GL_numgroup( stubstart, atof( stubround ), "high" );
	}

	

/* render minor tics.. */
if( isrc == INCREMENTAL && strncmp( mtics, "no", 2 )!= 0 ) {
	/* unit conversions.. */
	if( strcmp( scaleunits, "time" )==0 && strncmp( minorticunits, "hour", 4 ) ==0 ) {
		strcpy( minorticunits, "" );
		minorticinc = minorticinc * 60;
		}
	else if( strcmp( scaleunits, "datetime" )==0 && 
		(strncmp( minorticunits, "hour", 4 ) ==0 || strncmp( minorticunits, "minute", 3 )==0 )) {
		double winsize, mm; /* window size in hours */
		DT_getwin( &winsize );
		mm = ((24.0/winsize) / 24.0);
		if( strncmp( minorticunits, "minute", 3 )==0 ) mm /= 60.0;
		minorticinc *= mm;
		/* minorticinc = minorticinc / winsize;
		 * if( strncmp( minorticunits, "minute", 3 )==0 ) minorticinc = minorticinc / 60.0;
		 * strcpy( minorticunits, "" );
		 */
		}
	else if( strcmp( scaleunits, "time" )==0 && strncmp( minorticunits, "second", 3 ) ==0 ) {
		strcpy( minorticunits, "" );
		minorticinc = minorticinc / 60.0;
		}
	else if( strcmp( scaleunits, "date" )==0 && strncmp( minorticunits, "year", 4 ) ==0 ) {
		strcpy( minorticunits, "month" );
		minorticinc = minorticinc * 12;
		}
	else if( strcmp( scaleunits, "date" )==0 && strncmp( minorticunits, "month", 5 ) ==0 ) {
		strcpy( minorticunits, "month" );
		minorticinc = minorticinc * (365.25/12.0);
		}

	linedet( "minortics", mtics, 0.5 );
	if( minorticinc <= 0.0 ) {
		Eerr( 2340, "warning, minorticinc must be specified if doing minor tics", "" );
		minorticinc = 1.0;
		}
	y = stubstart;
	while( 1 ) {
		Emov( pos-mticout, Ea( Y, y ) ); 
		Elin( pos+mticin, Ea( Y, y ) );
		y += minorticinc;
		if( y >= stubstop ) break;
		}
	} 



/* --------------------------------- */
/* preliminaries based on stubs type */
/* --------------------------------- */

if( isrc == HERE ) txtlen = strlen( stubs );

if( isrc == FROMFILE ) {  /* if taking from file, read the file into PL_bigbuf */
	stubfp = fopen( filename, "r" );
	if( stubfp == NULL ) { Eerr( 303, "cannot open stub file", filename ); isrc = INCREMENTAL; }
	else	{
		i = 0;
		while( fgets( buf, 128, stubfp ) != NULL ) {
			strcpy( &PL_bigbuf[i], buf );
			i += strlen( buf );
			}
		fclose( stubfp );
		stubs = PL_bigbuf;
		txtlen = i;
		}
	}

if( isrc == FROMDATA ) {
	if( nstubdf >= 1 ) stubdf1--;  /* off-by-one */
	if( nstubdf >= 2 ) { stubdf2--; nstubdf = 2; }
	}

if( isrc == FROMCATS ) selfloc = 0; 	/* for rendering purposes don't treat category stubs as self locating */

if( isrc == 0 && (doingtics || doinggrid) ) {   /* no stubs but doing tics or doing grid */
	strcpy( incunits, ticincunits );
	incamount = ticincamount;
	} 


/* incunit conversions.. */
if( strcmp( scaleunits, "time" )==0 && strncmp( incunits, "hour", 4 ) ==0 ) {
	strcpy( incunits, "" );
	incamount = incamount * 60.0;
	}
else if( strcmp( scaleunits, "datetime" )==0 && 
	( strncmp( incunits, "hour", 4 ) ==0 || strncmp( incunits, "minute", 3 )==0 )) {
	double winsize; /* window size in hours */
	/* must be relative to window size.. */
	DT_getwin( &winsize );
	incamount = incamount / winsize;
	if( strncmp( incunits, "minute", 3 )==0 ) incamount = incamount / 60.0;
	strcpy( incunits, "" );
	/* incamount = incamount / 24.0; */
	}
else if( strcmp( scaleunits, "datetime" )==0 && strncmp( incunits, "minute", 3 ) ==0 ) {
	strcpy( incunits, "" );
	incamount = (incamount / 24.0) / 60.0;
	}
else if( strcmp( scaleunits, "time" )==0 && strncmp( incunits, "second", 3 ) ==0 ) {
	strcpy( incunits, "" );
	incamount = incamount / 60.0;
	}
else if( strcmp( scaleunits, "date" )==0 && strncmp( incunits, "year", 4 ) ==0 ) {
	strcpy( incunits, "month" );
	incamount = incamount * 12;
	}
else if( strncmp( incunits, "month", 5 )!=0 && 
	atof( incunits ) == 0.0 ) strcpy( incunits, "" ); /* prevent racecon */


Egetunitsubtype( xory, scalesubtype );

/* yymm (etc) implies inc unit of "months" (yy implies years) */
if( scalesubtype[0] != '\0' ) {
	if( GL_slmember( scalesubtype, "yymm yy?mm yyyy?mm mm?yy mm?yyyy" ) && incunits[0] == '\0'  ) 
			strcpy( incunits, "month" );
	if( strcmp( scalesubtype, "yy" )==0 && incunits[0] == '\0' ) 
			strcpy( incunits, "year" );
	}


if( isrc == INCREMENTAL || ( isrc == 0 && (doingtics || doinggrid )) ) {

	/* for special units, initialize */
	if( strncmp( incunits, "month", 5 )==0 ) {
		long l;
		if( ! GL_smember( scaleunits, "date datetime" ))  /* changed to include datetime as well as date - scg 8/11/05 */
			return( Eerr( 2476, "month increment only valid with date or datetime scale type", "" ) );
		specialunits = MONTHS;
		selfloc = 0; /* for rendering purposes don't treat month stubs as self locating.. */
		/* do the following to get starting m, d, y.. */
		Euprint( buf, xory, stubstart, "" ); /* buf[256] */
		stat = DT_jdate( buf, &l );
		DT_getmdy( &mon, &day, &yr ); 
		if( incamount > 0.0 ) inc = incamount;
		else inc = 1; 	/* added scg 3/29 */
		}

	else if( strcmp( scaleunits, "datetime" )==0 && 
			strncmp( incunits, "hour", 4 ) ==0 ) {
		inc = incamount / 24.0;
		}

		
		
	else	{
		if( incamount > 0.0 ) {
			if( incunits[0] == '\0' ) inc = incamount;
			else {
				double ftest;
				sscanf( incunits, "%lf", &ftest );
				inc = incamount * ftest;
				/* inc = incamount * atof( incunits ); repl scg 5/23/00 */
				}
			}
		else	{
			/* try to be smart about choosing default inc */
			PL_defaultinc( min, max, &inc );
			if( stubevery > 1 ) inc = inc * (double)stubevery;
			if( inc < stubmininc ) inc = stubmininc;
			}
		overrun = inc / 10.0;
		}
	}

if( xory == 'x' ) setfloatvar( "XINC", inc, "%g" );
else if( xory == 'y' ) setfloatvar( "YINC", inc, "%g" );

/* log mode */
logx = logy = 0;
if( Escaletype_x == E_LOG || Escaletype_x == E_LOGPLUS1 ) logx = 1;
if( Escaletype_y == E_LOG || Escaletype_y == E_LOGPLUS1 ) logy = 1;

/* sanity check.. added scg 8/7/00 */
if( inc <= 0.0 ) return( Eerr( 2705, "axis increment is zero or negative", "" ));





/* ---------------------------------------------- */
/* render the stubs and/or tics and/or grid lines.. */
/* ---------------------------------------------- */

textdet( "stubdetails", stubdetails, &align, &adjx, &adjy, -2, "R", 1.0 );   /* needs to be done here as well as within loop below */

if( stubreverse ) y = stubstop;
else y = stubstart;
ibb = 0;
irow = 0;
if( stubvert ) Etextdir( stubvert );
prevstub = NEGHUGE;


firsttime = 1;


if( isrc == HERE ) {  /* skip over initial "text" or "list" token */
	if( strncmp( stubs, "text", 4 )==0 ) GL_getseg( txt, stubs, &ibb, "\n" ); 
	else if( strncmp( stubs, "list", 4 )==0 ) GL_getok( stubs, &ibb ); 
	}

    
for( sanecount = 0; sanecount < 2000; sanecount++ ) {

	if( circuit_breaker_disable ) sanecount = 0;

	strcpy( txt, "" );

	/* if( isrc == 0 ) goto DOTIC;  */ /* no stubs */


	if( isrc == INCREMENTAL ) {

 		/* exception for log 0.0 */
                if( y <= 0.0 && ((xory == 'x' && logx ) || (xory == 'y' && logy ) ) ) goto NEXTSTUB;
                 
		}

	if( isrc == INCREMENTAL ) {
		double yy, fabs(), ftest, exp();
		if( revsign && fabs(y) > 0.0001 ) yy = y * -1;
		else yy = y;
		if( stubexp == 1 ) yy = exp( yy );
		else if( stubexp == 2 ) yy = exp( yy ) - 1;

		/* when generating incremental axes moving from negative to positive, for zero sprintf sometimes
           	gives -0.00 or very tiny values like -5.5579e-17.  The following is a workaround.. scg 7/5/01 */
		/* if( stubstart < 0.0 && stubstop > 0.0 && yy < 0.0000000000001 && yy > -0.0000000000001 ) yy = 0.0;  */
		/* 5/18/06 scg - problem still occurring.. adjusting to: */
		if( stubstart < 0.00001 && stubstop >= 0.0 && yy < 0.000000001 && yy > -0.000000001 ) yy = 0.0; 

		if( stubmult != 1.0 ) yy = yy * stubmult; /* added scg 10/2/03 */

		nt = sscanf( incunits, "%lf", &ftest );
		if( nt > 0 ) Euprint( txt, xory, yy/ftest, stubformat ); /* txt[256] */
		else Euprint( txt, xory, yy, stubformat ); /* txt[256] */
		if( PLS.clickmap && clickmap ) Euprint( cmtxt, xory, yy, cmvalfmt ); /* cmtxt[100] */
		}

	if( isrc == HERE || isrc == FROMFILE ) {
		if( ibb >= txtlen ) break;
		GL_getseg( txt, stubs, &ibb, "\n" );
		if( PLS.clickmap && clickmap ) strcpy( cmtxt, txt );
		}
		
	if( isrc == FROMDATA ) {
		if( irow >= Nrecords ) break;
		if( ( irow % stubevery ) != 0 ) { irow++; goto NEXTSTUB; }
		if( nstubdf == 1 )
			sprintf( txt, "%s", da( irow, stubdf1 ) );
		else if( nstubdf == 2 ) 
			sprintf( txt, "%s %s", da( irow, stubdf1 ), da( irow, stubdf2 ) );
		if( PLS.clickmap && clickmap ) strcpy( cmtxt, txt );
		irow++;
		}

	if( isrc == FROMCATS ) {
		stat = PL_getcat( xory, irow, txt, 255 );
		if( stat ) break;  /* reached end of cat list */
		if( ( irow % stubevery ) != 0 ) { irow++; goto NEXTSTUB; }
		y = Econv( xory, txt ); /* error checking not needed */
		if( PLS.clickmap && clickmap ) strcpy( cmtxt, txt );
		irow++;
		}

	/* special units.. */
	if( specialunits == MONTHS ) {
		/* put a stub at current mon/year */
		DT_makedate( yr, mon, day, "", buf );
		y = Econv( xory, buf );
		if( Econv_error() ) { 
			Eerr( 9675, "warning, error on date conversion", buf );
			break; /* goto NEXTSTUB; changed to avoid inf loop - scg 2/12/03 */
			}
		if( day != 1 ) goto NEXTSTUB; /* added scg 9/12/01 */
		if( ( y - stubstop ) > overrun ) break;
		DT_formatdate( buf, stubformat, txt ); /* buf holds date string made above */
		if( PLS.clickmap && clickmap ) DT_formatdate( buf, cmvalfmt, cmtxt ); /* buf holds date string made above */
		}

	/* autoyears */
	if( autoyears[0] != '\0' ) {
		int doit;
		doit = 0;
		if( specialunits == MONTHS ) {
			if( (firsttime && mon < 11) || yr != curyr ) doit = 1; /* mon<11 added scg 9/12/01 */
			}
		else 	{
			DT_getmdy( &mon, &day, &yr );
			if( (firsttime && (mon < 11 || day < 18 )) || yr != curyr ) doit = 1;
			}
		curyr = yr;
		if( doit ) { 
			if( strlen( autoyears ) == 2 ) sprintf( buf, "%s\n%02d", txt, yr % 100 );
			else if( strlen( autoyears ) == 3 ) sprintf( buf, "%s\n'%02d", txt, yr % 100 );
			else	{
				if( yr >= 100 ) sprintf( buf, "%s\n%d", txt, yr );
				else if( yr >= PIVOTYEAR ) sprintf( buf, "%s\n%d", txt, 1900+yr );
				else if( yr < PIVOTYEAR ) sprintf( buf, "%s\n%d", txt, 2000+yr );
				}
			strcpy( txt, buf );
			}
		}

	/* autodays */
	if( autodays[0] != '\0' ) {
		if( strcmp( scaleunits, "datetime" )!=0 ) Eerr( 9677, "warning, autodays is only valid with datetime scaling", "" );
		else	{
			double datepart;
			char dt[40];
			datepart = floor( y );
			if( firsttime || datepart != curday ) { /* render date  */
				DT_fromjul( (long)datepart, dt );
				DT_formatdate( dt, autodays, buf );
				strcat( txt, "\n" );
				strcat( txt, buf );
				}
			curday = datepart;
			}
		}

	/* automonths - can be used w/ date or datetime.. */
	if( automonths[0] != '\0' ) {
		char dt[40];
		long foo;
		int imon, iday, iyr;
		DT_fromjul( (long) y, dt );
		DT_jdate( dt, &foo ); /* to get m d y */
		DT_getmdy( &imon, &iday, &iyr );
		if( firsttime || imon != curmon ) {
			DT_formatdate( dt, automonths, buf );
			strcat( txt, "\n" );
			strcat( txt, buf );
			}
		curmon = imon;
		}
	

	/* last minute stub content overrides.. */
	if( stubsubpat[0] != '\0' && GL_wildcmp( txt, stubsubpat, strlen(stubsubpat), 0 )==0 ) strcpy( txt, stubsubnew );
	if( firststub[0] != '\0' && firsttime ) strcpy( txt, firststub );
	if( laststub[0] != '\0' && y+inc > stubstop ) strcpy( txt, laststub );

	firsttime = 0;
		


	/* by this point stub text (including any selfloc field) 
	   should be in txt and location in y.. */


	/* if selflocating, get embedded location */
	if( selfloc ) {
		i = 0;
		GL_getchunk( buf, txt, &i, " \t" );
		if( buf[0] == '\0' ) goto NEXTSTUB;
		y = Econv( xory, buf );
		if( Econv_error() ) { 
			Eerr( 9676, "warning, error on value conversion", buf );
			goto NEXTSTUB;
			}
		if( y < stubstart || y > stubstop ) continue;
		while( GL_member( txt[i], " \t" ) ) i++;
		strcpy( txt, &txt[i] ); /* now obliterate the location field */
		}


	/* out of plotting area.. */
	if( min - y > overrun ) goto NEXTSTUB;
	if( y - max > overrun ) goto NEXTSTUB;

	/* too close to previous stub.. supress.. */
	if( stubcull > 0.0 ) {
		double fabs();
		if( fabs( Ea( Y, y ) - Ea( Y, prevstub )) < stubcull && prevstub != NEGHUGE ) goto NEXTSTUB; 
		/* fixed bug.. in log space, NEGHUGE becomes 1, sometimes causing 1st stub to be culled ... scg 9/21/04 */
		else prevstub = y;
		}


	/* render grid line or block - done first so other content can be "on top" of it.. */
	/* moved here from below, grid line extent and gridblocks added - scg 11/22/00 */
	if( doinggrid ) {
		if( gridskip[0] != '\0' && y <= min && GL_smember( gridskip, "min minmax both" ) );
		else if( gridskip[0] != '\0' && y >= max && 
			GL_smember( gridskip, "max minmax both" ) );
		else if( gbcolor1[0] != '\0' ) {   /* grid blocks */
			if( gbstate == 0 ) {
				Ecblock( glemin, gbylast, glemax, Ea( Y, y ), gbcolor1, 0 );
				gbstate = 1;
				}
			else 	{
				Ecblock( glemin, gbylast, glemax, Ea( Y, y ), gbcolor2, 0 );
				gbstate = 0;
				}
			gbylast = Ea( Y, y );
			}
		else	{
			linedet( "grid", griddet, 0.5 );
			Emov( glemin, Ea( Y, y ) );
			Elin( glemax, Ea( Y, y ) );
			}
		}

	if( PLS.clickmap && clickmap ) {    /* save region.. */
		double halfdown, halfup;
		if( specialunits == MONTHS ) { halfdown = halfup = ( Ea( Y, y ) - cmylast ) / 2.0; }
		else 	{
			/* handle linear & log.. (doesn't work quite right for months).. */
			halfup =  ( Ea( Y, y+inc ) - Ea( Y, y ) ) / 2.0;  
			halfdown =  ( Ea( Y, y ) - Ea( Y, y-inc ) ) / 2.0;
			/* halfdist = ( Ea( Y, y+inc ) - Ea( Y, y ) ) / 2.0; */
			}

		/* urlencode the value now.. scg 5/29/06 */
		/* GL_urlencode( cmtxt, buf ); */ /* don't encode it now.. procedures were changed in clickmap.c  scg 1/25/08 */
		strcpy( buf, cmtxt ); 		  /* <-- do this instead */

		if( Eflip ) {
			clickmap_entry( 'r', buf, clickmap, (Ea(Y,y)-halfdown)+stubslide, cmemin, 
				Ea(Y,y)+halfup+stubslide, cmemax, 0, 1, "" );
			}
		else 	{
			clickmap_entry( 'r', buf, clickmap, cmemin, (Ea(Y,y)-halfdown)+stubslide, 
				cmemax, Ea(Y,y)+halfup+stubslide, 0, 2, "" );
			}
		cmylast = Ea( Y, y );
		}


	/* convert any embedded "\n" to newline.. */
	convertnl( txt );

	/* determine exact position and render stub text.. */ 
	if( isrc > 0 && !stubhide && ! GL_slmember( txt, stubomit ) ) {   /* stub can't be in omit list.. */ 
		textdet( "stubdetails", stubdetails, &align, &adjx, &adjy, -2, "R", 1.0 );  /* needs to be here within the loop (stub color) */
		if( stubvert && xory == 'x' ) { 
			if( align == '?' ) {
				if( stubvert >= 270 ) align = 'L';
				else align = 'R';
				}
			ofsx = adjx + (ticout * -2.0) ;
			ofsy = (adjy + 0.04) + stubslide;
			if( stubvert >= 270 ) ofsy -= (Ecurtextheight*0.6);
			}
		else if( xory == 'y' ) {
			if( align == '?' ) align = 'R';
			ofsx = (adjx + -0.15) /* + stubslide*/;
			/* ofsy = adjy + ( Ecurtextheight * -0.4 ); */
			ofsy = adjy + ( Ecurtextheight * -0.3 ) + stubslide; 
			}
		else if( xory == 'x' ) {
			if( align == '?' ) align = 'C';
			ofsx = adjx + (Ecurtextheight + ticout)*-1.0 ;
			ofsy = adjy + stubslide;
			}
		
		/* render text..   but don't do last stub if it is past range.. */
		if( ( (Ea(Y, y)+ofsy)  - Ea(Y, stubstop ) <= overrun ) || stubreverse ) {  
			Emov( pos+ofsx, Ea( Y, y)+ofsy ); 
			if( stublen && txt[stublen] != '\0' ) { txt[stublen] = '.'; txt[stublen+1] = '.'; txt[stublen+2] = '\0'; }
			Edotext( txt, align );
			}
		}



	/* render major tic mark  */
	if( doingtics ) {
		linedet( "tics", tics, 0.5 );
		Emov( pos-ticout, Ea( Y, y) + ticslide );  
		Elin( pos+ticin, Ea( Y, y ) + ticslide );
		}

	/* render grid line - was here */


	NEXTSTUB:

	/* increment stub location.. */

	if( specialunits == MONTHS ) {    /* increment to next */
		int newmon;

		mon += inc;

		/* wrap around year.. */
		newmon = ((mon-1) % 12 ) +1;
		yr += ((mon-1) / 12);
		mon = newmon;
		
		day = 1;
		}

	else if( ! selfloc ) {
		if( stubreverse ) {
			y -= inc;
			if( stubstart - y > overrun ) break;
			}
		else	{
			y += inc;
			if( ( y - stubstop ) > overrun ) break;
			}
		}
	}

if( stubvert ) Etextdir(0);


SKIPLOOP:


/* make the line..  do it last of all for appearance sake.. */
if( strcmp( axisline, "none" )!= 0 ) {
	linedet( "axisline", axisline, 0.5 );
	Emov( pos, Ea( Y, axlinestart ) ); Elin( pos, Ea( Y, axlinestop ) );
	if( axis_arrow ) PLG_arrow( pos, Ea( Y, axlinestop), pos, Ea( Y, axlinestop)+(arrowheadsize*2.0), arrowheadsize, 0.3, Ecurcolor );
	}


Eflip = 0;

if( sanecount >= 2000 ) return( Eerr( 5729, "warning, too many stubs/major tics, circuit breaker tripped", "" ));

return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
