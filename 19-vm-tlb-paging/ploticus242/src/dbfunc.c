/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* TDH script access (via functions) to retrieve rows from abstract SQL database */

#include "tdhkit.h"
#define MAXCONNECTS 4	/* if this is changed, see MAXCONNECTS elsewhere herein.. */

extern int TDH_sqlnames(), TDH_sqlrow(), TDH_sqlpushrow(), TDH_sqlrowcount(), TDH_sqltabdef(), TDH_sqlwritable();


static char *sqlnames[ MAXCONNECTS ][MAXITEMS];
static int nsqlnames[ MAXCONNECTS ] = { 0, 0, 0, 0 };
static char varprefix[ MAXCONNECTS ][30] = { "", "", "", "" };
static char stripprefix[ MAXCONNECTS ][30] = { "", "", "", "" };
static int spflen[ MAXCONNECTS ] = { 0, 0, 0, 0 };
static int errorcode[ MAXCONNECTS ] = { 0, 0, 0, 0 };
static int nullrep = 1;	/* 0 = noconvert, 1 = blank, 2 = null, 3 = nbsp */

int
TDH_dbfunctions( hash, name, arg, nargs, result, typ )
int hash;
char *name;
char *arg[];
int nargs;
char *result;
int *typ;
{
char *f[MAXITEMS];
char varname[NAMEMAXLEN];

int i, j, n, len, stat;
int dbc;

*typ = 0; /* numeric */
if( strcmp( arg[0], "2" )==0 ) dbc = 1;		
else if( strcmp( arg[0], "3" )==0 ) dbc = 2;		
else if( strcmp( arg[0], "4" )==0 ) dbc = 3;		/* MAXCONNECTS */
else dbc = 0;


if( hash == 625 ) { /* $sqlrow() - fetch a row of results.  Fields will be accessible by @name.
 			* Return 0 = normal, 1 = no row fetched and no more results, >1 = error */

	if( nsqlnames[dbc] <= 0 ) {
		/* get result field (column) names */
		stat = TDH_sqlnames( dbc, sqlnames[ dbc ], &nsqlnames[ dbc ] );

		/* check return status.. non-zero indicates error */
		if( stat != 0 || nsqlnames[dbc] <= 0 ) 
			err( 4720, "cannot retrieve result field names", "" );

		}

	/* get next result row.. */
	stat = TDH_sqlrow( dbc, f, &n );
        for( i = 0; i < nsqlnames[ dbc ]; i++ ) {
		if( strncmp( stripprefix[dbc], sqlnames[dbc][i], spflen[dbc] )==0 ) 
			sprintf( varname, "%s%s", varprefix[dbc], &sqlnames[dbc][i][spflen[dbc]] );
		else sprintf( varname, "%s%s", varprefix[dbc], sqlnames[dbc][i] );
		if( stat == 0 ) {
        	        if( stricmp( f[i], TDH_dbnull )==0 && nullrep ) {
				if( nullrep == 1 ) TDH_setvar( varname, "" );
				else if( nullrep == 2 ) TDH_setvar( varname, DBNULL );
				else if( nullrep == 3 ) TDH_setvar( varname, "&nbsp;" );
				}
               		else TDH_setvar( varname, f[i] );
			}
		else TDH_setvar( varname, "" );
                }

	/* check return status.. non-zero indicates no more rows.. */
	sprintf( result, "%d", stat );
	return( 0 );
	}

if( hash == 1604 ) { /* $sqlpushrow() - added 2/24/04 scg */
	TDH_sqlpushrow( dbc );
	strcpy( result, "0" );
	return( 0 );
	}

if( hash == 1882 ) { /* $sqlrowcount() - return number of rows presented or affected by last sql command */
	sprintf( result, "%d", TDH_sqlrowcount( dbc ) );
	return( 0 );
	}

if( hash == 997 ) { /* $sqlerror() - return error code of most recent sql command */
	sprintf( result, "%d", errorcode[ dbc ] );
	return( 0 );
	}


#ifdef DROP
if( hash == 1607 ) { /* $sqlgetnames( dumpmode ) - load sql result field names so fields can be accessed as variables
 			* by name later.  Dumpmode is an optional argument;
 			* if specified, field names are also written to standard output.
 			* if dumpmode is given as "dumptab", the field names are written as tab-delimited; 
 			* if given as "dumphtml", the field names are written as an html table row.  */
   
	/* get result field (column) names */
	stat = TDH_sqlnames( dbc, sqlnames[dbc], &nsqlnames[dbc] );

	/* check return status.. non-zero indicates error */
	if( stat != 0 ) { sprintf( result, "%d", stat ); return( 0 ); }

	/* optional output of field names */
	else if( stricmp( arg[0], "dumptab" )==0 || stricmp( arg[1], "dumptab" )==0 ) {
		for( i = 0; i < nsqlnames[dbc]; i++ ) printf( "%s	", sqlnames[dbc][i] );
		printf( "\n" );
		}
	else if( stricmp( arg[0], "dumphtml" )==0 || stricmp( arg[1], "dumphtml" )==0 ) {
		for( i = 0; i < nsqlnames[dbc]; i++ ) printf( "<td>%s</td>", sqlnames[dbc][i] );
		printf( "</tr>\n" );
		}

	/* return */
	strcpy( result, "0" );
	return( 0 );
	}
#endif

if( hash == 913 ) { 	/* $sqltabdef( table ) - return a commalist of all field names in table */
	TDH_altfmap( 1 );
	if( dbc ) j = 1;
	else j = 0;
	stat = TDH_sqltabdef( arg[j], sqlnames[dbc], &nsqlnames[dbc] );
	TDH_altfmap( 0 );

	/* check return status.. non-zero indicates error */
	if( stat != 0 ) { sprintf( result, "%d", stat ); return( 0 ); }

	/* return */
	strcpy( result, "" );
	for( len = 0, j = 0; j < nsqlnames[dbc]; j++ ) {
		sprintf( &result[len], "%s,", sqlnames[dbc][j] );
		len += strlen( sqlnames[dbc][j] ) + 1;
		}
	return( 0 );
	}
	

if( hash == 1168 ) { /* $sqlprefix() - set result field name prefix */
	if( GL_smember( arg[0], "1 2 3 4" ) ) strcpy( varprefix[dbc], arg[1] );		/* MAXCONNECTS */
	else strcpy( varprefix[dbc], arg[0] );
	strcpy( result, "0" );
	return( 0 );
	}

if( hash == 1523 ) { /* $sqlwritable() - return 0 if current process has write permission on database, nonzero otherwise */
	sprintf( result, "%d", TDH_sqlwritable() );
	return( 0 );
	}

if( hash == 2831 ) { /* $sqlstripprefix() - remove beginning of result field name */
	if( GL_smember( arg[0], "1 2 3 4" ) ) strcpy( stripprefix[dbc], arg[1] );	/* MAXCONNECTS */
	else strcpy( stripprefix[dbc], arg[0] );
	spflen[dbc] = strlen( stripprefix[dbc] );
	strcpy( result, "0" );
	return( 0 );
	}


return( err( 197, "unrecognized function", name ) ); /* not found */
}

/* ======================== */
/* NEWQUERY -   */
int
TDH_dbnewquery( dbc )
int dbc;
{
nsqlnames[dbc] = 0;
strcpy( varprefix[dbc], "" );
strcpy( stripprefix[dbc], "" );
spflen[dbc] = 1;
return( 0 );
}

/* ========================= */
/* SQLROW_NULLREP - set the null representation for $sqlrow() */
int
TDH_sqlrow_nullrep( rep )
int rep;
{
nullrep = rep;
return( 0 );
}

/* ========================== */
/* ERRORCODE - set error code */
int
TDH_dberrorcode( dbc, code, mode )
int dbc, code;
int mode; /* 0 = set unconditionally;  1 = set only when errorcode[dbc] is zero */
{
if( mode == 1 && errorcode[ dbc ] != 0 ) return( 0 ); 
errorcode[ dbc ] = code;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
