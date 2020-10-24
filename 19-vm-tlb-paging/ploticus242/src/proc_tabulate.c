/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC TABULATE - do frequency distributions, 1-D or 2-D */

/* Uses data read by most recent getdata; result becomes "current data set"

   scg 1/11/00  - added support for special units even when categories not in use
   scg 1/21/00  - changed tab[][], total[], and grantotal from int to double.
                  added accumfield option.
   scg 1/21/00  - for 2-way dist, column tags are used to set getdata field names

   scg 4/2/04   - added numfmt - %g doesn't give correct results for sets of very small values

*/
#include "pl.h"

#define WORDLEN 40
#define MAXBINS 200
#define MAXROWS 200
#define MAXCOLS 60


static int fsort(), freqsort();

int
PLP_tabulate()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char buf[256], val[256];
double tab[MAXROWS][MAXCOLS];
double total[2][MAXROWS];
double grantotal;
char list[2][MAXROWS][WORDLEN];
int stat, j, nlist[3], select[3], ndim;
int order[2][MAXROWS], valuesgiven[2], dopercents;
char ordering[2];
char tmp[WORDLEN];
char *GL_getok();
double atof();
/* --- */
char *valuelist[2], *rangespec[2], *selectex, *rangesepchar, *showrange, *fieldnamelist, *numfmt;
char axis[2], numbuf[80], hival[80], lowval[80], tag[80];
int field[2], accumfield;
int axisset[2]; /* added 1/11/00 scg */
int i, ix, showresults, irow, result, doranges[2], ixx;
double gran[2], hiv[MAXBINS], lowv[MAXBINS], fval, inc;


TDH_errprog( "pl proc tabulate" );


/* initialize */
field[0] = -1; field[1] = -1;
ndim = -1;
valuesgiven[0] = valuesgiven[1] = 0;
dopercents = 0;
ordering[0] = '?'; ordering[1] = '?';
gran[0] = gran[1] = 0.0;
doranges[0] = doranges[1] = 0;
axis[0] = 'x';
axis[1] = 'y';
rangespec[0] = rangespec[1] = "";
axisset[0] = axisset[1] = 0; 
accumfield = -1;
showresults = 0;
selectex = "";
rangesepchar = "-";
showrange = "";
numfmt = "%g";
valuelist[0] = valuelist[1] = "";
fieldnamelist = "";

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );  
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];


	if( strcmp( attr, "datafield1" )==0 ) { field[0] = fref( lineval ) - 1; ndim = 1; }
	else if( strcmp( attr, "datafield2" )==0 ) { field[1] = fref( lineval ) - 1; ndim = 2; }
	else if( strcmp( attr, "accumfield" )==0 ) accumfield = fref( lineval ) -1;
	else if( strcmp( attr, "valuelist1" )==0 ) valuelist[0] = lineval;
	else if( strcmp( attr, "valuelist2" )==0 ) valuelist[1] = lineval;
	else if( strcmp( attr, "doranges1" )==0 ) doranges[0] = getyn( lineval );
	else if( strcmp( attr, "doranges2" )==0 ) doranges[1] = getyn( lineval );
	else if( strcmp( attr, "showrangelowonly" )==0 ) { if( getyn( lineval ) ) showrange = "low"; }
	else if( strcmp( attr, "showrange" )==0 ) showrange = lineval;
	else if( strcmp( attr, "rangesepchar" )==0 ) rangesepchar = lineval;
	else if( strcmp( attr, "resultfieldnames" )==0 ) fieldnamelist = lineval;
	else if( strcmp( attr, "order1" )==0 ) {  /* mag, rev, nat, none */
		if( lineval[0] == 'm' || lineval[0] == 'r' || lineval[1] == 'a' ) ordering[0] = lineval[0];
		else ordering[0] = 0;
		}
	else if( strcmp( attr, "order2" )==0 ) {
		if( lineval[0] == 'm' || lineval[0] == 'r' || lineval[1] == 'a' ) ordering[1] = lineval[0];
		else ordering[1] = 0;
		}
	else if( strcmp( attr, "percents" )==0 ) dopercents = getyn( lineval );
	else if( strcmp( attr, "showresults" )==0 || strcmp( attr, "savetable" )==0 ) showresults = getyn( lineval );
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "axis1" )==0 ) { axis[0] = lineval[0]; axisset[0] = 1; }
	else if( strcmp( attr, "axis2" )==0 ) { axis[1] = val[0]; axisset[1] = 1; }
	else if( strcmp( attr, "rangespec1" )==0 ) rangespec[0] = lineval;
	else if( strcmp( attr, "rangespec2" )==0 ) rangespec[1] = lineval;
	else if( strcmp( attr, "numfmt" )==0 ) numfmt = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}


/* overrides and degenerate cases */
/* -------------------------- */
if( field[0] < 0 ) return( Eerr( 4984, "datafield1 must be specified", "" ) );

if( ordering[0] == '?' ) {
	if( valuelist[0][0] != '\0' ) ordering[0] = '0';
	else ordering[0] = 'n';
	}
if( ordering[1] == '?' ) {
	if( valuelist[1][0] != '\0' ) ordering[1] = '0';
	else ordering[1] = 'n';
	}



/* now do the computation work.. */
/* -------------------------- */

/* initialize tables */
grantotal = 0.0;
for( i = 0; i < MAXROWS; i++ ) {
        if( i < 2 )nlist[i] = 0;
        for( j = 0; j < MAXCOLS; j++ ) { tab[i][j] = 0.0; if( i < 2 ) { total[i][j] = 0.0; } }
	order[0][i] = order[1][i] = i;
        }


/* get value list (a comma or space delimited list of values; output distribution will
   be for only these values (in the order given?) */
for( j = 0; j < ndim; j++ ) { /* for all dimensions (1 or 2).. */


	if( valuelist[j][0] != '\0' ) {
	    for( i = 0, ix = 0;  ; i++ ) {
		if( GL_getseg( tmp, valuelist[j], &ix, " ," ) ) break;
		strcpy( list[j][i], tmp ); 
		nlist[j]++;
		if( doranges[j] ) { /* find lowv and hiv */
			if( i >= MAXBINS ) return( Eerr( 9285, "Sorry, max number of bins exceeded","" ) );
			ixx = 0;
			GL_getseg( lowval, tmp, &ixx, rangesepchar );
			strcpy( hival, &tmp[ixx] );
			if( strcmp( lowval, "c" )==0 && i > 0 ) lowv[i] = hiv[i-1]; /* contiguous*/
			else lowv[i] = Econv( axis[j], lowval );
			if( Econv_error( ) ) {   /* a non-conformant value.. */
				lowv[i] = PLHUGE; /* so that we know to compare 
							to value for non-numerics */
				hiv[i] = NEGHUGE;
				}
			else 	{
				hiv[i] = Econv( axis[j], hival );
				Euprint( lowval, axis[j], lowv[i], numfmt );  /* lowval[80] */
				if( showrange[0] == 'l' ) strcpy( list[j][i], lowval );
				else if( showrange[0] == 'a' ) Euprint( list[j][i], axis[j], (lowv[i]+hiv[i])/2.0, numfmt ); /* [40] */

				else sprintf( list[j][i], "%s%s%s", lowval, rangesepchar, hival );
				}
			}
		}
	    valuesgiven[j] = 1;
	    }

	/* automatic bins */
	if( rangespec[j][0] != '\0' ) {
		double binsiz, hilimit, rw;
		int nt;
		doranges[j] = 1; /* implied */
		nt = sscanf( rangespec[j], "%s %lf %s", lowval, &binsiz, hival );
		if( nt < 2 || nt > 3 ) return( Eerr( 2740, "2 or 3 values expected in rangespec", 
			rangespec[j] ) );
		rw = Econv( axis[j], lowval );
		if( Econv_error() ) return( Eerr( 2750, "warning: error on rangespec lowlimit", rangespec[j] ) );
		if( nt == 3 ) {
			hilimit = Econv(axis[j], hival );
			if( Econv_error() ) return( Eerr( 2750, "warning: error on rangespec hilimit", rangespec[j] ) );
			}
		else if( nt == 2 ) {
			if( !scalebeenset() ) return( Eerr( 2479, "rangespec must have 3 values since scaleing has not yet been set", "" ));
			hilimit = Elimit( axis[j], 'h', 's' );
			}
		for( i = 0; i < MAXBINS ; i++ ) {
			lowv[i] = rw;
			rw += binsiz;
			hiv[i] = rw;
			Euprint( lowval, axis[j], lowv[i], "" ); /* lowval[80] */
			Euprint( hival, axis[j], hiv[i], "" ); /* hival[80] */
			if( showrange[0] == 'l' ) strcpy( list[j][i], lowval );
			else if( showrange[0] == 'a' ) Euprint( list[j][i], axis[j], (lowv[i]+hiv[i])/2.0, numfmt );  /* [40] */
			else sprintf( list[j][i], "%s%s%s", lowval, rangesepchar, hival );
			nlist[j]++;
			if( rw > hilimit ) break;
			}
		valuesgiven[j] = 1;
		}

	if( doranges[j] && !valuesgiven[j] ) 
		return( Eerr( 2052, "A values list must be given when doing ranges.", "" ));
	}




/* process from data already read in earlier.. */
if( Nrecords < 1 ) return( Eerr( 32, "No data has been read yet.", "" ) );


/* process input data.. */
ix = 0;
for( irow = 0; irow < Nrecords; irow++ ) {

 	if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                stat = do_select( selectex, irow, &result );
                if( stat != 0 ) { Eerr( stat, "Select error", selectex ); continue; }
                if( result == 0 ) continue; /* reject */
                }

        /* for each item requested (1 or 2) */
        for( i = 0; i < ndim; i++ ) {

                /* strcpy( val, data[ field[i] ] ); */
		strcpy( val, da( irow, field[i] ));
		val[WORDLEN-1] = '\0';



		/****  if ranges have not been pre defined... */
                if( !doranges[i] ) { /* classify by value */

			/**** a. if axis has been set, use special units, Econv, etc. */
			if( axisset[i] ) { /* code copied from below; uses fval instead of val */
				fval = Econv( axis[i], val );

				/* see if already encountered (compare fval) */
				for( j = 0; j < nlist[i]; j++ ) 
					if( GL_close_to( fval, atof(list[i][j]), 0.001 ))
                                        	{ select[i] = j; break; }

				if( j == nlist[i] && valuesgiven[i] ) goto NEXT; 

				/* otherwise, add the internal representation to list */
				if( j == nlist[i] ) {
					if( (i == 0 && nlist[i] >= MAXROWS) || (i == 1 && nlist[i] >= MAXCOLS) ) 
					    fprintf( PLS.errfp, "tabulate warning, sorry, table capacity exceeded, skipping %s\n", val );
					else	{
						if( !Econv_error()) 
							sprintf( list[i][j], "%f", fval );
						else strcpy( list[i][j], val );
						select[i] = j; 
						nlist[i] ++; 
						}
					}
				}
			

			/**** b. axis has not been set, dont use Econv */
			else	{

	                        /* see if we've already encountered current val */
	                        for( j = 0; j < nlist[i]; j++ ) if( strcmp( val, list[i][j] )== 0 ) 
	                                                                { select[i] = j; break; }
	
				if( j == nlist[i] && valuesgiven[i] ) goto NEXT; /* not in value list-
										   discard */
	
	                        /* add it to list */
	                        if( j == nlist[i] ) { 
					if( (i == 0 && nlist[i] >= MAXROWS) || (i == 1 && nlist[i] >= MAXCOLS) ) 
						fprintf( PLS.errfp, "tabulate warning, sorry, table capacity exceeded, skipping %s\n", val );
					else	{
						strcpy( list[i][j], val ); 
						select[i] = j; 
						nlist[i] ++; 
						}
					}
	                        }
			}


		/**** if ranges have been defined.. */
                else if( doranges[i] ) {
			fval = Econv( axis[i], val );


			/* go through list backwards so that boundary values are 
			   put into higher category.. */
			for( j = nlist[i] - 1; j >= 0; j-- ) {
				if( lowv[j] >= PLHUGE && hiv[j] <= NEGHUGE ) {
					if( strcmp( list[i][j], val )==0 ) {
						select[i] = j;
						break;
						}
					}
				if( Econv_error() ) continue;
				if( fval >= lowv[j] && fval <= hiv[j] ) {
					select[i] = j;
					break;
					}
				}
			/* if( j == nlist[i] ) goto NEXT; */ /* not in value list- discard */
			if( j < 0 ) goto NEXT;  /* not in value list- discard */
			}

                }
	if( accumfield >= 0 ) inc = atof( da( irow, accumfield ) );
	else inc = 1.0;

        if( ndim == 1 ) { 
                tab[ 0 ][ select[ 0 ] ] += inc; 
                total[0][0] += inc; 
                }
        else if( ndim == 2 ) { 
                ( tab[ select[0] ][ select[1] ] ) += inc; 
                total[0][select[0]] += inc; 
                total[1][select[1]] += inc; 
                grantotal += inc; 
                }

	NEXT: continue;
        }




/* put rows/cols in some kind of order */
if( ndim == 1 ) {
	if( ordering[0] == 'n' ) fsort( list[0], nlist[0], order[0] );
	else if( ordering[0] == 'm' ) freqsort(tab[0], nlist[0], order[0], 0 );
	else if( ordering[0] == 'r' ) freqsort(tab[0], nlist[0], order[0], 1 );
	}
else if( ndim == 2 ) {
	if( ordering[0] == 'n' ) fsort( list[0], nlist[0], order[0] );
	else if( ordering[0] == 'm' ) freqsort(tab[0], nlist[0], order[0], 0 );
	else if( ordering[0] == 'r' ) freqsort(tab[0], nlist[0], order[0], 1 );

	if( ordering[1] == 'n' ) fsort( list[1], nlist[1], order[1] );
	else if( ordering[1] == 'm' ) freqsort(total[1], nlist[1], order[1], 0 );
	else if( ordering[1] == 'r' ) freqsort(total[1], nlist[1], order[1], 1 );
	}


/* --------------------- */
/* generate the results */
/* --------------------- */

/* make a description string */
if( showresults ) {
        if( ndim == 1 ) fprintf( PLS.diagfp, "// proc tabulate has computed this distribution on field %d %s\n", field[0]+1, selectex );
        else if( ndim == 2 ) fprintf( PLS.diagfp, "// proc tabulate has computed this 2-way distribution\n// on field %d (down) by field %d (across) %s\n",
                field[0]+1, field[1]+1, selectex );
	}



/* PL_newdataset(); */
PL_begindataset();

 
/* ------------------ */
/* for 1-way tables.. */
/* ------------------ */
if( ndim == 1 ) {


	/* do output lines.. */
        for( i = 0; i < nlist[0]; i++ ) {
		if( !doranges[0] && axisset[0] ) Euprint( tag, axis[0], atof(list[0][order[0][i]]), "" ); /* tag[80] */
		else strcpy( tag, list[0][order[0][i]] );


		PL_startdatarow();
		PL_catitem( tag ); /* label */

		/* n */
		sprintf( buf, numfmt, tab[0][order[0][i]] ); 
		PL_catitem( buf );

		if( dopercents ) {
			sprintf( buf, numfmt, (double)(tab[0][order[0][i]])/(total[0][0]+0.0001)*100  );
			PL_catitem( buf );
			}
		PL_enddatarow();
		}
	}


/* ------------------ */
/* for 2-way tables.. */
/* ------------------ */
else if( ndim == 2 ) { 

	if( fieldnamelist[0] != '\0' ) {
		definefieldnames( fieldnamelist );
		fprintf( PLS.diagfp, "proc tabulate: field names are now: %s\n", fieldnamelist );
		}
	 
	/* do output lines.. */
        for( j = 0; j < nlist[0]; j++ ) {
		if( !doranges[0] && axisset[0] ) Euprint( tag, axis[0], atof(list[0][order[0][j]]), "" ); /* tag[80] */
		else strcpy( tag, list[0][order[0][j]] );

		PL_startdatarow();
		PL_catitem( tag ); /* label */

                for( i = 0; i < nlist[1]; i++ ) {

			/* n */
			sprintf( numbuf, numfmt, tab[order[0][j]] [order[1][i]] );
			PL_catitem( numbuf );

			if( dopercents ) {
				sprintf( numbuf, numfmt, 
				  	(double)(tab[order[0][j]][order[1][i]])/ (total[1][order[1][i]]+0.00001) * 100 ); 
				PL_catitem( numbuf );
				}
			}
		
		/* row total */
		sprintf( numbuf, numfmt, total[0][order[0][j]] );
		PL_catitem( numbuf );

		if( dopercents ) {
			sprintf( numbuf, numfmt, (double)(total[0][order[0][j]]) / (grantotal+0.00001) * 100 ); 
			PL_catitem( numbuf );
			}
		
		PL_enddatarow();
                }

	}


PL_finishdataset( 0, 0 );

if( showresults ) for( i = 0; i < Nrecords; i++ ) {
	for( j = 0; j < Nfields; j++ ) fprintf( PLS.diagfp, "[%s]", da( i, j ) );
	fprintf( PLS.diagfp, "\n" );
	}
 

return( 0 );
}



/* ================ */
/* sort bin names numerically if all numeric, alphabetically otherwise */
static int
fsort( data, nd, order )
char data[][WORDLEN];
int nd, order[];
{
int i, j, used[MAXROWS], mincell, first, allnum, foo, idiff;
double atof(), diff;

allnum = YES;
for( i = 0; i < nd; i++ ) { 
        used[i] = 0; 
        if( atof( data[i] ) < -31999) strcpy( data[i], "-31000" );
        if( !GL_goodnum( data[i], &foo )) { allnum = NO; }
        }

for( i = 0; i < nd; i++ ) {
        first = YES;
        for( j = 0; j < nd; j++ ) {
                if( used[j] ) continue;
                if( first ) { mincell = j; first = NO; }
                if( allnum ) {
			diff = atof( data[j] ) - atof( data[mincell] );
                	if( diff <= 0.0 ) mincell = j;
			}
                else 	{
			idiff = (strcmp( data[j], data[mincell] ));
			if( idiff <= 0 ) mincell = j; 
			}
                }
        order[i] = mincell;
        used[mincell] = 1;
        }

return( 0 );
}

/* ================ */
static int
freqsort( counts, nd, order, reverse )
double counts[];
int nd, order[], reverse;
{
int i, j, used[MAXROWS], firstcell, first, diff;

for( i = 0; i < nd; i++ )
        used[i] = NO;

for( i = 0; i < nd; i++ ) {
        first = YES;
        for( j = 0; j < nd; j++ ) {
                if( used[j] ) continue;
                if( first ) { firstcell = j; first = NO; }
                diff = counts[j] - counts[firstcell];
                switch (reverse)
                    {
                    case YES: if (diff >= 0) firstcell = j; break;
                    case NO:  if (diff <= 0) firstcell = j; break;
                    }
                }
        order[i] = firstcell;
        used[firstcell] = 1;
        }
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
