/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */


#include <stdio.h> 
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

extern int GL_member(), GL_encode(), GL_wildcmp();
extern int getpid();

#define stricmp( s, t ) 	strcasecmp( s, t )
#define strnicmp( s, t, n )     strncasecmp( s, t, n )

#define DATAMAXLEN 256

static int getdecplaces(), wcmp();

static char Sep = ',';  /* separator character for lists */
static char Gettok_buf[260];
static char Wildcard = '*';
static char Wildcard1 = '?';
static int Maxlen = 99999;
static char Member_nullstring[10] = "";

/* ================================= */
int
GL_initstatic()
{
Sep = ',';
Wildcard = '*';
Wildcard1 = '?';
Maxlen = 99999;
strcpy( Member_nullstring, "" );
return( 0 );
}

/* ================================= */
/* thanks to Markus Hoenicka for this more portable sysdate and systime code */
/* SYSDATE - get today's date */ 

int
GL_sysdate( mon, day, yr )
int	*mon, *day, *yr ;
{
time_t clock;
struct tm *ltime;
time(&clock);
ltime = localtime(&clock);
*mon = ltime->tm_mon + 1;
*day = ltime->tm_mday;
*yr = ltime->tm_year;
if( (*yr) >= 100 ) (*yr) = (*yr) % 100;  /* scg y2k 11/10/98 */
return( 0 );
}

/* ================================= */
/* SYSTIME - get current time */ 
int
GL_systime( hour, min, sec )
int	*hour, *min, *sec ;
{
time_t clock;
struct tm *ltime;
time(&clock);
ltime = localtime(&clock);
*hour = ltime->tm_hour;
*min = ltime->tm_min;
*sec = ltime->tm_sec;
return( 0 );
}


/* ============================================= */
/* GETOK - get next whitespace-delimited token */
 
char 
*GL_getok( string, index )
char    *string;
int     *index;
{
int n;
while( GL_member( string[(*index)], " \t\n" ) ) (*index)++;
for( n=0;
        n <= 255 &&
        string[*index] != ' '  &&
        string[*index] != '\t'  &&
        string[*index] != '\n'  &&
        string[*index] != 13  &&       /* DOS LF added may 03 scg */
        string[*index] != '\0'  ;
                Gettok_buf[n++] = string[(*index)++] )  ;
Gettok_buf[n] = '\0' ;
return( Gettok_buf );
}

/* ====================================================================== */
/* MEMBER - returns char position if character c is a member of string s, 
		0 otherwise. Char positions start with 1 for this purpose.  */
int
GL_member( c, s )
char c, s[];
{
int i, len;
for( i = 0, len = strlen( s ); i < len; i++ ) if( s[i] == c ) return( i+1 );
return( 0 );
}

/* ===================================================================== */
/* SMEMBER - look for s in list t (white-space delimited).  Case sensitive.
   If found return 1 else 0. */

int
GL_smember( s, t )
char s[], t[];
{
char tok[DATAMAXLEN+1], *GL_getok();
int i;
i = 0;
while( 1 ) {
	strcpy( tok, GL_getok( t, &i ) );
	if( tok[0] == '\0' ) break;
	if( Member_nullstring[0] != '\0' ) {
		if( strcmp( s, Member_nullstring)== 0 && stricmp( tok, "null" )==0 )return( 1 );
		}
	if( strcmp( tok, s ) == 0 ) return( 1 );
	}
return( 0 );
}
/* ===================================================================== */
/* SMEMBERI - look for s in list t (white-space delimited).  Case insensitive.
   If found return 1 else 0. */

int
GL_smemberi( s, t )
char s[], t[];
{
char tok[DATAMAXLEN+1], *GL_getok();
int i;
i = 0;
while( 1 ) {
	strcpy( tok, GL_getok( t, &i ) );
	if( tok[0] == '\0' ) break;
	if( Member_nullstring[0] != '\0' ) {
		if( strcmp( s, Member_nullstring)== 0 && stricmp( tok, "null" )==0 )return( 1 );
		}
	if( stricmp( tok, s ) == 0 ) return( 1 );
	}
return( 0 );
}



/* ==================================================================== */
/* MEMBER_NULLMODE - set a special mode for the benefit of shsql, to consider
   "null" or "NULL" equivalent to some code such as "=" */

int
GL_member_nullmode( s )
char *s;
{
strcpy( Member_nullstring, s );
return( 0 );
}


/* ===================================================================== */
/* GOODNUM - checks a token to see if it is a legal number.  Returns 1 = yes  0 = no.
	'prec' is returned.. and is the precision of the number (position of decimal point).

	Number may contain unary + or -, and one decimal point.

	Leading and trailing whitespace are tolerated by this routine, eg. "  28.5 " is considered OK
*/

int
GL_goodnum( str, prec )
char *str;
int *prec;
{
int i, start, len, p, bad;

bad = 0; *prec = -1;
len = strlen( str );

/* find limit of trailing whitespace.. */
for( i = len-1; i >= 0; i-- ) if( !isspace( (int) str[i] ) ) break;
len = i+1;

/* skip over leading whitespace.. */
for( i = 0; i < len; i++ ) if( !isspace( (int) str[i] ) ) break;
start = i;

/* screen out degenerate cases.. */
if( len < 1 ) return( 0 ); /* degenerate case "" */
if( len-start == 1 && ( str[start] == '.' || str[start] == '+' || str[start] == '-' ) )
	return( 0 );       /* degenerate case; ".", "+", "-" */

/* check invididual characters.. */
for( p = start; p < len; p++ ) { 
	if( str[p] == '.' ) { 
		if( *prec == -1 ) *prec = p; 
		else bad=1; 
		}
	else if( p == start && ( str[p] == '-' || str[p] == '+' ) );
	else if( p > start && tolower(  (int) str[p]) == 'e' ) {  /* handle scientific notation ... scg 7/29/04 */
		p++;
		if( str[p] != '-' && str[p] != '+' ) bad=1;
		if( p+1 >= len ) bad=1;
		}
	else if( ! isdigit( (int) str[p]) ) bad=1;
	}

/* return.. */
if( bad ) return( 0 );
else return( 1 );
}

/* =========================================== */
/* GETSEG - Get fields, which are delimited by any member of sepstring.
   Similar to GL_getchunk(); however
   Whereas GL_getchunk() skips over adjacent separators,
   this routine delimits on EACH separator character encountered,
   Also, separator character is ignored if preceded in inbuf by a backslash(\). 

   Returns 1 when end-of-line is reached and no token is being returned.

   SCG 12-2-96
*/

int
GL_getseg( rtn, inbuf, i, sep )
char rtn[];
char inbuf[];
int *i;
char *sep;
{
int n;
int escaping;
int eol;

n = 0;
rtn[0] = '\0';
escaping = 0;
eol = 0;
while( 1 ){
	if( inbuf[*i] == '\0' ) { 
		if( n == 0 ) eol = 1; 
		break; 
		}
	else if( GL_member( inbuf[*i], sep ) && !escaping ) { (*i)++; break; }
	else if( inbuf[*i] == '\\' && GL_member( inbuf[(*i)+1], sep ) ) { 
		escaping = 1; 
		(*i)++; 
		continue; 
		}
	else rtn[n++] = inbuf[(*i)++];
	if( n >= 511 ) break; /* 512 max */
	escaping = 0;
	}
rtn[n] = '\0' ;
return( eol );
}

/* ==================================================================== */
/* GETCHUNK - Get tokens, which are separated by any member of sepstring */

int
GL_getchunk( rtn, line, i, sepstring )
char rtn[];
char line[];
int *i;
char sepstring[];
{

int n;

while( GL_member( line[(*i)], sepstring ) ) (*i)++; 
n = 0;
rtn[0] = '\0';
while( 1 ){
	if( GL_member( line[*i], sepstring ) || line[*i] == '\0' ) break;
	else rtn[n++] = line[(*i)++];
	if( n >= (Maxlen-1) ) break;
	}
rtn[n] = '\0' ;
return( 0 );
}

/* ============================================= */
/* MAKE_UNIQUE_STRING - generate an identifier using date, time, and pid */

int
GL_make_unique_string( s, i )
char *s;
int i;  /* may be sent as an integer.. if 0 getpid() will be used.. */
{
int mon, day, yr, hr, min, sec, pid;
GL_sysdate( &mon, &day, &yr );
GL_systime( &hr, &min, &sec );
s[0] = GL_encode( yr % 100 );
s[1] = GL_encode( mon );
s[2] = GL_encode( day );
s[3] = GL_encode( hr );
s[4] = GL_encode( min );
s[5] = GL_encode( sec );
if( i == 0 ) pid = getpid();
else pid = i;
s[6] = GL_encode( pid % 62 );
pid = pid / 62;
s[7] = GL_encode( pid % 62 );
s[8] = GL_encode( pid / 62 );
s[9] = '\0';

return( 0 );
}

/* encode - derive a character representation of a number (number must be in range 0 - 62) */
int
GL_encode( a )
int a;
{
if( a >= 0 && a <= 9 ) return( a + '0' );
else if( a > 35 ) return( (a-36) + 'A' ); /* A-Z    26 letters + 9 = 35 */
else if( a > 9 ) return( (a-10) + 'a' ); /* a-z */
else return( '0' );
}

/* decode - decode a character representation of a number */
int
GL_decode( a )
int a;
{
if( a >= '0' && a <= '9' ) return( a - '0' );
else if( a >= 'a' ) return( (a - 'a')+10 ); /* a-z */
else if( a >= 'A' ) return( (a - 'A')+36 ); /* A-Z    26 letters + 9 = 35 */
else return( '0' );
}

/* ============================================= */
/* EXPAND_TABS Takes a string parameter 'in' and expands tabs into spaces, placing the
      result into parameter 'out'.  
 */
int
GL_expand_tabs( out, in )
char in[], out[];
{
int i, j, k, l, len;

out[0] = '\0';
k = 0;
for( i = 0, len = strlen( in ); i < len; i++ ) {
	if( in[i] == '\t' ) {
		j =  8 - ( k % 8 ); /* 1 to 8 spaces needed */
		for( l = 0; l < j; l++ ) out[k++] = ' ';
		}
	else out[k++] = in[i];
	}
out[k] = '\0';
return( 0 );
}


/* ==================================================== */
/* WRAPTEXT - wrap txt so that no line exceeds maxchars.  Wrap is done by changing certain whitespace chars to '\n' */
int GL_wraptext( txt, maxchars )
char *txt;    /* the text */
int maxchars; /* max # of chars per line after wrap */
{
int i, lb, spaceat;
lb = 0;
spaceat = -1;
for( i = 0; txt[i] != '\0'; i++ ) {
        if( i - lb > maxchars ) {
                if( spaceat == -1 ) spaceat = i; /* for wierd situations - no space found, break right here */
                txt[spaceat] = '\n';
                lb = spaceat;
                spaceat = -1;
                }
        else if( txt[i] == '\n' ) { lb = i; spaceat = -1; } /* newline already present in txt.. respect it.. */
        else if( isspace( (int) txt[i] ) ) spaceat = i; 
        }
return( 0 );
}

/* test for wraptext */
/* main( argc, argv )
 * int argc; char **argv;
 * {
 * char buf[256];
 * if( argc < 2 ) exit(1);
 * strcpy( buf, argv[1] );
 * GL_wraptext( buf, atoi( argv[2] ) );
 * printf( "%s\n", buf );
 * }
 */


#ifndef BAREBONES  /* getgui, lxlogo */

/* ===================================================== */
/* RAND returns a "random" number between 0.0 and 1.0 */
double 
GL_rand()
{
double r;
static int first = 1;
if( first ) {
	srand( getpid() % 1000 );
	first = 0;
	}
r = rand() / (double)(RAND_MAX);
if( r < 0.0 || r > 1.0 ) { printf( "%f: rand return out of range\n", r ); return( -1.0 ); }
return( r );
}


/* =========================================== */
/* SLMEMBER - Return 1 if str is matches any items in list.
	List is a space-delimited list of tokens.  The tokens in the
	list may contain ? or * wildcard characters.  The match is
	Case insensitive.

	scg 3-19-97
*/
int
GL_slmember( str, list )
char *str;
char *list;
{
char tok[256], *GL_getok();
int i;
i = 0;
while( 1 ) {
        strcpy( tok, GL_getok( list, &i ) );
        if( tok[0] == '\0' ) break;
	if( Member_nullstring[0] != '\0' ) {
		if( strcmp( str, Member_nullstring)== 0 && stricmp( tok, "null" )==0 )return( 1 );
		}
        if( GL_wildcmp( str, tok, strlen(tok), 0 ) == 0 ) return( 1 );
        }
return( 0 );
}
/* ==================================================================== */
/* SETMAXLEN - set maximum token length for GETSEG (future: others) */
int
GL_setmaxlen( maxlen )
int maxlen;
{
if( maxlen == 0 ) Maxlen = 99999;
else Maxlen = maxlen;
return( 0 );
}


/* ===================================================================== */
/* WILDCMP - compare two strings s1 and s2.  S2 may contain 
   wildcards (* and ?).

   Function returns 0 on a match;  < 0 if s1 < s2;   > 0 if s1 > s2
   Prints an error message and returns -999 on error.

   * wildcard limited to the following uses: *ppp; ppp*; pp*pp; *ppp*
   ? can be used anywhere.

   Double asterisks at beginning and end are also handled (means the
   same as single asterisk).

   scg 3-4-96 (written elsewhere)

 */

int
GL_wildcmp( char *s1, char *s2, int len, int casecare )
/* s1  = data value
   s2  = query value which can contain wildcards - not null terminated.
   len = length of s2
   casecare = 0 for case-insensitive, 1 for case-sensitive
 */
{
int i, nwc, wcp, stat;


if( len == 0 ) return( strlen( s1 ) );
else if( s2[0] == Wildcard ) {
	if( len == 1 ) return( 0 ); /* everything matches */
	}
else if( s2[0] == Wildcard1 ) ; /* can't tell yet */
else if( tolower( s1[0] ) < tolower( s2[0] ) ) return( -1 ); /* way off */
else if( tolower( s1[0] ) > tolower( s2[0] ) ) return( 1 );  /* way off */

/* strip off extraneous * at beginning and end.. */
if( s2[0] == Wildcard && s2[1] == Wildcard ) { s2 = &s2[1]; len--; }
if( s2[len-1] == Wildcard && s2[len-2] == Wildcard ) len--;

/* see if any "*" wild cards were used.. */
nwc = 0;
for( i = 0; i < len; i++ ) if( s2[i] == Wildcard ) { nwc++; wcp = i; }

if( nwc < 1 ) {  /* straight match */
	if( strlen( s1 ) > len ) return( wcmp( s1, s2, strlen( s1 ), casecare));
	else return( wcmp( s1, s2, len, casecare ) ); 
	}

else if( nwc == 1 ) {                /* wildcard match */
	/* find beginning of what we need to compare */
	i = strlen( s1 ) - (len - (wcp+1) );

	/* case 1: wc at end.. */
	if( wcp == len-1 ) {
		return( wcmp( s1, s2, len-1, casecare ) );
		}

	/* case 2: wc at beginning.. */
	if( wcp == 0 ) {
		return( wcmp( &s1[i], &s2[ 1 ], len-1, casecare ) );
		}

	/* case 3: wc in middle.. */
	else	{
		int frontlen, backlen;

		frontlen = wcp;

		/* do front compare.. */
		stat = wcmp( s1, s2, frontlen, casecare ); 
		if( stat != 0 ) return( stat );

		backlen = strlen( s2 ) - (frontlen + 1);
		if( strlen( s1 )  < frontlen + backlen ) return( 1 );  /* fail if s1 too short */

		/* do back compare.. */
		stat = wcmp( &s1[ strlen( s1 ) - backlen ], &s2[ strlen( s2 ) - backlen ], backlen, casecare );
		return( stat );

		}
	}

else if( nwc == 2 ) {
	int stop;
	/* case 4: wc at beginning and end.. */
	if( wcp != (len-1) ) goto ERR;
	else if( s2[0] != Wildcard ) goto ERR;
	stop = ( strlen( s1 ) - len ) + 2;
	for( i = 0; i <= stop; i++ ) {
		if( wcmp( &s1[i], &s2[1], len-2, casecare ) == 0 ) return( 0 );
		}

	return( -1 );
	}
else 	{
	ERR:
	fprintf( stderr, "Wild card match error (%s vs %s).\n", s1, s2 );
	return( -999 );
	}
}

/* WCMP - compare two strings.  S2 may contain ? wildcards which matches any
       single character.  Len is the # of characters to check.
 */
static int
wcmp( char *s1, char *s2, int len, int casecare )
{
int i;

for( i = 0; i < len; i++ ) {
	
	if( s1[i] == '\0' ) { return( -1 ); }  /* added scg 10/22/03 ... abc???? was matching abcde */
									   /* was returning 1.. changed scg 3/29/04 */

	if( ! casecare ) {
		if( tolower(s1[i]) < tolower(s2[i]) && s2[i] != Wildcard1 ) { return( -1 ); }
		else if( tolower(s1[i]) > tolower(s2[i]) && s2[i] != Wildcard1 ) { return( 1 ); }
		}
	else	{
		if( s1[i] < s2[i] && s2[i] != Wildcard1 ) return( -1 );
		else if( s1[i] > s2[i] && s2[i] != Wildcard1 ) return( 1 );
		}
	}
return( 0 );
}

/* WILDCHAR - set the wildcard symbol to be used instead of '*' */
int
GL_wildchar( c, d )
char c, d;
{
Wildcard = c;
Wildcard1 = d;
return( 0 );
}



/* ============================================= */
/* ADDMEMBER - append a new member to the end of a comma-delimited list */
int
GL_addmember( newmem, list )
char *newmem;
char *list;
{
/* hard code a destination length limit of 256 for now.. scg 3/22/07 */
if( strlen( newmem ) + strlen( list ) > 254 ) return( 0 );  /* silently don't do it.. */
if( list[0] == '\0' ) strcpy( list, newmem );
else	{
	strcat( list, "," );
	strcat( list, newmem );
	}
return( 0 );
}

/* ============================================= */
/* DELETEMEMBER - remove member(s) from a comma-delimited list.
   Mem may contain wild cards.  Returns number of members removed. */
int
GL_deletemember( mem, inlist, resultlist )
char *mem;
char *inlist;
char *resultlist;
{
int i, ix, len, outlen, found, memlen;
char tok[ 256 ];

resultlist[0] = '\0';
outlen = 0;
if( inlist[0] == '\0' ) return( 1 );
memlen = strlen( mem );
len = strlen( inlist );
for( i = 0, ix = 0, found = 0; ; ) {
        if( ix >= len ) break;
        GL_getseg( tok, inlist, &ix, "," );
        if( GL_wildcmp( tok, mem, memlen, 0 )==0 ) { found++; continue; }
	if( i > 0 ) strcpy( &resultlist[ outlen++ ], "," );
	strcpy( &resultlist[ outlen ], tok );
	outlen += strlen( tok );
	i++;
	}
resultlist[ outlen ] = '\0';
return( found );
}


/* ============================================= */
/* CONTAINS - if string s contains any of chars in clist, return position (1=first)
     of first occurance in list.  0 if not found at all.
     example: contains( "\"*'", "'hello'" )  -> 1
 */
int
GL_contains( clist, s )
char *clist, *s;
{

int i, len;

for( i = 0, len = strlen( s ); i < len; i++ ) {
	if( GL_member( s[i], clist )) return( i+1 );
	}
return( 0 );
}
/* ============================================= */
/* SUBST - change all occurances of s1 to s2, in t.

   Returns 0 if successful, 1 if no occurance of s1 found. 
   scg 11/5/07 - now silently truncates the result to fit within buf[1024]
*/

int
GL_substitute( s1, s2, t )
char *s1, *s2, *t;
{
char buf[1024];
int i, j, len1, len2, buflen, found;

len1 = strlen( s1 );
if( len1 < 1 ) return( 1 );
strcpy( buf, t );
buflen = strlen( buf );
if( buflen < 1 ) return( 1 );
len2 = strlen( s2 );

found = 0;
j = 0;
for( i = 0; i < buflen; i++ ) {
	if( strncmp( &buf[i], s1, len1 )==0 ) {
		if( j + len2 > 1020 ) break; /* scg 11/3/07 */
		strcpy( &t[j], s2 );
		j += len2;
		i += (len1 - 1);
		found = 1;
		}
	else t[j++] = buf[i];
	if( j > 1020 ) break; /* scg 11/3/07 */
	}
t[j] = '\0';
if( found ) return( 0 );
else return( 1 );
}

/* ============================================= */
/* CHANGECHARS - go through string s and if any characters in clist found, change
	the character to newchar */

int
GL_changechars( clist, s, newchar )
char *clist, *s, *newchar;
{
int i, special;

special = 0;
if( strcmp( clist, "not_alnum" )==0 ) special = 1;

if( newchar[0] == '\0' ) return( 1 );
for( i = 0; s[i] != '\0'; i++ ) {
	if( special && !isalnum( (int)s[i] )) s[i] = newchar[0];
	else if( !special && GL_member( s[i], clist )) s[i] = newchar[0];
	}
return( 0 );
}

/* ============================================= */
/* DELETECHARS - go through string s and if any characters in clist found, delete
	the character. */

int
GL_deletechars( clist, s )
char *clist, *s;
{
int i, j, special;

special = 0;
if( strcmp( clist, "not_alnum" )==0 ) special = 1;

for( i = 0, j = 0; s[i] != '\0';  ) {
	if( special && !isalnum( (int)s[i] )) i++; 
	else if( !special && GL_member( s[i], clist )) i++; 
	else s[j++] = s[i++];
	}
s[j] = '\0';
return( 0 );
}
/* ======================================================================== */
/* SUBSTRING - 
   GL_substring( result, str, fromchar, nchar )
   char *result;   // substring is copied into this variable
   char *str;	// the original string
   int fromchar;   // starting point from which to take the substring
   int nchar;	// length of the substring

   In all cases the first char is 1, not 0.

   Two ways it can operate:
    If <fromchar> is greater than 0, the result will be the portion of <string>
	beginning at position <fromchar>, for a length of <nchar>, or until
	the end is reached.
    If <fromchar> is less than 0, and <nchar> is greater than 0:
	we will begin counting from the end of <string>, 
	leftward.  for (abs)<fromchar> characters.  Then, we will take the 
	substring beginning from that character 
	for a length of <nchar>, or until the end is reached.
	
    Examples: substring( result, "02001.fv02", 7, 4 )    -- result would be "fv02"
              substring( result, "02001.fv02", -4, 99 )   -- result would be "fv02"
*/

int
GL_substring( result, str, fromchar, nchar )
char *result;
char *str;
int fromchar;
int nchar;
{
int i, j;
int len;

len = strlen( str );
if( fromchar > 0 ) fromchar -= 1;
else if( fromchar < 0 ) fromchar += len;

for( i = fromchar, j = 0; i < fromchar + nchar; i++ ) { 
	if( i > len-1 ) break;
	result[j++] = str[i];
	}
result[j] = '\0';
return( 0 );
}

/* ===================================================================== */
/* VARSUB - given string s, find every occurance 
   of symbol (case-sensitive) and change it to value.  
    Copies result back into s... s must be able to accomodate.
    Returns number of times a substitution was made, 0 if none.
    This routine is not sophisticated about delimiting the symbol;
      e.g. if s contains $NUMBER and varsub() is looking for $NUM it will find it.

    -scg 11/5/07 - now silently truncates result to fit within rtnbuf[1024]
*/

int
GL_varsub( s, symbol, value )
char *s, *symbol, *value;
{
int i, j, len, vlen, found;
int slen;
char rtnbuf[1024];

len = strlen( symbol );
slen = strlen( s );
vlen = strlen( value );
found = 0;
for( i = 0, j = 0; i < slen; i++, j++ ) { 
	if( strncmp( &s[i], symbol, len )==0 ) {  /* note- strncmp man page says that it won't go beyond null terminator */
		if( j + vlen > 1020 ) break;
		strcpy( &rtnbuf[j], value );
		j = strlen( rtnbuf ) - 1;
		i+= (len-1);
		found++;
		}
	else rtnbuf[j] = s[i]; 
	if( j >= 1020 ) break; /* scg 11/3/07 */
	}
rtnbuf[j] = '\0';
strcpy( s, rtnbuf );
return( found );
}



/* ============================================= */
/* AUTOROUND - round the decimal portion a number reasonably based on its magnitude.
   val is the value, represented as a string.
   decoffset controls the precision of the rounded result as follows:
	If decoffset = 0 then nothing happens.  
	If decoffset = 1 then rounding will go to 1 additional decimal place.  
	decoffset = -1 then rounding will go to one less decimal place than normal.

   The rounded result is copied back into val.

   If val is non-numeric or a whole number then it is left unchanged.
*/

int
GL_autoround( val, decoffset )
char *val;
int decoffset; 
{
int precision, decplaces, stat;
char roundingfmt[50];
double g, atof();

stat = GL_goodnum( val, &precision );
if( stat && precision > 0 ) {
	g = atof( val );
	decplaces = getdecplaces( g );
	if( decplaces > -99 ) {
		if( decplaces < 0 ) decplaces = 0;
		sprintf( roundingfmt, "%%.%df", decplaces + decoffset );
		sprintf( val, roundingfmt, g );
		}
	}
return( 0 );
}

/* ============================================= */
/* AUTOROUNDF - variant of autoround(), takes val as a double, return value is character rep..*/

char *
GL_autoroundf( val, decoffset )
double val;
int decoffset;
{
int decplaces;
char roundingfmt[50];
static char result[50];

sprintf( result, "%g", val ); /* fallback */
decplaces = getdecplaces( val );
if( decplaces > -99 ) {
	if( decplaces < 0 ) decplaces = 0;
	sprintf( roundingfmt, "%%.%df", decplaces + decoffset );
	sprintf( result, roundingfmt, val );
	}
return( result );
}

static int getdecplaces( val )
double val;
{
int decplaces;
double g, fabs();

g = fabs( val );
decplaces = -99;
if( g >= 1000 ) decplaces = 0; 
else if( g >= 100 ) decplaces = 0; 
else if( g >= 10 ) decplaces = 1;
else if( g >= 1.0 ) decplaces = 2;
else if( g >= 0.1 ) decplaces = 3;
else if( g >= 0.01 ) decplaces = 4;
else if( g >= 0.001 ) decplaces = 5;
else if( g >= 0.0001 ) decplaces = 6;

return( decplaces );
}

#ifdef HOLD
/* ======================== */
/* FMOD  */
double fmod( a, b )
double a, b;
{
double x, y;

x = a / b;
y = (int) (x) * b;
return( a - y );
}
#endif

/* ========================= */
/* NUMGROUP - convert val to a nearby multiple of h, taking mode (low, mid, high) into account */

double
GL_numgroup( val, h, mode )
double val, h;
char *mode;
{
double fmod(), ofs, modf;
ofs = 0.0;
if( mode[0] == 'm' ) ofs = h / 2.0;
else if( mode[0] == 'h' ) ofs = h;
/* the following bug fix contributed by  Phil Carmody <thefatphil@yahoo.co.uk> */
modf = h*(int)(val/h+0.5);
return(modf+ofs);
/* was:
 * modf = fmod( val, h );
 * return( (val - modf) + ofs );
 */

}


/* ======================================================================== */
/* RANGER - take a range specification of integers and return an enumeration of all members.
 *	    Examples: "3-8" would return in list array: 3,4,5,6,7,8
 *		      "4,5,7-9,12-last" would return (for a 15 member list): 4,5,7,8,9,12,13,14,15
 *		      "4,5 7-9 12-last" would be equivalent to the above example.
 *		      "1-last" would return (for an 8 member list): 1,2,3,4,5,6,7,8
 *
 * 	    There may be no embedded spaces within the dash construct.
 */

int
GL_ranger( spec, list, n )
char *spec;
int *list;  /* array */
int *n;  /* in: size of list (max number of members)
	    out: number of members in list that have been filled */
{
int i, ix, p, j, lo, hi;
char tok[256], histr[80];

/* parse spec.. */
ix = 0;
i = 0;
while( 1 ) {
	/* split up on commas or spaces */
	GL_getchunk( tok, spec, &ix, ", " );
	if( tok[0] == '\0'  ) break;

	if( GL_goodnum( tok, &p ) ) {
		list[i] = atoi( tok );
		i++;
		}
	else	{
		sscanf( tok, "%d-%s", &lo, histr );
		if( stricmp( histr, "last" )==0 ) hi = *n;
		else hi = atoi( histr );
		if( hi < lo ) {
			fprintf( stderr, "bad range specification: %s\n", tok );
			return( 1 );
			}
		for( j = lo; j <= hi; j++ ) {
			list[i] = j;
			i++;
			if( i >= (*n) -1 ) break;  /* truncate */
			}
		}
	}
*n = i;
return( 0 );
}


/* ============================== */
/* CLOSE_TO - test two floating point numbers to see if
        they are within a small tolerance. */

int
GL_close_to( a, b, tol )
double a, b;
double tol;
{
if( a == b ) return( 1 );
else if( a > b && a - b < tol ) return( 1 );
else if( b > a && b - a < tol ) return( 1 );
else return( 0 );
}


/* ============================= */
/* COMMONMEMBERS - compare two commalists and return number of members
   that are in common.. */

int
GL_commonmembers( list1, list2, mode )
char *list1;
char *list2;
int mode; 	/* 0 = return a count; 1 = quit when one found */
{
int i, j, ii, ij, count;
int len1, len2;
char tok1[DATAMAXLEN+1], tok2[DATAMAXLEN+1];

count = 0;
len1 = strlen( list1 );
len2 = strlen( list2 );
for( i = 0, ii = 0; ; i++ ) {
	if( ii >= len1 ) break;
	GL_getseg( tok1, list1, &ii, "," );
	for( j = 0, ij = 0; ; j++ ) {
		if( ij >= len2 ) break;
		GL_getseg( tok2, list2, &ij, "," );
		if( stricmp( tok1, tok2 )==0 ) {
			if( mode == 1 ) return( 1 );
			count++;
			}
		}
	}
return( count );
}

/* ==================================== */
/* LISTMEMBER - see if s is in list (comma-delimited); 
   if so return 1 and list position (first=1) and string position (first=0) */

int
GL_listmember( s, list, memnum, pos )
char *s;
char *list;
int *memnum;
int *pos;
{
int ix, i, lastix, len;
char tok[256];

*memnum = 0;
ix = 0;
len = strlen( list );
lastix = 0;
for( i = 1, ix = 0; ; i++ ) {
	if( ix >= len ) break;
	lastix = ix;
	GL_getseg( tok, list, &ix, "," );
	if( strcmp( tok, s )==0 ) {
		*memnum = i;	
		*pos = lastix;
		return( 1 );
		}
	}
return( 0 );
}

/* ===================================== */
/* GETCGIARG - get next arg from CGI QUERY_STRING (encoded constructs are converted) */

int
GL_getcgiarg( arg, uri, pos, maxlen )
char *arg, *uri;
int *pos; /* current position */
int maxlen; /* max size of string, including terminator */
{
int i, j;
char hex[10];
unsigned int val;

for( i = *pos, j = 0; j < maxlen; i++ ) {

	if( uri[i] == '&' || uri[i] == '\0' || j >= maxlen ) {
		arg[j] = '\0';
		if( uri[i] == '\0' ) *pos = i;
		else *pos = i+1;
		return( 0 );
		}

	else if( uri[i] == '%' && isxdigit( (int) uri[i+1] ) && isxdigit( (int) uri[i+2] ) ) {  /* urldecode */
		sprintf( hex, "%c%c", uri[i+1], uri[i+2] );
       		sscanf( hex, "%x", &val );
		arg[j++] = (char) val;
		i += 2;
		}

	else if( uri[i] == '+' ) arg[j++] = ' ';  /* added scg 10/9/07 */

	else arg[j++] = uri[i];

	}

return( 0 );
}

/* ================================================= */
/* URLENCODE - perform url encoding (any questionable characters changed to %XX hex equivalent */
/* added scg 5/29/06 */
int
GL_urlencode( in, out )
char *in, *out;
{
int i, j, c;
for( i = 0, j = 0; in[i] != '\0'; i++ ) {
	c = in[i];

	/* per the wikipedia entry for "Query string"... changed scg 6/4/07 */
	if( c >= 48 && c <= 57 ) out[j++] = c;  /* 0-9 */
	else if( c >= 65 && c <= 90 ) out[j++] = c;  /* A-Z */
	else if( c >= 97 && c <= 122 ) out[j++] = c;  /* a-z */
	else if( GL_member( c, ".-~_" )) out[j++] = c; 
	/* else if( c == ' ' ) out[j++] = '+'; */  /* encode spaces as %20 ... scg 3/9/09 */
	else { sprintf( &out[j], "%%%X", c ); j += 3; } /* encode as %FF */
	}
out[j] = '\0'; /* terminate */
return( 0 );
}
/* ================================================= */
/* URLDECODE - perform url decoding (any %XX constructs changed to char equivalent */
/* added scg 5/29/06 */
int GL_urldecode( in, out )
char *in, *out;
{
int i, j, c;
char tok[10];
for( i = 0, j = 0; in[i] != '\0'; i++ ) {
	if( in[i] == '%' && in[i+1] != '\0' && in[i+2] != '\0' ) {
		tok[0] = in[i+1]; tok[1] = in[i+2]; tok[2] = '\0';
		sscanf( tok, "%x", &c );
		out[j++] = c;
		i += 2;
		}
	else out[j++] = in[i];
	}
out[j] = '\0'; /* terminate */
return( 0 );
}


/* ================================================= */
/* GETCWORD - get next word, as delimited by any sequence of spaces and punct chars - related to 'contains' */

int
GL_getcword( rtn, line, i )
char rtn[];
char line[];
int *i;
{
int n, j;
j = *i;
while( isspace( (int) line[j] ) || ispunct( (int) line[j] ) ) j++; 
n = 0;
rtn[0] = '\0';
while( 1 ){
	if( line[j] != '*' && ( isspace( (int) line[j] ) || ispunct( (int) line[j] ) || line[j] == '\0' )) break;
	else rtn[n++] = line[j];
	j++;
	}
*i = j;
rtn[n] = '\0' ;
return( 0 );
}

/* =================================== */
/* STRIP_WS strip white-space off of front and end of string s */

int
GL_strip_ws( s )
char *s;
{
int i, j, len;

/* don't do anything if first and last characters are non-space.. */
if( !isspace( (int) s[0] ) && !isspace( (int) s[ strlen( s ) - 1 ] ) ) return( 0 );
 
/* find last significant char and put a null after it */
for( j = strlen( s ) -1; j >= 0; j-- )
	if( !GL_member( s[j], " \t\n" )) break;
s[j+1] = '\0';
/* find 1st significant char at position i */
for( i = 0, len = strlen( s ); i < len; i++ ) 
	if( !GL_member( s[i], " \t\n" )) break; 
strcpy( s, &s[i] );
return( 0 );
}
#endif

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
