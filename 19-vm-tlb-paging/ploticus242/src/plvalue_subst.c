/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "tdhkit.h"
#include <ctype.h>
extern int PL_fref(), PL_fref_error();
extern int GL_urlencode();

extern int atoi();

/* PL_VALUE_SUBST - take a text line and substitute values for variables.  
 * This is similar to TDH_value_subst, but is stripped down and  accepts a data array that 
 * is an array of pointers rather than a 2d array.  It also knows about ploticus field names (PL_fref() )
 */

/* scg 11/5/07 - now supporting data = NULL for situations where data array is n/a.
 *	Also, in the case of bad variable substitutions, the @varname is inserted into the output 
 *	(no error handling had apparently been done before).
 */


int
PL_value_subst( out, in, data, mode )
char *out; /* result buffer */
const char *in;  /* input buffer */
char *data[ MAXITEMS ];  /* can be passed as NULL if n/a .. scg 11/6/07 */
int mode;  /* either FOR_CONDEX (1), indicating that the line will be passed to condex() (minor hooks);
		   URL_ENCODED (2), indicating that values substituted in should be urlencoded;
		    or NORMAL (0) */
{
int i, k;
char itemname[512];
char value[512];
int found;
int infunction;
int ifld;
int inlen;
int outlen;
int vallen;
int inamelen;
char tmpvalue[256];
int stat;

found = 0;

infunction = 0;
inlen = strlen( in );
outlen = 0;
strcpy( out, "" );
for( i = 0; i < inlen; i++ ) {

	/* handle @@ (escape of @) */
	if( in[i] == '@' && in[i+1] == '@' ) {
		strcat( out, "@" );
		i ++;
		continue;
		}
		
	/* @item or @1 */
	if( in[i] == '@' ) {

		sscanf( &in[i+1], "%s", itemname );
		found = 1;

		/* truncate itemname at first char which is not alphanumeric or _ */
		inamelen = strlen( itemname );
		for( k = 0; k < inamelen; k++ ) {
			if( !isalnum( (int) itemname[k] ) && itemname[k] != '_' ) { 
				itemname[k] = '\0';
				break;
				}
			}
		inamelen = strlen( itemname );

		/* @1, @2, etc... */
		ifld = atoi( itemname );
		if( ifld > 0 && ifld < MAXITEMS ) {
			if( data == NULL ) strcpy( value, itemname ); /* scg 11/5/07 */
			else if( mode == URL_ENCODED ) GL_urlencode( data[ ifld-1 ], value );
			else strcpy( value, data[ ifld-1 ] );
			}

		/* @fieldname .. */
		else	{
			ifld = PL_fref( itemname );
			if( ! PL_fref_error() && data != NULL ) { 
				if( mode == URL_ENCODED ) GL_urlencode( data[ ifld-1 ], value );
				else strcpy( value, data[ ifld -1 ] );
				}
			else	{  /* try a tdh varname .. added scg 11/5/07  */
		   	 	stat = TDH_getvar( itemname, tmpvalue );
				if( stat ) strcpy( value, itemname ); /* scg 11/5/07 */
			 	else if( mode == URL_ENCODED ) GL_urlencode( tmpvalue, value );
			 	else strcpy( value, tmpvalue );
			 	}
			}


		/* special case of 0 length data item when in a condex expression but 
		   not within a function arg list.. to prevent condex syntax errors  */
		if( strcmp( value, "" )==0 && mode == FOR_CONDEX && !infunction )
			strcpy( value, "_null_" ); 

		vallen = strlen( value );
		for( k = 0; k < vallen; k++ ) {
			if( value[k] == ' ' && mode == FOR_CONDEX ) out[ outlen + k ] = '_';
			else out[ outlen + k ] = value[k];
			}
		out[ outlen + k ] = '\0';

		outlen += vallen;

		i+= inamelen; /* advance past @itemname */
		}

	else	{
		if( in[i] == '$' && isalpha( (int) in[i+1] ) ) infunction = 1;
		if( isspace( (int) in[i] ) ) infunction = 0;
		/* len = strlen( out ); */
		out[ outlen ] = in[i];
		out[ outlen +1 ] = '\0';
		outlen++;
		}
	}
return( found );
}

/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
