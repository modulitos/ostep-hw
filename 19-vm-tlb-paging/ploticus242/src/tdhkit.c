/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* TDHKIT.C - read config file; global TDH vars  */

/* compile defines:  
	TDH_NOREC = omit FDF code   
	TDH_DB = select sql database (see dbinterface.c) (undefined if no sql database)
*/

   
#include "tdhkit.h"
#include <ctype.h>

extern int DT_setdateparms();
extern int putenv();

char TDH_scriptdir[ MAXPATH] = "./";	/* root directory for scripts */
char TDH_configfile[ MAXPATH ] = "";	/* path name of config file */
char TDH_tmpdir[ MAXPATH ] = "/tmp";	/* directory for tmp files */
char TDH_dbnull[ 10 ] = "=";		/* null representation in data files */
int TDH_debugflag = 0;			/* may be set to 1 for extra debugging output */
char TDH_decpt = '.';			/* decimal point char */

char *TDH_dat = NULL;			/* points to data array for condex */
char *TDH_recid = NULL;			/* points to recordid for condex */
char TDH_progname[20] = "";
int TDH_initialized = 0;
char TDH_shellmetachars[30] = "\"'`$\\;|"; /* shell meta characters to strip out of variables when building
					    shell commands for security purposes */ 

#ifndef TDH_NOREC
char TDH_fdfpath[ MAXPATH ] =      "./";	/* directory where FDF files are kept */
#endif

int TDH_inquisp = 0;			/* this is needed with the new ploticus api PL_initstatics() 
					   so that we don't wipe out variables, and other script
					   settings already in action.. quisp sets this to 1 */

#define MAXPE 512
static char putenvstring[MAXPE+2];
static int pelen = 0;

/* ================================ */
int
TDH_readconfig_initstatic()
{
TDH_initialized = 0;
/* everything else will be set just before the read is done, below.. */
return( 0 );
}


/* ================================ */
int
TDH_readconfig( loc )
char *loc; /* environment var or file=filename */
{
FILE *fp;
char buf[512];
char tag[80];
char value[512];
int nt;
int stat;
char *getenv();
int slen;


if( TDH_initialized ) return( 0 );

/* set tdh defaults now.. */
strcpy( TDH_scriptdir, "./" );
strcpy( TDH_configfile, "" );
strcpy( TDH_tmpdir, "/tmp" );
strcpy( TDH_dbnull, "=" );
TDH_debugflag = 0;
TDH_decpt = '.';
TDH_dat = NULL;
TDH_recid = NULL;
strcpy( TDH_progname, "" );
strcpy( TDH_shellmetachars, "\"'`$\\;|" );
#ifndef TDH_NOREC
  strcpy( TDH_fdfpath, "./" );
#endif
pelen = 0;


if( strncmp( loc, "file=", 5 )==0 ) strcpy( TDH_configfile, &loc[5] );
else	{
	if( getenv( loc ) == NULL ) return( 1 ); /* no config environment var exists .. ok */
	else strcpy( TDH_configfile, getenv( loc ) );
	if( TDH_configfile[0] == '\0' ) return( 1 ); /* env var empty - ok */
	}
fp = fopen( TDH_configfile, "r" );
if( fp == NULL ) {
	return( err( 1250, "warning: cannot open config file", TDH_configfile ) );
	}


/* get user settings.. */
while( fgets( value, 511, fp ) != NULL ) {

	TDH_value_subst( buf, value, NULL, "", NORMAL, 1 ); /* evaluate variables */

	buf[ strlen( buf ) -1 ] = '\0';
	nt = sscanf( buf, "%s %s", tag, value );
	if( nt < 1 ) continue; /* skip blank lines */
	if( tag[0] == '/' ) continue; /* skip comments */

	if( nt < 2 ) strcpy( value, "" );

	if( stricmp( tag, "scriptdir:" )==0 || stricmp( tag, "pagesdir:" )==0 ) strcpy( TDH_scriptdir, value );
	/* note.. mrcgi sets this by way of projdir: */

	else if( stricmp( tag, "tmpdir:" )==0 ) strcpy( TDH_tmpdir, value );  
	else if( stricmp( tag, "shellmetachars:" )==0 ) strcpy( TDH_shellmetachars, value );
#ifndef TDH_NOREC
	else if( stricmp( tag, "fdfpath:" )==0 ) strcpy( TDH_fdfpath, value );
#endif
	else if( stricmp( tag, "dbnull:" )==0 ) { 
		if( strlen( value ) > 4 ) {
			err( 1282, "dbnull symbol may not be longer than 4 chars", "" );
			}
		strncpy( TDH_dbnull, value, 4 ); TDH_dbnull[4] = '\0';
		}

	else if( stricmp( tag, "varvalue:" )==0 ) {   
		int i, tt;
		char var[40], val[255];
		for( i = 0, slen = strlen( value ); i < slen; i++ ) {
			if( value[i] == '=' ) {
				value[i] = ' ';
				break;
				}
			}
		tt = sscanf( value, "%s %s", var, val );
		if( tt == 1 ) strcpy( val, "" );
		stat = TDH_setvar( var, val );
		if( stat != 0 ) return( stat );
		}
	else if( stricmp( tag, "putenv:" )==0 ) {
		/* cannot use automatic storage for putenv */
		if( pelen + strlen( value ) > MAXPE )
			err( 1251, "tdhconfig: too many putenv entries", "" );
		else	{
			strcpy( &putenvstring[ pelen ], value );
			putenv( &putenvstring[ pelen ] );
			pelen += strlen( value ) + 1;
			}
		}

	else if( stricmp( tag, "decpt:" )==0 ) TDH_decpt = value[0];

#ifndef SHSQL

	/* this is shielded for SHSQL standalone applications.  QUISP needs functions code and thus
	   must be linked such that tdhkit.a has presidence over libshsql.a */

        else if( GL_slmember( tag, "dateformat: pivotyear: months* weekdays: omitweekends: lazydates: strictdatelengths:" )) {
		int ix;
		ix = 0;
		GL_getok( buf, &ix ); /* skip tag.. */
		while( isspace( (int) buf[ix] )) ix++;
                stat = DT_setdateparms( tag, &buf[ix] );
                if( stat != 0 ) return( err( stat, "date parm error in config file", buf ) );
                }
#endif
	}

fclose( fp ); fp = NULL;
TDH_initialized = 1;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
