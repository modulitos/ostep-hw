/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"

/* constants for clustering.. */
#define NOFS
static double xofst[38] = { 0, 0, 4, 0, -4, 4, -4, -4, 4,
        0, -8, 0, 8, 4, -8, 4, 8, -4, -8, -4, 8,
        0, 0, 12, -12, 4, 4, 12, -12, -4, -4, 12, -12,
        8, -8, -8, 8 };

static double yofst[38] = { 0, 4, 0, -4, 0, 4, -4, 4, -4,
        -8, 0, 8, 0, -8, 4, 8, 4, -8, -4, 8, -4,
        12, -12, 0, 0, 12, -12, 4, 4, 12, -12, -4, -4,
        8, -8, 8, -8 };


#ifdef NONANSI
static int ptcompare();
#else
static int ptcompare(const void *a, const void *b);
#endif

int
PLP_scatterplot()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

int i, nt, stat, align, result, cluster, dupcount, subdupcount, clustevery, verttext, nrow, realrow;
int clustermeth, symfield_userange, dupsleg, irow, dorect, rectoutline, flop2, maxdups, doing_alt;
int xfield, yfield, lblfield, sizefield, symfield;
char *symbol, *linedetails, *text, *textdetails, *selex, *xrange, *yrange;
char *mapurl, *maplabel, *expandedurl, *expandedlabel, *labelword, *altsym, *altwhen, *qcolor;
char buf[512], symcode[80], rhi[40], rlo[40];
char linedir, reqlinedir;
char legendlabel[256];
int colorfield;
char symtmp[80];

double adjx, adjy, linelen, xloc, yloc, radius, x, y, cy, hlinelen, sizescale;
double ox[38], oy[38], clusterfact, oldx, oldy, xlo, xhi, ylo, yhi, clusterdiff, ptx, pty, hw, txhi, rectw, recth, vennden;


TDH_errprog( "pl proc scatterplot" );


/* initialize */
xfield = -1; yfield = -1;
symbol = ""; linedetails = ""; text = ""; textdetails = ""; selex = ""; xrange = ""; yrange = "";
mapurl = ""; maplabel = ""; altsym = ""; altwhen = "";
strcpy( legendlabel, "" );
labelword = "@VAL";
linelen = -1.0;
xloc = 0.0; yloc = 0.0;
vennden = 0.0;
sizescale = 0.5/72.0; /* correspond roughly with pt size */
clusterfact = 0.01;
clusterdiff = 0.001;
lblfield = -1;
sizefield = -1;
colorfield = -1;
symfield = -1;
cluster = 0;   /* changed and added to "breakers" in docs, scg 5/29/06 */
verttext = 0; clustevery = 0; clustermeth = 0; dupsleg = 0; symfield_userange = 0; dorect = 0; rectoutline = 0; doing_alt = 0;
linedir = reqlinedir = '\0'; /* scg 3/4/03 */


/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "xfield" )==0 ) xfield = fref( lineval ) -1;
	else if( strcmp( attr, "yfield" )==0 ) yfield = fref( lineval ) -1;
	else if( strcmp( attr, "labelfield" )==0 ) lblfield = fref( lineval ) -1;
	else if( strcmp( attr, "sizefield" )==0 ) sizefield = fref( lineval ) -1;
	else if( strcmp( attr, "colorfield" )==0 ) colorfield = fref( lineval ) -1;
	else if( strcmp( attr, "symbol" )==0 ) symbol = lineval;
	else if( strcmp( attr, "text" )==0 ) text = lineval;
	else if( strcmp( attr, "textdetails" )==0 ) textdetails = lineval;
	else if( strcmp( attr, "sizescale" )==0 ) sizescale = ftokncpy( lineval ) * 0.5/72.0;
	else if( strcmp( attr, "xrange" )==0 ) xrange = lineval;
	else if( strcmp( attr, "yrange" )==0 ) yrange = lineval;
	else if( strcmp( attr, "clickmapurl" )==0 ) mapurl = lineval;
	else if( strcmp( attr, "clickmaplabel" )==0 ) maplabel = lineval;
        else if( strcmp( attr, "clickmaplabeltext" )==0 ) maplabel = getmultiline( lineval, "get" ); 
	else if( strcmp( attr, "linelen" )==0 ) {
		if( lineval[0] == '\0' ) linelen = -1.0;
		else	{ linelen = ftokncpy( lineval ); if( PLS.usingcm ) linelen /= 2.54; }
		}
	else if( strcmp( attr, "linedir" )==0 ) reqlinedir = lineval[0];
	else if( strcmp( attr, "linedetails" )==0 ) linedetails = lineval;
	else if( strcmp( attr, "xlocation" )==0 ) {
		Eposex( lineval, X, &xloc ); /* val -> lineval scg 5/3/99 */
		if( Econv_error() ) Eerr( 2394, "invalid xlocation", lineval );
		}
	else if( strcmp( attr, "ylocation" )==0 ) {
		Eposex( lineval, Y, &yloc ); /* val -> lineval 5/3/99 */
		if( Econv_error() ) Eerr( 2395, "invalid ylocation", lineval );
		}
	else if( strcmp( attr, "select" )==0 ) selex = lineval;
	else if( strcmp( attr, "legendlabel" )==0 ) { strncpy( legendlabel, lineval, 255 ); legendlabel[255] = '\0'; }
	else if( strcmp( attr, "cluster" )==0 ) cluster = getyn( lineval );
	else if( strcmp( attr, "clusterdiff" )==0 ) { cluster = 1; clusterdiff = ftokncpy( lineval ); }
	else if( strcmp( attr, "clustermethod" )==0 ) { cluster = 1; clustermeth = lineval[0]; } /* h, v, 2, u, r, ..  */ 
	else if( strcmp( attr, "clusterfact" )==0 ) { cluster = 1; clusterfact = ftokncpy( lineval ) * .01; }
	else if( strcmp( attr, "clustevery" )==0 ) { cluster = 1; clustevery = itokncpy( lineval ); if( clustevery < 1 ) clustevery = 1; }
	else if( strcmp( attr, "dupsleg" )==0 ) {
		dupsleg = getyn( lineval );
		if( dupsleg ) { cluster = 1; clustermeth = 'l'; symbol = "sym6a"; }   /* symbol set here and below to guarantee symbol mode */
		}
	else if( strcmp( attr, "symfield" )==0 ) { symbol = "sym6a"; symfield = fref( lineval ) -1; symfield_userange = 0; }
	else if( strcmp( attr, "symrangefield" )==0 ) { symbol = "sym6a"; symfield = fref( lineval ) -1; symfield_userange = 1; }
	else if( strcmp( attr, "verticaltext" )==0 ) verttext = getyn( lineval );
	else if( strcmp( attr, "rectangle" )==0 ) {
		nt = sscanf( lineval, "%lf %lf %s", &rectw, &recth, buf );
		if( nt == 3 ) rectoutline = 1;
		rectw *= 0.5;
		recth *= 0.5;
		rectw = Eax( rectw ) - Eax( 0.0 );
		recth = Eay( recth ) - Eay( 0.0 );
		dorect = 1;
		}
	else if( strcmp( attr, "labelword" ) == 0 ) labelword = lineval;
	else if( strcmp( attr, "vennden" ) == 0 ) vennden = ftokncpy( lineval );
	else if( strcmp( attr, "altsymbol" )==0 ) altsym = lineval;
        else if( strcmp( attr, "altwhen" )==0 ) altwhen = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
/* -------------------------- */
if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );
if( !scalebeenset() )
         return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );

if( xfield < 0 && yfield < 0 ) return( Eerr( 2205, "Niether xfield nor yfield defined", "" ));

if( lblfield >= 0 ) cluster = 0;  /* added scg 12/21/00 */

if( strcmp( legendlabel, "#usexname" )==0 ) getfname( xfield+1, legendlabel ); /* legendlabel[256] */
if( strcmp( legendlabel, "#useyname" )==0 ) getfname( yfield+1, legendlabel ); /* legendlabel[256] */

if( dorect ) symbol = "";


/* now do the plotting work.. */
/* -------------------------- */

if( cluster ) {
	/* make offsets */
	for( i = 0; i < 38; i++ ) {
		ox[i] = xofst[i] * clusterfact;
		oy[i] = yofst[i] * clusterfact;
		}

	/* determine cluster method */
	if( clustermeth == 0 ) {
		if( yfield < 0 ) clustermeth = 'v'; 	 /* 1-d horizontal - cluster vertically (was 'h'-scg 4/21/05) */
		else if( xfield < 0 ) clustermeth = 'h'; /* 1-d vertical - cluster horizontally (was 'v'-scg 4/21/05) */
		else clustermeth = '2'; 		 /* 2-d cluster */
		}
	}

/* ranges */
xlo = EDXlo;
xhi = EDXhi;
ylo = EDYlo;
yhi = EDYhi;
if( xrange[0] != '\0' ) {
	nt = sscanf( xrange, "%s %s", rlo, rhi );
	xlo = Econv( X, rlo );
	if( Econv_error() ) { Eerr( 3958, "xrange bad format", rlo ); xlo = EDXlo; }
	if( nt == 2 ) xhi = Econv( X, rhi );
	if( Econv_error() ) { Eerr( 3958, "xrange bad format", rhi ); xhi = EDXhi; }
	}
if( yrange[0] != '\0' ) {
	nt = sscanf( yrange, "%s %s", rlo, rhi );
	ylo = Econv( Y, rlo );
	if( Econv_error() ) { Eerr( 3958, "yrange bad format", rlo ); ylo = EDYlo; }
	if( nt == 2 ) yhi = Econv( Y, rhi );
	if( Econv_error() ) { Eerr( 3958, "yrange bad format", rhi ); yhi = EDYhi; }
	}




nrow = 0;
for( i = 0; i < Nrecords; i++ ) {

	if( selex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selex, i, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selex ); continue; }
                if( result == 0 ) continue; /* reject */
                }

	/* get x value.. */
	if( xfield >= 0 ) {
		x = fda( i, xfield, 'x' );
        	if( Econv_error() ) { conv_msg( i, xfield, "xfield" ); continue; }
		if( x < xlo || x > xhi ) continue;
		}

	/* get y value.. */
	if( yfield >= 0 ) {
		y = fda( i, yfield, 'y' );
        	if( Econv_error() ) { conv_msg( i, yfield, "yfield" ); continue; }
		if( y < ylo || y > yhi ) continue;
		}

	/* go to absolute units.. */
	if( xfield < 0 ) x = xloc;
	else x = Eax(x);
	if( yfield < 0 ) y = yloc;
	else y = Eay(y);


	/* put (x,y) into PLV array so points can be sorted.. */
	if( nrow >= PLVthirdsize ) {
		fprintf( PLS.errfp, "point capacity exceeded, skipping data point (raise using -maxvector)\n" );
		continue;
		}
	dat3d( nrow, 0 ) = x;
	dat3d( nrow, 1 ) = y;
	dat3d( nrow, 2 ) = (double)i;  /* added scg 12/21/00 - went from dat2d to dat3d */
				       /* need to keep track of actual location in data array for labels, sizefield, etc.. */
	nrow++;
	}


/* if clustering and not using a label field, sort PLV array */
if( cluster && lblfield < 0 && sizefield < 0 && colorfield < 0 ) {
	if( PLS.debug ) fprintf( PLS.diagfp, "sorting points for scatterplot\n" );
	qsort( PLV, nrow, sizeof(double)*3, ptcompare );
	}



if( verttext ) Etextdir( 90 );

/* these are used in clustering.. */
oldx = NEGHUGE;
oldy = NEGHUGE;
dupcount = 0;
subdupcount = 0;
maxdups = 0;

strcpy( symcode, "sym6a" );
radius = 0.04;


/* in the following, text must come before symbol.. */
if( text[0] != '\0' || lblfield >= 0 )  textdet( "textdetails", textdetails, &align, &adjx, &adjy, -3, "R", 1.0 );
	
if( symbol[0] != '\0' ) symdet( "symbol", symbol, symcode, &radius );
	
if( linelen > 0.0 || rectoutline )  linedet( "linedetails", linedetails, 0.5 );
	
cy = Ecurtextheight * 0.3;
hlinelen = linelen * 0.5;
txhi = cy + cy;
if( text[0] != '\0' ) hw = strlen( text ) * Ecurtextwidth * 0.5;

if( colorfield >= 0 ) strcpy( symtmp, symcode );

/* now display points.. */
for( irow = 0; irow < nrow; irow++ ) {
	x = dat3d( irow, 0 );
	y = dat3d( irow, 1 );
	realrow = (int)dat3d( irow, 2 ); /* added scg 12/21/00 */

	/* in this loop, you MUST USE REALROW, NOT IROW for accessing ancillary data fields!! */

	if( cluster ) {
		if( GL_close_to( x, oldx, clusterdiff ) && GL_close_to( y, oldy, clusterdiff ) ) {
			subdupcount++;
			if( subdupcount >= clustevery ) {
				dupcount++;
				subdupcount = 0;
				}

			if( dupcount % 2 == 0 ) flop2 = 1;
			else flop2 = -1;

			if( clustermeth == '2' && dupcount > 37 ) {
				maxdups = 37;
				dupcount = 0; /* mod */
				}

			if( clustermeth == 'h' ) x += ((dupcount+1)/2) * clusterfact * 2.0 * flop2;
			else if( clustermeth == 'v' ) y += ((dupcount+1)/2) * clusterfact * 2.0 * flop2;
			else if( clustermeth == 'u' ) y += dupcount * clusterfact * 2.0; /* 1D upward */
			else if( clustermeth == 'r' ) x += dupcount * clusterfact * 2.0; /* 1D rightward */
			else if( clustermeth == 'l' ) ; /* legend lookup, no offset */
			else if( clustermeth == '2' ) {  x += ox[dupcount%38]; y += oy[dupcount%38]; } /* 2-D */

			if( clustermeth == 'l' ) { /* if more duplicate points coming, skip.. */
				if( irow < nrow-1 ) {
					double nextx, nexty;
					nextx = dat3d( irow+1, 0 );
					nexty = dat3d( irow+1, 1 );
					if( GL_close_to( x, nextx, clusterdiff ) && 
					    GL_close_to( y, nexty, clusterdiff ) ) continue;
					}
				}
			}
		else {
			if( dupcount > maxdups ) maxdups = dupcount;
			oldx = x;
			oldy = y;
			dupcount = 0;
			subdupcount = 0;
			}
		}

	/* allow @field substitutions into url */
        if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) {
		expandedurl = &PL_bigbuf[0];
		expandedlabel = &PL_bigbuf[2000];
		do_subst( expandedurl, mapurl, realrow, URL_ENCODED );
		do_subst( expandedlabel, maplabel, realrow, NORMAL );
		}



	/* render text, mark or line.. */
	/* text can be combined with mark if text and symbol both specified */

	/* symbol or rectangle.. */
	if( symbol[0] != '\0' || dorect || ( text[0] == '\0' && linelen <= 0.0 && lblfield < 0 ) ) {
		if( symfield >= 0 ) {  /* look it up in legend list.. */
			if( symfield_userange ) symbol = PL_get_legent_rg( atof( da( realrow, symfield ) ) );
                	else symbol = PL_get_legent( da( realrow, symfield ));
			if( symbol[0] == '\0' ) Eerr( 7429, "warning: symfield: no matching legend entry tag found", da( realrow, symfield ) );
			if( !dorect ) symdet( "symfield", symbol, symcode, &radius );
			}
		if( dupsleg ) {  /* look it up in legend list.. */
                	symbol = PL_get_legent_rg( (double)dupcount+1 );
			if( symbol[0] == '\0' ) Eerr( 7692, "warning: dupsleg: no appropriate legend entry tag\n", da( realrow, symfield ) );
			if( !dorect ) symdet( "symfield", symbol, symcode, &radius );
			/* note: currently all marks will be rendered; the last one will be on "top"  */
			}

		if( sizefield >= 0 ) radius = sqrt((atof( da( realrow, sizefield ) ) * sizescale)/3.1415927); /* scale the area, not the diameter */

		if( colorfield >= 0 ) {
			qcolor = da( realrow, colorfield );
			sprintf( symcode, symtmp, qcolor );  /* for filled.. build new symcode (symtmp contains %s) */
			Ecolor( qcolor ); /* for symbols made up of lines.. */
			}

		if( dorect ) {
			char *color;
			color = ""; /* added scg 9/1/05 - heatmap bug */
			if( symfield >=0 || dupsleg ) color = symbol; /* was: sscanf( symbol, "%s", color ); // strip off any trailing space */
			if( colorfield >= 0 ) color = qcolor;
			Ecblock( x-rectw, y-recth, x+rectw, y+recth, color, rectoutline );
			symbol = "";
			}

		else if( vennden > 0.0 ) {  /* vennden (undocumented) repeats the symbol progressively bigger (bullseye pattern),
						(early attempt at venn diagram.. not sure if good for anything) */
			double urad;
			for( urad = 0.01; urad < radius; urad += vennden ) Emark( x, y, symcode, urad );
			} 

		else 	{  /* standard scatterplot data point is done here.. */

			if( altwhen[0] != '\0' ) {  /* check for alternate */
				stat = do_select( altwhen, realrow, &doing_alt );
                		if( stat != 0 ) { Eerr( stat, "Select error", altwhen ); continue; }
                		if( doing_alt == 1 ) symdet( "altsym", altsym, symcode, &radius );
				}

			Emark( x, y, symcode, radius );  

			if( doing_alt == 1 ) symdet( "symbol", symbol, symcode, &radius );  /* restore */
			}

		if( PLS.clickmap && (mapurl[0] != '\0' || maplabel[0] != '\0' )) {
			if( dorect ) clickmap_entry( 'r', expandedurl, 0, x-rectw, y-recth, x+rectw, y+recth, 0, 0, expandedlabel );
			else clickmap_entry( 'r', expandedurl, 0, x-radius, y-radius, x+radius, y+radius, 0, 0, expandedlabel );
			}
			
		}

	/* text */
	if( text[0] != '\0' ) {
		if( symbol[0] != '\0' )  /* set text color etc... */
			textdet( "textdetails", textdetails, &align, &adjx, &adjy, -3, "R", 1.0 );
		if( sizefield >= 0 ) Etextsize( (int) (atof( da( realrow, sizefield ) ) * sizescale) );
		if( colorfield >= 0 ) Ecolor( da( realrow, colorfield ) );
		if( verttext ) { ptx = (x+cy)+adjx; pty = y; } /* cy puts midheight of character on point */
		else { ptx = x+adjx; pty = (y-cy)+adjy; }

		convertnl( text );  /* caution (*text) but ok since result is always smaller than original */
		Emov( ptx, pty );
		if( align == '?' ) Edotext( text, 'C' );
		else Edotext( text, align );
		if( symbol[0] != '\0'  )  /* restore symbol color etc... */
			symdet( "symbol", symbol, symcode, &radius );

		if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) 
			clickmap_entry( 'r', expandedurl, 0, ptx-hw, pty, x+hw, y+txhi, 0, 0, expandedlabel );
		}

	/* label from data */
	else if( lblfield  >= 0 ) {
		if( sizefield >= 0 ) Etextsize( (int) (atof( da( realrow, sizefield ) ) * sizescale) );
		if( colorfield >= 0 ) Ecolor( da( realrow, colorfield ) );
		if( verttext) { ptx = (x+cy)+adjx; pty = y+adjy; } /* cy puts midheight of character on point */
		else { ptx = x+adjx; pty = (y-cy)+adjy; } 

		strcpy( buf, labelword );
		GL_varsub( buf, "@VAL", da( realrow, lblfield ) ); /* buf[512] */

		Emov( ptx, pty );
		if( align == '?' ) Edotext( buf, 'C' );
		else Edotext( buf, align );

		if( PLS.clickmap && ( mapurl[0] != '\0' || maplabel[0] != '\0' )) {
			hw = strlen( buf ) * Ecurtextwidth * 0.5;
			if( GL_member( align, "C?" ))clickmap_entry( 'r', expandedurl, 0, ptx-hw, pty, x+hw, y+txhi, 0, 0, expandedlabel );
			else if( align == 'L' ) clickmap_entry( 'r', expandedurl, 0, ptx, pty, x+(hw*2.0), y+txhi, 0, 0, expandedlabel );
			else if( align == 'R' ) clickmap_entry( 'r', expandedurl, 0, ptx-(hw*2.0), pty, x, y+txhi, 0, 0, expandedlabel );
			}
		}

	/* line */         /* (no clickmap support) */   /* no legend support either (?) */
	else if( linelen > 0.0 ) {
		if( sizefield >= 0 ) hlinelen = linelen * 0.5 * atof( da( realrow, sizefield ) ); 
					/* sizefield acts as a scale factor to linelen */
		if( colorfield >= 0 ) Ecolor( da( realrow, colorfield ) );

		if( reqlinedir != '\0' ) linedir = reqlinedir;
		else if( xfield >= 0 && yfield >= 0 ) linedir = 'h'; 	/* arbitrary .. scg 5/16/03 */
		else if( xfield >= 0 ) linedir = 'v';
		else linedir = 'h';			/* scg 3/5/03 */

		if( linedir == 'v' ) { Emov( x, y-hlinelen ); Elin( x, y+hlinelen ); }
		else if( linedir == 'u' ) { Emov( x, y ); Elin( x, y+(hlinelen*2.0) ); }
		else if( linedir == 'r' ) { Emov( x, y ); Elin( x+(hlinelen*2.0), y ); }
		else { Emov( x-hlinelen, y ); Elin( x+hlinelen, y ); }
		}

	}
if( verttext ) Etextdir( 0 );

if( legendlabel[0] != '\0' ) {
	char s[40];
	sprintf( s, "%d", nrow );
	GL_varsub( legendlabel, "@NVALUES", s ); /* legendlabel[256] */
	if( linelen <= 0.0 && lblfield < 0 && text[0] == '\0' )
		PL_add_legent( LEGEND_SYMBOL, legendlabel, "", symbol, "", "" );
	else if( symbol[0] != '\0' && text[0] != '\0' )
		PL_add_legent( LEGEND_SYMBOL+LEGEND_TEXT, legendlabel, "", text, textdetails, symbol );
	else if( linelen > 0.0 ) {
		char dirstr[8];
		sprintf( dirstr, "%c", linedir );
		PL_add_legent( LEGEND_LINEMARK, legendlabel, "", linedetails, dirstr, "" );
		}
	}

setintvar( "NVALUES", nrow );
maxdups++;
setintvar( "MAXDUPS", maxdups );

return( 0 );
}
/* ======================= */

static int
ptcompare( a, b )
const void *a, *b;

/* static int ptcompare( f, g )
 * double *f, *g;
 */  /* changed to eliminate gcc warnings  scg 5/18/06 */

{
double *f, *g;
double *f2, *g2;

f = (double *)a;
g = (double *)b;

if( *f > *g ) return( 1 );
else if( *f < *g ) return( -1 );
else	{
	/* advance to Y component */
	f2 = f+1;
	g2 = g+1;
	if( *f2 > *g2 ) return( 1 );
	else if( *f2 < *g2 ) return( -1 );
	else return( 0 ); /* same */
	}
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
