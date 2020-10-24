/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC PROCESSDATA - perform various processing to the data set */

#include "pl.h"
#define MAXFLD 80

extern int unlink();
extern int DT_group();

#define MAXBREAKFLDS 5

static int rejfld[MAXFLD], nrejfld, kpfld[MAXFLD], nkpfld, breakcurrow = 0, eofcount = 0;
static char *outfile;
static FILE *outfp;
static int bor(), out(), eor(), dofld(), jadvance();

#ifdef NONANSI
static int dblcompare();
#else
static int dblcompare(const void *a, const void *b);
#endif


/* ================================= */
int
PLP_processdata_initstatic()
{
outfile = "";
breakcurrow = 0;
eofcount = 0;
return(0);
}


/* ================================= */

int
PLP_processdata( )
{
int i, j, k, lvp, first;
char attr[NAMEMAXLEN];
char *line, *lineval;

char *action, *selectex, *select1, *select2, *curcon, *startval, *binsize, *binmod, *fieldnames;
char buf[256], tok[256], outbuf[256];
char breakbuf[ MAXBREAKFLDS ][52];
char rformat[40], nacode[20];
int ix, stat, showdata, nfld, keepall, dispformatnum, resetbns, istart;
int select_result, select_error, nocloseoutfp, tagfld, breakfound, valfld, complen;
int fld[MAXFLD];
double accum[MAXFLD], count;
char newfstr[256];

TDH_errprog( "pl proc processdata" );


/* initialize */
selectex = ""; select1 = ""; select2 = "";
action = ""; outfile = "";
binsize = ""; binmod = "mid";
fieldnames = "";
strcpy( nacode, "=" );
nfld = 0;
strcpy( rformat, "%g" );
keepall = nrejfld = nkpfld = 0;
showdata = dispformatnum = select_error = nocloseoutfp = 0;
tagfld = -1;
valfld = -1;
complen = 50;
strcpy( newfstr, "" );

	

/* get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "action" )==0 ) action = lineval;
	else if( strcmp( attr, "fieldnames" )==0 ) fieldnames = lineval;
	else if( strncmp( attr, "field", 5 )==0 ) {
		ix = 0; i = 0; 
		while( 1 ) {
			strcpy( tok, GL_getok( lineval, &ix ) );
			if( tok[0] == '\0' ) break;
			fld[i] = fref( tok ) -1;
			i++;
			if( i >= MAXFLD ) break;
			}
		nfld = i;
		}
	else if( strcmp( attr, "keepfields" )==0 ) {
		ix = 0; i = 0; 
		while( 1 ) {
			strcpy( tok, GL_getok( lineval, &ix ) );
			if( tok[0] == '\0' ) break;
			kpfld[i] = fref( tok ) -1;
			i++;
			if( i >= MAXFLD ) break;
			}
		nkpfld = i;
		}
	else if( strcmp( attr, "rejectfields" )==0 ) {
		ix = 0; i = 0; 
		while( 1 ) {
			strcpy( tok, GL_getok( lineval, &ix ) );
			if( tok[0] == '\0' ) break;
			rejfld[i] = fref( tok ) -1;
			i++;
			if( i >= MAXFLD ) break;
			}
		nrejfld = i;
		}
	else if( strcmp( attr, "tagfield" )==0 ) tagfld = fref( lineval )-1;
	else if( strcmp( attr, "valfield" )==0 ) valfld = fref( lineval )-1; 
	else if( strcmp( attr, "complen" )==0 ) { complen = itokncpy( lineval ); if( complen > 50 || complen < 1 ) complen = 50; }
	else if( strcmp( attr, "resultformat" )==0 ) tokncpy( rformat, lineval, 40 );
	else if( strcmp( attr, "select" )==0 ) selectex = lineval; /* used by join */
	else if( strcmp( attr, "leftselect" )==0 ) select1 = lineval; /* used by join */
	else if( strcmp( attr, "rightselect" )==0 ) select2 = lineval; /* used by join */
	else if( strcmp( attr, "missingdatacode" )==0 ) tokncpy( nacode, lineval, 20 ); /* used by join */
	else if( strcmp( attr, "showresults" )==0 || strcmp( attr, "showdata" )==0 ) showdata = getyn( lineval );
        else if( strcmp( attr, "keepall" )==0 ) keepall = getyn( lineval );
        else if( strcmp( attr, "outfile" )==0 ) outfile = lineval;
	else if( strcmp( attr, "binsize" )==0 ) binsize = lineval;
	else if( strcmp( attr, "binmod" )==0 ) binmod = lineval;
        else Eerr( 1, "attribute not recognized", attr );
	}

if( strcmp( action, "" )==0) { Eerr( 7395, "warning, no action specified, defaulting to action: echo", "" ); action = "echo"; }

if( Nrecords < 1 ) return( Eerr( 17, "No data has been read yet w/ proc getdata", "" ) );  /* added scg 4/2/08 */

if( strcmp( action, "breakreset" )!= 0 ) {
	if( Nrecords < 1 ) return( Eerr( 17, "Current data set is empty, nothing to process", "" ) );
	}
	
if( GL_slmember( action, "per* tot* acc*") && nfld < 1 ) 
	return( Eerr( 2870, "one or more 'fields' must be specified with percents, totals, or accumulate", "" ) );

if( strcmp( action, "count" )==0 && ( nfld < 1 || nfld > 2 ) )
	return( Eerr( 2874, "'count' action requires one or two fields", "" ));

if( strcmp( action, "segment" )==0 && ( nfld < 1 || nfld > 2 ) )
	return( Eerr( 2874, "'segment' action requires one or two fields", "" ));

if( strcmp( action, "select" )== 0 && selectex[0] == '\0' ) 
	return( Eerr( 3879, "if action is 'select' a selection expression must be given", "" ));




/* now do the work.. */
/* -------------------------- */


if( rformat[0] == 'n' ) {  /* if resultformat begins with 'n', user wants rewritenum to be applied.. */
	dispformatnum = 1; 
	strcpy( rformat, &rformat[1] );
	}
for( i = 0; i < MAXFLD; i++ ) accum[i] = 0.0;

if( outfile[0] == '\0' ) {
	stat = PL_begindataset(); 
	if( stat != 0 ) return( stat );
	}
else 	{
	outfp = fopen( outfile, "w" );
	if( outfp == NULL ) { PLS.skipout = 1; return( Eerr( 75925, "cannot open outfile", outfile ) ); }
	}



/* action: breaks ... break processing - calling script can detect when end is reached by looking at NRECORDS or BREAKFIELD1 */
if( strcmp( action, "breaks" )==0 ) { 
	char breakvarname[20];

	/* start at current row */
	if( breakcurrow >= Nrecords ) {
		eofcount++;
		if( eofcount > 10 ) {
			PLS.skipout = 1;
			return( Eerr( 4729, "unterminated loop (processdata action=breaks)", "" ) );
			}
		goto SKIPBREAK;
		}
	i = breakcurrow;

	/* save initial contents of break fields.. */
	/* also set vars BREAKFIELD1 .. N */
	for( j = 0; j < nfld; j++ ) {
		strncpy( breakbuf[j], da( i, fld[j] ), complen );
		breakbuf[j][complen] = '\0';
		sprintf( breakvarname, "BREAKFIELD%d", j+1 );
		setcharvar( breakvarname, breakbuf[j] );
		}


	for( ; i < Nrecords; i++ ) {

		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
               		stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
                	if( select_result == 0 || stat ) continue; /* reject */
                	}

		/* compare against contents of break fields.. when any differences found, break.. */
		breakfound = 0;
		for( j = 0; j < nfld; j++ ) {
			if( strncmp( breakbuf[j], da( i, fld[j] ), complen ) != 0 ) {
				breakfound = 1;
				break;
				}
			}

		if( breakfound ) break; 

		else 	{
			bor();
			for( j = 0; j < Nfields; j++ ) if( dofld( j )) out( da(i,j) );
			eor();
			}
		}
	breakcurrow = i;
	SKIPBREAK: ;
	}

/* action: breakreset */
else if( strcmp( action, "breakreset" )==0 ) {
	breakcurrow = 0;
	eofcount = 0;
	return( 0 );
	}


/* action: reverse */
else if( strcmp( action, "reverse" )==0 ) { 
	for( i = Nrecords-1; i >= 0; i-- ) {
		
		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
               		stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
                	if( select_result == 0 || stat ) continue; /* reject */
                	}

		bor();
		for( j = 0; j < Nfields; j++ ) if( dofld( j )) out( da(i,j) );
		eor();
		}
	}


/* action: rotate */
else if( strcmp( action, "rotate" )==0 ) {     
	for( j = 0; j < Nfields; j++ ) {
		if( dofld( j )) {
			bor();
			for( i = 0; i < Nrecords; i++ ) {
				if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
		               		stat = do_select( selectex, i, &select_result );
					if( stat ) select_error += stat;
		                	if( select_result == 0 || stat ) continue; /* reject */
		                	}
				out( da( i,j) );
				}
			eor();
			}
		}
	}


/* action: percents */
else if( strncmp( action, "percent", 7 )==0 ) {    
	/* find all totals.. */
	for( i = 0; i < nfld; i++ ) {
		for( j = 0; j < Nrecords; j++ ) {
			
			if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	               		stat = do_select( selectex, i, &select_result );
				if( stat ) select_error += stat;
	                	if( select_result == 0 || stat ) continue; /* reject */
	                	}

			accum[i] += atof( da( j, fld[i] ) );
			}
		}

	for( i = 0; i < Nrecords; i++ ) {

		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	              	stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
	               	if( select_result == 0 || stat ) continue; /* reject */
	               	}

		bor();
		for( j = 0; j < Nfields; j++ ) {
			if( dofld( j )) {
				/* see if j is a 'hot' field */
				for( k = 0; k < nfld; k++ ) if( j == fld[k] ) break;
				if( k != nfld ) {
				    if( keepall ) out( da( i, j ) );
				    sprintf( outbuf, rformat, (atof(da( i, j )) / accum[k]) * 100.0 ); /* changed scg 5/18/06 - quoted
														values not plottable */
				    out( outbuf );
				    }
				else out( da( i, j ) );
				}
			}
		eor();
		}
	}


/* action: accumulate */
else if( strncmp( action, "accum", 5 )==0 ) {     

	for( i = 0; i < Nrecords; i++ ) {

		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	              	stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
	               	if( select_result == 0 || stat ) continue; /* reject */
	               	}

		bor();
		for( j = 0; j < Nfields; j++ ) {
			/* see if j is a 'hot' field */
			if( dofld( j )) {
				for( k = 0; k < nfld; k++ ) if( j == fld[k] ) break;
				if( k != nfld ) {
				    accum[k] += atof( da( i, j ) );
				    if( keepall ) out( da( i, j ) );
				    /* out( da( i, j ) ); */
				    sprintf( tok, rformat, accum[k] ); /* fixed scg 10/1/03 */
				    out( tok ); /* fixed scg 10/1/03 */
				    }
				else out( da( i, j ) );
				}
			}
		eor();
		}
	}


/* action: count - may be used with one or two fields.
 * If one field, result has these fields: 1) field contents 2) count
 * If two fields, result has these fields: 1) field1 contents 2) sum of field 2 
 * Output has a fixed number of fields; kpfld and nkpfld do not apply to this action.. 
 * 'binsize' and 'binmod' attributes can be used in order to do numeric or date-based grouping.
 * 'complen' attribute can be used to dictate number of significant chars in string comparisons.
 */
else if( strcmp( action, "count" )==0 ) {
	double fval, fbin;
	char *adjval, *curval, snum[80];

	if( nfld == 1 ) strcpy( newfstr, "bin count" );
	else if( nfld == 2 ) strcpy( newfstr, "bin sum" );
	adjval = tok;
	curval = buf;
	count = 0.0;
	strcpy( curval, "" );
	for( i = 0; i < Nrecords; i++ ) {

		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	              	stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
	               	if( select_result == 0 || stat ) continue; /* reject */
	               	}

		if( binsize[0] != '\0' ) {
			fbin = atof( binsize );
			if( fbin != 0.0 ) {
				fval = atof( da( i, fld[0] ));
				sprintf( adjval, "%g", GL_numgroup( fval, fbin, binmod ) );
				}
			else	{
				stat = DT_group( binsize[0], binmod[0], da( i, fld[0] ), adjval );
				if( stat != 0 ) { PLS.skipout = 1; return( Eerr( 27395, "invalid date/time data, or invalid binsize or binmod", "" )); }
				}
			}
		else 	{
			strncpy( adjval, da( i, fld[0] ), complen );
			adjval[ complen ] = '\0';
			}


		if( strcmp( adjval, curval )!=0 ) {
			if( i == 0 ) strcpy( curval, adjval );
			else	{
				bor();
				out( curval );
				sprintf( snum, "%g", count ); out( snum ); 
				eor();
				}
			strcpy( curval, adjval );
			count = 0.0;
			}
		if( nfld == 1 ) count = count + 1.0;
		else if( nfld == 2 && action[0] == 'c' ) count = count + atof( da( i, fld[1] ) );
		}
	/* last round.. */
	bor();
	out( curval );
	sprintf( snum, "%g", count ); out( snum ); 
	eor();
	}


/* action: segment - may be used with one or two fields.
 * If one field, result has these fields: 1) field contents 2) beginning record# 3) ending record#
 * If two fields, result has these fields: 1) field1 contents 2) beginning record field2 value 3) ending record field2 value
 *
 * action: segmentb - same as segment, but segments butt up against each other (end point coincides with beginning point of next seg)
 * 
 * output has a fixed number of fields; kpfld and nkpfld do not apply to this action.. 
 */
else if( strncmp( action, "segment", 7 ) ==0 ) {    
	if( nfld == 1 ) strcpy( newfstr, "bin startrow endrow" );
	else if( nfld == 2 ) strcpy( newfstr, "bin startval endval" );
	count = 0.0;
	curcon = "";
	for( i = 0; i < Nrecords; i++ ) {

		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	              	stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
	               	if( select_result == 0 || stat ) continue; /* reject */
	               	}

		if( strcmp( da( i, fld[0] ), curcon )!=0 ) {
			if( i == 0 ) curcon = da( i, fld[0] ); 
			else	{
				bor();
				out( curcon );
				if( nfld == 2 ) {
					out( startval );
					if( action[7] == 'b' ) out( da( i, fld[1] ));
					else out( da( i-1, fld[1] ));
					}
				else	{
					sprintf( buf, "%d", istart ); 
					out( buf );
					if( action[7] == 'b' ) sprintf( buf, "%d", i+1 );
					else sprintf( buf, "%d", i ); 
					out( buf );
					}
				eor();
				curcon = da( i, fld[0] );
				count = 0.0;
				}

			if( action[0] == 's' ) {
				if( nfld == 2 ) startval = da( i, fld[1] ); 
				else istart = i+1;
				}
			}
		}
	/* last round.. */
	bor();
	out( curcon );
	if( nfld == 2 ) { out( startval ); out( da( i-1, fld[1] )); }
	else	{ sprintf( buf, "%d", istart ); out( buf ); sprintf( buf, "%d", i ); out( buf ); }
	eor();
	}


/* action: summary  and action: summaryplus  */
else if( strncmp( action, "summary", 7 )==0 ) {  
	int icount, prec, lastdone, nvect;
	double fval, mean, sd, se, min, max, pctile;
	char *val;

	if( valfld < 0 ) return( Eerr( 5723, "action: summary requires valfield", "" ) );

 	/* note: output has a fixed number of fields; kpfld and nkpfld do not apply to this action.. */

	/* initialize breakfields.. */
	for( j = 0; j < nfld; j++ ) { strncpy( breakbuf[j], da( 0, fld[j] ), complen ); breakbuf[j][complen] = '\0'; }

	/* set up usable field names for result:  id1 .. idN  mean sd se n_obs sum   */
	for( j = 0; j < nfld; j++ ) { sprintf( outbuf, "id%d ", j+1 ); strcat( newfstr, outbuf ); }
	if( action[7] == 'p' ) strcat( newfstr, "mean sd se n_obs min max sum pctl5th pctl25th median pctl75th pctl95th" );
	else strcat( newfstr , "mean sd se n_obs min max sum" );

	/* go thru the data records.. */
	lastdone = 0; 
	accum[0] = 0.0; accum[1] = 0.0; icount = 0; min = PLHUGE; max = NEGHUGE; nvect = 0;
	for( i = 0; i < Nrecords; i++ ) {

                if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                        stat = do_select( selectex, i, &select_result );
                        if( stat ) select_error += stat;
                        if( select_result == 0 || stat ) continue; /* reject */
                        }

                /* compare against contents of break fields.. when any differences found, break.. */
                breakfound = 0;
                for( j = 0; j < nfld; j++ ) {
                        if( strncmp( breakbuf[j], da( i, fld[j] ), complen ) != 0 ) {
                                breakfound = 1;
                                break;
                                }
                        }

                if( breakfound ) {  /* write out mean, sd, etc. */
			LASTCASE:
			if( icount > 0 ) {
                        	bor();
                		for( j = 0; j < nfld; j++ ) out( breakbuf[j] );	/* identifiers */
				mean = accum[0] / (double)icount;
				sprintf( outbuf, rformat, mean ); out( outbuf ); /* mean */
				if( icount == 1 ) sd = 0.0;
				else sd = sqrt( (accum[1] - (accum[0]*accum[0] / (double)icount ) ) / (double)(icount-1) );
				sprintf( outbuf, rformat, sd ); out( outbuf ); /* sd */
				se = sd / sqrt( (double) icount );
				sprintf( outbuf, rformat, se ); out( outbuf ); /* se */
				sprintf( outbuf, "%d", icount ); out( outbuf ); /* N */
				sprintf( outbuf, rformat, min ); out( outbuf ); /* min */
				sprintf( outbuf, rformat, max ); out( outbuf ); /* max */
				sprintf( outbuf, rformat, accum[0] ); out( outbuf ); /* summation */
				if( action[7] == 'p' ) {  /* compute median and quartiles/percentiles */
					int n;
					n = nvect;

					qsort( &PLV[1], nvect, sizeof(double), dblcompare);

					/* these formulas depend on values being placed into array at cells PLV[1] thru PLV[n] 
					   (probably because they were translated from fortran or similar */
					pctile = (n % 20 ) ? PLV[(n/20) + 1] :  (PLV[n/20] + PLV[(n/20) + 1] ) /2.0 ;  /* 5th */
					sprintf( outbuf, rformat, pctile ); out( outbuf );
					pctile = ( n % 4 ) ?  PLV[(n/4) + 1]  :  (PLV[n/4] + PLV[(n/4) + 1])/2.0 ;      /* 25 */
					sprintf( outbuf, rformat, pctile ); out( outbuf );
					pctile = ( n % 2 ) ?  PLV[(n+1) / 2]  :  (PLV[n/2] + PLV[(n/2)+1])/2.0 ;   /* median/ 50th */
					sprintf( outbuf, rformat, pctile ); out( outbuf );
					pctile = ( n % 4 )  ? PLV[n - (n/4)]  :  (PLV[(n+1) - (n/4)] + PLV[n-(n/4)])/2.0 ;   /* 75 */
					sprintf( outbuf, rformat, pctile ); out( outbuf );
					pctile = ( n % 20 ) ? PLV[n - (n/20)] : (PLV[(n+1) - (n/20)] + PLV[n - (n/20)]) / 2.0 ;  /* 95 */
					sprintf( outbuf, rformat, pctile ); out( outbuf );
					}
                        	eor();
				if( lastdone ) break;
				}
			/* get ready to continue.. */
			for( j = 0; j < nfld; j++ ) { strncpy( breakbuf[j], da( i, fld[j] ), complen ); breakbuf[j][complen] = '\0'; }
			accum[0] = 0.0; accum[1] = 0.0; icount = 0; min = PLHUGE; max = NEGHUGE; nvect = 0;
			}

                /* add to accum.. (accum[0] holds sum,  accum[1] holds sum squared) */
		val = da( i, valfld );
		if( GL_goodnum( val, &prec )) {
			fval = atof( val );
			accum[0] += fval;
			accum[1] += fval*fval;
			if( fval < min ) min = fval;
			if( fval > max ) max = fval;
			if( action[7] == 'p' ) {
				if( nvect <= PLVsize-1 ) { nvect++; PLV[nvect] = fval; } /* save for median/quartiles computation */
                		else return( Eerr( 248, "cannot compute median, vector capacity exceeded (raise using -maxvector)\n", "" ) );
				}
			icount++;
                        }
                }
	if( !lastdone ) { lastdone = 1; goto LASTCASE; }
	}

/* action: raccum */
else if( strncmp( action, "raccum", 6 )==0 ) {
	for( i = 0; i < Nrecords; i++ ) {

                if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
                        stat = do_select( selectex, i, &select_result );
                        if( stat ) select_error += stat;
                        if( select_result == 0 || stat ) continue; /* reject */
                        }

		for( j = 0; j < nfld; j++ ) accum[j] = 0.0;  /* initialize */
		for( j = 0; j < nfld; j++ ) {  /* do the progressive summation operation */
			if( j == 0 ) accum[j] = atof( da( i, fld[j] ));
			else if( j > 0 ) accum[j] = atof( da( i, fld[j] )) + accum[j-1];
			}

		/* output the data row, including rewritten fields */
		bor();
		for( j = 0; j < Nfields; j++ ) {
			for( k = 0; k < nfld; k++ ) if( fld[k] == j ) break;
			if( k != nfld ) { 
				if( keepall ) out( da( i, j ) );
				sprintf( outbuf, rformat, accum[k] ); out( outbuf ); 
				}
			else if( dofld( j )) out( da( i, j ));
			}
		eor();
		}
	}


/* action: total */
else if( strncmp( action, "total", 5 )==0 )  {
	for( i = 0; i < nfld; i++ ) {
		for( j = 0; j < Nrecords; j++ ) {
			
			if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
		              	stat = do_select( selectex, i, &select_result );
				if( stat ) select_error += stat;
		               	if( select_result == 0 || stat ) continue; /* reject */
		               	}

			accum[i] += atof( da( j, fld[i] ) );
			}
		}
	}


/* action: join */
else if( strcmp( action, "join" )==0 || strcmp( action, "leftjoin" )==0 || strcmp( action, "rightjoin" )==0 ) {
	int irec1, irec2, diff, prec, more1, more2;
	char *f1, *f2;
	more1 = more2 = 1;
	irec1 = irec2 = -1;
	jadvance( select1, &irec1, &more1 );   /* advance LHS to first eligible record (based on leftselect) */  
	jadvance( select2, &irec2, &more2 );   /* advance RHS to first eligible record (based on rightselect) */

	while( 1 ) {
		/* fprintf( stderr, "[%d][%d]\n", more1, more2 ); */
		if( !more1 || !more2 ) break;
	
		/* compare all join fields.. when first difference encountered break; save strcmp diff */
		for( diff = 0, i = 0; i < nfld; i++ ) {
			f1 = da( irec1, fld[i] );
			f2 = da( irec2, fld[i] );
			/* fprintf( stderr, "[%s vs. %s]..", f1, f2 ); */
			/* if both are integers, do a numeric comparison.. */
			if( GL_goodnum( f1, &prec ) && GL_goodnum( f2, &prec )) diff = atoi( f1 ) - atoi( f2 );
			/* otherwise do a strcmp */
			else diff = strcmp( da( irec1, fld[i]),   da( irec2, fld[i] ));
			if( diff != 0 ) break;
			}
		/* fprintf( stderr, "diff=%d..", diff ); */

		if( diff == 0 ) {
			/* if diff == 0 then join left record with right record and output */
			bor();
			for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( da( irec1, i ));
			for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( da( irec2, i ));
			eor();
			/* fprintf( stderr, "LR..\n" ); */

			jadvance( select1, &irec1, &more1 );   /* advance LHS to next eligible record */  
			jadvance( select2, &irec2, &more2 );   /* advance RHS to next eligible record */
			}

		else if( diff < 0 ) {
			if( action[0] == 'l' ) { /* leftjoin... output LHS fields and missingcode for the RHS fields.. */
				bor();
				for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( da( irec1, i ));
				for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( nacode );
				eor();
				/* fprintf( stderr, "L=..\n" ); */
				}
			jadvance( select1, &irec1, &more1 );   /* advance LHS to next eligible record */
			}

		else if( diff > 0 ) {
			if( action[0] == 'r' ) { /* rightjoin... output missingcode for the LHS fields and output RHS fields.. */
				bor();
				for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( nacode );
				for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( da( irec2, i ));
				eor();
				/* fprintf( stderr, "=R..\n" ); */
				}
			jadvance( select2, &irec2, &more2 );   /* advance RHS to next eligible record */
			}
		}

	/* with leftjoin and rightjoin there may be orphan records at the end to take care of... */
	if( action[0] == 'l' && more1 ) while( more1 ) {   /* output the rest of LHS */
		bor();
		for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( da( irec1, i ));
		for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( nacode );
		eor();
		jadvance( select1, &irec1, &more1 );   
		}
	else if( action[0] == 'r' && more2 ) while( more2 ) { /* output the rest of RHS */
		bor();
		for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( nacode );
		for( i = 0; i < Nfields; i++ ) if( dofld( i )) out( da( irec2, i ));
		eor();
		jadvance( select2, &irec2, &more2 );   
		}
	}


/* action: stats */
else if( strcmp( action, "stats" )==0 )  {
	double val, min, max, sqrt();
	int n, prec, nbad;

	n = 0; 
	nbad = 0;
	max = NEGHUGE;
	min = PLHUGE;
	
	/* accum[0] holds sum, accum[1] holds sumsq.. */
	for( j = 0; j < Nrecords; j++ ) {
			
		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
	              	stat = do_select( selectex, j, &select_result );
			if( stat ) select_error += stat;
	               	if( select_result == 0 || stat ) continue; /* reject */
	               	}
		for( k = 0; k < nfld; k++ ) {
			strcpy( tok, da( j, fld[k] ) );
			if( !GL_goodnum( tok, &prec )) { nbad++; continue; }
			n++;
			val = atof( tok );
			accum[0] += val;  /* sum */
			accum[1] += (val * val); /* sum squared */
			if( val > max ) { 
				max = val; 
				if( tagfld >= 0 ) strcpy( breakbuf[0], da( j, tagfld )); 
				}
			if( val < min ) { 
				min = val; 
				if( tagfld >= 0) strcpy( breakbuf[1], da( j, tagfld )); 
				}
			}
		}
	setintvar( "N", n );
	setfloatvar( "TOTAL", accum[0], rformat );
	setintvar( "NMISSING", nbad );
	if( n == 0 ) { setcharvar( "MEAN", "n/a" ); }
	else	{
		setfloatvar( "MEAN", accum[0]/(double)n, rformat );
		if( n > 1 ) setfloatvar( "SD", sqrt( ( accum[1] - (accum[0]*accum[0]/(double)n )) / ((double)n-1.0) ), rformat );
		else setcharvar( "SD", "n/a" ); 
		setfloatvar( "MAX", max, rformat );
		if( tagfld >= 0 ) setcharvar( "MAX_ID", breakbuf[0] ); /* fixed scg 3/8/05 */
		setfloatvar( "MIN", min, rformat );
		if( tagfld >= 0 ) setcharvar( "MIN_ID", breakbuf[1] ); /* fixed scg 3/8/05 */
		}
	return( 0 );
	}


else if( strcmp( action, "echo" )==0 || strcmp( action, "numberrows" )==0 ) {
	int do_numrows, foundrows;
	char numstr[20];

	do_numrows = 0;
	if( action[0] == 'n' ) do_numrows = 1; 

	/* just write out fields */
	for( i = 0, foundrows = 0; i < Nrecords; i++ ) {
		if( selectex[0] != '\0' ) { /* process against selection condition if any.. */
               		stat = do_select( selectex, i, &select_result );
			if( stat ) select_error += stat;
                	if( select_result == 0 || stat ) continue; /* reject */
                	}
		foundrows++;
		bor();
		if( do_numrows ) { sprintf( numstr, "%d", foundrows ); out( numstr ); }
		for( j = 0; j < Nfields; j++ ) if( dofld( j ) ) {
			out( da( i, j ) );
			}
		eor();
		}
	}

else 	{
	PLS.skipout = 1;
	return( Eerr( 471, "unrecognized action specified.", action ) );
	}

if( select_error ) Eerr( 472, "warning, an error occurred during 'select'", "" );


if( GL_slmember( action, "per* acc* tot*" )) {
	/* make a comma-delimited list of totals for TOTALS */
	strcpy( buf, "" );
	for( i = 0; i < nfld; i++ ) {
		char out[40];
		sprintf( out, rformat, accum[i] );
		if( dispformatnum ) {  /* rewrite using numbernotation */
			resetbns = 0;
			if( PLS.bignumspacer == '\0' ) {
				PLS.bignumspacer = ',';
				resetbns = 1;
				}
			rewritenums( out ); /* rewrite w/spacing, decimal pt options*/
			if( resetbns ) PLS.bignumspacer = '\0';
			}
		strcat( buf, out );
		strcat( buf, "," );
		}
	buf[ strlen( buf ) -1 ] = '\0'; /* last comma */
	setcharvar( "TOTALS", buf );

	/* if doing totals, exit here */
	if( strncmp( action, "tot", 3 )==0 ) return( 0 );
	}

if( outfile[0] != '\0' ) fclose( outfp );
else PL_finishdataset( 0, 0 );

if( fieldnames[0] != '\0' ) definefieldnames( fieldnames ); 
else	{
	/* if field names not given in this proc, currently defined field names (if any) are guaranteed to be wrong 
	   for certain actions or parameter combinations... for those clear the field name list now.. 
	   Was done earlier but this messed up execution of select conditions (based on old field names) */
	if( nrejfld != 0 || nkpfld != 0 || keepall != 0 ) definefieldnames( "" );
	if( GL_smember( action, "rotate join leftjoin rightjoin numberrows" )) definefieldnames( "" );  

	/* other situations... new field names are a given... */
	if( newfstr[0] != '\0' ) definefieldnames( newfstr );
	}


if( showdata ) {
	getfname( 1, buf ); /* buf[256] */
	fprintf( PLS.diagfp, "// proc processdata created the following data set (action = %s)\n", action );
	if( buf[0] != '\0' ) { 
		fprintf( PLS.diagfp, "// field names are: " ); 
		for( j = 0; j < Nfields; j++ ) { getfname( j+1, buf ); fprintf( PLS.diagfp, "%s|", buf ); } /* buf[256] */
		fprintf( PLS.diagfp, "\n" );
		}
	else fprintf( PLS.diagfp, "// (no field names defined)\n" );
	for( i = 0; i < Nrecords; i++ ) {
		for( j = 0; j < Nfields; j++ ) fprintf( PLS.diagfp, "%s|", da( i, j ) );
		fprintf( PLS.diagfp, "\n" );
		}
	fprintf( PLS.diagfp, "// end of data set\n" );
	}

return( 0 );
}

/* ================ */
/* DOFLD - return 1 or 0 depending on whether field has been listed
   in keepfields or rejectfields */
static int
dofld( fld )
int fld;
{
int i;
if( nrejfld > 0 ) {
	for( i = 0; i < nrejfld; i++ ) if( fld == rejfld[i] ) break;
	if( i != nrejfld ) return( 0 ); /* no */
	}
if( nkpfld > 0 ) {
	for( i = 0; i < nkpfld; i++ ) if( fld == kpfld[i] ) break;
	if( i != nkpfld ) return( 1 ); /* yes */
	else return( 0 ); /* no */
	}
return( 1 );
}

/* ================= */
/* OUTPUT mgmt routines */

static int 
bor( )
{
if( outfile[0] == '\0' ) return( PL_startdatarow() );
else return( 0 );
}

static int
out( s )
char *s;
{
if( outfile[0] != '\0' ) fprintf( outfp, "%s	", s );
else PL_catitem( s );
return( 0 );
}

static int
eor()
{
if( outfile[0] != '\0' ) fprintf( outfp, "\n" );
else PL_enddatarow(); 
return( 0 );
}

/* ============================= */
/* for action: join  ... */

static int
jadvance( select, irow, moreflag )
char *select;
int *irow, *moreflag;
{
int sresult, newirow;
newirow = *irow;
newirow++;
for( ; newirow < Nrecords; newirow++ ) {
	do_select( select, newirow, &sresult );
	if( sresult ) break;
	}
if( newirow >= Nrecords ) *moreflag = 0;  /* and irow remains the same as passed in..  scg 11/15/07 */
else  *irow = newirow; 
return( 0 );
}


/* ============================= */
/* for qsort */
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
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

