/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* management of multiple data sets */

#include "pl.h"
#include <string.h> 

static int buflen;
static int nfields, prevnfields;


/* ====================== */
/* DA - return contents of a cell in the current data set using a row and column location.  */
char *
PL_da( r, c )
int r, c;  /* row, col */
{
int base;
base = PLD.dsfirstdf[ PLD.curds ];
if( r >= Nrecords ) { Eerr( 72035, "out-of-bounds data access", "" ); return( "" ); }
if( c >= Nfields ) { Eerr( 72036, "out-of-bounds data access", "" ); return( "" ); }
return( PLD.df[ base + ( r * Nfields ) + c ] );
}


/* ========================= */
/* CLEARDATASETS - free any/all previously malloc'ed datarow memory, and initialize counters */
int
PL_cleardatasets()
{
int i;
for( i = 0; i < PLD.currow; i++ ) free( PLD.datarow[i] ); /* free rows */
PLD.curds = -1;
PLD.curdf = 0;
PLD.currow = 0;
if( PLS.debug ) fprintf( PLS.diagfp, "Clearing all data sets.\n" );
return( 0 );
}

/* ========================= */
/* POPDATASET - pop back to previous dataset on stack, or all the way back to original data set;
 *     also free all datarow memory associated with the higher level datasets which will no longer be available.
 */
int
PL_popdataset( npop )
int npop;  /* 0 = pop all... go back to the original data set  ...or...  number of stack elements to pop */
{
int i, newds;

if( npop ) newds = PLD.curds-npop;
else newds = 0;

if( newds < 0 ) newds = 0;

if( PLD.curds > 0 ) {
	for( i = PLD.dsfirstrow[ newds+1 ]; i < PLD.currow; i++ ) free( PLD.datarow[i] ); /* free all rows in datasets 1 and up */
	PLD.currow = PLD.dsfirstrow[ newds+1];
	PLD.curdf = PLD.dsfirstdf[ newds+1 ];
	}

PLD.curds = newds;

setintvar( "NRECORDS", PLD.nrecords[ PLD.curds ] );
setintvar( "NFIELDS", PLD.nfields[ PLD.curds ] );

if( PLS.debug ) fprintf( PLS.diagfp, "Popping back to dataset %d  which has %d rows and %d fields.\n", 
	PLD.curds+1, PLD.nrecords[PLD.curds], PLD.nfields[PLD.curds] );
return( 0 );
}

/* ========================= */
/* BEGINDATASET - begin the process of creating a new data set.  Data will later be added to the data set 
 * using startdatarow(), catitem(), and enddatarow() (which allocate PLD rows) * ..or.. by proc_getdata() for in-script data,
 * where we don't allocate PLD rows but do allocate df pointers into proclines.
 * This routine can be used even if it's not definite that a new data set will eventually be created.
 */
int
PL_begindataset()
{
int newds;

newds = PLD.curds + 1;
if( newds >= MAXDS ) { PLS.skipout = 1; return( Eerr( 2358, "too many data sets in memory", "" )); }
PLD.nrecords[ newds ] = 0;
PLD.dsfirstdf[ newds ] = PLD.curdf;
PLD.dsfirstrow[ newds ] = PLD.currow;
return( 0 );
}

/* ========================== */
/* FINISHDATASET - finish the process of creating a new data set.  
 * When this routine returns, dataaccess will be ready to get data from the newly created data set.
 */
int
PL_finishdataset( nrows, nfields )
int nrows, nfields;  /* these may be specified if known or passed as 0, 0  which means figure it out here.. */
{
int newds;
newds = PLD.curds+1;
if( nrows == 0 && nfields == 0 ) {
	PLD.nrecords[ newds ] = PLD.currow - PLD.dsfirstrow[ newds ];
	/* turns out we can't do the following.. for example processdata action: breaks a dataset of nrows=0 is valid */
	/* if( PLD.nrecords[ newds ] < 1 ) return( 0 ); */ /* no data.. abort */
	if( PLD.nrecords[newds] < 1 ) PLD.nfields[ newds ] = 0;
	else PLD.nfields[ newds ] = (PLD.curdf - PLD.dsfirstdf[ newds ]) / PLD.nrecords[ newds ];
	}
else	{
	PLD.nrecords[ newds ] = nrows;
	PLD.nfields[ newds ] = nfields;
	}
PLD.curds = newds;
setintvar( "NRECORDS", PLD.nrecords[ PLD.curds ] );
setintvar( "NFIELDS", PLD.nfields[ PLD.curds ] );

if( PLS.debug ) fprintf( PLS.diagfp, "Creating dataset %d  which has %d rows and %d fields.\n", 
	PLD.curds+1, PLD.nrecords[newds], PLD.nfields[newds] );

return( 0 );
}



/* ================================ */
/* STARTDATAROW - when building a new data set directly, (eg. proc processdata) 
   this is used to indicate the start of a new data row.
 */

int
PL_startdatarow()
{
buflen = 0;
nfields = 0;
prevnfields = 0;
return( 0 );
}

/* ================================ */
/* CATITEM - when building a new data set directly, (eg. proc processdata) 
   this is used to append a new piece of data to the current row.
 */

int
PL_catitem( item )
char *item;
{
int len;
if( PLS.skipout ) return( 1 );
len = strlen( item );
strcpy( &PL_bigbuf[ buflen ], item );
buflen += len;
strcpy( &PL_bigbuf[ buflen ], "\t" );
buflen++;
nfields++;
return( 0 );
}

/* ================================ */
/* ENDDATAROW - when building a new data set directly, (eg. proc processdata) 
   this is used to terminate a data row.  The row is actually
   added to the pl data structures at this point.. 
 */

int
PL_enddatarow()
{
int i, state;
char *r;

if( PLS.skipout ) return( 1 );

r = (char *) malloc( buflen+1 );
if( r == NULL ) { PLS.skipout = 1; return( Eerr( 2378, "malloc error", "" )); }

if( PLD.currow >= (PLD.maxrows-1)) { PLS.skipout = 1; return( Eerr( 2380, "new data set truncated, too many rows", "" )); }

PLD.datarow[ PLD.currow++ ] = r;

strcpy( r, PL_bigbuf );

/* parse fields and assign data field pointers.. */
for( i = 0, state = 0; i < buflen; i++ ) {
	if( state == 0 ) { 
		if( PLD.curdf >= PLD.maxdf ) { 
			PLS.skipout = 1; 
			return( Eerr( 2381, "new data set truncated, too many fields overall", "" ));
			}
		PLD.df[ PLD.curdf++ ] = &r[i];
		state = 1;
		}
	if( r[i] == '\t' ) {
		r[i] = '\0';
		state = 0;
		}
	}

/* update nrecords and nfields */
/* (PLD.nrecords[ newds ])++; */
if( prevnfields > 0 && nfields != prevnfields ) {
	PLS.skipout = 1;
	return( Eerr( 2379, "build data row: inconsistent nfields across rows", "" ));
	}
prevnfields = nfields;
/* PLD.nfields[ newds ] = nfields; */

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */


#ifdef HOLD

struct pldata {
        char **datarow;         /* array of pointers to malloc'ed data row buffers */
        int currow;             /* current number of members in datarow array */
        int maxrows;            /* total malloc'ed size of datarow array */
                                /* note: in-script data is stored in persistent proclines, not data rows */

        char **df;              /* array of field pointers */
        int curdf;              /* next available field pointer in df array */
        int maxdf;              /* total malloc'ed size of df array; */
                                /* note: field pointers point into datarows (or into proclines for in-script data) */

        /* data sets are managed as a stack of up to MAXDS elements.  proc getdata always clears the stack and fills at ds=0.  */
        int curds;              /* identifies the current dataset (or stack size).  First is 0 */
        int dsfirstdf[MAXDS];   /* where a dataset begins in the df array */
        int dsfirstrow[MAXDS];  /* where a dataset begins in the datarow array.. if data set in procline array this is -1 */
        int nrecords[ MAXDS ];  /* number of records in a dataset */
        int nfields[ MAXDS ];   /* number of fields in a dataset */
        };

/* ================================ */
/* NEWDATASET - when building a new data set directly (eg. proc processdata), to initialize.
 *
 * Note, this doesn't advance PLD.curds.  This must be done after the
 * new data set has been completely built.
 */
int
PL_newdataset( )
{
newds = PLD.curds + 1;
if( newds >= MAXDS ) { PLS.skipout = 1; return( Eerr( 2358, "max number of data sets exceeded", "" )); }
PL_checkds( newds );  
PLD.nrecords[ newds ] = 0;
PLD.dsfirstdf[ newds ] = PLD.curdf;
PLD.dsfirstrow[ newds ] = PLD.currow;
return( 0 );
}
/* ================================= */
/* CHECKDS - indicate that the ds will be used, and check if ds has been used previously.
 *	If ds has been used previously, free datarow memory and set currow and curdf back.
 *	We don't attempt to free procline memory for embedded data sets.
 */
int
PL_checkds( ds )
int ds;
{
int i;
if( ds <= highwater ) {
	if( PLS.debug ) fprintf( PLS.diagfp, "Obliterating data sets %d thru %d.\n", ds, highwater );

	if( PLD.dsfirstrow[ ds ] >= 0 ) {  /* it can be -1 when using inline data (see proc_getdata.c) */
		for( i = PLD.dsfirstrow[ ds ]; i < PLD.currow; i++ ) free( PLD.datarow[i] ); /* free rows */
		PLD.currow = PLD.dsfirstrow[ ds ];
		}

	if( ds == 0 ) {  /* added this condition.. scg 11/15/07 */
		PLD.curdf = PLD.dsfirstdf[ ds ]; 
		} 
	}
if( ds > highwater ) highwater = ds;

return( 0 );
}

#endif
