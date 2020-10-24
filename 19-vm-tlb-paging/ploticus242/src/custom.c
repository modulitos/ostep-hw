/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"

extern int sleep();
#ifndef NOGD
extern int PLGG_freetype_twidth();
#endif

/* ----------------------------- */
/* CUSTOM_FUNCTION - returns 0 on success, 1 if named function not found */

/* Note: custom_function( name, arg, nargs, result, typ ) is not referenced in ploticus
 * due to an #ifdef in functions.c
 *
 * char *name;
 * char *arg[];
 * int nargs;
 * char *result;
 * int *typ;
 * {
 * int stat;
 * 
 * stat = custom_pl_functions( name, arg, nargs, result, typ );
 * return( stat );
 * }
 */

int
PL_custom_function( name, arg, nargs, result, typ )
char *name;
char *arg[];
int nargs;
char *result;  /* result data is copied into this var */
int *typ;
{

double PLG_conv(), PLG_limit();
char buf[100];
char axis;

*typ = 0;
strcpy( result, "0" );

/* ------------------- */
/* hash:615 $inrange(value,axis) or $inrange(value,axis,min,max)
   return 1 if value in range for the axis, 0 if not.
   min and max are optional args */
if( strcmp( name, "inrange" )==0 ) {
	double fval, flow, fhi;
	axis = tolower( arg[1][0] );
	fval = Econv( axis, arg[0] );
	if( nargs > 2 ) flow = Econv( axis, arg[2] );
	else flow = Elimit( axis, 'l', 's' );
	if( nargs > 3 ) fhi = Econv( axis, arg[3] );
	else fhi = Elimit( axis, 'h', 's' );
	if( fval >= flow && fval <= fhi ) sprintf( result, "1" );
	else sprintf( result, "0" );	
	return( 0 );
	}
/* ------------------- */
/* hash:564 $icolor( i ) - return one of 20 color entries, chosen for contrast */
else if( strcmp( name, "icolor" )==0 ) {
	int i;
	char *colorname;
	i = atoi( arg[0] );
	if( i < 1 ) i = 1;
	i--;
	colorname = Eicolor( i );
	strcpy( result, colorname );
	*typ = 1;
	return( 0 );
	}
/* ------------------- */
/* hash:849 $dataitem(row,field) - return the contents of row,field of current data set.
   Row is a number (first = 1).  Field is a datafield (name or number 1..n) */
else if( strcmp( name, "dataitem" )==0 ) {
	int row, fld;
	row = atoi( arg[0] ) -1;
	fld = fref( arg[1] ) -1;
	sprintf( result, "%s ", da( row, fld ) );
	*typ = 1;
	return( 0 );
	}


/* -------------------- */
/* hash: 1306  $defaultinc(min,max) - get a default increment */
else if( strcmp( name, "defaultinc" )==0 ) {
	double inc;
	PL_defaultinc( atof( arg[0] ), atof( arg[1] ), &inc );
	sprintf( result, "%g", inc );
	return( 0 );
	}

/* ------------------- */
/* hash: 1662  $boundingbox( on/off ) */
else if( strcmp( name, "boundingbox" )==0 ) {
	PLG_pcodeboundingbox( atoi( arg[0] ));
	sprintf( result, "0" );
	return( 0 );
	}

/* -------------------- */
/* hash: 2959  $squelch_display(mode) */
else if( strcmp( name, "squelch_display" )==0 ) {
	if( atoi( arg[0] ) == 1 ) Esquelch( "on" );
	else Esquelch( "off" );
	strcpy( result, "" );
	return( 0 );
	}

/* ------------------- */
/* hash:987 $nextstub(maxlen) - return next pltab row stub, "" if no more */
else if( strcmp( name, "nextstub" )==0 ) {
        int maxlen, i, slen;
        maxlen = atoi( arg[0] );
        if( maxlen == 0 ) maxlen = 80;

        /* see which axis is using categories or pltab scaling.. */
        Egetunits( 'y', buf );
        strcpy( result, "" );
        if( strcmp( buf, "categories" )==0 ) PL_nextcat( 'y', result, maxlen );
        else    {
                Egetunits( 'x', buf );
                if( strcmp( buf, "categories" )==0 ) PL_nextcat( 'x', result, maxlen );
                else Eerr( 2709, "$nextstub only valid with pltab or categories", "" );
                }
        /* convert spaces to underscores */
        for( i = 0, slen = strlen( result ); i < slen; i++ ) if( result[i] == ' ' ) result[i] = '_';
        *typ = 1;
        return( 0 );
        }


/* -------------------- */
/* hash:206 $max(f1,f2..fn) */
else if( strcmp( name, "max" )==0 ) {
	int i, prec;
	double max;
        max = -999999999999999.0;
	for( i = 0; i < nargs; i++ ) {
                if( !GL_goodnum( arg[i], &prec ) ) continue; /* added scg 11/15/00 */
                if( atof( arg[i] ) > max ) max = atof( arg[i] );
                }
        sprintf( result, "%g", max );
        return( 0 );
	}

/* ------------ */
/* hash:3396 $data_to_absolute(axis,val) - given a data coordinate, return the absolute location.
   Example: #set AVAL = $data_to_absolute(Y,@DVAL)
 */
else if( strcmp( name, "data_to_absolute" )==0 ) {
	double f, PLG_conv(), PLG_a();
	f = Econv( tolower(arg[0][0]), arg[1] );
	sprintf( result, "%g", Ea( tolower(arg[0][0]), f ) );
	return( 0 );
	}


/* ------------------- */
/* hash:376 sleep( n ) - delay for N seconds */
else if( strcmp( name, "sleep" )==0 ) {
	int n;
	Eflush();
	n = atoi( arg[0] );
	sleep( n );
	return( 0 );
	}

/* ------------------- */
/* hash:793 getclick( label ) - produce a button; when user clicks on it, return */
else if( strcmp( name, "getclick" )==0 ) {
	if( Edev == 'x' ) PL_do_x_button( "More.." );
	return( 0 );
	}



/* ------------------- */
/* hash:956 $fieldname(n) - return the field name assigned to field n.  First is 1 */
else if( strcmp( name, "fieldname" )==0 ) {
	getfname( atoi( arg[0] ), result );  /* result[n] as passed */
        *typ = 1;
	return( 0 );
	}


/* ------------- */
/* hash:615 $exists(val) - return 1 if val has length > 0 */
else if( strcmp( name, "exists" )==0 ) {
	if( arg[0][0] != '\0' ) strcpy( result, "1" );
	else strcpy( result, "0" );
	return( 0 );
	}

/* ------------- */
/* hash:1303 $notexists(val) - return 1 if val has length > 0 */
else if( strcmp( name, "notexists" )==0 ) {
	if( arg[0][0] != '\0' ) strcpy( result, "0" );
	else strcpy( result, "1" );
	return( 0 );
	}

/* ------------- */
/* hash:1143 $errmsgpre(txt) - set the error message beginning - scg 3/25/04 */
else if( strcmp( name, "errmsgpre" )==0 ) {
	TDH_errprogsticky( arg[0] );
	strcpy( result, "1" );
	return( 0 );
	}

/* ------------- */
/* hash:1170 $textwidth(text,font,size) */
/* return horizontal width of freetype bounding box */
/* this returns useful result only with freetype fonts, otherwise it returns 0. added 8/5/05 scg - sugg by Erik Zachte  */
else if( strcmp( name, "textwidth" )==0 ) {
	double twidth;
	twidth = 0;
#ifndef NOGD
	PLGG_freetype_twidth( arg[0], arg[1], atof(arg[2]), &twidth );
#endif
	sprintf( result, "%.4f", twidth );
	return( 0 );
	}

/* ------------- */
/* hash:1488 $rewritenum(f) - return a displayable representation of number f,
   applying  proc settings numbernotation */
else if( strcmp( name, "rewritenum" )==0 ) {
	strcpy( result, arg[0] );
	PL_rewritenums( result );
	return( 0 );
	}

/* -------------- */
/* hash: 1750 $changeunits( axis, newunitspec ) */
/* change the units associated with an existing axis (used to be done using proc defineunits) */
else if( strcmp( name, "changeunits" )==0 ) {
	Esetunits( arg[0][0], arg[1] );
	return( 0 );
	}

return( 1 ); 
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
