/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROCESS_ARG - process one command line argument from pl */

#include "pl.h"
extern char TDH_tmpdir[];
extern int PLGS_showtag(), PLGS_zlevel(), PLGS_setxmlparms(), PLGG_setimpixsize();
extern int chmod(), fchmod(), chdir();


int
PL_process_arg( opt_in, val, valused, found )
char *opt_in; /* option */
char *val; /* argument that follows option */
int *valused; /* 1 if val was used, 0 if not */
int *found; /* 1 if option found here, 0 if not */
{
int j, nt, slen, rtnstat, olen;
char buf[256], opt[256];

if( PLS.debug ) { fprintf( PLS.diagfp, "Got command line arg(s): %s ", opt_in ); fflush( PLS.diagfp ); }

*found = 1;
rtnstat = 0;

/* all parameters and args processed herein are truncated silently to length 255.  changed, scg 5/30/06 */
if( opt_in[0] == '-' ) strncpy( opt, &opt_in[1], 255 ); 
else strncpy( opt, opt_in, 255 ); 
opt[255] = '\0';  

/* see if it is a var=value parameter.. if so get it, we're done */
if( opt_in[0] != '-' ) {   /* condition added scg 5/30/06 */
	for( j = 0, slen = strlen( opt ); j < slen; j++ ) {
		if( opt[j] == '=' ) { 	/* yes, its a var=value parameter */
			int ix;
			char varname[40];
       			strcpy( buf, opt );
       			ix = 0;
        		GL_getseg( varname, buf, &ix, "=" );
        		TDH_setvar( varname, &buf[ix] );
			if( PLS.debug ) fprintf( PLS.diagfp, " ...setting var %s to: %s.\n", varname, &buf[ix] );
			*valused = 0;
			return( rtnstat );
			}
		}
	}

/* add to list of parameters set on command line.. */
olen = strlen( opt );
if( olen < 40 ) {  /* length check added.. scg 3/16/06 */
	if( (strlen( PLS.cmdlineparms ) + olen) < 290 ) {
		strcat( PLS.cmdlineparms, opt );
		strcat( PLS.cmdlineparms, " " );
		}
	}

*valused = 1;


if( GL_slmember( opt, "x11 gif png jpeg wbmp ps bwps eps svg svgz swf" )) { 
	rtnstat = devnamemap( &(PLS.device), opt, 1 ); 
	*valused = 0; 
	}

else if( strcmp( opt, "debug" )==0) { 
	PLS.debug = 1; 
	*valused = 0; 
	if( PLS.cgiargs != NULL ) {
		/* -debug in cgi mode writes stuff to fixed temp dir files.. */
		sprintf( buf, "%s%cplcgi_diag", TDH_tmpdir, PATH_SLASH );
		PLS.diagfp = fopen( buf, "w" );
		if( PLS.diagfp == NULL ) PLS.diagfp = stderr;
#ifdef UNIX
		else fchmod( fileno( PLS.diagfp ), 00666 ); 
#endif
		sprintf( buf, "%s%cplcgi_err", TDH_tmpdir, PATH_SLASH );
		PLS.errfp = fopen( buf, "w" );
		if( PLS.errfp == NULL ) PLS.errfp = stderr;
#ifdef UNIX
		else fchmod( fileno( PLS.errfp ), 00666 ); 
#endif
		}
	}

else if( strcmp( opt, "bbdebug" )==0 ) {
        Epcodedebug( 2, stderr ); /* tell pcode.c to output bounding box diagnostics */
	*valused = 0; 
	}

else if( strcmp( opt, "tightcrop" )==0 ) { Etightbb(1); *valused = 0; }


else if( strcmp( opt, "crop" )==0 || strcmp( opt, "croprel" ) ==0 ) {
	double cropx1, cropy1, cropx2, cropy2;
	nt = sscanf( val, "%lf,%lf,%lf,%lf", &cropx1, &cropy1, &cropx2, &cropy2 );
	if( nt != 4 ) 
		fprintf( PLS.errfp, "pl argument error, correct usage is: -crop x1,y1,x2,y2 OR -croprel left,bottom,right,top\n" );
	else {
		if( PLS.usingcm ) { cropx1 /= 2.54; cropy1 /= 2.54; cropx2 /= 2.54; cropy2 /= 2.54; }
		if( strcmp( opt, "croprel" )==0 ) Especifycrop( 2, cropx1, cropy1, cropx2, cropy2 ); /* relative to tight */
		else Especifycrop( 1, cropx1, cropy1, cropx2, cropy2 ); /* absolute */
		}
	}

else if( strcmp( opt, "pagesize" )==0 ) {
	nt =  sscanf( val, "%lf,%lf", &(PLS.winw), &(PLS.winh) );
	if( nt != 2 ) fprintf( PLS.errfp, "pl argument error, correct usage is: -pagesize width,height  (in inches or cm)\n" );
	else PLS.winsizegiven = 1;
	if( PLS.usingcm ) { PLS.winw /= 2.54; PLS.winh /= 2.54; }
	}


else if( strcmp( opt, "pixsize" )==0 ) {  /* added scg 1/9/08 */
	int reqwidth, reqheight;
	nt = sscanf( val, "%d,%d", &reqwidth, &reqheight );
	if( nt != 2 ) fprintf( PLS.errfp, "pl argument error, correct usage is: -pixsize width,height  (in pixels)\n" );
#ifndef NOGD
	PLGG_setimpixsize( reqwidth, reqheight );
#endif
	if( PLS.device != 'g' ) fprintf( PLS.errfp, "-pixsize ignored.. it's only applicable when generating png/gif/jpeg images\n" ); 
	}
	

else if( strcmp( opt, "winloc" )==0 ) {
	nt = sscanf( val, "%d,%d", &(PLS.winx), &(PLS.winy) );
	if( nt != 2 ) {
		PLS.winx = 100; PLS.winy = 0;
		fprintf( PLS.errfp, "pl argument error, correct usage is: -winloc x,y\n" );
		}
	}

else if( strcmp( opt, "o" )==0 ) {
	strncpy( PLS.outfile, val, MAXPATH-2 ); 
	PLS.outfile[MAXPATH-2] = '\0';
	}

else if( strcmp( opt, "scale" )==0) {
	double sx, sy;
	nt =  sscanf( val, "%lf,%lf", &sx, &sy );
	if( nt == 1 ) Esetglobalscale( sx, sx );
	else if( nt == 2 ) Esetglobalscale( sx, sy );
	}

else if( strcmp( opt, "posteroffset" )==0) {
	double x, y;
	nt = sscanf( val, "%lf,%lf", &x, &y );
	if( nt != 2 ) fprintf( PLS.errfp, "pl argument error, correct usage is -posteroffset x,y\n" );
	else	{
		if( PLS.usingcm ) { x /= 2.54; y /= 2.54; }
		Esetposterofs( x, y );
		}
	}

else if( strcmp( opt, "maxrows" )==0 ) { if( atoi( val ) > 1000 ) PLD.maxrows = atoi( val ); }
else if( strcmp( opt, "maxfields" )==0 ) { if( atoi( val ) > 10000 ) PLD.maxdf = atoi( val ); }
else if( strcmp( opt, "maxproclines" )==0 ) { if( atoi( val ) > 500 ) PLL.maxproclines = atoi( val ); }
else if( strcmp( opt, "maxvector" )==0 ) { if( atoi( val ) > 500 ) PLVsize = atoi( val ); }
else if( strcmp( opt, "maxdrawpoints" )==0 ) { PLG_setmaxdrivervect( atoi( val )); }
else if( strcmp( opt, "dir" )==0 ) chdir( val );
else if( strcmp( opt, "outlabel" )==0 ) Esetoutlabel( val );

else if( strcmp( opt, "map" )==0 ) {   /* server side map */
	PLS.clickmap = 1; *valused = 0; 
	fprintf( PLS.errfp, "warning, server-side clickmaps (-map) are deprecated in 2.40 in favor of client-side (-csmap)\n" );
	}   
else if( strcmp( opt, "mapdemo" )==0 ) { 
	PLS.clickmap = 1; *valused = 0; 
	PL_clickmap_demomode( 1 ); 
	fprintf( PLS.errfp, "warning, server-side clickmaps (-map) are deprecated in 2.40 in favor of client-side (-csmap)\n" );
	}   
else if( strcmp( opt, "csmap" )==0 ) { PLS.clickmap = 2; *valused = 0; } /* client side map */
else if( strcmp( opt, "csmapdemo" )==0 ) { PLS.clickmap = 2; *valused = 0; PL_clickmap_demomode( 1 ); } /* client side map */
else if( strcmp( opt, "mapfile" )==0 ) { 
	strcpy( PLS.mapfile, val );
	if( PLS.clickmap == 0 ) PLS.clickmap = 1; 
	*valused = 1; 
	}   
else if( strcmp( opt, "landscape" )==0 ) { PLS.landscape = 1; *valused = 0; }
else if( strcmp( opt, "portrait" )==0 ) { PLS.landscape = 0; *valused = 0; PLS.winw = 8.5; PLS.winh = 11.0; } 
else if( strcmp( opt, "showbad" )==0 ) { suppress_convmsg( 0 ); *valused = 0; }
else if( strcmp( opt, "noshell" )==0 || strcmp( opt, "nosh" )==0 ) { PLS.noshell = 1; TDH_prohibit_shell( 1 ); *valused = 0; }
else if( strcmp( opt, "font" )==0) strcpy( Estandard_font, val ); 
else if( strcmp( opt, "textsize" )==0) Estandard_textsize = atoi( val );
else if( strcmp( opt, "linewidth" )==0) Estandard_lwscale = atof( val );
else if( strcmp( opt, "color" )==0) strcpy( Estandard_color, val );
else if( strcmp( opt, "cm" )==0) { PLS.usingcm = 1; *valused = 0; setintvar( "CM_UNITS", 1 ); }
else if( strcmp( opt, "inches" )==0) { PLS.usingcm = 0; *valused = 0; setintvar( "CM_UNITS", 0 ); }

else if( strcmp( opt, "backcolor" )==0) {
	strcpy( Estandard_bkcolor, val );
	Ebackcolor( val );
	PLS.bkcolorgiven = 1;
	}

else if( strcmp( opt, "viewer" )==0 || strcmp( opt, "v" )==0 ) { if( PLS.cgiargs == NULL ) strcpy( PLS.viewer, val ); }
else if( strcmp( opt, "errmsgpre" )==0 ) TDH_errprogsticky( val ); /* added 3/25/04 - scg */
else if( strcmp( opt, "drawdump" )==0 ) {
	PLG_setdumpfile( val, "w" ); /* added 8/5/04 - scg */
	PLS.device = 'n'; /* null device */
	}
else if( strcmp( opt, "drawdumpa" )==0 ) {
	PLG_setdumpfile( val, "a" ); /* added 8/5/04 - scg */
	PLS.device = 'n'; /* null device (added 6/7/05 - scg) */
	}
#ifndef NOSVG
else if( strcmp( opt, "tag" )==0) { PLGS_showtag( 1 ); *valused = 0; }
else if( strcmp( opt, "zlevel" )==0) PLGS_zlevel( atoi( val ) ); 
else if( strcmp( opt, "xml_encoding" )==0 ) PLGS_setxmlparms( "encoding", val );
else if( strcmp( opt, "omit_xml_declaration" )==0 ) { PLGS_setxmlparms( "xmldecl", "0" ); *valused = 0; }
else if( strcmp( opt, "svg_tagparms" )==0 ) PLGS_setxmlparms( "svgparms", val );
#endif


else if( strcmp( opt, "diagfile" )==0 ) {
	if( PLS.cgiargs != NULL ) goto FINISH; /* don't allow this in cgi mode.. scg 2/8/02 */
	if( GL_smemberi( val, "stderr" )) PLS.diagfp = stderr; /* portability  */
	else if( GL_smemberi( val, "stdout" )) PLS.diagfp = stdout; /* portability  */
	else	{
                PLS.diagfp = fopen( val, "w" ); /* diagnostics */
                if( PLS.diagfp == NULL ) {
                        fprintf( PLS.errfp, "warning, cannot open -diagfile %s.. using stderr\n", val );
                        PLS.diagfp = stderr;
                        }
                }
	}

else if( strcmp( opt, "errfile" )==0 ) {
	if( PLS.cgiargs != NULL ) goto FINISH; /* don't allow this in cgi mode.. scg 2/8/02 */
	if( GL_smember( val, "stderr STDERR" )) { PLS.errfp = stderr; TDH_errmode( "stderr" ); } 
	else if( GL_smember( val, "stdout STDOUT" )) { PLS.errfp = stdout; TDH_errmode( "cgi" ); } 
	else	{
                PLS.errfp = fopen( val, "w" ); /* diagnostics */
                if( PLS.errfp == NULL ) {
                        fprintf( stderr, "warning, cannot open -errfile %s, using stderr\n", val );
                        PLS.errfp = stderr;
                        }
		else TDH_errfile( PLS.errfp ); /* set it for TDH */
                }
	}

else if( strcmp( opt, "echo" )==0 ) {
	if( strcmp( val, "stdout" )==0 ) PLS.echolines = 1;
	else if( strcmp( val, "diag" )==0 ) PLS.echolines = 2;
	else { PLS.echolines = 2; *valused = 0; }
	}

#ifndef WIN32
else if( strcmp( opt, "cpulimit" )==0 ) TDH_reslimits( "cpu", atoi( val ) );
#endif


else if( strcmp( opt, "ping" )==0 ) {
	printf( "ploticus %s \n", PLVERSION );
	exit(0);
	}
 

else 	{ *found = 0; *valused = 0; }

FINISH:
if( PLS.debug ) {
	if( *valused ) fprintf( PLS.diagfp, " %s\n", val );
	else fprintf( PLS.diagfp, "\n" );
	}

return( rtnstat );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
