/* ======================================================= *
 * Copyright 1998-2006 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"
#include "tdhkit.h"


#ifndef CONFIGFILE
#define CONFIGFILE ""
#endif


#ifdef LOCALE
#include <locale.h>
#endif

extern int PLGS_setparms(), PLGF_setparms();
extern int fchmod(), chdir();
extern int TDH_inquisp;
extern int PLGG_initstatic(), PLGP_initstatic(), PLGS_initstatic(), PLGF_initstatic();



/* =========================================== */
/* DO_PRELIMINARIES - set defaults, read config file, etc. */
int
PL_do_preliminaries()
{
char buf[512];
FILE *fp;
char *filename, *getenv();
char attr[80];
char val[512];
char *lineval;
int ix;
int valused, found;
int i, stat, j;
int projectrootfound;
char pathslash;
char uniq[80];
char configfile[MAXPATH];
char cgierrfile[80];


TDH_errprog( "pl" );


/* set pre-config (hard-coded) defaults.. */
PLS.debug = 0;
PLS.echolines = 0;
PLS.skipout = 0;
PLS.eready = 0;
PLS.prefabsdir = NULL;
strcpy( PLS.outfile, "" );
PLS.winx = 100; PLS.winy = 0; PLS.winw = 8.0; PLS.winh = 8.0;
PLS.winsizegiven = 0;
PLS.bkcolorgiven = 0;
PLS.clickmap = 0;
PLS.usingcm = 0;
strcpy( PLS.viewer, "" );
strcpy( PLS.mapfile, "" );
PLS.noshell = 0;
TDH_prohibit_shell( 0 );

#ifndef WIN32
TDH_reslimits( "cpu", CPULIMIT );
#endif
#ifdef LOCALE
 setlocale(LC_CTYPE, "" );
 setlocale(LC_COLLATE, "" );
#endif

#ifdef NOX11
PLS.device = 'e';
#else
PLS.device = 'x';    
#endif

PLD.maxrows = MAXDROWS;
PLD.maxdf = MAXD;
PLL.maxproclines = MAXPROCLINES;
PLVsize = MAXDAT;

PLG_set_early_defaults();

PLS.errfp = stderr; 	/* portability? */
PLS.diagfp = stderr;	/* portability? */
PLS.bignumspacer = '\0'; 	/* use standard number notation */
PLS.bignumthres = 4; 
suppress_convmsg( 1 ); /* suppress unplottable data msgs by default - user can turn on */
DT_checkdatelengths( 0 ); /* don't be strict about the length of date items */
setintvar( "CM_UNITS", 0 );
projectrootfound = 0;

strcpy( TDH_tmpdir, TMPDIR );
pathslash = PATH_SLASH;

/* set this now, but it might be updated depending on what's in config file.. */
GL_make_unique_string( uniq, 0 );
sprintf( PLS.tmpname, "%s%cplo%s", TDH_tmpdir, pathslash, uniq ); 

/* make cgierrfile (default /tmp/plcgi_err) for cgi errors */
sprintf( cgierrfile, "%s%cplcgi_err", TDH_tmpdir, PATH_SLASH );




/* reads and process config file, if any.. */
if( PLS.cgiargs != NULL ) {
	/* determine name of config file.. (can't use PLOTICUS_CONFIG in CGI context) */
	char *cgiprogname;
	strcpy( buf, "file=" );
	strcat( buf, CONFIGFILE );
	if( strlen( buf ) == 5 ) {   /* CONFIGFILE not set.. retrieve prog name from CGI environment and 
					 build config file name from that..  */
		cgiprogname = getenv( "SCRIPT_FILENAME" );
		if( cgiprogname == NULL ) {
			PLS.errfp = fopen( cgierrfile, "w" );
			if( PLS.errfp != NULL ) {
				fprintf( PLS.errfp, "cgi var SCRIPT_FILENAME not found.\n" );
#ifdef UNIX
				fchmod( fileno( PLS.errfp ), 00666 );
#endif
				}
			TDH_errfile( PLS.errfp ); /* set it for TDH */
			return( 1 );
			}
		strcat( buf, cgiprogname );
		j = strlen( buf ) -4;
		if( strcmp( &buf[ j ], ".cgi" )==0 ) buf[ j ] = '\0';
		else if( strcmp( &buf[ j ], ".exe" )==0 ) buf[ j ] = '\0';
		strcat( buf, ".cnf" );
		}
	strcpy( configfile, buf );
	}
else	{
	/* command line usage.. check PLOTICUS_CONFIG.. */
	filename = getenv( "PLOTICUS_CONFIG" );
	if( filename == NULL ) goto SKIPCONFIG;
	sprintf( configfile, "file=%s", filename );
	}

if( strlen( configfile ) == 5 ) {
	if( PLS.cgiargs != NULL ) goto BAD_CGI_CONFIG;
	else goto SKIPCONFIG;  /* no config file given.. */
	}


stat = TDH_readconfig( configfile );
/* no point in checking return stat.. */


/* do this again because TDH_tmpdir might have been updated.. */
GL_make_unique_string( uniq, 0 );
sprintf( PLS.tmpname, "%s%cplo%s", TDH_tmpdir, pathslash, uniq ); 

/* now read it again to get pl-specific items.. */

fp = fopen( &configfile[5], "r" );
if( fp == NULL ) {
	if( PLS.cgiargs != NULL ) {
		BAD_CGI_CONFIG:
		PLS.errfp = fopen( cgierrfile, "w" );
		if( PLS.errfp != NULL ) {
			fprintf( PLS.errfp, "cgi mode: cannot open config file (%s).\n", &configfile[5] );
#ifdef UNIX
			fchmod( fileno( PLS.errfp ), 00666 );
#endif
			}
		return( 1 );
		}
	else Eerr( 15060, "Cannot open ploticus config file", &configfile[5] );
	return( 0 );
	}


/* get user settings.. */
while( fgets( buf, 511, fp ) != NULL ) {
	buf[ strlen( buf ) -1 ] = '\0';
	ix = 0;
	strcpy( attr, GL_getok( buf, &ix ) );
	if( attr[0] == '\0' ) continue;
	if( attr[0] == '#' || attr[0] == '/' ) continue; /* skip comments of various kinds */
	while( isspace( (int) buf[ix] ) )ix++;
	lineval = &buf[ix];
	strcpy( val, GL_getok( buf, &ix ) );

	if( attr[ strlen( attr ) -1 ] == ':' ) attr[ strlen( attr ) - 1 ] = '\0';


	/* attributes that exist in config file, but not proc settings, go here: */

	if( strcmp( attr, "projectroot" )==0 ) {
		stat = chdir( val );
		if( stat != 0 ) goto CGI_BAD_CHDIR;
		projectrootfound = 1;
		if( PLS.debug ) fprintf( PLS.diagfp, "config: found projectroot.. chdir to %s..\n", val );
		}

	else if( strcmp( attr, "option" )==0 ) {
		val[0] = '\0';
		sscanf( buf, "%*s %s %s", attr, val );

		/* check for embedded '=' in attr.. if found indicates prefab parm setting.. send lineval as attr.. */
		for( i = 0, found = 0; attr[i] != '\0'; i++ ) if( attr[i] == '=' ) { found = 1; break; }
		if( found ) { strcpy( attr, lineval ); strcpy( val, "" ); }

		if( PLS.debug ) fprintf( PLS.diagfp, "config file: got option: %s %s\n", attr, val );
		PL_process_arg( attr, val, &valused, &found );	
		if( !found ) Eerr( 2784, "invalid 'option:' in config file", attr );
		}

	/* shared settings takes care of settings that can be set in config file OR proc settings */
	else 	{
		stat = PL_sharedsettings( attr, lineval ); 
		/*was: stat = PL_sharedsettings( attr, val, lineval );  */
		if( stat == 0 && PLS.debug ) fprintf( PLS.diagfp, "config: setting %s to %s\n", attr, lineval );
		}

	/* don't forget that there are other settings (tmpdir, date-related, etc.) 
	 * that were handled by the TDH config file reader !! 
	 */

	}

fclose( fp );

SKIPCONFIG:
if( PLS.cgiargs != NULL && !projectrootfound ) {
	CGI_BAD_CHDIR:
	PLS.errfp = fopen( cgierrfile, "w" );
	if( PLS.errfp != NULL ) {
		fprintf( PLS.errfp, "cgi mode: no projectroot in config file, or could not chdir to projectroot\n" );
#ifdef UNIX
		fchmod( fileno( PLS.errfp ), 00666 );
#endif
		}
	return( 1 );
	}

/* get prefabs directory name if available.. */
/* this must come after config file is read, because in cgi mode PLOTICUS_PREFABS is set via config file. */
PLS.prefabsdir = getenv( "PLOTICUS_PREFABS" );

/* maybe PREFABS_DIR was set in pl.h  ... */
if( PLS.prefabsdir == NULL ) PLS.prefabsdir = PREFABS_DIR ;
else if( PLS.prefabsdir[0] == '\0' ) PLS.prefabsdir = PREFABS_DIR ;
if( PLS.prefabsdir[0] == '\0' ) PLS.prefabsdir = NULL;

if( PLS.prefabsdir != NULL ) {
	TDH_setspecialincdir( PLS.prefabsdir ); /* set special include directory (#include $foo) */
        /* note: prefabsdir must reference static storage, either via getenv() or constant */
	}


return( 0 );
}


/* ====================================== */
/* BEGIN - initializations that are done AFTER config file and args are looked at.. */
int
PL_begin()
{
char buf[128];

/* graphcore initializations.. */
Esetsize( PLS.winw, PLS.winh, PLS.winx, PLS.winy );
EDXlo = 0.0; EDXhi = 0.0; EDYlo = 0.0; EDYhi = 0.0;
PLS.eready = 0;

#ifndef NOSVG
if( PLS.device == 's' ) PLGS_setparms( PLS.debug, PLS.tmpname, PLS.clickmap ); 
#endif
#ifndef NOSWF
if( PLS.device == 'f' ) PLGF_setparms( PLS.debug, PLS.tmpname, Estandard_font );  /* pass user selected -font if any */
#endif


/* initialize the data structures.. */
PL_init_mem();

if( PLS.debug ) {
        fprintf( PLS.diagfp, "Version: pl %s\n", PLVERSION );
        if( PLS.cgiargs != NULL ) fprintf( PLS.diagfp, "operating in CGI mode\n" );
        Epcodedebug( 1, PLS.diagfp ); /* tell pcode.c to output diagnostics too */
        }

/* set PLVERSION variable.. */
sprintf( buf, "ploticus %s http://ploticus.sourceforge.net (GPL)", PLVERSION );
TDH_setvar( "PLVERSION", buf );

if( PLS.clickmap ) {  /* .map filename */
	if( PLS.mapfile[0] == '\0' ) {
		if( PLS.clickmap == 2 ) strcpy( PLS.mapfile, "stdout" );  /* csmap defaults to stdout..  scg 8/26/04  */
		else if( PLS.outfile[0] != '\0' ) makeoutfilename( PLS.outfile, PLS.mapfile, 'm', 1);
		else strcpy( PLS.mapfile, "unnamed.map" );
		}
	PL_clickmap_init();
	}

return( 0 );
}


/* ================================== */
/* INIT_STATICS - initialize static variables */
int
PL_init_statics()
{
PLG_cblock_initstatic();
PLG_init_initstatic();
PLG_mark_initstatic();
PLG_pcode_initstatic();
PLG_stub_initstatic();
PL_execline_initstatic();
PL_fieldnames_initstatic();
PL_units_initstatic();
PL_lib_initstatic();
PLP_bars_initstatic();
/* PLP_getdata_initstatic(); */
PLP_legend_initstatic();
PLP_processdata_initstatic();
#ifndef NOGD
PLGG_initstatic() ;
#endif
#ifndef NOPS
PLGP_initstatic();
#endif
#ifndef NOSVG
PLGS_initstatic();
#endif
#ifndef NOSWF
PLGF_initstatic();
#endif
/* no initstatic for X11 .. doesn't seem necessary now */

/* the following static initializations shouldn't be done if ploticus is being invoked
   from environments (eg quisp) where the TDH stuff is already in action.. */
if ( ! TDH_inquisp ) {
  GL_initstatic();
  TDH_condex_initstatics();
  TDH_err_initstatic();
  TDH_functioncall_initstatic();
  TDH_valuesubst_initstatic();
  TDH_setvar_initstatic();
  TDH_shell_initstatic();
  DT_initstatic();
  DT_time_initstatic();
  DT_datetime_initstatic();

  TDH_readconfig_initstatic();  /* some doubt on this one */
}


return( 0 );
}

/* ================================== */
/* INIT_MEM - initialize pl data structures */
int
PL_init_mem()
{
/* data array stuff.. */
PLD.datarow = (char **) malloc( PLD.maxrows * sizeof( char * ) ); 
PLD.df = (char **) malloc( PLD.maxdf * sizeof( char * ) );
PLD.currow = 0;
PL_cleardatasets();

PLL.procline = (char **) malloc( PLL.maxproclines * sizeof( char * ) );

PLV = (double *) malloc( PLVsize * sizeof( double ));
PLVhalfsize = PLVsize / 2;
PLVthirdsize = PLVsize / 3;

return( 0 );
}


/* ================================== */
/* FREE - free all mallocated memory.  */
int
PL_free( )
{
int i;

PL_clickmap_free();
PL_catfree();

free( PLD.df );

for( i = 0; i < PLD.currow; i++ ) free( PLD.datarow[ i ] );
free( PLD.datarow );

for( i = 0; i < PLL.nlines; i++ ) free( PLL.procline[ i ] );
free( PLL.procline );

free( PLV ); /* scg 5/16/03 */

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

