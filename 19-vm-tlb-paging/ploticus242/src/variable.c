/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* VARIABLE.C - assign/ get value of a variable */

#include "tdhkit.h"
#define NAMEMAX 30

static int Ns = 0;
static char Name[MAXVAR][NAMEMAX];
static char Value[MAXVAR][VARMAXLEN+1];
static int badvarwarn = 0;
static int Lasti = 0; /* remember which slot was set / gotten most recently .. */

/* =================================== */
int
TDH_setvar_initstatic()
{
Ns = 0;
badvarwarn = 0;
Lasti = 0;
return( 0 );
}

/* =================================== */
/* SETVAR - set the named variable to the given value.
	Allocates a space in the vars list if necessary.

	scg 11/5/07 - robusticated with respect to varname lengths and NAMEMAX
 */
int
TDH_setvar( name, value )
char *name, *value;
{
int i;
for( i = 0; i < Ns; i++ ) if( strncmp( Name[i], name, NAMEMAX-1 )==0 ) break;
if( i == Ns ) {
	/* if not found, allocate a space in vars list.. scg 2/15/01 */
	if( Ns >= MAXVAR ) {
		for( i = 0; i < Ns; i++ ) printf( "[%s] \n", Name[i] );
		printf( "(%d/%d)\n", Ns, MAXVAR );
		return( err( 1324, "Sorry, var table full", name ) );
		}
	strncpy( Name[Ns], name, NAMEMAX-1 ); Name[Ns][NAMEMAX] = '\0'; 
	Lasti = Ns;
	Ns++;
	}
if( strlen( value ) > VARMAXLEN ) return( 1321 ); /* value too long */
strcpy( Value[i], value );
return( 0 );
}
/* =================================== */
/* GETVAR - get the value of the named variable */
int
TDH_getvar( name, value )
char *name, *value;
{
int i;

/* for( i = 0; i < Ns; i++ ) if( strcmp( Name[i], name )==0 ) break; */
for( i = Ns-1; i >= 0; i-- ) if( strcmp( Name[i], name )==0 ) break;   /* search, beginning with most recently set   scg 11/5/07 */
if( i < 0 ) {
	if( badvarwarn )  err( 1322, "Undeclared variable", name ); /* we don't always want a message */
	return( 1322 ); /* variable not declared */
	}
strcpy( value, Value[i] );
Lasti = i;
return( 0 );
}
/* =================================== */
/* GETVARP - return a pointer to the value of the named variable */
char *
TDH_getvarp( name )
char *name;
{
int i;
if( name == NULL ) return( NULL );
/* for( i = 0; i < Ns; i++ ) if( strcmp( Name[i], name )==0 ) break; */
for( i = Ns-1; i >= 0; i-- ) if( strcmp( Name[i], name )==0 ) break;   /* search, beginning with most recently set   scg 11/5/07 */
if( i < 0 ) return( NULL );
return( Value[i] );
}

/* =================================== */
/* CLEARALLVAR - clear the list of variables. */
int
TDH_clearallvar( )
{
Ns = 0;
return( 0 );
}

/* ===================================== */
/* WARN_ON_BADVAR - turn on warning when undeclared vars are encountered. */
int
TDH_warn_on_badvar()
{
badvarwarn = 1;
return( 0 );
}

/* ===================================== */
/* SETVARCON - do a setvar() but perhaps do shsql field conversions first */
int
TDH_setvarcon( name, value, convert )
char *name, *value;
int convert; /* if 1 do conversion */
{
int i;
if( convert ) {
	if( strcmp( value, TDH_dbnull )==0 ) return( TDH_setvar( name, "" ) );

	for( i = 0; value[i] != '\0'; i++ ) if( value[i] == '_' ) value[i] = ' ';
	return( TDH_setvar( name, value ) );
	}
return( TDH_setvar( name, value ) );
}

/* ====================================== */
/* GET_VAR_I - get Lasti */
int
TDH_get_var_i()
{
return( Lasti );
}

/* ====================================== */
/* SHOWVARS - print a list of existing variable names to stdout */
int 
TDH_showvars( mode )
int mode;
{
int i;
if( mode == 1 ) { /* show var names and values */
	for( i = 0; i < Ns; i++ ) printf( "[%s:%s] \n", Name[i], Value[i] );
	}
else for( i = 0; i < Ns; i++ ) printf( "[%s] \n", Name[i] );
printf( "... (%d/%d)\n", Ns, MAXVAR );
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2007 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
