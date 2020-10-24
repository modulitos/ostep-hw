/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC SETTINGS - date, unit, notation settings */

#include "pl.h"
extern int PLGP_settings(), PLGS_setxmlparms(), PL_clickmap_urlenc1();
extern int setuid(), setgid();

int
PLP_settings()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first, stat;


TDH_errprog( "pl proc settings" );

/* get attributes.. */
first = 1;
while( 1 ) {
	line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	/* settings that aren't shared with config file go here: */

	/* do this here since "format" is an old attr name still supported.. */
	if( strcmp( attr, "format" )==0 || strcmp( attr, "dateformat" )==0 ) DT_setdatefmt( lineval );

	else if( GL_slmember( attr, "pivotyear months* weekdays omitweekends lazydates" )) {
                stat = DT_setdateparms( attr, lineval );
		}

	/* the rest are shared settings.. */
	else 	{
		stat = PL_sharedsettings( attr, lineval );
		if( stat ) Eerr( 1, "attribute not recognized", attr );
		}
	}

return( 0 );
}


/* ========================================= */
/* common code for setting attributes from proc settings or config file */
int
PL_sharedsettings( attr, lineval )
char *attr, *lineval;
{
char val[80];

tokncpy( val, lineval, 80 );

if( strcmp( attr, "units" )==0 ) {
	if(  val[0] == 'c' ) { PLS.usingcm = 1; setintvar( "CM_UNITS", 1 ); }
	else { PLS.usingcm = 0; setintvar( "CM_UNITS", 0 ); }
	}

#ifndef WIN32
else if( strcmp( attr, "uid" )==0 ) setuid( atoi( val ) );
else if( strcmp( attr, "gid" )==0 ) setgid( atoi( val ) );
else if( strcmp( attr, "cpulimit" )==0 ) TDH_reslimits( "cpu", atoi( val ) );
#endif

else if( strcmp( attr, "numbernotation" )==0 ) {
	if( strcmp( val, "us" )==0 ) PLS.bignumspacer = ',';
	else if( strcmp( val, "euro" )==0 ) PLS.bignumspacer = '.';
	else PLS.bignumspacer = '\0';
	}
else if( strcmp( attr, "numberspacerthreshold" )==0 ) PLS.bignumthres = atoi( val ); /* scg 2/28/02 */

else if( strcmp( attr, "font" )==0 ) strcpy( Estandard_font, lineval ); 

else if( strcmp( attr, "encodenames" )==0 ) {					      /* added scg 8/4/04 */
        if( strncmp( val, "y", 1 )==0 ) PL_encode_fnames( 1 ); 
        else PL_encode_fnames( 0 );
	}

#ifndef NOPS
else if( strcmp( attr, "ps_latin1_encoding" )==0 ) {
        if( strncmp( val, "y", 1 )==0 ) PLGP_settings( "ps_latin1_encoding", "1" ); /* added 7/28/04 */
        else PLGP_settings( "ps_latin1_encoding", "0" ); /* added 7/28/04 */
	}
#endif

#ifndef NOSVG
else if( strcmp( attr, "xml_encoding" )==0 ) PLGS_setxmlparms( "encoding", val );
else if( strcmp( attr, "xml_declaration" )==0 ) {
	if( strncmp( val, "y", 1 )==0 ) PLGS_setxmlparms( "xmldecl", "1" );
	else PLGS_setxmlparms( "xmldecl", "0" );
	}
else if( strcmp( attr, "svg_tagparms" )==0 ) PLGS_setxmlparms( "svgparms", lineval );
else if( strcmp( attr, "svg_linkparms" )==0 ) PLGS_setxmlparms( "linkparms", lineval );
else if( strcmp( attr, "svg_mouseover_js" )==0 ) PLGS_setxmlparms( "mouseover_js", val );
#endif

else if( strcmp( attr, "dtsep" )==0 ) DT_setdtsep( val[0] );
else if( strcmp( attr, "errmsgpre" )==0 ) TDH_errprogsticky( lineval ); /* added 3/25/04 scg */
else if( strcmp( attr, "enable_suscripts" )==0 ) {
	if( strncmp( val, "y", 1 )==0 ) PLG_textsupmode( 1 );
	else PLG_textsupmode( 0 );
	}
#ifdef HOLD
else if( strcmp( attr, "sanezone" )==0 ) {
	double sanex, saney;
	sscanf( lineval, "%lf %lf", &sanex, &saney );
	PLG_sanezone( 0.0, 0.0, sanex, saney );
	}
#endif

else return( 1 ); /* not found */

return( 0 ); /* ok */
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
