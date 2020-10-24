/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC CATEGORIES - define or set attribute(s) for a set of categories */

#include "pl.h"

int
PLP_categories( in_areadef )
int in_areadef;
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char buf[256];
char *catspec, *selectex, *listsize, *compmethod;
char *ecat[100], ecattype[100];
int i, nextra;
char axis;
double slideamount;
int parm, checkuniq, roundrobin, datafield_given;

TDH_errprog( "pl proc categories" );

slideamount = -99999.9;
axis = '\0';
selectex = ""; catspec = ""; listsize = ""; compmethod = "";
roundrobin = datafield_given = 0;
checkuniq = 1;
nextra = 0;

/* note: old AREADEF and CATSLIDE category-related attributes supported here as 'overlays'..
 *	hence, beware of attribute name collision with those procs */

/*  get attributes.. */
first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
	if( line == NULL ) break;
	first = 0;
	lineval = &line[lvp];

	if( strcmp( attr, "axis" )==0 ) axis = lineval[0]; 

	else if( strcmp( attr, "listsize" )==0 ) listsize = lineval;
	else if( GL_slmember( attr, "?categories categories" )) {
		if( strncmp( lineval, "datafield", 9 )==0 ) catspec = lineval;
		else catspec = getmultiline( lineval, "get" ); 
		if( attr[0] == 'x' || attr[0] == 'y' ) axis = attr[0]; /* support old areadef syntax */
		}
	else if( strcmp( attr, "datafield" )==0 ) { catspec = lineval; datafield_given = 1; }

	else if( GL_slmember( attr, "?extracategory extracategory" )) {
		if( nextra+1 >= 100 ) Eerr( 57933, "too many extra categories", "" );
		else	{
			ecat[ nextra ] = lineval;
			if( catspec[0] == '\0' ) ecattype[ nextra ] = '<';
			else ecattype[ nextra ] = '>'; 
			nextra++;
			}
		}

	else if( GL_slmember( attr, "comparemethod catcompmethod" )) compmethod = lineval;
	else if( strcmp( attr, "slideamount" )==0 || strcmp( attr, "amount" )==0 ) slideamount = ftokncpy( lineval );
	else if( strcmp( attr, "select" )==0 ) selectex = lineval;
	else if( strcmp( attr, "checkuniq" )==0 ) checkuniq = getyn( lineval );
	else if( strcmp( attr, "roundrobin" )==0 ) roundrobin = getyn( lineval ); 
	else if( !in_areadef ) Eerr( 1, "attribute not recognized", attr ); 
	/* can't do this in_areadef because there are other attributes not enumerated here.. */
	}

/* now do the work.. */

if( axis == '\0' ) return( Eerr( 2795, "'axis' attribute expected", "" ));


/* things to be done before filling cats list.. */
if( listsize[0] != '\0' ) PL_setcatparms( axis, "listsize", itokncpy( listsize ) );
if( compmethod[0] == '\0' ) PL_setcatparms( axis, "compmethod", -1 ); 
else	{
	if( strcmp( compmethod, "exact" )==0 ) parm = -1;
	else if( strncmp( compmethod, "length=", 7 )==0 ) parm = atoi( &compmethod[7] );
	else parm = 0;
	if( in_areadef ) { PL_setcatparms( 'x', "compmethod", parm ); PL_setcatparms( 'y', "compmethod", parm ); } /* old synax does both axes*/
	else PL_setcatparms( axis, "compmethod", parm );
	}
for( i = 0; i < nextra; i++ ) if( ecattype[i] == '<' ) PL_addcat( axis, "pre", ecat[i] );  /* pre- extras */

/* fill cats list.. */
if( datafield_given ) {
	sprintf( buf, "datafield=%s %s%s", catspec, (selectex[0]=='\0')?"":"selectrows=", selectex );  /* combine with selectex */
	catspec = buf;  /* make catspec point to the whole thing.. */
	}
if( catspec[0] != '\0' ) PL_setcats( axis, catspec );

/* things to be done after filling list.. */
for( i = 0; i < nextra; i++ ) if( ecattype[i] == '>' ) PL_addcat( axis, "post", ecat[i] );  /* post- extras */

/* things that apply to lookups.. */
PL_setcatparms( axis, "checkuniq", checkuniq );
PL_setcatparms( axis, "roundrobin", roundrobin );
if( slideamount > -100.0 && slideamount < 100.0 ) Esetcatslide( axis, slideamount );

return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
