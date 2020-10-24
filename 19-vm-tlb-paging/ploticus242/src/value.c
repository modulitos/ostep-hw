/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* VALUE.C - assign / get value of data array field or variable */

#include "tdhkit.h"
extern int atoi();
#ifdef PLOTICUS
  extern int PL_fref_showerr(), PL_fref(), PL_fref_error();
#endif

/* ======================== */
/* SETVALUE - set the named variable to the given value.
		Returns 0 if ok, 1 if itemname not recognized.  */

int
TDH_setvalue( itemname, value, data, recordid )
char *itemname, *value;
char data[ MAXITEMS ][ DATAMAXLEN+1 ];	/* data array */
char *recordid;  /* see top of this file */
{
int j;
int stat;
int p;


/* see if itemname is an integer field number; 
   if so set it and return.. scg 2-18-98 */
stat = GL_goodnum( itemname, &p );
if( stat && p <= 0 ) {
	if( data == NULL ) return( 1301 );
	strcpy( data[ atoi( itemname ) -1 ], value );
	return( 0 );
	}

if( strcmp( recordid, "" )==0 ) j = -1;
#ifndef TDH_NOREC
else if( recordid[0] == '@' ) {
	int ival;
	ival = atoi( &recordid[1] );
	if( ival > 0 && ival < MAXITEMS ) {
		if( data == NULL ) return( 1302 );
		j = TDH_fieldmap( data[ival-1], itemname );
		}
	else return( 1303 ); /* invalid @N */
	}
else j = TDH_fieldmap( recordid, itemname );
#endif


if( j < 0 ) {
	stat = TDH_setvar( itemname, value );
	return( stat );
	}
else 	{
	if( strlen( value ) > DATAMAXLEN ) return( 1304 ); /* value too long */
	if( data == NULL ) return( 1305 );
	strcpy( data[ j ] , value );
	}
return( 0 );
}

/* ========================= */
/* GETVALUE - get the value of the named variable.
 	Returns 0 if ok, 1 if itemname not recognized. 

	4/30/01 - now handles field#s, e.g. itemname = 2
 */

int
TDH_getvalue( value, itemname, data, recordid )
char *value;
char *itemname;
char data[ MAXITEMS ][ DATAMAXLEN+1 ];	/* data array */
char *recordid;  /* see top of this file */
{
int j;
int stat;
int ival;


if( GL_goodnum( itemname, &stat ) ) {  /* @3, etc. */
	ival = atoi( itemname );
	if( ival < 1 || ival > MAXITEMS ) return( 1309 );  /* field# out of range */
	if( data == NULL ) return( 1308 ); /* attempt to access data item but data array not supplied */
	strcpy( value, data[ ival-1 ] );
	return( 0 );
	}

if( strcmp( recordid, "" )==0 ) j = -1;
#ifndef TDH_NOREC
else j = TDH_fieldmap( recordid, itemname );
#endif

if( j < 0 ) {
	stat = TDH_getvar( itemname, value );
#ifdef PLOTICUS 
	if( stat != 0 && data != NULL ) {  /* try data field name.. */
		PL_fref_showerr( 0 );
		j = PL_fref( itemname ) - 1; 
		PL_fref_showerr( 1 );
		if( PL_fref_error() ) return( 1308 );
			/* return( err( 1308, "unrecognized variable or data field name", itemname )); */
		/* if( data == NULL ) return( 1308 ); */ /* moved above.. scg 11/5/07 */
		strcpy( value, data[ j ] );
		return( 0 );
		}
#endif
	return( stat );
	}
else 	{
	if( data == NULL ) return( 1308 ); /* attempt to access data item but data array not supplied */
	strcpy( value, data[ j ] );
	}
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
