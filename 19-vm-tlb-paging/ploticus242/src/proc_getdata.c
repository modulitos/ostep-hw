/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC GETDATA - get some data.  Data may be specified literally, or gotten from a file or a command.  */

/* scg 11/6/07 - removed the specialmode related to certain uses of proc processdata */

#include "pl.h"
#include "tdhkit.h"
#include <string.h>

#define COMMANDMAX 1024

/* field delimitation methods */
#define WHITESPACE 0
#define SPACEQUOTE 1
#define TAB 2
#define CSV 3
#define BAR 4
#define AUTODETERMINE 5

static int do_filter();


/* =============================== */
int
PLP_getdata( )
{
int i, j, lvp, stat, first;
char attr[NAMEMAXLEN], *line, *lineval;

char buf[ MAXRECORDLEN ];   /* holds input data rows */
char tok[256];
char *datafile, *pathname, *command, *selectex, *fieldnamerows;
char *pfnames, *row;
char commentchar[12];
char datasource;
FILE *dfp, *popen();
int delim, standardinput, fieldnameheader, buflen, literaldata, reqnfields;
int datastart, ndatarows, irow, nrecords, nfields, totalitems, foo, sqlflag, doing_set, nfldnames;
int blankrow, hold_delim, nscriptrows, scriptstart, showdata;
int samplerate, readcount;

TDH_errprog( "pl proc getdata" );


/* initialize */
datafile = "";
pathname = "";
selectex = "";
command = "";
pfnames = "";
strcpy( commentchar, "//" );
delim = SPACEQUOTE;
showdata = standardinput = 0;
sqlflag = nfldnames = hold_delim = 0;
fieldnameheader = literaldata = nscriptrows = reqnfields = 0;
samplerate = 1;

definefieldnames( "" ); /* any existing field names will be wrong */

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "file" )==0 ) {
		if( PLS.cgiargs != NULL ) pathname = lineval;
		else	{
#ifdef WIN32
			pathname = lineval;    /* to avoid invoking 'cat' .. */ 
#else
			datafile = lineval;
#endif
			}
		}

	else if( strcmp( attr, "pathname" )==0 ) pathname = lineval;
	else if( strcmp( attr, "command" )==0 ) command = lineval;
	else if( strcmp( attr, "commandmr" )==0 ) command = getmultiline( lineval, "get" );
	else if( strcmp( attr, "sql" )==0 ) { command = lineval; sqlflag = 1; }
	else if( strcmp( attr, "sqlmr" )==0 ) { command = getmultiline( lineval, "get" ); sqlflag = 1; }
	else if( strncmp( attr, "delim", 5 )==0 ) {
		if( lineval[0] == 'w' ) delim = WHITESPACE;
		else if( lineval[0] == 't' ) delim = TAB;
		else if( lineval[0] == 'c' ) delim = CSV;
		else if( lineval[0] == 'b' ) delim = BAR;
		else if( lineval[0] == 'a' ) delim = AUTODETERMINE;
		}
	else if( strcmp( attr, "commentchar" )==0 ) tokncpy( commentchar, lineval, 12 ); 
	else if( strcmp( attr, "data" )==0 ) {
		if( strcmp( lineval, "" )!=0 ) {
			datastart = PLL.curline-1;
			strcpy( PLL.procline[ PLL.curline-1 ], lineval ); /* remove 'data:' */
			}
		else datastart = PLL.curline;
		getmultiline( lineval, "skip" );
		ndatarows = (PLL.curline - datastart);  /* was -1 */
		literaldata = 1;
		PL_holdmem( 1 ); /* tell execline() not to free this storage when the proc is done executing.. */
		}
	else if( strcmp( attr, "filter" )==0 ) {
		if( strcmp( lineval, "" )!= 0 ) {
			scriptstart = PLL.curline-1;
			strcpy( PLL.procline[ PLL.curline-1 ], lineval ); /* remove 'filter:' */
			}
		else scriptstart = PLL.curline;
		getmultiline( lineval, "skip" ); /* use this to skip over filter script */
		nscriptrows = (PLL.curline - scriptstart); /* was -1 */

		/* in all script lines, convert double pound signs (##) to single poundsign.. */
		for( i = scriptstart; i < scriptstart+nscriptrows; i++ ) {
			for( j = 0; PLL.procline[i][j] != '\0'; j++ ) {
				if( PLL.procline[i][j] == '#' && PLL.procline[i][j+1] == '#' ) {
					PLL.procline[i][j] = ' ';
					break;
					}
				else if( !isspace( (int) PLL.procline[i][j] )) break;  /* no need to search entire line.. scg 11/25/02 */
				}
			}
		}

	else if( strcmp( attr, "showdata" )==0 || strcmp( attr, "showresults" )==0 ) showdata = getyn( lineval );
	else if( strcmp( attr, "standardinput" )==0 ) standardinput = getyn( lineval ); 
	else if( strcmp( attr, "fieldnameheader" )==0 ) fieldnameheader = getyn( lineval ); 
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "nfields" )==0 ) reqnfields = itokncpy( lineval );
	else if( strcmp( attr, "samplerate" )==0 ) samplerate = itokncpy( lineval );
	else if( strcmp( attr, "fieldnames" )==0 ) nfldnames = definefieldnames( lineval ); 
	else if( strcmp( attr, "fieldnamerows" )==0 ) {
		fieldnamerows = getmultiline( lineval, "get" );
		nfldnames = definefieldnames( fieldnamerows );
		}
	else if( strcmp( attr, "pf_fieldnames" )==0 ) pfnames = lineval;
	else Eerr( 1, "attribute not recognized", attr );
	}


/* now do the work.. */

PLP_processdata_initstatic();   /* reset proc_processdata "break" pointer.. scg 8/4/04 */


/* determine source of data.. */

if( sqlflag ) datasource = 'q';

else if( command[0] != '\0' ) datasource = 'c';					/* shell command */

else if( literaldata ) {
	if( nscriptrows > 0 ) Eerr( 5792, "Warning, filter ignored (it cannot be used with in-script data)", "" );
	datasource = 'd';    							/* in-script data statement */
	}

else if( standardinput || strcmp( datafile, "-" ) ==0 || strcmp( pathname, "-" )==0 ) datasource = 's'; /* stdin */

else if( datafile[0] != '\0' ) { 						/* shell-expandable file name */
	sprintf( buf, "cat \"%s\"", datafile ); 
	command = buf;
	datasource = 'c'; 
	} 

else if( pathname[0] != '\0' ) datasource = 'p'; 				/* explicit full pathname */

else { PLS.skipout = 1; return( Eerr( 407, "No data, file, or command was specified", "" ) ); }


/* get ready to loop thru data.. */
if( datasource != 'd' ) { 
	datastart = 0; 
	ndatarows = 99999999; 
	}
if( datasource == 'p' ) {  /* 'pathname' given.. */
	dfp = fopen( pathname, "r" );
	if( dfp == NULL ) { PLS.skipout = 1; return( Eerr( 401, "Cannot open data file", pathname ) ); }
	}
else if( datasource == 's' ) dfp = stdin;
else if( datasource == 'c' ) {
	if( PLS.noshell ) return( Eerr( 401, "-noshell prohibits #proc getdata command", "" ) );
	dfp = popen( command, "r" );
	if( dfp == NULL ) { PLS.skipout = 1; return( Eerr( 401, "Cannot execute command", command ) ); }
	}



/* get the PLD data structure ready.. */

/* when proc getdata gets a data set, it always becomes ds 0 */
PL_cleardatasets();
PL_begindataset();


/* handle embedded sql using db abstraction interface - see dbinterface.c */
if( sqlflag ) {
	char *fields[128];
	int blen, nf2;
	/* config file must have already been read.. */
	stat = TDH_sqlcommand( 0, command );
	if( stat != 0 ) return( Eerr( stat, "error on sql command", command ));
	/* get result field names and use these to set ploticus field names (what about joins?) */
	stat = TDH_sqlnames( 0, fields, &nfields );
	if( stat != 0 ) return( Eerr( stat, "error on sql result fieldnames", command ));
	for( i = 0, blen = 0; i < nfields; i++ ) {
		strcpy( &PL_bigbuf[blen], fields[i] ); blen += strlen( fields[i] );
		strcpy( &PL_bigbuf[blen], " " ); blen++;
		}
	PL_bigbuf[blen] = '\0';
	nfldnames = definefieldnames( PL_bigbuf );
	for( nrecords = 0; ; nrecords++ ) {
		if( PLD.curdf + nfields >= PLD.maxdf ) {
			Eerr( 406, "Data capture truncated...too many data fields (try raising -maxfields)", "" );
			break;
			}

		stat = TDH_sqlrow( 0, fields, &nf2 );  /* changed scg 6/30/06 */
		if( stat > 1 ) return( Eerr( stat, "error on sql row retrieval", command ));
		if( stat != 0 ) break;
		if( nfields != nf2 ) return( Eerr( 461, "sql retrieval inconsistency", "" ));

		/* copy fields from shsql space into pl data space .. added scg 6/30/06 */
		PL_startdatarow();
		for( j = 0; j < nf2; j++ ) PL_catitem( fields[j] );
		PL_enddatarow();
		}
	goto READ_DONE; /* skip down to do the finish-up stuff.. */
	}


/* all other means of getting data use the following.. */

/* loop thru lines of data.. */
first = 1;
nrecords = 0;
readcount = 0;
for( irow = datastart; irow < datastart+ndatarows; irow++ ) {

	if( datasource != 'd' ) {
		if( fgets( buf, MAXRECORDLEN-1, dfp ) == NULL ) break;
		readcount++;
		if( samplerate != 1 ) {
			/* keep every Nth input row... reject the rest.  scg 12/7/09 */
			if( readcount % samplerate != 0 ) continue;
			}
		row = buf;
		}
	else row = PLL.procline[ irow ];


	/* skip empty lines */
	if( row[0] == '\n' || row[0] == '\0' ) continue;


	/* note.. with comma-delimited data there can be very long lines with no whitespace */

	/* skip lines containing nothing but whitespace chars.. and also skip comment lines */
	for( j = 0, blankrow = 1; row[j] != '\0'; j++ ) if( !isspace( (int)row[j] )) { blankrow = 0; break; }
	if( blankrow ) continue;	
	if( strncmp( &row[j], commentchar, strlen( commentchar ))==0 ) continue;

	buflen = strlen( row );

	if( datasource != 'd' && row[ buflen-2 ] == 13 ) strcpy( &buf[ buflen-2 ], "\n" ); /* DOS LF */

	/* look for #set.. */
	doing_set = 0;
	for( j = 0; row[j] != '\0'; j++ ) if( strncmp( &row[j], "#set ", 5 )== 0 ) { doing_set = 1; break; }

	/* #set var = value .. this can be used in data files.. - added scg 11/13/00 */
	if( doing_set ) {
		int ix, oldix;
		char varname[40];
		ix = 0;
		GL_getchunk( tok, row, &ix, " 	" ); /* #set */
		GL_getchunk( varname, row, &ix, " 	=" ); /* varname */
		oldix = ix;
		GL_getchunk( tok, row, &ix, " 	" ); /* optional = */
		if( strcmp( tok, "=" ) != 0 ) ix = oldix;
		row[ buflen - 1 ] = '\0'; /* strip off trailing newline - scg 7/27/01 */
		buflen--;

		if( row[ix+1] == '"' ) stat = TDH_setvar( varname, &row[ix+2] );
		else stat = TDH_setvar( varname, &row[ix+1] );
		if( stat ) { PLS.skipout = 1; return( Eerr( stat, "Fatal error, embedded #set statement, value is too long (max=250 chars)", "" )); }
		continue;
		}

	if( delim == AUTODETERMINE ) { /* attempt to auto-detect the delim character..  scg 10/29/07 */
		int ntabs, nbars, ncommas;
		char *dword;
		for( j = 0, ntabs = 0; row[j] != '\0'; j++ ) if( row[j] == '\t' ) ntabs++;
		for( j = 0, nbars = 0; row[j] != '\0'; j++ ) if( row[j] == '|' ) nbars++;
		for( j = 0, ncommas = 0; row[j] != '\0'; j++ ) if( row[j] == ',' ) ncommas++;
		delim = WHITESPACE; dword = "whitespace";
		if( ntabs > 2 && ntabs > nbars && ntabs > ncommas ) { delim = TAB; dword = "tab"; }
		else if( nbars > 2 && nbars > ntabs && nbars > ncommas ) { delim = BAR; dword = "bar"; }
		else if( ncommas > 2 && ncommas > ntabs && ncommas > nbars ) { delim = CSV; dword = "csv"; }
		if( PLS.debug ) fprintf( PLS.diagfp, "automatic determination delim: looks like  %s delimited\n", dword );
		}

	if( delim == BAR || hold_delim == BAR ) {   /* added scg 10/25/07 */
		for( j = 0; row[j] != '\0'; j++ ) if( row[j] == '|' ) row[j] = '\t';
		hold_delim = BAR;  
		delim = TAB;
		}


	/* field name header.. */
	if( first && fieldnameheader ) {
		nfldnames = definefieldnames( row );  /* takes whitespace or comma delimited.. */
		first = 0;
		continue;
		}
	first = 0;

	
	/* if field names given and nfields not given, set expected # fields using # field names.. scg 3/15/06 */
	/* but not if we're doing a filter.. */
	if( reqnfields <= 0 && nfldnames > 0 && nscriptrows == 0 ) reqnfields = nfldnames; 


	/* optional select */
	if( selectex[0] != '\0' ) { 
		stat = do_filter( row, selectex, delim, 1, scriptstart, nscriptrows ); /*doesn't modify row*/
		if( ! stat ) continue;
		}

	/* optional filter data processing.. */
	if( datasource != 'd' && nscriptrows > 0  ) {
		do_filter( buf, "", delim, 0, scriptstart, nscriptrows ); /* modifies row */
		if( buf[0] == '\0' ) continue;  	/* nothing printed, skip row.. added scg 3/19/03 */
		buflen = strlen( buf ); 		/* because row has been modified above */
		if( buf[ buflen -1 ] != '\n' ) strcpy( &buf[ buflen-1 ], "\n" );
		}

	/* if we reach here, keep the line.. */

	if( datasource != 'd' ) {
		/* copy the row into malloced storage.. */
		if( PLD.currow >= PLD.maxrows ) { 
			Eerr( 429, "Data input truncated... too many data rows (try raising -maxrows)", "" );
			break;
			}
		row = (char *) malloc( buflen+1 );
		if( row == NULL ) return( err( 2480, "malloc error", "" ) );
		strcpy( row, buf );
		PLD.datarow[ PLD.currow++ ] = row;
		}


	/* parse the row into fields.. */
	if( reqnfields > 0 ) nfields = reqnfields;
	else if( nrecords == 0 ) nfields = 0;
	stat = PL_parsedata( row, delim, commentchar, &(PLD.df[ PLD.curdf ]), MAXITEMS, &foo, &nfields, &totalitems );
	if( stat != 0 ) { PLS.skipout = 1; return( Eerr( stat, "Parse error on input data.", "" )); }

	PLD.curdf += nfields;

	if( PLD.curdf + nfields >= PLD.maxdf ) {
		Eerr( 406, "Data input truncated... too many data fields (try raising -maxfields)", "" );
		break;
		}

	nrecords++;
	}
	

if( datasource == 's' ) ;
else if( datasource == 'p' ) fclose( dfp );
else if( datasource == 'c' ) pclose( dfp );

READ_DONE:

PL_finishdataset( nrecords, nfields );
if( nscriptrows > 0 && pfnames[0] != '\0' ) definefieldnames( pfnames ); /* assign any post-filter field names.. */

if( showdata ) {
        getfname( 1, buf ); /* buf[256] */
	fprintf( PLS.diagfp, "// proc getdata has read & parsed these data:\n" );
        if( buf[0] != '\0' ) { 
                fprintf( PLS.diagfp, "// field names are: " );
                for( j = 0; j < Nfields; j++ ) { getfname( j+1, buf ); fprintf( PLS.diagfp, "%s|", buf ); }  /* buf[256] */
                fprintf( PLS.diagfp, "\n" );
                }
        else fprintf( PLS.diagfp, "// (no field names defined)\n" );

 	for( i = 0; i < Nrecords; i++ ) {
 		for( j = 0; j < Nfields; j++ ) fprintf( PLS.diagfp, "%s|", da(i,j) );
		fprintf( PLS.diagfp, "\n" );
 		}
	fprintf( PLS.diagfp, "// end of data set\n" );
	}


return( 0 );
}


/* ======================================== */
/* DO_FILTER - implement 'select' and 'filter'.
   This needs to be reworked again sometime for efficiency.. 
 */

static int
do_filter( buf, scriptname, delim, mode, scriptstart, nscriptrows )
char *buf;
char *scriptname;
int delim;
int mode; /* 0 = filter    1 = select */
int scriptstart, nscriptrows;
{
int stat;
char recordid[80]; 
char data[MAXITEMS][DATAMAXLEN+1];
char *df[MAXITEMS];
char str[MAXRECORDLEN], str2[MAXRECORDLEN]; /* size increased from 255  scg 6/27/01 */
int nfields, nrecords, nd;
int i;
char commentchar[12];
struct sinterpstate ss;

strcpy( recordid, "" ); /* not used */
strcpy( commentchar, "//" ); /* not used? */

/* split up buf into fields.. */
strcpy( str, buf );
nfields = 0;
PL_parsedata( str, delim, commentchar, df, MAXITEMS, &nrecords, &nfields, &nd ); 

if( mode == 1 ) { /* condex processing.. */
	strcpy( str2, str );
	stat = PL_value_subst( str2, scriptname, df, FOR_CONDEX );
	if( stat > 1 ) Eerr( 2208, "value_subst error", scriptname );
	stat = TDH_condex( str2, 0 );
	return( stat );
	}

/* for sinterp we need to copy data into array.. */
for( i = 0; i < MAXITEMS; i++ ) strcpy( data[i], "" ); /* null out data array.. added scg 11/15/00 */
for( i = 0; i < nfields; i++ ) strcpy( data[i], df[i] );



stat = TDH_sinterp_openmem( &(PLL.procline[ scriptstart ]), nscriptrows, &ss ); /* scriptstart & nscriptrows set above.. */
if( stat != 0 ) return( Eerr( stat, "filter script error", "" ) );

/* do filter processing.. */
strcpy( buf, "" );
while( 1 ) {
	stat = TDH_sinterp( str, &ss, recordid, data );
        if( stat > 255 ) return( Eerr( 169, "filter script error.. quitting..", "" ) );
        else if( stat != SINTERP_MORE ) break;
	strcat( buf, str ); /* strcat ok */
	}
return( 0 ); /* return results in buf */
}


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
