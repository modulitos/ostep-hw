/* ======================================================= *
 * Copyright 1998-2009 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PL - ploticus main module */

#include "pl.h"
#include <unistd.h>
#ifdef WIN32
#include <fcntl.h>  /* for _O_BINARY */
#endif

extern int PLGG_setimfmt();


/* ============================================= */
int
PL_version_msg( longmsg )
int longmsg;
{
char outputformats[80];
PL_devstring( outputformats );


#ifdef WIN32
fprintf( PLS.diagfp, "ploticus %s (win32).", PLVERSION );
#else
fprintf( PLS.diagfp, "ploticus %s (unix).", PLVERSION );
#endif
fprintf( PLS.diagfp, "  %s\n", outputformats );
fprintf( PLS.diagfp, "Copyright 1998-2009 Steve Grubb, http://ploticus.sourceforge.net\n\n" );

if( longmsg ) fprintf( PLS.diagfp, "Please see the Copyright file or web site for additional credits and information.\n\
\n\
This program is free software; you can redistribute it and/or modify it\n\
under the terms of the GNU General Public License as published by the\n\
Free Software Foundation; either version 2 of the License, or (at your\n\
option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful, but \n\
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY \n\
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License\n\
for more details.\n" );
exit( 1 );
}

/* ============================================= */
/* MAIN */
/* ============================================= */
int
main( argc, argv )
int argc;
char **argv;
{
int argi, use_stdin, stat, found, valused, stoparg, ci, cii;
char buf[256];
char scriptfile[MAXPATH];
char prefabname[80];
char *arg, *nextarg, *getenv();
char *outnamebase;





/* check to see if we are operating as a direct CGI program.. */
PLS.cgiargs = NULL;
if( argc == 1 ) {
	PLS.cgiargs = getenv( "QUERY_STRING" );
	if( getenv( "SCRIPT_FILENAME" )==NULL ) {  /* some web servers don't set SCRIPT_FILENAME.. scg 8/27/02 */
		sprintf( buf, "SCRIPT_FILENAME=%s", argv[0] );
		putenv( buf );
		}
	}


/* set hard defaults and process config file if any.. */
stat = PL_do_preliminaries();
if( stat ) exit( 1 );
use_stdin = 0;


/* if in direct cgi mode, initialize and set some useful defaults.. */
if( PLS.cgiargs != NULL ) {
	/* begin parsing QUERY_STRING.. */
	stoparg = 80; 
	ci = 0;
	
	strcpy( PLS.outfile, "stdout" );
#ifdef WIN32
        /* must set stdin and stdout to binary mode */
        _setmode( _fileno( stdin ), _O_BINARY );
        _setmode( _fileno( stdout ), _O_BINARY );
#endif /* WIN32 */

	PLS.device = 's';
#ifndef NOGD
	PLS.device = 'g';
	/* try to be smart about picking default output format.. */
	if( devavail( "gif" )) PLGG_setimfmt( "gif" );
	else if( devavail( "jpeg" )) PLGG_setimfmt( "jpeg" );
	else if( devavail( "png" )) PLGG_setimfmt( "png" );
#endif
	}

else stoparg = argc;

strcpy( scriptfile, "" );
strcpy( prefabname, "" );

/* process command line arguments.. (if direct CGI, arguments are parsed from URL).. */
for( argi = 1; argi < stoparg; argi++ ) {

	if( PLS.cgiargs != NULL ) {
		/* parse next 2 args from QUERY_STRING.. */
		GL_getcgiarg( PL_bigbuf, PLS.cgiargs, &ci, 252 ); 
		if( PL_bigbuf[0] == '\0' ) break;
		arg = PL_bigbuf;
		cii = ci;
		GL_getcgiarg( &PL_bigbuf[500], PLS.cgiargs, &cii, 252 );  /* share PL_bigbuf */
		nextarg = &PL_bigbuf[500]; 
		}
	else	{
		arg = argv[ argi ];
		if( argi+1 < argc ) nextarg = argv[argi+1];
		else nextarg = "";
		}

	if( strcmp( arg, "-stdin" )==0 && PLS.cgiargs == NULL ) use_stdin = 1;


#ifdef HOLD
	else if( strcmp( arg, "-png" )==0 && !devavail( "png" ) && PLS.cgiargs == NULL ) { 
		char *p[100];
		if( strlen( argv[0] ) >= 5 ) {
			if( strcmp( &argv[0][strlen(argv[0])-5], "plpng" )==0 ) {
				Eerr( 5279, "png not available in plpng", "" );
				PL_version_msg( 0 ); exit(1);
				}
			}
		p[0] = "plpng";
		for( i = 1; i < argc; i++ ) p[i] = argv[i];
		p[argc] = NULL;
		execvp( "plpng", argv );
		fprintf( PLS.errfp, "PNG not supported in this build (plpng not found).\n" );
                PL_version_msg( 0 ); exit(1);
		}
#endif

	else if( strcmp( arg, "-f" )==0 ) {
		if( strlen( nextarg ) > MAXPATH-10 ) { /* allow extra for output file suffix add */
			fprintf( PLS.errfp, "pl: script file name too long" );
			PL_version_msg( 0 ); exit( 1 );
			}
		strcpy( scriptfile, nextarg );
		argi++;
		}

	else if( strcmp( arg, "-prefab" )==0 ) {
		if( PLS.cgiargs != NULL ) {
			Eerr( 4916, "-prefab not available in direct cgi mode", "" ); PL_version_msg( 0 ); exit( 1 );
			}
		if( PLS.prefabsdir == NULL ) {
			Eerr( 4899, "PLOTICUS_PREFABS environment var not found (pathname of dir where prefab files reside)", "" );
			PL_version_msg( 0 ); exit( 1 );
			}
		sprintf( prefabname, "%s.pl", nextarg );
		sprintf( scriptfile, "%s%c%s", PLS.prefabsdir, PATH_SLASH, prefabname );
		if( PLS.debug ) fprintf( PLS.diagfp, "Prefabs dir is: %s\nScript file is %s\n", PLS.prefabsdir, scriptfile );
		argi++;
		}

	else if( GL_smember( arg, "-? -help -ver -version" ) ) { PL_version_msg( 1 ); exit(0); }

	else 	{
		stat = PL_process_arg( arg, nextarg, &valused, &found );
		if( stat != 0 ) exit( 1 );
		if( !found && arg[0] == '-' ) Eerr( 4892, "warning, unrecognized argument", arg );
		else if( !found && scriptfile[0] == '\0' ) {
			if( strlen( arg ) > MAXPATH-10 ) { /* allow extra for output file suffix add */
				fprintf( PLS.errfp, "pl: script file name too long" );
				PL_version_msg( 0 ); exit( 1 );
				}
			strcpy( scriptfile, arg  );  
			}
		argi += valused;
		if( PLS.cgiargs != NULL && valused ) ci = cii; /* jump ahead */
		}
	}



/* CGI header stuff.. */
if( PLS.cgiargs != NULL ) {
	char imagetype[20];
	strcpy( PLS.outfile, "stdout" );
	/* check for loopy script file names.. */
	if( scriptfile[0] == '/' && prefabname[0] == '\0' ) {   /* changed scg 2/6/02 */
		Eerr( 2740, "cgi mode: script file name may not begin with '/'", scriptfile );
		PL_version_msg( 0 ); exit(1);
		}
	if( GL_slmember( scriptfile, "*..* .*" ) ) {
		Eerr( 2740, "cgi mode: script file name may not begin with '.' or contain '..'", scriptfile );
		PL_version_msg( 0 ); exit(1);
		}
	/* output the HTTP content-type header.. */
	devnamemap( &(PLS.device), imagetype, 2 );
	if( PLS.device == 's' ) printf( "Content-type: image/%s-xml\n\n", imagetype );
	else printf( "Content-type: image/%s\n\n", imagetype );

	/* be sure clickmap is off - incompatible with direct cgi mode.. */
	PLS.clickmap = 0;
	}



/* if script coming from stdin, copy stdin to a tmp file.. */
if( use_stdin ) {
	FILE *tfp;
	sprintf( scriptfile, "%s_I", PLS.tmpname );
	tfp = fopen( scriptfile, "w" ); /* temp file, unlinked below */
	if( tfp == NULL ) { Eerr( 102, "Cannot open tmp file for stdin script\n", scriptfile ); PL_version_msg( 0 ); exit(1); }
	while( fgets( PL_bigbuf, MAXBIGBUF-1, stdin ) != NULL ) fprintf( tfp, "%s", PL_bigbuf ); /* was 255 scg 5/20/03 */
	fclose( tfp );
	}
	

if( scriptfile[0] == '\0' ) {
	fprintf( PLS.diagfp, "usage: pl scriptfile [options] ...or...  pl -prefab prefabname [options]\n" );
	PL_version_msg( 0 );
	}



/* DEVICE variable.. */

if( TDH_getvar( "DEVICE", buf ) != 0 ) { /* DEVICE not given on command line, set DEVICE */
	stat = devnamemap( &(PLS.device), buf, 2 );
	if( stat != 0 ) { PL_version_msg( 0 ); exit( 1 ); }
	TDH_setvar( "DEVICE", buf );
	}
else	{ /* DEVICE given on command line, set PLS.device from DEVICE */
	TDH_getvar( "DEVICE", buf );
	stat = devnamemap( &(PLS.device), buf, 1 );
	if( stat != 0 ) { PL_version_msg( 0 ); exit( 1 ); }
	}
if( PLS.debug ) fprintf( PLS.diagfp, "Device code is %c\n", PLS.device );



/* build output file names.. */

if( prefabname[0] != '\0' ) outnamebase = prefabname;
else outnamebase = scriptfile;

if( PLS.outfile[0] == '\0' && GL_member( PLS.device, "egsf" ) ) makeoutfilename( outnamebase, PLS.outfile, PLS.device, 1);


/* if a viewcommand is specified and an outfile has not been and the device 
   is paginated postscript, we need to set the output file to out.ps */
if( PLS.viewer[0] != '\0' && PLS.outfile[0] == '\0' && GL_member( PLS.device, "cp")) strcpy( PLS.outfile, "out.ps" );

if( PLS.outfile[0] != '\0' && 
    strcmp( PLS.outfile, "-" )!= 0 ) {
	if( PLS.debug ) fprintf( PLS.diagfp, "Setting output file name to %s\n", PLS.outfile );
	Esetoutfilename( PLS.outfile );
	}


PL_begin(); /* various other initializations that must be done after config & args processing.. */



/* execute the script file to produce the plot.. */
if( PLS.debug ) fprintf( PLS.diagfp, "Script file is: %s\n", scriptfile );
stat = PL_exec_scriptfile( scriptfile );
if( stat != 0 ) { PL_version_msg( 0 ); exit( 1 ); }


/* finish up (x11: button, etc.) */
if( PLS.eready && PLS.device == 'x' ) PL_do_x_button( "End." );

if( PLS.eready ) Eendoffile();
if( use_stdin ) unlink( scriptfile );

if( PLS.viewer[0] != '\0' && PLS.device != 'x' && PLS.cgiargs == NULL ) {
	int len;
	strcpy( buf, PLS.viewer );
	len = strlen( buf );
	strcpy( &buf[len++], " " );
	Egetoutfilename( &buf[ len ] );
	if( strnicmp( &buf[ len ], "stdout", 6 )==0 ) fprintf( PLS.diagfp, "Cannot use -o stdout with -viewer\n" );
	else if( PLS.noshell ) fprintf( PLS.diagfp, "-noshell prohibits -viewer" );
	else 	{ 
		fprintf( PLS.diagfp, "Executing '%s' to view results..\n", buf ); 
		system( buf ); 
		}
	}

PL_free();
exit( 0 );
}

/* ======================================================= *
 * Copyright 1998-2009 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
