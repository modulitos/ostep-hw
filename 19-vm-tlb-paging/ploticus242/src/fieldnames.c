/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"
#define MAXNAMES 200

static char fname[MAXNAMES][NAMEMAXLEN];
static int nfname = 0;
static int errflag = 0;
static int showerr = 1;
static int encodemode = 0; /* if 1, '_' will be converted to ' ' and '|' to comma when presenting field names */



/* ============================ */
int
PL_fieldnames_initstatic()
{
nfname = 0;
errflag = 0;
showerr = 1;
encodemode = 0;
return( 0 );
}

/* ============================ */
/* DEFINEFIELDNAMES - define field names from a list 
   (space or comma-delimited).  Returns # of field names. */

int
PL_definefieldnames( list )
char *list;
{
int i, slen;

nfname = 0;

slen = strlen( list );
for( i = 0; i < slen; i++ ) if( list[i] == ',' || list[i] == '\n' || list[i] == '"' ) list[i] = ' ';

i = 0;
if( PLS.debug ) {
	if( slen < 1 ) fprintf( PLS.diagfp, "Clearing data set field names.\n" );
	else fprintf( PLS.diagfp, "Setting data set field names to: " );
	}	
while( 1 ) {
	strncpy( fname[ nfname ], GL_getok( list, &i ), NAMEMAXLEN-1 ); /* changed to strncpy() scg 8/4/04 */
	if( fname[ nfname ][0] == '\0' ) break;
	fname[ nfname ][ NAMEMAXLEN-1 ]  = '\0';			/* scg 8/4/04 */
	if( PLS.debug ) fprintf( PLS.diagfp, "%s ", fname[nfname] ); 
	nfname++;
	if( nfname >= MAXNAMES ) { fprintf( PLS.diagfp, "Too many field names.. ignoring extras\n" ); return( nfname ); }
	}
if( PLS.debug ) fprintf( PLS.diagfp, "\n" );
return( nfname );
}


/* ============================ */
/* FREF - given a field name or number, return the field number (1st = 1) */
int
PL_fref( name )
char *name;
{
int i, fld, prec, stat;

fld = 0;
errflag = 0;

/* plain number.. */
i = atoi( name );

/* stat added scg 9/26/00 */
stat = GL_goodnum( name, &prec );

if( i > 0 && stat ) fld = i; 

/* @number.. */
else if( name[0] == '@' ) {
	stat = GL_goodnum( &name[1], &prec );
	i = atoi( &name[1] );
	if( i > 0 && stat ) fld = i;
	}

/* name.. */
else if( nfname > 0 ) {
	for( i = 0; i < nfname; i++ ) {
		if( strcmp( name, fname[i] ) == 0 ) {
			fld = i + 1;
			break;
			}
		}
	}

if( fld < 1 ) {  if( showerr ) Eerr( 2479, "No such data field", name ); errflag = 1; return( 1 );  }

return( fld );
}

/* ============================ */
/* GETFNAME - given a field number, return the field name assigned to
	that field (first is 1).  Field name is copied into result var.
	Result will be "" if no field name has been assigned to field N. */

int
PL_getfname( n, result )
int n;
char *result;
{
int i;
if( n > nfname || n < 1 ) strcpy( result, "" );
else strcpy( result, fname[ n-1 ] );
if( encodemode ) {
	for( i = 0; result[i] != '\0'; i++  ) {
		if( result[i] == '_' ) result[i] = ' ';
		if( result[i] == '|' ) result[i] = ',';
		}
	}
return( 0 );
}

/* ============================= */
/* FREF_ERROR - get fref error flag */
int
PL_fref_error()
{
return( errflag );
}
/* ============================= */
/* FREF_SHOWERR - turn off/on "No such data field" message */
int
PL_fref_showerr( mode )
int mode;
{
showerr = mode;
return( 0 );
}

/* =============================== */
/* ENCODE_FNAMES - turn off/on space and comma encoding */
int
PL_encode_fnames( mode )
int mode;
{
encodemode = mode;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
