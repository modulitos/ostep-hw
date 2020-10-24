/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* TDH abstract database interface.  
   Interfaces to other databases should be implemented here.

   If compile flag TDH_DB is undefined or set to 0, no sql connectivity is available.

   If TDH_DB is set to 2, shsql calls will be made.  SHSQL has no concept of "connect".
*/

#include "tdhkit.h"

extern int TDH_dbnewquery(), TDH_dberrorcode();


#define SHSQL  2
#define MYSQL  10
#define ORACLE  20
#define SYBASE  30
#ifndef TDH_DB
#define TDH_DB 0
#endif

#if TDH_DB == SHSQL
  extern int SHSQL_sql(), SHSQL_getrow(), SHSQL_pushrow(), SHSQL_getnames(), SHSQL_tabdef(), SHSQL_getnrows(), SHSQL_writable();
#endif



/* =============================================================================
   SQLCOMMAND - submit an sql command and return its execution status (0 = normal). 
 */

int 
TDH_sqlcommand( dbc, sql )
int dbc; 	/* connection identifier (0 - 3) */
char *sql; 	/* sql command */
{
int stat;

#if TDH_DB == 0
  return( err( 7949, "sql support not included in this build", "" ) );
#else
  TDH_dbnewquery( dbc ); /* notify $sqlrow() function of new query */
#endif

#if TDH_DB == SHSQL
  stat = SHSQL_sql( dbc, sql );
#endif

#if TDH_DB != 0
  TDH_dberrorcode( dbc, stat ); /* save return code so $sqlerror() can provide it later.. */
#endif
return( stat );
}

/* ========================================================================== 
   SQLROW - get one row of results from most recent SQL SELECT.
   Return 0 if row fetched, 1 if not (no more rows), or an error code. 
   All result fields should be character strings, including "null".
   FIELDS is an array of char pointers; each will be made to point to a result field in shsql space.
	(app should immediately copy into app space or risk obliteration on subsequent shsql retrieval)
   N will be set to the number of fields.
*/

int
TDH_sqlrow( dbc, fields, n )
int dbc;
char *fields[];
int *n;
{
int stat;

stat = 0;

#if TDH_DB == SHSQL 
  stat = SHSQL_getrow( dbc, fields, n );
#endif

#if TDH_DB > 1
  /* this must be done here to report on locked records when a SELECT .. FOR UPDATE is done.  Added scg 3/8/06  */
  TDH_dberrorcode( dbc, stat ); /* save return code so $sqlerror() can provide it later.. */
#endif

return( stat );
}

/* ========================================================================== 
   SQLPUSHROW - allow next call to sqlrow() to get same row again.
*/

int
TDH_sqlpushrow( dbc )
int dbc;
{

#if TDH_DB == SHSQL 
  return( SHSQL_pushrow( dbc ) );
#endif

return( 0 );
}


/* ========================================================================= 
   SQLNAMES - fetch names of result fields from most recent SQL SELECT.
   Return 0 or an error code.  
   FIELDS is an array of char pointers; each will point to a name.
   N will be set to the number of names (same as number of fields).
 */

int
TDH_sqlnames( dbc, fields, n )
int dbc;
char *fields[]; 
int *n;
{

#if TDH_DB == SHSQL
  return( SHSQL_getnames( dbc, fields, n ) );
#endif

return( 0 );
}

/* ===========================================================================
   SQLTABDEF - fetch the names of a table's fields 
 */

int
TDH_sqltabdef( table, fields, n )
char *table;
char *fields[];
int *n;
{

#if TDH_DB == SHSQL
  return( SHSQL_tabdef( table, fields, n ) );
#endif

return( 0 );
}

/* ==========================================================================
   SQLROWCOUNT - return # of rows presented or affected by last sql command 
 */

int
TDH_sqlrowcount( dbc )
int dbc;
{

#if TDH_DB == SHSQL
  return( SHSQL_getnrows( dbc ) );
#endif

return( 0 );
}

/* ==========================================================================
   SQLWRITABLE - return 0 if current process is allowed to write to the database, non-zero otherwise.
 */
int
TDH_sqlwritable()
{

#if TDH_DB == SHSQL
  return( SHSQL_writable() );
#endif

return( 0 );
}



/* ======== The following are convenience routines that call the above TDH routines.  ====== */

/* ============================= */
/* SQLGET - convenience routine to retrieve one field using default db connection.  
   Return 0 or an error code. */

int
TDH_sqlget( sql, result )
char *sql;
char *result;
{
int stat, n;
char *f[10];

strcpy( result, "" );

stat = TDH_sqlcommand( 0, sql );
if( stat != 0 ) return( stat );

stat = TDH_sqlrow( 0, f, &n );
if( stat != 0 ) return( stat );
if( n != 1 ) return( 5 );

strcpy( result, f[0] );
return( 0 );
}

/* =============================== */
/* SQLGETS - convenience routine to retrieve multiple fields.  Return 0 or an error code. */

int
TDH_sqlgets( sql, fields )
char *sql;
char *fields[];
{
int stat, n;

stat = TDH_sqlcommand( 0, sql );
if( stat != 0 ) return( stat );

stat = TDH_sqlrow( 0, fields, &n );
if( stat != 0 ) return( stat );

return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
