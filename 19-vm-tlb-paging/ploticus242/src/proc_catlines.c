/* ======================================================= *
 * Copyright 1998-2008      Stephen C. Grubb               *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* proc catlines generates one lineplot per category */
/* category scaling must be in effect in X */
/* input records must be clustered by category then subcategory. */


#include "pl.h"

#define MAXSUBCATLEN 40
#define MAXSUBCATS 20
#define LINES 0
#define BARS 1
#define SYMONLY 2

int
PLP_catlines()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

int i, j, catfield, subcatfield, valfield, errfield, stat, result, nsubcats, ix, gapmissing, curj, doing_alt;
char *selectex, *linedet, *errlinedet, *symdet, *altsym, *altwhen;
char *barcolor, *crossover, *plotwidth;
char buf[256], symcode[40];
char subcatlist[MAXSUBCATS][MAXSUBCATLEN];
char subcat[MAXSUBCATLEN], curcat[MAXSUBCATLEN];
char cat[MAXSUBCATLEN];
char colorlist[MAXSUBCATS][COLORLEN];
char axis;
double xcenter, yval; double tf, ofs, radius; double errval; double panelwidth, halfwidth; double taillen, halftail;
int plotmode;
double ymin, barlinewidth;
int ncolorlist;

TDH_errprog( "pl proc catlines" );

catfield = -1;
subcatfield = -1;
valfield = -1;
errfield = -1;
selectex = "";
linedet = ""; errlinedet = ""; 
symdet = "style=filled color=gray(0.7) shape=circle radius=0.02";
altsym = "";
altwhen = "";
nsubcats = 0;
gapmissing = 1;
taillen = 0.04;
doing_alt = 0;
plotmode = LINES;
crossover = "";
barlinewidth = 2.0;
ncolorlist = 0;
barcolor = "green";
for( i = 0; i < MAXSUBCATS; i++ ) strcpy( colorlist[i], "green" );
plotwidth = "auto";


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "plotmode" )==0 ) { 
		if( lineval[0] == 'b' ) plotmode = BARS;
		else if( lineval[0] == 's' ) plotmode = SYMONLY;
		else plotmode = LINES;
		}
	else if( strcmp( attr, "catfield" )==0 ) catfield = fref( lineval ) - 1;
	else if( strcmp( attr, "subcatfield" )==0 ) subcatfield = fref( lineval ) - 1;
	else if( strcmp( attr, "valfield" )==0 ) valfield = fref( lineval ) - 1;
	else if( strcmp( attr, "subcats" )==0 ) {  /* a list of subcats, or "auto" */
		for( i = 0; lineval[i] != '\0'; i++ ) if( lineval[i] == ',' ) lineval[i] = ' ';
		ix = 0;
		for( i = 0; i < MAXSUBCATS; i++ ) { 
			strcpy( subcatlist[i], GL_getok( lineval, &ix )); 
			if( subcatlist[i][0] == '\0' ) break;
			}
		nsubcats = i;
		}
	else if( strcmp( attr, "errfield" )==0 ) errfield = fref( lineval ) - 1;
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "linedetails" )==0 ) linedet = lineval;
	else if( strcmp( attr, "errbardetails" )==0 ) errlinedet = lineval;
	else if( strcmp( attr, "dpsymbol" )==0 ) symdet = lineval;
	else if( strcmp( attr, "gapmissing" )==0 ) gapmissing = getyn( lineval );
	else if( strcmp( attr, "plotwidth" )==0 ) plotwidth = lineval;
	else if( strcmp( attr, "taillen" )==0 ) taillen = ftokncpy( lineval );
	else if( strcmp( attr, "altsymbol" )==0 ) altsym = lineval;
        else if( strcmp( attr, "altwhen" )==0 ) altwhen = lineval;
	else if( strcmp( attr, "crossover" )==0 ) crossover = lineval;
	else if( strcmp( attr, "barlinewidth" )==0 ) barlinewidth = ftokncpy( lineval );
	else if( strcmp( attr, "barcolor" )==0 ) barcolor = lineval;
	else if( strcmp( attr, "barcolorlist" )==0 ) {		
		for( i = 0; lineval[i] != '\0'; i++ ) if( lineval[i] == ',' ) lineval[i] = ' ';
		ix = 0;
		for( i = 0; i < MAXSUBCATS; i++ ) { 
			strcpy( colorlist[i], GL_getok( lineval, &ix )); 
			if( colorlist[i][0] == '\0' ) break;
			}
		ncolorlist = i;
		}
	else Eerr( 1, "attribute not recognized", attr );
	}


/* -------------------------- */
/* overrides and degenerate cases */
/* -------------------------- */
if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() ) 
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

PL_getunits( 'x', buf );
if( strcmp( buf, "categories" ) != 0 ) 
         return( Eerr( 57, "Category scaletype required in order to use proc catlines", "" ) );

if( (catfield < 0 || catfield >= Nfields )) return( Eerr( 601, "catfield out of range", "" ) );
if( (subcatfield < 0 || subcatfield >= Nfields )) return( Eerr( 601, "subcatfield out of range", "" ) );
if( (valfield < 0 || valfield >= Nfields )) return( Eerr( 601, "valfield out of range", "" ) );
if( nsubcats < 1 ) return( Eerr( 601, "mandatory attribute 'subcats' not specified", "" ) );


/* -------------------------- */
/* now do the plotting work.. */
/* -------------------------- */

if( plotmode == BARS ) {
	axis = 'y';
	if( crossover[0] == '\0' ) ymin = Elimit( axis, 'l', 's' );
	else ymin = Econv( axis, crossover );
	if( ymin < Elimit( axis, 'l', 's' )) ymin = Elimit( axis, 'l', 's' );  /* be sure crossover is in range */
	if( ymin > Elimit( axis, 'h', 's' )) ymin = Elimit( axis, 'h', 's' );  /* be sure crossover is in range */
	}


/* if subcats was given as 'auto' ... scan data to build subcat list (subcats will be in order encountered)... */
if( nsubcats == 1 && strcmp( subcatlist[0], "auto" )==0 ) { 
	nsubcats = 0;
	for( i = 0; i < Nrecords; i++ ) {
		strcpy( subcat, da( i, subcatfield ));
		for( j = 0; j < nsubcats; j++ ) {
			if( strcmp( subcat, subcatlist[j] )==0 ) break;
			}
		if( j == nsubcats ) strcpy( subcatlist[nsubcats++], subcat );
		}
	}

if( nsubcats < 1 ) return( Eerr( 602, "not enough subcategories", "" ) );

/* determine panelwidth (width of one plot not including margin area on either side) */
if( strncmp( plotwidth, "auto", 4 )==0 ) {
	if( plotmode == LINES ) panelwidth = 0.6;
	else 	{     /* bars and symonly */
		panelwidth = (double)nsubcats*0.1;
		if( panelwidth > 0.8 ) panelwidth = 0.8;
		}
	}
else panelwidth = atof( plotwidth );

/* normalize subcatlist over plotwidth */
halfwidth = panelwidth/2.0;
if( nsubcats == 1 ) tf = panelwidth;
else tf = panelwidth/((double)nsubcats-1.0); 


/* input records must be clustered by category, and then in plot (left-to-right) order within category */

/* do error bars if errfield was given.. do these first so they appear "on bottom"... */
if( errfield > -1 ) {

	linedet( "errbardetails", errlinedet, 0.3 );
	
	for( i = 0; i < Nrecords; i++ ) {
	
		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	                stat = do_select( selectex, i, &result );
	                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
	                if( result == 0 ) continue;
			}
	
		xcenter = fda( i, catfield, X );
		strcpy( cat, da( i, catfield ));
		strcpy( subcat, da( i, subcatfield ) );
		yval = fda( i, valfield, Y );
		errval = fda( i, errfield, Y );
	
		/* look up subcat in list.. */
		for( j = 0; j < nsubcats; j++ ) {
			if( strcmp( subcatlist[j], subcat )== 0 ) break;
			}
		if( j == nsubcats ) continue;
		ofs = (j*tf)-halfwidth;
		Emovu( xcenter+ofs, yval-errval );
		Elinu( xcenter+ofs, yval+errval );

		/* tails.. (taillen is in absolute units).. */

		halftail = Edx(EXlo+(taillen/2.0));
		Emovu( (xcenter+ofs)-halftail, yval+errval );
		Elinu( (xcenter+ofs)+halftail, yval+errval );
		/* bottom tail.. */
		Emovu( (xcenter+ofs)-halftail, yval-errval );
		Elinu( (xcenter+ofs)+halftail, yval-errval );
		}
	}


/* render the curves (or bars)... */
if( plotmode == LINES ) linedet( "linedetails", linedet, 0.3 );

	
strcpy( curcat, "" );
curj = -1;
if( plotmode != SYMONLY ) {
	for( i = 0; i < Nrecords; i++ ) {

		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	                stat = do_select( selectex, i, &result );
	                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
	                if( result == 0 ) continue;
	                }
	
		xcenter = fda( i, catfield, X );
		strcpy( cat, da( i, catfield ));
		strcpy( subcat, da( i, subcatfield ) );
		yval = fda( i, valfield, Y );
	
		/* look up subcat in list.. */
		for( j = 0; j < nsubcats; j++ ) {
			if( strcmp( subcatlist[j], subcat )== 0 ) break;
			}
		if( j == nsubcats ) { Eerr( 2759, "encountered a subcat value that wasn't listed in 'subcats'", subcat ); continue; }
	
		ofs = (j*tf)-halfwidth;
	
		/* fprintf( stderr, "[%s %s %g %g]\n", cat, subcat, xcenter, ofs ); */
		if( plotmode == LINES ) {
			if( strcmp( cat, curcat ) != 0 ) Emovu( xcenter+ofs, yval ); /* category break */
			else if( gapmissing && j-1 != curj ) Emovu( xcenter+ofs, yval ); /* missing data - skip */
			else Elinu( xcenter+ofs, yval ); 
			}
		else if( plotmode == BARS ) {
			if( ncolorlist > 0 ) barcolor = colorlist[j];
			sprintf( buf, "width=%g color=%s", barlinewidth, barcolor );
			linedet( "barlinedetails", buf, 0.3 );
			Emovu( xcenter+ofs, ymin );	
			Elinu( xcenter+ofs, yval );
			if( altwhen[0] != '\0' ) {  /* if alternative sym specified, render it w/ bars ... added 2/25/10 */
                       		stat = do_select( altwhen, i, &doing_alt );
                        	if( stat != 0 ) { Eerr( stat, "Select error", altwhen ); continue; }
                        	if( doing_alt == 1 ) {
					symdet( "altsym", altsym, symcode, &radius );
					Emark( Eax( xcenter+ofs ), Eay( yval ), symcode, radius );
					}
                        	}
			}	
		strcpy( curcat, cat );
		curj = j;
		}
	}



/* do data points if required... */
if( plotmode != BARS && strcmp( symdet, "none" ) != 0 ) {
	radius = 0.02;
	symdet( "dpsymbol", symdet, symcode, &radius );
	
	for( i = 0; i < Nrecords; i++ ) {
	
		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	                stat = do_select( selectex, i, &result );
	                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
	                if( result == 0 ) continue;
			}
	
		xcenter = fda( i, catfield, X );
		strcpy( cat, da( i, catfield ));
		strcpy( subcat, da( i, subcatfield ) );
		yval = fda( i, valfield, Y );
	
		/* look up subcat in list.. */
		for( j = 0; j < nsubcats; j++ ) {
			if( strcmp( subcatlist[j], subcat )== 0 ) break;
			}
		if( j == nsubcats ) continue;
		ofs = (j*tf)-halfwidth;

		if( altwhen[0] != '\0' ) {  /* check for alternate */
                        stat = do_select( altwhen, i, &doing_alt );
                        if( stat != 0 ) { Eerr( stat, "Select error", altwhen ); continue; }
                        if( doing_alt == 1 ) symdet( "altsym", altsym, symcode, &radius );
                        }

		Emark( Eax( xcenter+ofs ), Eay( yval ), symcode, radius );

                if( doing_alt == 1 ) symdet( "symbol", symdet, symcode, &radius );  /* restore */
		}
	}

return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
