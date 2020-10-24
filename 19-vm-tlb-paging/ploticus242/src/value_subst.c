/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* VALUE_SUBST.C - perform variable evaluation for one line */

/* Variables may be: 
		Field numbers like @1, @2, @3, etc. (@1 is first field)
		TDH_data item names like @age

	Returns 1 if one or more substitutions were done, 0 if there were no 
	substitutions done, or an error code > 0.

	There are also options for retrieving involved fields and translating
	field names to field numbers (possible uses in data cleaning / logical checking).

	7/12/00 scg - unrecognized @items are now left as is, with no error being 
			generated.
*/

#include "tdhkit.h"
#include <ctype.h>
extern int atoi();

static int involved[ MAXITEMS ];
static int ninvolved = 0;
static int translate_to_fn = 0;
static int allowinlinecodes = 0;
static int hideund = 0;
static int suppressdll = 0;
static char varsym = '@';
static int omitws = 0;
static int shieldquotedvars = 0;
static int showwithquotes = 0;
static char punctlist[10] = "_";
static int sqlmode = 0;  /* if 1, TDH_dequote will be more strict about quote errors.. */
static int omit_shell_meta = 0;
static int globqs = 0; /* if 1, TDH_dequote will only reset the var serial# when told (by a call to settings()) */
static int curvar = 1; /* the var serial# */

/* =============================== */
int
TDH_valuesubst_initstatic()
{
ninvolved = 0;
translate_to_fn = 0;
allowinlinecodes = 0;
hideund = 0;
suppressdll = 0;
varsym = '@';
omitws = 0;
shieldquotedvars = 0;
showwithquotes = 0;
strcpy( punctlist, "_" );
sqlmode = 0;
omit_shell_meta = 0;
globqs = 0;
return( 0 );
}

/* =============================== */
int
TDH_value_subst( out, in, data, recordid, mode, erronbadvar )
char *out; /* result buffer */
char *in;  /* input buffer */
char data[ MAXITEMS ][ DATAMAXLEN+1 ];	/* data array */
char *recordid; /* recordid or "" */
int mode;  /* either FOR_CONDEX, indicating that the line will be passed to condex(),
		   (minor hooks related to this) or NORMAL */
int erronbadvar;
{
int i, j, k;
char itemname[ 512 ]; /* big because arbitrary tokens are being stored in it */
char value[ DATAMAXLEN+1 ];
int found, stat, infunction, inlen, inamelen, vlen;
int lineconvatt, lineconvdone, lastgoodoutpos;
int dectab;
char fillchar[10];
char varsymstring[4];
int instring;
char oldvarsym;
int esc;


found = 0;
ninvolved = 0;
infunction = 0;
lineconvatt = lineconvdone = 0;
lastgoodoutpos = 0;
dectab = 0;
strcpy( fillchar, " " );
sprintf( varsymstring, "%c", varsym );
instring = 0;
esc = 0;

strcpy( out, "" );
j = 0; /* j will track current length of out */


for( i = 0, inlen = strlen( in ); i < inlen; i++ ) {


	/* if shielding quoted vars, handle unescaped quote (") */
	if( instring && i > 0 && in[ i-1 ] == '\\' && !esc ) esc = 1;
	else esc = 0;
	if( shieldquotedvars && in[i] == '"' && !esc ) {
		if( !instring ) {
			instring = 1;
			oldvarsym = varsym;
			varsym = '\0';
			}
		else if( instring ) {
			instring = 0;
			varsym = oldvarsym;
			}
		}

	/* handle @@ (escape of @) */
	if( in[i] == varsym && in[i+1] == varsym ) {
		strcpy( &out[j], varsymstring ); 
		j += 1;
		i++;
		continue;
		}
		

	if( in[i] == varsym && !isspace( (int) in[i+1] ) ) {  /* @fieldname or @fieldnum .. fill 'value'.. */

		sscanf( &in[i+1], "%s", itemname );

		/* truncate itemname at first char which is not alphanumeric or _ (or . as of 4/24/01) */
		inamelen = strlen( itemname );
		for( k = 0; k < inamelen; k++ ) {
			if( !isalnum( (int) itemname[k] ) && !GL_member( itemname[k], punctlist )) { 
				itemname[k] = '\0';
				break;
				}
			}
		inamelen = strlen( itemname );

		lineconvatt++;

		if( strcmp( itemname, "_RECORDID" )==0 ) strcpy( value, recordid ); /* 02/19/01 */

		/* @n, @itemname, @varname.. */
		else	{
			stat = TDH_getvalue( value, itemname, data, recordid );
			if( stat != 0 ) {
				if( erronbadvar ) return( err( 1402, "unrecognized symbol", itemname ) );
				else sprintf( value, "%c%s", varsym, itemname );  /* replace @token on stream.. */
				}

#ifndef TDH_NOREC
			else 	{
				int ifld;
				/* do the following to add item to list of involved fields */
				ifld = TDH_fieldmap( recordid, itemname );
				if( ifld >= 0 ) involved[ ninvolved++ ] = ifld; 

				if( translate_to_fn ) sprintf( value, "%c%d", varsym, ifld+1 ); /* supercedes
								effect of the previous lines*/
				found = 1;
				if( value[0] != '\0' ) lineconvdone++;
				}
#endif
			}

		/* special case of 0 length data item when in a condex expression but 
		   not within a function arg list.. to prevent condex syntax errors  */
		if( strcmp( value, "" )==0 && mode == FOR_CONDEX && !infunction )
			strcpy( value, "_null_" ); 


		if( dectab ) {  /* decimal tab / right-align tab */
			int dpos;
			dpos = GL_member( TDH_decpt, value );
			if( dpos == 0 ) dpos = strlen( value );
			else dpos--;
			for( k = (j-lastgoodoutpos); k < ((lastgoodoutpos + dectab)-dpos); k++ ) {
				strcpy( &out[j], fillchar );
				j+=strlen( fillchar );
				}
			dectab = 0;
			}

		/* append value to outbuf.. */
		vlen = strlen( value );
		if( hideund ) for( k = 0; k < vlen; k++ ) if( value[k] == '_' ) value[k] = ' ';		/* datadelim - ok */
		if( mode == FOR_CONDEX ) for( k = 0; k < vlen; k++ ) if( value[k] == ' ' ) value[k] = '_'; /* datadelim - ok */
		if( showwithquotes ) {
			sprintf( &out[j], "\"%s\"", value );
			j += (vlen+2);
			}
		else if( omit_shell_meta ) {
			for( k = 0; k < vlen; k++ ) {
				if( strncmp( &value[k], "../", 3 )==0 ) { k+=2; continue; }  /* added scg 5/19/05 */
				else if( GL_member( value[k], TDH_shellmetachars )) continue;
				else out[j++] = value[k];
				}
			out[j] = '\0';
			}
		else 	{
			strcpy( &out[j], value ); 
			j += vlen;
			}
		i += inamelen; /* advance past @itemname */
		}

		
	else	{
		if( in[i] == '$' && isalpha( (int) in[i+1] ) ) infunction = 1;
		if( isspace( (int) in[i] ) ) infunction = 0;  /* ???? 3/22/01 */

		if( allowinlinecodes && in[i] == ':' ) {
			if( strncmp( &in[i], ":u+ ", 4 )==0 ) { hideund = 0; i += 3; continue; }
			else if( strncmp( &in[i], ":u- ", 4 )==0 ) { hideund = 1; i += 3; continue; }
			else if( strcmp( &in[i], ":c\n" ) == 0 ) break;
			else if( strncmp( &in[i], ":col", 4 )== 0 && isdigit( (int) in[i+4] )) {  /* col pos */
				int colnum;
				colnum = atoi( &in[ i+4 ] );
				for( k = (j-lastgoodoutpos); k < ((lastgoodoutpos+colnum)-2); k++ ) out[j++] = ' ';
				if(colnum < 10)i+=5; else if(colnum < 100)i+=6; else if(colnum < 1000)i+=7;
				}
			else if( strncmp( &in[i], ":dot", 4 )== 0 && isdigit( (int) in[i+4] )) { /* leader dots to pos */
				int colnum;
				colnum = atoi( &in[ i+4 ] );
				for( k = (j-lastgoodoutpos); k < ((lastgoodoutpos+colnum)-2); k++ ) out[j++] = '.';
				if(colnum < 10)i+=5; else if(colnum < 100)i+=6; else if(colnum < 1000)i+=7;
				}
			else if( strncmp( &in[i], ":dec", 4 )== 0 && isdigit( (int) in[i+4] )) { /* decimal tab at pos */
				dectab = atoi( &in[ i+4 ] ); strcpy( fillchar, " " );
				if(dectab < 10)i+=5; else if(dectab < 100)i+=6; else if(dectab < 1000)i+=7;
				}
			else if( strncmp( &in[i], ":dch", 4 )== 0 && isdigit( (int) in[i+4] )) { /* decimal tab at pos */
				dectab = atoi( &in[ i+4 ] ); strcpy( fillchar, "&nbsp;" );
				if(dectab < 10)i+=5; else if(dectab < 100)i+=6; else if(dectab < 1000)i+=7;
				}
			}
		if( in[i] == '\n' || i == inlen-1 ) {  /* 2nd clause added 3/22/01 */
			if( suppressdll && lineconvatt > 0 && lineconvdone == 0 ) { /* suppress line */
				j = lastgoodoutpos;
				lineconvatt = lineconvdone = 0;
				continue;
				}
			lineconvatt = lineconvdone = 0;
			lastgoodoutpos = j+1; 
			}

		if( omitws && isspace( (int) in[i] ) ) continue;

		out[j] = in[i];
		j++;
		}
	}

/* when suppressdll, for case of @varname immediately followed by eol.. */
if( suppressdll && lineconvatt > 0 && lineconvdone == 0 ) j = lastgoodoutpos;

out[j] = '\0';
return( found );
}

/* ========================= */
/* GET_INVOLVED_ITEMS  - allow an application to access the field #s of all 
	data items involved in the most recent value_subst call.  Tmp vars
	are not included since they have no field # */
int
TDH_get_involved_items( n, list )
int *n;
int list[ MAXITEMS ];
{
int i;
*n = ninvolved;
for( i = 0; i < ninvolved; i++ ) list[i] = involved[i];
return( 0 );
}

/* ========================= */
int
TDH_valuesubst_settings( tag, value )
char *tag;
int value; /* 1 = on, 0 = off */
{
if( strcmp( tag, "hideund" )==0 ) hideund = value;
else if( strcmp( tag, "allowinlinecodes" )==0 ) allowinlinecodes = value;
else if( strcmp( tag, "suppressdll" )==0 ) suppressdll = value;
else if( strcmp( tag, "translate_to_fn" )==0 ) translate_to_fn = value;
else if( strcmp( tag, "varsym" )==0 ) varsym = (char)value;
else if( strcmp( tag, "omitws" )==0 ) omitws = value;
else if( strcmp( tag, "shieldquotedvars" )==0 ) shieldquotedvars = value;
else if( strcmp( tag, "showwithquotes" )==0 ) showwithquotes = value;
else if( strcmp( tag, "sqlmode" )==0 ) sqlmode = value;
else if( strcmp( tag, "omit_shell_meta" )==0 ) omit_shell_meta = value;
else if( strcmp( tag, "globqs" )==0 ) { 
	if( value == 2 ) { curvar = 1; return( 0 ); }  /* reset the var serial #   scg 10/28/04 */
	else globqs = value;
	}
else if( strcmp( tag, "dot_in_varnames" )==0 ) {
	if( value ) strcpy( punctlist, "_." );
	else strcpy( punctlist, "_" );
	}
return( 0 );
}

/* ===================================== */
/* DEQUOTE - scan a line and convert quoted "strings" */

int
TDH_dequote( out, in, prefix )
char *out;
char *in;
char *prefix;  /* keeps string constants from colliding: sql uses QS; sinterp uses SL */
{

int i, j, k, len, instring, esc;
char tok[DATAMAXLEN+1];
char vartag[20];
char quotecharused; /* added 3/29/01 */
int truncflag;

if( !globqs ) curvar = 1;  /* normally the var serial# is reset for every call (every line) 
			      but w/ globqs it isn't - globqs added scg 10/28/04 */

truncflag = 0;

len = strlen( in );

sprintf( vartag, "_%s%02d", prefix, curvar ); /* limitation (99 _QS vars) */

instring = 0;
for( i = 0, j = 0; i < len; i++ ) {

	if( instring && i > 0 && in[ i-1 ] == '\\' && !esc ) {
		esc = 1;
		k--; /* rm the backslash from tok */
		}
	else esc = 0;

	/* non-quoted comment symbol encountered.. stop */
	if( !instring && in[i] == '/' && in[i+1] == '/' ) break;

	/* unescaped quote encountered */
	if( ( in[i] == '"' || in[i] == '\'' ) && !esc ) {
		if( instring && in[i] == quotecharused ) {
			if( sqlmode && in[i+1] != '\0' && !isspace( (int) in[i+1] ) && !GL_member( in[i+1], ",)" ) )
				return( err( 2734, "quote error", "" ));
			tok[k] = '\0';
			/* out[j++] = '@'; */ /* changed scg 10/28/04 */
			out[j++] = varsym;
			strcpy( &out[j], vartag );	
			j+= strlen( vartag );
			/* if( mode == FOR_CONDEX && strlen( tok ) == 0 ) strcpy( tok, "_null_" ); */
			TDH_setvar( vartag, tok );

			curvar++;
			sprintf( vartag, "_%s%02d", prefix, curvar );

			instring = 0;
			}
		else if( instring ) {	 /* other quote - treat as normal char.. */
			if( k >= DATAMAXLEN-1 ) { tok[DATAMAXLEN-1] = '\0'; truncflag = 1; }
			tok[k++] = in[i];  
			}
		else 	{
			quotecharused = in[i];
			instring = 1;
			k = 0;
			}
		continue;
		}
	if( instring ) {
		if( k >= DATAMAXLEN-1 ) { tok[DATAMAXLEN-1] = '\0'; truncflag = 1; }
		tok[k++] = in[i];
		}
	else out[j++] = in[i];
	}

if( instring ) { /* no ending quote.. */
	if( sqlmode ) return( err( 2735, "mismatched quotes", "" ));
	tok[k] = '\0';
	/* out[j++] = '@'; */ /* changed scg 10/28/04 */
	out[j++] = varsym;
	strcpy( &out[j], vartag );	
	j+= strlen( vartag );
	TDH_setvar( vartag, tok );
	}

/* back up j to last non-white-space */
j--;
while( j >= 0 && isspace( (int) out[j] ) ) j-- ;
out[j+1] = '\0';
if( truncflag ) err( 1409, "value is too long and has been truncated", out );
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
