/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC AREADEF - Set up a graphics area with scaling */

#include "pl.h"

/* constants */
static int nareas = 17;
static char *areas[17] = {
"standardp    1.2 3.5 7.4 8.0   1.5 1.5 9.0 6.2",
"standard     1.2 2.5 7.4 7.0   1.5 1.5 9.0 6.2",
"square       1.2 2.0 7.2 8.0   2.2 0.8 8.2 6.8",
"whole        1.2 1.0 7.4 9.0   1.5 1.2 9.0 7.0",
"2hi          1.0 5.5 7.6 9.0   1.0 4.5 10.0 7.0",
"2lo          1.0 1.0 7.6 4.5   1.0 1.0 10.0 3.5",
"2left        1.0 1.0 4.0 9.5   1.0 1.0 5.25 6.5",
"2right       5.0 1.0 8.0 9.5   6.25 1.0 10.5 6.5",
"3hi          1.0 7.0 7.6 9.0   1.0 5.5 10.0 7.5",
"3mid         1.0 4.0 7.6 6.0   1.0 3.0 10.0 5.0",
"3lo          1.0 1.0 7.6 3.0   1.0 0.5 10.0 2.5",
"4nw          1.0 6.0 4.0 9.0   1.0 4.0 5.25 7.0",
"4ne          4.5 6.0 7.5 9.0   6.25 4.0 10.5 7.0",
"4sw          1.0 1.5 4.0 4.5   1.0 0.5 5.25 3.5",
"4se          4.5 1.5 7.5 4.5   6.25 0.5 10.5 3.5",
"slide        1 0.7 5.85 3.2    1 0.7 5.85 3.2",
"lifetab      1.0 1.0 7.0 4.5   1.5 1.0 7.5 4.5",
};

int
PLP_areadef()
{
int i, lvp, first;
char attr[NAMEMAXLEN];
char *line, *lineval;

char *title, *title2, *titledet, *title2det, *frame, *linebottom, *lineside, *mapurl, *maplabel;
char *xscaletype, *yscaletype, *areaname, *areacolor;
char tok[80], ahwalign[40], xminstr[256], xmaxstr[120], yminstr[256], ymaxstr[120];  
double xlo, xhi, ylo, yhi, adjx, adjy, height, width;
double autowf, autowmin, autowmax, autohf, autohmin, autohmax, cmadj, clickmap_adjx, clickmap_adjy;
int nt, stat, nlines, maxlen, doframe, namedarea, align, gotarea, gotxrange, gotyrange, doxaxis, doyaxis, locspec;
int docats, catbinsadjust;

TDH_errprog( "pl proc areadef" );
 

/* initialize */
title = ""; title2 = ""; titledet = ""; title2det = ""; frame = ""; linebottom = ""; lineside = ""; mapurl = ""; maplabel = "";
xscaletype = "linear"; yscaletype = "linear";

/* areaname = "standard"; */  /* changing this... paper is no longer our primary medium.. scg 1/11/08 */
areaname = "2lo"; 

areacolor = "";
gotxrange = gotyrange = 0;
if( PLS.device == 'c' || PLS.device == 'p' ) areaname = "standardp"; /* for sheet of paper */
namedarea = 1;
height = 4.0; width = 6.0;
xlo = 1.5; ylo = 5.0;
strcpy( xminstr, "" ); strcpy( xmaxstr, "" ); strcpy( yminstr, "" ); strcpy( ymaxstr, "" );
gotarea = 1;
autowf = 0.0; autowmin = 0.0; autowmax = 20.0;
autohf = 0.0; autohmin = 0.0; autohmax = 20.0;
clickmap_adjx = clickmap_adjy = 0.0;
strcpy( ahwalign, "" );
doframe = doxaxis = doyaxis = 0;
locspec = docats = catbinsadjust = 0;


/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
        if( line == NULL ) break;
        first = 0;
        lineval = &line[lvp];

	if( strcmp( attr, "areaname" )==0 ) { areaname = lineval; gotarea = 1; }
	else if( strcmp( attr, "rectangle" )==0 ) {
		stat = getbox( "rectangle", lineval, &xlo, &ylo, &xhi, &yhi );
		if( stat ) { PLS.skipout = 1; return( 1 ); }
		namedarea = 0;
		gotarea = 1;
		}
	else if( strcmp( attr, "box" )==0 ) {
		sscanf( lineval, "%lf %lf", &width, &height );
		if( PLS.usingcm ) { width /= 2.54; height /= 2.54; }
		namedarea = 0;
		gotarea = 1;
		}	
	else if( strcmp( attr, "location" )==0 ) {
		sscanf( lineval, "%lf %lf", &xlo, &ylo );
		if( PLS.usingcm ) { xlo /= 2.54; ylo /= 2.54; }
		locspec = 1;
		}	
	else if( strcmp( attr, "autowidth" )==0 ) nt = sscanf( lineval, "%lf %lf %lf %s", &autowf, &autowmin, &autowmax, ahwalign );
	else if( strcmp( attr, "autoheight" )==0 ) nt = sscanf( lineval, "%lf %lf %lf %s", &autohf, &autohmin, &autohmax, ahwalign );
	else if( strcmp( attr, "xrange" )==0 || strcmp( attr, "xautorange" )==0 ) {
		tokncpy( tok, lineval, 80 );
		if( strncmp( tok, "datafield", 9 )==0 ) strcpy( xminstr, lineval );
		else if( strcmp( tok, "categories" )==0 ) { xscaletype = "categories"; continue; }
		else	{
			nt = sscanf( lineval, "%s %s", xminstr, xmaxstr );
			if( nt != 2 ) { PLS.skipout = 1; return( Eerr( 105, "both min and max expected", "xrange" )); }
			}
		gotxrange = 1;
		}
	else if( strcmp( attr, "yrange" )==0 || strcmp( attr, "yautorange" )==0 ) {
		tokncpy( tok, lineval, 80 );
		if( strncmp( tok, "datafield", 9 )==0 ) strcpy( yminstr, lineval );
		else if( strcmp( tok, "categories" )==0 ) { yscaletype = "categories"; continue; }
		else	{
			nt = sscanf( lineval, "%s %s", yminstr, ymaxstr );
			if( nt != 2 ) { PLS.skipout = 1; return( Eerr( 105, "both min and max expected", "yrange" )); }
			}
		gotyrange = 1;
		}
	else if( strcmp( attr, "xscaletype" )==0 ) xscaletype = lineval;
	else if( strcmp( attr, "yscaletype" )==0 ) yscaletype = lineval;
	else if( strcmp( attr, "frame" )==0 ) {
		tokncpy( tok, lineval, 80 );
		if( strncmp( tok, "no", 2 )==0 ) doframe = 0;
		else { frame = lineval; doframe = 1; }
		}
	else if( strcmp( attr, "title" )==0 ) { title = lineval; convertnl( title ); }
	else if( strcmp( attr, "title2" )==0 ) { title2 = lineval; convertnl( title2 ); }
	else if( strcmp( attr, "titledetails" )==0 ) titledet = lineval;
	else if( strcmp( attr, "title2details" )==0 ) title2det = lineval;
	else if( strcmp( attr, "areacolor" )==0 ) areacolor = lineval;
	else if( strcmp( attr, "linebottom" )==0 ) { if( strncmp( lineval, "no", 2 )!=0 ) linebottom = lineval; }
	else if( strcmp( attr, "lineside" )==0 ) { if( strncmp( lineval, "no", 2 )!= 0 ) lineside = lineval; }
	else if( strcmp( attr, "clickmapurl" )==0 ) mapurl = lineval; 
	else if( strcmp( attr, "clickmaplabel" )==0 ) maplabel = lineval; 
        else if( strcmp( attr, "clickmaplabeltext" )==0 ) maplabel = getmultiline( lineval, "get" ); 
	else if( strcmp( attr, "clickmapadjust" )==0 ) sscanf( lineval, "%lf %lf", &clickmap_adjx, &clickmap_adjy );
	else if( strcmp( attr, "catbinsadjust" )==0 ) catbinsadjust = itokncpy( lineval );
	else if( strcmp( attr, "axes" )==0 ) {
		if( strcmp( lineval, "none" )==0 ) { doxaxis = 0; doyaxis = 0; }
		else if( strcmp( lineval, "x" )==0 ) doxaxis = 1;
		else if( strcmp( lineval, "y" )==0 ) doyaxis = 1;
		else if( strcmp( lineval, "both" )==0 ) { doxaxis = 1; doyaxis = 1; }
		}
	else if( GL_slmember( attr, "?axis.stubs ?axis.selflocatingstubs" )) {
		if( strncmp( attr, "xaxis.", 6 )==0 ) doxaxis = 1; 
		if( strncmp( attr, "yaxis.", 6 )==0 ) doyaxis = 1; 
		tokncpy( tok, lineval, 80 );
		if( GL_slmember( tok, "none inc* datematic file datafield* list categories usecategories" )) ;
		else getmultiline( lineval, "skip" ); /* just to skip past it..*/
		}
	else if( strncmp( attr, "xaxis.", 6 )==0 ) { doxaxis = 1; continue; }
	else if( strncmp( attr, "yaxis.", 6 )==0 ) { doyaxis = 1; continue; }
	else if( GL_slmember( attr, "axisline tic* minortic*" )) continue;

	else if( GL_slmember( attr, "?categories ?extracategory catcompmethod" )) {
		docats = 1;
		tokncpy( tok, lineval, 80 );
		if( GL_slmember( attr, "?categories" ) && strncmp( tok, "datafield", 9 )!=0 ) getmultiline( lineval, "skip" ); 
		if( attr[0] == 'x' ) xscaletype = "categories";
		if( attr[0] == 'y' ) yscaletype = "categories";
		fprintf( PLS.errfp, "Warning, category specification within proc areadef is deprecated in 2.40 in favor of proc categories\n" );
		}

	else Eerr( 1, "areadef attribute not recognized", attr );
	}


/* now do the plotting work.. */

PL_resetstacklist();  /* (obscure) - reset the bar graph list that tries to
			keep track of fields for automatic stacking */

/* go set up category sets if needed.. */
if( docats ) {
	PLP_categories( 1 ); 
	TDH_errprog( "pl proc areadef" );
	}

if( strcmp( xscaletype, "categories" )==0 ) {
	if( PL_ncats('x') < 1 ) { 
		PLS.skipout = 1; 
		return( Eerr( 5972, "no x categories exist", "" ) ); 
		}
	gotxrange = 1;  /* note- categories could have been filled from a previous data set */
	strcpy( xminstr, "0" );
	sprintf( xmaxstr, "%d", PL_ncats('x')+(1-catbinsadjust) );
	}

if( strcmp( yscaletype, "categories" )==0 ) {
	if( PL_ncats('y') < 1 ) { 
		PLS.skipout = 1; 
		return( Eerr( 5973, "no y categories exist", "" ) ); 
		}
	gotyrange = 1; /* note- categories could have been filled from a previous data set */
	sprintf( yminstr, "%d", catbinsadjust );
	sprintf( ymaxstr, "%d", PL_ncats('y')+1 );
	/* strcpy( yminstr, "0" );
	 * sprintf( ymaxstr, "%d", PL_ncats('y')+(1+catbinsadjust) );
 	 */
	}

if( locspec ) { /* location and box specified, calculate xhi and yhi.. */
	xhi = xlo + width;
	yhi = ylo + height;
	}

/* determine area.. */
if( namedarea ) {
	RETRY:
	for( i = 0; i < nareas; i++ ) {
		if( strncmp( areas[i], areaname, strlen( areaname ) )==0 ) {
			if( Ecurpaper == 0 ) sscanf( areas[i], "%*s %lf %lf %lf %lf", &xlo, &ylo, &xhi, &yhi );
			else if( Ecurpaper == 1 ) sscanf( areas[i], "%*s %*s %*s %*s %*s %lf %lf %lf %lf", &xlo, &ylo, &xhi, &yhi );
			break;
			}
		}
	if( i == nareas ) { Eerr( 110, "warning, areaname not recognized", areaname ); areaname = "standard"; goto RETRY; }
	}
		
if( !gotarea && !locspec ) {  /* && !locspec added scg 11/21/00 */
	PLS.skipout = 1;
	return( Eerr( 130, "No plotting area has been specified", "" ) );
	}
if( !gotxrange ) {
	PLS.skipout = 1;
	return( Eerr( 130, "No xrange has been specified", "" ) );
	}
if( !gotyrange ) {
	PLS.skipout = 1;
	return( Eerr( 130, "No yrange has been specified", "" ) );
	}

/* set scaling type and special units if any.. */
stat = Escaletype( xscaletype, 'x' );
if( stat != 0 ) Escaletype( "linear", 'x' );
if( strncmp( xscaletype, "log", 3 )==0 ) Esetunits( 'x', "linear" );
else Esetunits( 'x', xscaletype );

stat = Escaletype( yscaletype, 'y' );
if( stat != 0 ) Escaletype( "linear", 'y' );
if( strncmp( yscaletype, "log", 3 )==0 ) Esetunits( 'y', "linear" );
else Esetunits( 'y', yscaletype );


/* if autoranging is specified, do it now.. */
if( strncmp( xminstr, "datafield", 9 ) == 0 ) PLP_autorange( 'x', xminstr, xminstr, xmaxstr );
if( strncmp( yminstr, "datafield", 9 ) == 0 ) PLP_autorange( 'y', yminstr, yminstr, ymaxstr );

/* if scaletype is log but there are data values = 0.0, go to log+1 scaling.. */
if( strcmp( xscaletype, "log" )==0 && atof( xminstr ) <= 0.0 ) Escaletype( "log+1", 'x' );
if( strcmp( yscaletype, "log" )==0 && atof( yminstr ) <= 0.0 ) Escaletype( "log+1", 'y' );

/* if using autowidth or autoheight, revise plotting area now.. */
if( autowf != 0.0 ) {
	double xmin, xmax;
	if( strcmp( xscaletype, "categories" )== 0 ) { 
		xmin = atof( xminstr );
		xmax = atof( xmaxstr );
		}
	else	{
		xmin = Econv( 'x', xminstr );
		xmax = Econv( 'x', xmaxstr );
		}
	if( strcmp( ahwalign, "align=right" )== 0 ) {
		xlo = xhi - ((xmax-xmin) * autowf);
		if( xhi - xlo < autowmin ) xlo = xhi - autowmin;
		else if( xhi - xlo > autowmax ) xlo = xhi - autowmax;
		}
	else	{
		xhi = xlo + ((xmax-xmin) * autowf);
		if( xhi - xlo < autowmin ) xhi = xlo + autowmin;
		else if( xhi - xlo > autowmax ) xhi = xlo + autowmax;
		}
	}
if( autohf != 0.0 ) {
	double ymin, ymax;
	if( strcmp( yscaletype, "categories" )== 0 ) {
		ymin = atof( yminstr );
		ymax = atof( ymaxstr );
		}
	else	{
		ymin = Econv( 'y', yminstr );
		ymax = Econv( 'y', ymaxstr );
		}
	if( strcmp( ahwalign, "valign=top" )== 0 ) {
		ylo = yhi - ((ymax-ymin) * autohf);
		if( yhi - ylo < autohmin ) ylo = yhi - autohmin;
		else if( yhi - ylo > autohmax ) ylo = yhi - autohmax;
		}
	else	{ 
		yhi = ylo + ((ymax-ymin) * autohf);
		if( yhi - ylo < autohmin ) yhi = ylo + autohmin;
		else if( yhi - ylo > autohmax ) yhi = ylo + autohmax;
	 	} 
	}

	


/* set the scaling for the plot area.. */
stat = Esetscale( 'x', xlo, xhi, xminstr, xmaxstr );
stat += Esetscale( 'y', ylo, yhi, yminstr, ymaxstr );


if( PLS.debug ) fprintf( PLS.diagfp, "areadef: lowerleft: %g,%g  upperright: %g,%g\n",
		xlo, ylo, xhi, yhi );
if( PLS.debug ) fprintf( PLS.diagfp, "areadef:   xrange is %s to %s.   yrange is %s to %s.\n",
		xminstr, xmaxstr, yminstr, ymaxstr );

if( stat != 0 ) return( stat );
	
DT_suppress_twin_warn( 0 );


/* set variables to hold plot area bounds and scale min/max.. */
if( PLS.usingcm ) cmadj = 2.54;
else cmadj = 1.0;
setfloatvar( "AREATOP", EYhi * cmadj, "%g" );
setfloatvar( "AREABOTTOM", EYlo * cmadj, "%g" );
setfloatvar( "AREALEFT", EXlo * cmadj, "%g" );
setfloatvar( "AREARIGHT", EXhi * cmadj, "%g" );
setcharvar( "XMIN", xminstr );
setcharvar( "XMAX", xmaxstr );
setcharvar( "YMIN", yminstr );
setcharvar( "YMAX", ymaxstr );


if( areacolor[0] != '\0' ) Ecblock( EXlo, EYlo, EXhi, EYhi, areacolor, 0 );
if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) {  
	PL_clickmap_adjust( clickmap_adjx, clickmap_adjy ); /* set adjustment, if any.  added scg 10/24/06 */
	if( GL_slmember( mapurl, "*@XVAL*" ) || GL_slmember( mapurl, "*@YVAL*" )) clickmap_seturlt( mapurl );
	else clickmap_entry( 'r', mapurl, 0, EXlo, EYlo, EXhi, EYhi, 0, 0, maplabel );
	}

/* draw a frame of the graphics area */
if( doframe ) {
	if( strcmp( frame, "bevel" )==0 ) {
		Ecblockdress( EXlo, EYlo, EXhi, EYhi, 0.1, "0.6", "0.8", 0.0, "" );
		}
	else	{
		linedet( "frame", frame, 1.0 );
		Ecblock( EXlo, EYlo, EXhi, EYhi, "", 1 );
		}
	}



/* draw bottom line */
if( linebottom[0] != '\0' ) {
	linedet( "linebottom", linebottom, 1.0 );
	Emov( EXlo, EYlo );
	Elin( EXhi, EYlo );
	}

/* draw side line */
if( lineside[0] != '\0' ) {
	linedet( "lineside", lineside, 1.0 );
	Emov( EXlo, EYlo );
	Elin( EXlo, EYhi );
	}

/* title */
if( title[0] != '\0' ) {
	textdet( "titledetails", titledet, &align, &adjx, &adjy, 0, "R", 1.0 );
	measuretext( title, &nlines, &maxlen );
	if( align == '?' ) align = 'L';
	if( align == 'L' ) Emov( EXlo + adjx, 
				EYhi + ((nlines-1) * Ecurtextheight ) + adjy + 0.02 );
	else if( align == 'C' ) 
		Emov( EXlo+((EXhi-EXlo) / 2.0 ) + adjx, 
			EYhi + ((nlines-1) * Ecurtextheight ) +adjy + 0.02 );
	else if( align == 'R' ) 
		Emov( EXhi + adjx, EYhi + ((nlines-1) * Ecurtextheight ) +adjy + 0.02);
	Edotext( title, align );
	}
/* title 2 */
if( title2[0] != '\0' ) {
	textdet( "title2details", title2det, &align, &adjx, &adjy, 0, "R", 1.0 );
	measuretext( title2, &nlines, &maxlen );
	if( align == '?' ) align = 'L';
	if( align == 'L' ) Emov( EXlo + adjx, 
		EYhi + ((nlines-1) * Ecurtextheight ) + adjy + 0.02 );
	else if( align == 'C' ) 
		Emov( EXlo+((EXhi-EXlo) / 2.0 ) + adjx, 
			EYhi + ((nlines-1) * Ecurtextheight ) +adjy + 0.02 );
	else if( align == 'R' ) Emov( EXhi + adjx, 
		EYhi + ((nlines-1) * Ecurtextheight ) +adjy + 0.02 );
	Edotext( title2, align );
	}


if( doxaxis ) { 
	if( PLS.debug ) fprintf( PLS.diagfp, "   areadef xaxis...\n" );
	PLP_axis( 'x', 6 ); 
	}

if( doyaxis ) {
	if( PLS.debug ) fprintf( PLS.diagfp, "   areadef yaxis...\n" );
	PLP_axis( 'y', 6 );
	}

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
