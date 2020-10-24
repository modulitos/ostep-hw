/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* Compile flag NO_DT will disable all date/time functionality.  */

/*
SCG 6/16/04  
	Date arithmetic problem related to pre-1970 leap years fixed.  While working on this, 
	I decided to change the base julian year to 1970 (was 1977) to conform to community 
	expectations v/v unix epoch  (this should be transparent unless somebody is taking our 
	julian dates and exporting them).  Also, dates back to 1754 are now ok.

SCG 11/01/04 ... but the above broke day of week computation..  jul=0 was a Saturday, now a 
	Thursday.. there were some hidden dependencies on jul0 = Saturday.  Fixed as noted in code.
	NOTE: Noweekend mode is not supported for dates before 1/1/70 (day of week result is incorrect).
 */


#ifndef NO_DT

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <math.h>  /* for floor() */

int DT_setdatefmt(), DT_setlazydates(), DT_makedate(), DT_weekday(), DT_getdtsep();
extern int TDH_err();
extern int GL_sysdate(), GL_goodnum(), GL_slmember(), GL_smember(), GL_member(), GL_getchunk();
 
extern long atol();
extern int atoi();

#define stricmp( s, t )         strcasecmp( s, t )
#define strnicmp( s, t, n )     strncasecmp( s, t, n )
#define err(a,b,c) 		TDH_err(a,b,c)

#define BASEYR  1970		/* 1970 = start of unix epoch */
#define BASEOLDYR  1754		/* old, but not so old as to run into Sep 1752 anomoly */
#define DAYS_FROM_BASEOLD_TO_BASE  78892     /* # days from 1/1/1755 to 1/1/1970 */
				/* assert: BASEYR and BASEOLDYR must be 2 yrs past a leap year */
#define NUMBER 0
#define ALPHA 1

#define NTYPES 28

/* constants */
static int Dmdays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/* constants */
static char *Df[16] = { "%d-%d-%d", "%d/%d/%d", "%2d%2d%d", "%d.%d.%d",
		"%3s-%d-%d", "%3s_%d_%d", "%3s/%d/%d",
		"%d-%3s-%d", "%d_%3s_%d", "%d/%3s/%d", "%d%3s%d",
		"%4d%2d%2d" };

/* constants */
static char *Dtag[NTYPES] = { "mm-dd-yy", "mm/dd/yy", "mmddyy", "mm.dd.yy",
		 /*4*/      "dd-mm-yy", "dd/mm/yy", "ddmmyy", "dd.mm.yy",
			    "", "", 
		 /* note: mmm stands for text month rep. e.g. jan */
		 /* 10 */   "mmm-dd-yy", "mmm_dd_yy", "mmm/dd/yy", 
		 /* 13 */   "dd-mmm-yy", "dd_mmm_yy", "dd/mmm/yy", "ddmmmyy",
			    "yyyy-mmm-dd", "yyyy_mmm_dd", "yyyymmmdd",
		 /* 20 */   "yy-mm-dd", "yy/mm/dd", "yymmdd", "yy.mm.dd",
		 /* 24 */   "yyyy-mm-dd", "yyyy/mm/dd", "yyyymmdd", "yyyy.mm.dd" };


static char Dmonths[12][10] = { "jan", "feb", "mar", "apr", "may", "jun", 
				"jul", "aug", "sep", "oct", "nov", "dec" };
				  /* should be 3 chars each, lower-case */

static char Fullmonth[12][16] = { "January", "February", "March", "April", "May", "June",
				"July", "August", "September", "October", "November", "December" };

static char Abbrevmonth[12][10] = { "Jan", "Feb", "Mar", "Apr", "May", "June", 
				"July", "Aug", "Sept", "Oct", "Nov", "Dec" };

static char Dwdays[8][10] = { "", "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" }; 
				  /* should be 3 chars each, capitalized as shown */

static char Fmtstring[20] = "mmddyy"; /* the current format */
static char Defaultfmt[20] = "mmddyy"; /* date format the user has chosen as "default" */

static int Pivotyear = 70;  /* a year >= pivotyear is considered 1900's.  a year < pivotyear is considered 2000's. */

/* the following are maintained internally.. */
static int Dispfmt = 2;

static int Longyr = 0;
static char Moncase[4] = "Aaa";
static int Yr;	/* most recent year sent to jdate (4 digit year) */
static int Mon;	/* most recent month sent to jdate (1-12) */
static int Day; /* most recent day sent to jdate */
/* static int Iwk; */ /* integer representation of day of week, 1 = sunday */
static int Lazydays; /* 0 = normal  1 = allow 00 for day portion to signify 'unknown' */
static int Lazymonths; /* 0 = normal  1 = allow 00 for month portion to signify 'unknown' */
static int Lazydayinuse;  
static int Lazymonthinuse;
static int Noweekendmode = 0;
static int checklengths = 1;
 
static int jul();


/* ========================= */
int
DT_initstatic()
{
strcpy( Dmonths[0], "jan" ); strcpy( Dmonths[1], "feb" ); strcpy( Dmonths[2], "mar" );
strcpy( Dmonths[3], "apr" ); strcpy( Dmonths[4], "may" ); strcpy( Dmonths[5], "jun" );
strcpy( Dmonths[6], "jul" ); strcpy( Dmonths[7], "aug" ); strcpy( Dmonths[8], "sep" );
strcpy( Dmonths[9], "oct" ); strcpy( Dmonths[10], "nov" ); strcpy( Dmonths[11], "dec" );

strcpy( Fullmonth[0], "January" ); strcpy( Fullmonth[1], "February" ); strcpy( Fullmonth[2], "March" );
strcpy( Fullmonth[3], "April" ); strcpy( Fullmonth[4], "May" ); strcpy( Fullmonth[5], "June" );
strcpy( Fullmonth[6], "July" ); strcpy( Fullmonth[7], "August" ); strcpy( Fullmonth[8], "September" );
strcpy( Fullmonth[9], "October" ); strcpy( Fullmonth[10], "November" ); strcpy( Fullmonth[11], "December" );

strcpy( Abbrevmonth[0], "Jan" ); strcpy( Abbrevmonth[1], "Feb" ); strcpy( Abbrevmonth[2], "Mar" );
strcpy( Abbrevmonth[3], "Apr" ); strcpy( Abbrevmonth[4], "May" ); strcpy( Abbrevmonth[5], "Jun" );
strcpy( Abbrevmonth[6], "Jul" ); strcpy( Abbrevmonth[7], "Aug" ); strcpy( Abbrevmonth[8], "Sep" );
strcpy( Abbrevmonth[9], "Oct" ); strcpy( Abbrevmonth[10], "Nov" ); strcpy( Abbrevmonth[11], "Dec" );

strcpy( Dwdays[0], "" ); strcpy( Dwdays[1], "Sun" ); strcpy( Dwdays[2], "Mon" ); strcpy( Dwdays[3], "Tue" ); 
strcpy( Dwdays[4], "Wed" ); strcpy( Dwdays[5], "Thu" ); strcpy( Dwdays[6], "Fri" ); strcpy( Dwdays[7], "Sat" );

strcpy( Fmtstring, "mmddyy" ); /* the current format */
strcpy( Defaultfmt, "mmddyy" ); /* date format the user has chosen as "default" */

Pivotyear = 77;
Dispfmt = 2;
Longyr = 0;
strcpy( Moncase, "Aaa" );
Noweekendmode = 0;
checklengths = 1;

return( 0 );
}

#endif

/* ========================= */
/* SETDATEPARMS - allow parameters such as month names, weekday names, etc.
   to be set dynamically. */
int
DT_setdateparms( parmname, value )
char *parmname;
char *value;
{
int  nt, stat;

#ifdef NO_DT
  return( err( 7950, "Date/time support not included in this build", "" ));
  }
#else


if( strnicmp( parmname, "dateformat", 10 ) == 0 ) {
	strcpy( Defaultfmt, value );
	stat = DT_setdatefmt( value );
	if( stat != 0 ) { printf( "[%d]", stat ); return( err( 2301, "date format is invalid", value )); }
	}
else if( strnicmp( parmname, "Pivotyear", 9 ) == 0 ) {
	Pivotyear = atoi( value );
	}
else if( strnicmp( parmname, "Lazydates", 10 ) == 0 ) {
	if( strnicmp( value, "b", 1 )==0 ) DT_setlazydates( 1, 1 );
	else if( strnicmp( value, "m", 1 )==0 ) DT_setlazydates( 1, 0 );
	else if( strnicmp( value, "d", 1 )==0 ) DT_setlazydates( 0, 1 );
	}
else if( strnicmp( parmname, "Months.abbrev", 13 )==0 ) {
	nt = sscanf( value, "%s %s %s %s %s %s %s %s %s %s %s %s", 
		Abbrevmonth[0], Abbrevmonth[1], Abbrevmonth[2], Abbrevmonth[3], 
		Abbrevmonth[4], Abbrevmonth[5], Abbrevmonth[6], Abbrevmonth[7], 
		Abbrevmonth[8], Abbrevmonth[9], Abbrevmonth[10], Abbrevmonth[11] );
	if( nt != 12 ) return( err( 2302, "Months.abbrev: 12 months expected", "" ));
	}
else if( strnicmp( parmname, "Months.full", 11 )==0 ) {
	nt = sscanf( value, "%s %s %s %s %s %s %s %s %s %s %s %s", 
		Fullmonth[0], Fullmonth[1], Fullmonth[2], Fullmonth[3], 
		Fullmonth[4], Fullmonth[5], Fullmonth[6], Fullmonth[7], 
		Fullmonth[8], Fullmonth[9], Fullmonth[10], Fullmonth[11] );
	if( nt != 12 ) return( err( 2303, "Months.full: 12 months expected", "" ));
	}
else if( strnicmp( parmname, "Months", 6 )==0 ) {
	nt = sscanf( value, "%s %s %s %s %s %s %s %s %s %s %s %s", 
		Dmonths[0], Dmonths[1], Dmonths[2], Dmonths[3], 
		Dmonths[4], Dmonths[5], Dmonths[6], Dmonths[7], 
		Dmonths[8], Dmonths[9], Dmonths[10], Dmonths[11] );
	if( nt != 12 ) return( err( 2304, "Months: 12 months expected", "" ));
	}
else if( strnicmp( parmname, "Weekdays", 8 )==0 ) {
	nt = sscanf( value, "%s %s %s %s %s %s %s", 
		Dwdays[1], Dwdays[2], Dwdays[3], Dwdays[4],
		Dwdays[5], Dwdays[6], Dwdays[7] );
	if( nt != 7 ) return( err( 2305, "Weekdays: 7 weekdays expected", "" ));
	}
else if( strnicmp( parmname, "Omitweekends", 12 )==0 ) {
	if( stricmp( value, "adjust" )==0 || stricmp( value, "yes" )==0 ) Noweekendmode = 1;
	else if( stricmp( value, "omit" )==0 ) Noweekendmode = 2;
	else Noweekendmode = 0;
	}
else if( strnicmp( parmname, "Strictdatelengths", 17 )==0 ) {
	if( tolower(value[0]) == 'y' ) checklengths = 1;
	else checklengths = 0;
	}
	
else return( err( 2306, "Unrecognized dates parameter", parmname ));

return( 0 );
}


/* ============================== */
/* JDATE -  convert the date s to julian. */
/* Handles various date formats.  The keyword TODAY may also be used. */
/* Returns 0 if date is ok; otherwise an error code. */

int
DT_jdate( s, h )
char *s; 	/* input date */
long *h;	/* julian date result */
{ 
int mon, day, yr;
int i;
char cmon[20]; /* most recent text month sent to jdate */
int nt;

if( stricmp( s, "today" )==0 ) {
	GL_sysdate( &mon, &day, &yr );
	return( jul( yr, mon, day, h ) );
	}

switch( Dispfmt ) {				/* display formats */
						/* yr can be 2 or 4 digits */

	case 0:	/* mm-dd-yy */
	case 1: /* mm/dd/yy */ 
	case 2: /* mmddyy */
	case 3:	/* mm.dd.yy */
		nt = sscanf( s, Df[ Dispfmt ], &mon, &day, &yr );  break;

	case 4:	/* dd-mm-yy */
	case 5: /* dd/mm/yy */ 
	case 6:	/* ddmmyy */
	case 7:	/* dd.mm.yy */
		nt = sscanf( s, Df[ Dispfmt-4 ], &day, &mon, &yr ); break;
	

	case 10: /* feb-17-94 */
	case 11: /* feb 17 94 */
	case 12: /* feb/17/94 */
		nt = sscanf( s, Df[ Dispfmt-6 ], cmon, &day, &yr ); 
		break;

	case 13: /* 17-feb-94 */
	case 14: /* 17 feb 94 */
	case 15: /* 17/feb/94 */
	case 16: /* 17feb94 */
		nt = sscanf( s, Df[ Dispfmt-6 ], &day, cmon, &yr ); 
		break;

	case 17: /* yyyy-mmm-dd */
		nt = sscanf( s, "%4d-%3s-%d", &yr, cmon, &day ); break;
	case 18: /* yyyy mmm dd */
		nt = sscanf( s, "%4d_%3s_%d", &yr, cmon, &day ); break;
	case 19: /* yyyymmmdd */
		nt = sscanf( s, "%4d%3s%2d", &yr, cmon, &day ); break;

	case 20: /* yy-mm-dd */
	case 21: /* yy/mm/dd */
	case 22: /* yymmdd */
	case 23: /* yy.mm.dd */
		nt = sscanf( s, Df[ Dispfmt-20 ], &yr, &mon, &day ); break;
	case 24: /* yyyy-mm-dd */
	case 25: /* yyyy/mm/dd */
	case 27: /* yyyy.mm.dd */
		nt = sscanf( s, Df[ Dispfmt-24 ], &yr, &mon, &day ); break;
	case 26: /* yyyymmdd */
		nt = sscanf( s, Df[ 11 ], &yr, &mon, &day ); break;
	}

if( nt != 3 ) return( 2 ); /* not three tokens */

/* check on lengths.. */
/* length must match exactly for any formats that don't end in yy .. */
if( checklengths ) {
	if( Dispfmt >= 17  && strlen( Dtag[Dispfmt] ) != strlen( s ) ) return( 3 );
	/* formats that end in yy may be 2 or 4 digit years; check length and year */
	if( Dispfmt < 17 ) {
		int p;
		if( strlen( Dtag[Dispfmt] ) != strlen( s ) &&        /* back to && 2/7/00; was && .. bug? 10/18/00 */
		    strlen( Dtag[Dispfmt] )+2 != strlen( s ) ) return( 4 );
		if( ! GL_goodnum( &s[ strlen( Dtag[Dispfmt] ) -2 ], &p ) ) return( 5 );
		}
	}
	

if( Dispfmt > 9 && Dispfmt <= 19 ) {  /* text months.. */
	if( Lazymonths && strncmp( cmon, "???", 3 )==0 ) mon = 0;
        else    {
		for( i = 0; i < 12; i++ ) 
			if( stricmp( Dmonths[i], cmon ) == 0 ) break;
		if( i == 12 ) return( 6 ); /* bad month */
		else mon = i + 1;
		}
	}


/* handle lazy dates.. */
Lazymonthinuse = 0; Lazydayinuse = 0;
if( Lazymonths && mon == 0 ) { Lazymonthinuse = 1; mon = 1; }
if( Lazydays && day == 0 ) { Lazydayinuse = 1; day = 1; }

/* message( "[nt=%d yr=%d mon=%d day=%d]", nt, yr, mon, day ); sleep( 1 ); */

/* sanity checks.. */
if( !Longyr && yr > 99 ) return( 7 );
if( Longyr && ( yr < BASEOLDYR || yr > 2400 ) ) return( 8 );
if( mon < 1 || yr < 0 || day < 0 || day > 31 ) return( 9 );


return( jul( yr, mon, day, h ) );
}

/* ======================== */
/* JUL - take y,m,d and return julian date.  */
/* Returns 0 if ok or -1 for error */

static int 
jul( y, m, d, julian )
int y;		/* year */
int m;		/* month */
int d;		/* day */
long *julian;	/* julian date result */
{
int i, leap, refyear, yfc, rfc, win1, win2;
long jul;

refyear = BASEYR;

/* make sure month is in range.. */
if( m < 1 || m > 12 ) return( -1 ); 
Mon = m;

/* decrement month so it serves as index into array... */
m--; 				    
			
/* if input date contains a two-digit year - adjust using pivot */
if( y < 100 ) {			    
	if( y >= Pivotyear ) y += 1900;
	else if( y < Pivotyear ) y += 2000;
	}
Yr = y; /* remember year */

/* handle input dates before 1970.. */
if( y < BASEOLDYR ) return( -1 );  /* too old */
if( y < BASEYR ) refyear = BASEOLDYR;  /* switch to BASEOLDYR as year basis for calculations */

/* see if input date is a leap year.. a leap year is every 4 years except century boundary (except when year % 400 == 0) */
if( y % 4 == 0 && (y % 100 != 0 || y % 400 == 0 )) leap = 1;
else leap = 0;

/* make sure day is in range.. */
if( leap && m == 1 ) {
	if( d < 1 || d > Dmdays[m]+leap ) return( -1 );
	}
else if( d < 1 || d > Dmdays[ m ] ) return( -1 ); /* bad day */
Day = d;

/* calculate julian date.. */
jul = ( y - refyear ) * 365;  			/* accumulate days in the years up until the current year */
for( i = 0; i < m; i++ ) jul += Dmdays[i];	/* accumulate days in months this year up until the current month */
jul += (d -1); 					/* and add days in the current month */

/* add 1 for every Feb 29 crossed.. */
yfc = y % 4;
rfc = refyear % 4;
win1 = refyear + rfc;   /* win1 will be the leapyear following ref yr */
win2 = y - yfc;		/* win2 will be the leapyear preceding input yr */

if( win1 > win2 ) ; 		    /* do nothing; no Feb 29s crossed */
else	{
	if( win1 == win2 ) ;       /* do nothing; perhaps 1 Feb 29 crossed; */
	else jul += ((win2 - win1) / 4);  /* 1 or more Feb 29s crossed */
	if( yfc > 0 || m > 1 ) jul += 1;  /* if a non-leap year, or if a leap year date after Feb, add 1 */
	}
/* fprintf( stderr, "[y = %d  yfc=%d  win1=%d  win2=%d  (win2-win1)/4= %d   jul after: %d]\n", 
 * 	y, yfc, win1, win2, (win2-win1)/4, jul ); */


/* subtract 1 for every century boundary (1800 or 1900, but not 2000) crossed.. */
if( refyear == BASEOLDYR ) {
	/* there's no Feb 29 in 1800.. so if our date is after Feb 28, 1800 subtract 1.. */
	/* and there's no Feb 29 in 1900.. so if our date is after Feb 28, 1900 subtract 1.. */
	if( y == 1800 && m > 1 ) jul--;			
	else if( y > 1800 ) jul--;
	if( y == 1900 && m > 1 ) jul--;			
	else if( y > 1900 ) jul--;
	}

if( refyear == BASEOLDYR ) jul -= DAYS_FROM_BASEOLD_TO_BASE;   /* put back in 0=1970 space */

if( Noweekendmode ) {    /* added scg 5/22/00 */
	int nweekends;
	i = jul % 7; 
	if( i == 0 ) i = 7;
	if( i < 0 ) i = (7 + i);

	/* changed the following ... day-of-week bug - scg 11/01/04 */
	/* 2==saturday  3==sunday */
	if( Noweekendmode == 2 && ( i == 2 || i == 3 ) ) return(  -1 );  /* reject sunday or saturday */
        if( i == 3 ) jul += 1; /* sunday: change to monday */
        if( i == 2 ) jul -= 1; /* saturday: change to friday */


	/* nweekends =  (jul / 7) + 1; */ /* changed ... day-of-week bug - scg 11/01/04 */
	nweekends =  (jul / 7);
	if( i >= 2 && i < 7 ) nweekends++; 
	jul -= (nweekends * 2);  /* deduct 2 days per weekend */
	}

*julian = jul;
return( 0 );
}

/* ======================== */
/* FROMJUL - given a julian date, return text representation in the current format. */

int
DT_fromjul( jul, result )
long jul;	/* input julian date */
char *result;	/* result */
{
int y, m, day, n;
int stat;
int baseyear;

if( jul < 0 ) { 	/* use BASEOLDYR */
	jul += DAYS_FROM_BASEOLD_TO_BASE;
	baseyear = BASEOLDYR;
	}
else baseyear = BASEYR;

if( jul < 0 ) return( 1 );  /* date too old */

if( Noweekendmode ) {  /* added scg 5/22/00 */
	int nweekends, iw;

	/* nweekends = (jul / 5) + 1; */ /* changed ... day-of-week bug.. scg 11/01/04 */
	iw = jul % 5;
	nweekends = jul / 5;
	if( iw >= 2 ) nweekends++;

	jul += (nweekends * 2);  /* add 2 days per weekend */
	}


for( y = baseyear; ; y++ ) {
	if( y % 4 == 0 && (y % 100 != 0 || y % 400 == 0 )) n = 366;
	else n = 365;
	if( n > jul ) break;
	jul = jul - n;
	}
for( m = 0; ; m++ ) {
	n = Dmdays[m];
	if( m == 1 && ( y % 4 == 0 && (y % 100 != 0 || y % 400 == 0 )) ) n++;
	if( n > jul ) break;
	jul = jul - n;
	}
day = jul;
stat = DT_makedate( y, m+1, day+1, "", result );
return( stat );
}

/* ======================== */
/* SETDATEFMT - set the current date format to s.
   s may be any of the strings in Dtag array, optionally with 4 digit years.
   Capitalization of text months is determined by case of the 'm'.
   Returns 0 if OK; returns 1 if date format unrecognized. */

int
DT_setdatefmt( s )
char *s;	/* the new date format */
{
int i;
if( strcmp( s, Fmtstring )==0 ) return( 0 ); /* no change */
for( i = 0; i < NTYPES; i++ ) {
	if( Dtag[i][0] == '\0' ) continue;
	if( strncasecmp( s, Dtag[i], strlen( Dtag[i] ) )==0 ) {
		Dispfmt = i;

		/* set Longyr flag */
		if( strlen( s ) - strlen( Dtag[i] ) == 2 ) Longyr = 1;
		else if( strncmp( Dtag[i], "yyyy", 4 )==0 ) Longyr = 1;
		else Longyr = 0;

		/* set Moncase case format */
		if( Dispfmt >= 10 && Dispfmt <= 12 ) strncpy( Moncase, s, 3 );
		else if( Dispfmt >= 13 && Dispfmt <= 15 ) strncpy( Moncase, &s[3], 3 );
		else if( Dispfmt == 16 ) strncpy( Moncase, &s[2], 3 );
		else if( Dispfmt >= 17 && Dispfmt <= 19 ) strncpy( Moncase, &s[4], 3 );

		strcpy( Fmtstring, s );
		return( 0 );
		}
	}
return( 1 );
}

/* ======================== */
/* GETDATEFMT - get the current date format. */
int
DT_getdatefmt( s )
char *s;
{
strcpy( s, Fmtstring );
return( 0 );
}

/* ============================= */
/* DEFAULTDATEFMT - set current date format back to user-specified default. */
int
DT_defaultdatefmt( )
{
DT_setdatefmt( Defaultfmt );
return( 0 );
}

/* ============================= */
/* SETLAZYDATES - turn on/off lazy months or lazy days */
int
DT_setlazydates( mon, day )
int mon;	/* 1 = allow lazy months; 0 = don't */
int day; 	/* 1 = allow lazy days; 0 = don't */
{
Lazymonths = mon;
Lazydays = day;
return( 0 );
}


/* ============================= */
/* MAKEDATE - produce a date string using m, d, y,
   in 'format' or if format="", the current date format.
   Returns 0 if ok, or non-zero on error. */
int
DT_makedate( yr, mon, day, format, result )
int yr, mon, day; /* year month and day */
char *format;	  /* format to use, or "" for current date format */
char *result;	  /* result */
{
char year[10];
char cmon[10];
char oldformat[20];
int stat;
int i;

strcpy( oldformat, Fmtstring );
if( format[0] != '\0' ) {
	stat = DT_setdatefmt( format );
	if( stat != 0 ) return( stat );
	}

if( Lazymonthinuse && mon == 1 ) mon = 0;
if( Lazydayinuse && day == 1 ) day = 0;

if( Longyr ) {
	if( yr < 100 ) { /* convert using pivot year */
        	if( yr >= Pivotyear ) yr += 1900;
        	else if( yr < Pivotyear ) yr += 2000;
        	}
	sprintf( year, "%04d", yr );
	}
else if( yr >= 100 ) {
	sprintf( year, "%d", yr );
	strcpy( year, &year[2] );
	}
/* else sprintf( year, "%d", yr ); changed scg 4/30/99 */
else sprintf( year, "%02d", yr );

if( Dispfmt >= 10 && Dispfmt <=19 ) { /* handle capitalization of text months */
	if( mon == 0 ) strcpy( cmon, "???" );
	else 	{
		strcpy( cmon, Dmonths[ mon-1 ] );
		for( i = 0; i < 3; i++ ) if( isupper( (int) Moncase[i] )) cmon[i] = toupper( (int) cmon[i] );
		}
	}
	

switch( Dispfmt ) {                             /* display formats */
                                                /* yr can be 2 or 4 digits */
        case 0: sprintf( result, "%02d-%02d-%s", mon, day, year ); break;
        case 1: sprintf( result, "%02d/%02d/%s", mon, day, year ); break;
        case 2: sprintf( result, "%02d%02d%s", mon, day, year ); break;
        case 3: sprintf( result, "%02d.%02d.%s", mon, day, year ); break;
        case 4: sprintf( result, "%02d-%02d-%s", day, mon, year ); break;
        case 5: sprintf( result, "%02d/%02d/%s", day, mon, year ); break;
        case 6: sprintf( result, "%02d%02d%s", day, mon, year ); break;
        case 7: sprintf( result, "%02d.%02d.%s", day, mon, year ); break;

	case 10:sprintf( result, "%s-%02d-%s", cmon, day, year ); break;
	case 11:sprintf( result, "%s_%02d_%s", cmon, day, year ); break;
	case 12:sprintf( result, "%s/%02d/%s", cmon, day, year ); break;

	case 13:sprintf( result, "%02d-%s-%s", day, cmon, year ); break;
	case 14:sprintf( result, "%02d_%s_%s", day, cmon, year ); break;
	case 15:sprintf( result, "%02d/%s/%s", day, cmon, year ); break;
	case 16:sprintf( result, "%02d%s%s", day, cmon, year ); break;
	case 17: /* yyyy-mmm-dd */
		sprintf( result, "%4d-%s-%02d", yr, cmon, day ); break;
	case 18: /* yyyy mmm dd */
		sprintf( result, "%4d_%s_%02d", yr, cmon, day ); break;
	case 19: /* yyyymmmdd */
		sprintf( result, "%4d%s%02d", yr, cmon, day ); break;

	case 20:
	case 24: sprintf( result, "%s-%02d-%02d", year, mon, day ); break;
	case 21:
	case 25: sprintf( result, "%s/%02d/%02d", year, mon, day ); break;
	case 22:
	case 26: sprintf( result, "%s%02d%02d", year, mon, day ); break;
	case 23:
	case 27: sprintf( result, "%s.%02d.%02d", year, mon, day ); break;
	}
if( format[0] != '\0' ) DT_setdatefmt( oldformat );
return( 0 );
}


/* ============================= */
/* TODAYSDATE - produce today's date in the "current date format". */
int
DT_todaysdate( s )
char *s;	/* returned - todays date in the current date format */
{
long jul;
int stat;
stat = DT_jdate( "today", &jul );
stat += DT_fromjul( jul, s );
return( stat );
}

/* ============================= */
/* FORMATDATE - take s which should be in "current date format"
   and produce a date of format fmt. 

   Lazy dates are always acceptable to formatdate().

   In addition to the formats defined at the top of this file, these
   formats are also supported:  
				month_dd,_yyyy  or full
			        mon_dd,_yyyy
				dd_month,_yyyy
				dd_mon,_yyyy
	yymm, yy?mm,  yyyymm, yyyy?mm
	mmyy, mm?yy,  mmyyyy, mm?yyyy

	yymmm yy?mmm yy?mmm yy?mm, etc.
*/
int
DT_formatdate( date, fmt, result )
char *date;	/* old date */
char *fmt; 	/* new format */
char *result;	/* result- date converted to new format */
{
long jul;
int stat;
int oldlazyday, oldlazymon;
int i;
char wkd[20];
int found;
int slen;
int iwk;

/* go into lazy mode always, (save/restore current lazy state) */
oldlazyday = Lazydays;
oldlazymon = Lazymonths;
Lazydays = 1; Lazymonths = 1;
stat = DT_jdate( date, &jul );
Lazydays = oldlazyday; Lazymonths = oldlazymon;

if( stat != 0 )return( stat );
stat = 0;
found = 0;

strcpy( result, "" );

/* the most common formats */
if( GL_slmember( fmt, "mmddyy* mm?dd?yy* ddmmyy* dd?mm?yy* dd?mmm?yy*" )) {  
 	stat = DT_makedate( Yr, Mon, Day, fmt, result );
        found = 1;
	}
else if( GL_slmember( fmt, "mon* dd_* full wfull www_* '*" )) {
	/* embedded underscores removed from result - scg 3/21/01 */
	if( GL_smember( fmt, "month_dd,_yyyy full" )) 
		sprintf( result, "%s %d, %04d", Fullmonth[Mon-1], Day, Yr );
	else if( strcmp( fmt, "mon_dd,_yyyy" )==0 ) 
		sprintf( result, "%s %02d, %04d", Abbrevmonth[Mon-1], Day, Yr );
	else if( strcmp( fmt, "dd_month,_yyyy" )==0 ) 
		sprintf( result, "%02d %s, %04d", Day, Fullmonth[Mon-1], Yr );
	else if( strcmp( fmt, "dd_mon,_yyyy" )==0 ) 
		sprintf( result, "%02d %s, %04d", Day, Abbrevmonth[Mon-1], Yr );
	else if( GL_smember( fmt, "wfull www_*" )) {
		DT_weekday( date, wkd, &iwk );
		if( strcmp( fmt, "www_dd_mon,_yyyy" )==0 ) 
			sprintf( result, "%s %d %s, %d", wkd, Day, Abbrevmonth[Mon-1], Yr );
		else
			sprintf( result, "%s %s %d, %d", wkd, Abbrevmonth[Mon-1], Day, Yr );
		}
  	else if( GL_slmember( fmt, "'yy" )) sprintf( result, "'%02d", Yr%100 );
	return( 0 );
	}
else if( tolower(fmt[0]) == 'y' ) {
  found = 1;
  if(          strcmp( fmt, "yymm" )==0 ) sprintf( result, "%02d%02d", (Yr%100), Mon );
  else if(     stricmp( fmt, "yymmm" )==0 ) sprintf( result, "%02d%c%s", (Yr%100), fmt[2], Dmonths[Mon-1] );
  else if( GL_slmember( fmt, "yy?mmm" )) sprintf( result, "%02d%c%s", (Yr%100), fmt[2], Dmonths[Mon-1] );
  else if( GL_slmember( fmt, "yy?mm" )) sprintf( result, "%02d%c%02d", (Yr%100), fmt[2], Mon );
  else if(     strcmp( fmt, "yyyymm" )==0 ) sprintf( result, "%4d%02d", Yr, Mon );
  else if( GL_slmember( fmt, "yyyy?mm" )) sprintf( result, "%4d%c%02d", Yr, fmt[4], Mon );
  else if(      strcmp( fmt, "yy" )==0 ) sprintf( result, "%02d", Yr%100 );
  else if(      strcmp( fmt, "yyyy" )==0 ) sprintf( result, "%4d", Yr );
  else if(     stricmp( fmt, "yyqn" )==0 ) { sprintf( result, "%02dq%d", (Yr%100), ((Mon-1)/3)+1 ); }
  else if(     stricmp( fmt, "yyyyqn" )==0 ) { sprintf( result, "%dq%d", Yr, ((Mon-1)/3)+1 ); }
  else found = 0;
  }
else if( tolower(fmt[0]) == 'm' ) {
  found = 1;
  if(           strcmp( fmt, "mmyy" )==0 ) sprintf( result, "%02d%02d", Mon, (Yr%100) );
  else if(     stricmp( fmt, "mmmyy" )==0 ) sprintf( result, "%s%02d", Dmonths[Mon-1], (Yr%100) );
  else if( GL_slmember( fmt, "mmm?yy" )) sprintf( result, "%s%c%02d", Dmonths[Mon-1], fmt[3], (Yr%100) );
  else if( GL_slmember( fmt, "mm?yy" )) sprintf( result, "%d%c%02d", Mon, fmt[2], (Yr%100) );
  else if(      strcmp( fmt, "mmyyyy" )==0 ) sprintf( result, "%02d%4d", Mon, Yr );
  else if(     stricmp( fmt, "mmmyyyy" )==0 ) sprintf( result, "%s%4d", Dmonths[Mon-1], Yr );
  else if( GL_slmember( fmt, "mm?yyyy" )) sprintf( result, "%d%c%4d", Mon, fmt[2], Yr );
  else if(     stricmp( fmt, "mmmdd" )==0 ) sprintf( result, "%s%d", Dmonths[Mon-1], Day );
  else if( GL_slmember( fmt, "mm?dd" )) sprintf( result, "%d%c%d", Mon, fmt[2], Day );
  else if( GL_slmember( fmt, "mmm?dd" )) sprintf( result, "%s%c%d", Dmonths[Mon-1], fmt[3], Day );
  else if(     stricmp( fmt, "mmm" )==0 ) sprintf( result, "%s", Dmonths[Mon-1] );
  else if(      strcmp( fmt, "mm" )==0 ) sprintf( result, "%d", Mon );
  else if(     stricmp( fmt, "m" )==0 ) sprintf( result, "%c", Dmonths[Mon-1][0] );
  else found = 0;
  }
else	{
  found = 1;
  if(       strcmp( fmt, "dd" )==0 ) sprintf( result, "%02d", Day );
  else if(  strcmp( fmt, "d" )==0 ) sprintf( result, "%d", Day );
  else if( GL_slmember( fmt, "dd?mmm" )) sprintf( result, "%02d%c%s", Day, fmt[2], Dmonths[Mon-1] ); /* added scg 5/11/04 */
  else if( stricmp( fmt, "ddmmm" )==0 ) sprintf( result, "%02d%s", Day, Dmonths[Mon-1] ); /* added scg 5/11/04 */
  else if( stricmp( fmt, "w" )==0 ) { DT_weekday( date, wkd, &iwk ); sprintf( result, "%c", wkd[0] ); }
  else if( stricmp( fmt, "www" )==0 ) { DT_weekday( date, wkd, &iwk ); sprintf( result, "%s", wkd ); }
  else if( stricmp( fmt, "q" )==0 ) { sprintf( result, "%d", ((Mon-1)/3)+1 ); }
  else if( stricmp( fmt, "nq" )==0 ) { sprintf( result, "%dq", ((Mon-1)/3)+1 ); }
  else if( stricmp( fmt, "qn" )==0 ) { sprintf( result, "q%d", ((Mon-1)/3)+1 ); }
  else if( stricmp( fmt, "nqyy" )==0 ) { sprintf( result, "%dq%02d", ((Mon-1)/3)+1, (Yr%100) ); }
  else if( stricmp( fmt, "nqyyyy" )==0 ) { sprintf( result, "%dq%d", ((Mon-1)/3)+1, Yr ); }
  else found = 0;
  }

if( !found ) stat = DT_makedate( Yr, Mon, Day, fmt, result );

/* capitalization.. */
for( i = 0, slen = strlen( fmt ); i < slen; i++ ) {
	if( isupper( (int) fmt[i] ) ) result[i] = toupper( (int) result[i] );
	}

return( stat );
}

/* ============================= */
/* DAYSDIFF - find # days difference between two dates.
   Both dates should be in "current date format". 
   Return 0 if OK, or non-zero on error (bad date argument). */
int
DT_daysdiff( d1, d2, ndays )
char *d1, *d2;  /* two dates, in current date format */
long *ndays;	/* result: difference d1 - d2 in days */
{
int stat;
long j1, j2;
stat = DT_jdate( d1, &j1 );
stat += DT_jdate( d2, &j2 );
if( stat != 0 ) return( stat );
*ndays = j1 - j2;
return( 0 );
}
/* ============================= */
/* DATEINRANGE - determine if testdate is within the range
   defined by earlydate and late date, exclusive.
   Returns 1 if so, 0 if not, or -1 on error. */
int
DT_dateinrange( testdate, earlydate, latedate )
char *testdate, *earlydate, *latedate;
{
long d1, d2;
int stat1, stat2;
stat1 = DT_daysdiff( earlydate, testdate, &d1 );
stat2 = DT_daysdiff( testdate, latedate, &d2 );
if( stat1 + stat2 != 0 ) return( -1 );
else if( d1 <= 0 && d2 <= 0 ) return( 1 );
else return( 0 );
}

/* ============================= */
/* DAYSDIFF_M - find # days difference between two dates, of different format.
   Each date arg may be in any supported format- format for each date arg must be specified.
   Return 0 if OK, or non-zero on error (bad date argument). */
int
DT_daysdiff_m( d1, fmt1, d2, fmt2, ndays )
char *d1, *d2; /* two dates, in any supported format */
char *fmt1, *fmt2; /* format of date1 and date2 */
long *ndays;	/* result: difference d1 - d2 in days */
{
int stat;
long j1, j2;
char oldfmt[30];
strcpy( oldfmt, Fmtstring );
DT_setdatefmt( fmt1 );
stat = DT_jdate( d1, &j1 );
DT_setdatefmt( fmt2 );
stat += DT_jdate( d2, &j2 );
DT_setdatefmt( oldfmt );
if( stat != 0 ) return( stat );
*ndays = j1 - j2;
return( 0 );
}

/* ============================= */
/* DATEADD - add ndays to date to produce a new date.
   date should be in "current date format". 
   Return 0 if OK, or non-zero on error (bad date argument). */
int
DT_dateadd( date, ndays, result )
char *date;	/* date, in current date format */
int ndays;	/* number of days to add to date */
char *result;	/* result */
{
int stat;
long jul;
stat = DT_jdate( date, &jul );
if( stat != 0 ) return( stat );
jul += ndays;
stat = DT_fromjul( jul, result );
return( 0 );
}
/* ============================= */

/* ============================= */
/* WEEKDAY - find day of week that a certain date fell on / falls on.
   TDH_date should be in "current date format".
   Return 0 if OK, or non-zero on error (bad date) */
int
DT_weekday( date, result, iwk )
char *date; 	/* date, in current date format */
char *result;	/* result */
int *iwk;	/* 1 - 7 representation of result */
{
long jul;
int i, stat, wdix;

stat = DT_jdate( date, &jul );
if( stat != 0 ) return( stat );
if( Noweekendmode ) {
	i = jul % 5;
	/* changed .. day-of-week bug  scg 11/1/04 */
	if( i >= 2 && i <= 4) *iwk = i;
	else *iwk = i+5;
	strcpy( result, Dwdays[ (*iwk) ] );
	return( 0 );
	}
else	{
	i = jul % 7;
	if( i == 0 ) i = 7;
	if( i < 0 ) i = (7 + i);
	wdix = ((i + 4) % 7) + 1;
	strcpy( result, Dwdays[wdix] ); 
	*iwk = wdix;
	return( 0 );
	}
}

/* ============================= */
/* YEARSOLD - compute the age in years between birthdate and the given date.
	TDH_dates are expected to be in the "current date format".
	Calculates age in years.
	Returns 0 if OK; -1 if a bad date was encountered.

	Four-digit year values are recommended.
	IF your date format uses 2 digit year values, the following apply:
	   1. birthdates which have a year greater than the current year are 
		considered 'old', i.e.  if today is Feb 19, 1997 and the 
		birthday argument is 022097 the birthday is assumed to be 
		Feb 20, 1897.
	                --
	    2. The 'date' argument, on the other hand, is assumed to be 
		'fairly recent', so 75-99  are considered 1900's and 00-74 
		are considered 2000's.
	    3. Ages over 100 cannot be handled.
*/
int
DT_yearsold( birthday, date, age )
char *birthday, *date;  /* two dates in current date format */
int *age;		/* result: age in years */
{
int by, bm, bd, dy, dm, dd, ty;
int yeardiff;
long jdt;
int stat;

stat = DT_jdate( "today", &jdt );
ty = Yr;

/* add century to birthday year and 'date' year, to handle turn of century.. */
stat = DT_jdate( birthday, &jdt );
by = Yr; bm = Mon; bd = Day;

stat += DT_jdate( date, &jdt );
dy = Yr; dm = Mon; dd = Day;
if( stat != 0 ) return( -1 );

/* if birthdate appears to be later than today.. */
if( by > dy ) {
	if( ty >= Pivotyear ) by += 1800;
	if( ty < Pivotyear ) by += 1900;
	}
else 	{
	if( ty >= Pivotyear ) by += 1900;
	if( ty < Pivotyear ) by += 2000;
	}
if( ty >= Pivotyear ) dy += 1900;
else dy += 2000;


yeardiff = dy - by;

/* if date's month/day is before birthday's , subtract 1 */
if( dm < bm ) yeardiff--;
else if( dm == bm ) {
	if( dd < bd ) yeardiff--;
	}

if( yeardiff < 0 ) yeardiff += 100;
*age = yeardiff;
return( 0 );
}

/* ================================= */
/* GROUP - adjust date for grouping */
int
DT_group( interval, mode, in, out )
char interval; /*  'h' = hour, 'd' = day, 'w' = week, 'm' = month, 'q' = quarter, 'y' = year */
char mode;   /* 'm' = mid  'f' = first of */
char *in, *out;
{
int i, stat;
long jul;
char buf[20];
int datetime;
char dtsep[4];

if( mode == 'l' ) mode = 'f';  /* "low" maps to "first" - scg 11/26/07 */

DT_getdtsep( dtsep ); /* added scg 5/25/06 */

/* strip off time portion if 'in' is datetime.. */
datetime = 0;
for( i = 0; in[i] != '\0'; i++ ) if( in[i] == dtsep[0] ) break;
if( in[i] == dtsep[0] ) {
	strcpy( buf, &in[i+1] );
	in[i] = '\0';
	datetime = 1;
	}

if( in[2] == ':' ) {
	strcpy( buf, in ); /* or get lone time value.. */
	in[0] = '\0';
	}
else 	{
	stat = DT_jdate( in, &jul ); /* sets Yr, Mon, Day */
	if( stat != 0 ) return( stat );
	}

if( interval == 'm' ) {
	if( mode == 'm' ) DT_makedate( Yr, Mon, 15, "", out );
	else DT_makedate( Yr, Mon, 1, "", out );
	}
else if( interval == 'w' ) {
	int iwk;
	DT_weekday( in, buf, &iwk ); 
	if( mode == 'm' ) jul += (4 - iwk);
	else jul -= (iwk - 1);
	DT_fromjul( jul, out );
	}
else if( interval == 'd' ) {
	strcpy( out, in );
	if( datetime && mode == 'm' ) { strcat( out, dtsep ); strcat( out, "12:00" ); }
	else if( datetime && mode == 'm' ) { strcat( out, dtsep ); strcat( out, "00:01" ); }
	}
else if( interval == 'h' ) {
	/* time portion is in buf.. */
	if( mode == 'm' && in[0] ) sprintf( out, "%s%s%c%c:30", in, dtsep, buf[0], buf[1] );
	else if( mode == 'm' && !in[0] ) sprintf( out, "%c%c:30", buf[0], buf[1] );
	else if( mode == 'f' && in[0] ) sprintf( out, "%s%s%c%c:00", in, dtsep, buf[0], buf[1] );
	else if( mode == 'f' && ! in[0] ) sprintf( out, "%c%c:00", buf[0], buf[1] );
	}
else if( interval == 'q' ) {
	if( mode == 'm' ) i = 15;
	else i = 1;
	if( Mon >= 1 && Mon <= 3 ) DT_makedate( Yr, 2, i, "", out );
	else if( Mon >= 4 && Mon <= 6 ) DT_makedate( Yr, 5, i, "", out );
	else if( Mon >= 7 && Mon <= 9 ) DT_makedate( Yr, 8, i, "", out );
	else if( Mon >= 10 && Mon <= 12 ) DT_makedate( Yr, 11, i, "", out );
	}
else if( interval == 'y' && mode == 'm' ) DT_makedate( Yr, 6, 30, "", out );
else if( interval == 'y' && mode == 'f' ) DT_makedate( Yr, 1, 1, "", out );
	
if( datetime && GL_member( interval, "mwqy" )) { strcat( out, dtsep ); strcat( out, "00:01" ); }
return( 0 );
}


/* ================================= */
/* MONTHNAME - allow apps to access month names stored herein */
int
DT_monthname( m, format, result )
int m; /* month where 1 = Jan */
char *format;
char *result;
{
if( strcmp( format, "char" )==0 ) sprintf( result, "%c", Abbrevmonth[m-1][0] );
else if( strcmp( format, "abbrev" )==0 ) sprintf( result, "%s", Abbrevmonth[m-1] );
return( 0 );
}

/* ================================= */
/* GETMDY - get month, day, and year of date most recently processed with jdate() */
int
DT_getmdy( m, d, y )
int *m, *d, *y;
{
*m = Mon;
*d = Day;
*y = Yr;
return( 0 );
}

/* ======================== */
/* CHECKDATELENGTHS - turn on/off checking of date lengths.  On (1) is more
   strict. */
int
DT_checkdatelengths( mode )
int mode;
{
checklengths = mode;
return( 0 );
}

/* ======================== */
/* REASONABLE - given stype (date or datetime), and min and max, 
   determine a reasonable default increment amount and units, a 
   default display format, and other parameters useful to ploticus.
   Returns 0 if it comes up with something, 1 on error. */

int 
DT_reasonable( stype, min, max, inc, units, dispfmt, autom, autoy, autod, minc, munits, nearest )
char *stype;
double min, max; 	/* in basic units (days for date & datetime;  minutes for time stype) */
double *inc; 		/* returned: stub inc  */
char *units, *dispfmt, *autom, *autoy, *autod;  	/* returned: stub inc units, stubformat, automonts, autoyears, autodays */
double *minc; 		/* returned: minor tic inc */
char *munits, *nearest; /* returned: minor tic units, autorange nearest= */
{
double diff;
int dtflag;

strcpy( nearest, "" ); strcpy( dispfmt, "" ); strcpy( autom, "" ); strcpy( autoy, "" ); strcpy( autod, "" ); 
strcpy( munits, "" ); strcpy( nearest, "" );
*inc = 0.0; *minc = 0.0;

diff = max - min;
if( diff <= 0.0 ) return( 1 );

if( strcmp( stype, "datetime" )==0 ) dtflag = 1;
else dtflag = 0;

if( strncmp( stype, "date", 4 )==0 ) { /* date and datetime */

	if( diff < 0.6 && dtflag ) { *inc = 1.0; strcpy( units, "hour" ); strcpy( dispfmt, "hha" ); strcpy( nearest, "hour" ); }
	else if( diff < 2.0 && dtflag ) { 
		*inc = 6.0; strcpy( units, "hour" ); strcpy( dispfmt, "hha" ); 
		*minc = 1.0; strcpy( munits, "hour" ); strcpy( nearest, "6hour" ); 
		}
	else if( diff < 22.0 ) { *inc = 1.0; strcpy( units, "day" ); strcpy( dispfmt, "d" ); strcpy( nearest, "day" ); } 
	else if( diff < 120.0 ) { 
		if( Noweekendmode ) *inc = 5.0;
		else *inc = 7.0;
		strcpy( units, "day" ); strcpy( dispfmt, "d" ); 
		*minc = 1.0; strcpy( munits, "day" ); strcpy( nearest, "monday" );
		} 
	else if( diff < 185.0 ) { 
		*inc = 1.0; strcpy( units, "month" ); strcpy( dispfmt, "Mmm" ); 
		*minc = 1.0; strcpy( munits, "day" ); strcpy( nearest, "month" ); 
		}
	else if( diff < 350.0 ) { *inc = 1.0; strcpy( units, "month" ); strcpy( dispfmt, "Mmm" ); strcpy( nearest, "month" ); }
	else if( diff < 1500.0 ) { *inc = 1.0; strcpy( units, "month" ); strcpy( dispfmt, "M" ); strcpy( nearest, "month" ); }
	else if( diff < 4000.0 ) { 
		*inc = 1.0; strcpy( units, "year" ); strcpy( dispfmt, "'yy" ); 
		*minc = 3.0; strcpy( munits, "month" ); strcpy( nearest, "year" ); 
		}
	else if( diff < 8000.0 ) { *inc = 5; strcpy( units, "years" ); strcpy( dispfmt, "'yy" ); strcpy( nearest, "5year" ); } 
	else { *inc = 10; strcpy( units, "years" ); strcpy( dispfmt, "'yy" ); strcpy( nearest, "10year" ); } 

	/* perhaps set some autodays/months/years defaults */  /* strcmp clauses added scg 3/9/09 */
	if( diff < 7.0 && dtflag && strncmp( units, "day", 3 )!= 0 ) strcpy( autod, "ddMmm" );  
	else if( diff < 120.0 && strncmp( units, "month", 5 )!= 0 ) strcpy( autom, "Mmmyy" ); 
	else if( diff < 1500.0 && strncmp( units, "year", 4 )!= 0 ) strcpy( autoy, "'yy" );
	}

else if( strcmp( stype, "time" )==0 ) {
	/* diff in minutes */
	if( diff < 1.5 ) { *inc = 0.166666666666; strcpy( dispfmt, "mm:ss" ); strcpy( nearest, "minute" ); }
	else if( diff < 12.0 ) { 
		*inc = 1.0; strcpy( dispfmt, "mm:ss" ); strcpy( nearest, "minute" ); 
		*minc = 0.166666666666; strcpy( munits, "minute" );
		}
	else if( diff < 30.0 ) { 
		*inc = 5.0; strcpy( dispfmt, "hh:mm" ); strcpy( nearest, "minute" ); 
		*minc = 1; strcpy( munits, "minute" );
		}
	else if( diff < 60.0 ) {
		*inc = 10.0; strcpy( dispfmt, "hh:mm" ); strcpy( nearest, "10minute" ); 
		*minc = 1.0; strcpy( munits, "minute" );
		}
	else if( diff < 180.0 ) {
		*inc = 30.0; strcpy( dispfmt, "hh:mm" ); strcpy( nearest, "30minute" ); 
		*minc = 10.0; strcpy( munits, "minute" );
		}
	else if( diff < 500.0 ) {
		*inc = 60.0; strcpy( dispfmt, "hhA" ); strcpy( nearest, "hour" ); 
		*minc = 30.0; strcpy( munits, "minute" );
		}
	else 	{
		*inc = 180.0; strcpy( dispfmt, "hhA" ); strcpy( nearest, "hour" ); 
		*minc = 60.0; strcpy( munits, "minute" );
		}
	}

return( 0 );
}


#endif
/* ======================== */
/* DATEFUNCTIONS - entry point for script date functions.
   Return 0 if function found and executed normally.
   Return 1 on error.
   Return 2 if function not found here.
 */

int
DT_datefunctions( hash, name, arg, nargs, result, typ )
int hash;
char *name;
char *arg[];
int nargs;
char *result;
int *typ;
{
int stat;

#ifdef NO_DT
  return( err( 199, "date functions not supported in this build", "" ) ); 
  }
#else

*typ = ALPHA;

if( hash < 1000 ) {
	
	if( hash == 402 ) goto TODAY;

	if( hash == 795 || hash == 665 ) { /* $daysdiff(d1,d2) - result is # of days between dates.  was: $datecmp() */
		long diff;
		stat = DT_daysdiff( arg[0], arg[1], &diff );
		if( stat != 0 ) { err( 1605, "$daysdiff error", "" ); return( 1 ); }
		sprintf( result, "%ld", diff );
		*typ = NUMBER;
		return( 0 );
		}
	
	
	if( hash == 992 ) { /* $datevalid( d ) */
		long x;
		stat = DT_jdate( arg[0], &x );
		if( stat == 0 ) sprintf( result, "1" );
		else sprintf( result, "0" );
		*typ = NUMBER;
		return( 0 );
		}
	
	
	if( hash == 492 ) { /* $julian(date) - return julian date */
		long jul;
		stat = DT_jdate( arg[0], &jul );
		if( stat != 0 ) { err( 1613, "$jdate error", arg[0] ); return( 1 ); }
		sprintf( result, "%ld", jul );
		*typ = NUMBER;
		return( 0 );
		}
	
	if( hash == 540 ) { /* $dateadd(date,ndays) - result is new date given date + ndays */
		stat = DT_dateadd( arg[0], atoi( arg[1] ), result );
		if( stat != 0 ) { err( 1608, "$dateadd error", "" ); return( 1 ); }
		return( 0 );
		}
	
	
	if( hash == 881 ) { /* $yearsold(birthdate,testdate) - result is # years age (whole number) 
	    		     * between birthdate and testdate */
		int nyears;
		stat = DT_yearsold( arg[0], arg[1], &nyears );
		if( stat != 0 ) { err( 1613, "$yearsold error", arg[0] ); return( 1 ); }
		sprintf( result, "%d", nyears );
		*typ = NUMBER;
		return( 0 );
		}
	}
else	{

	if( hash == 1053 ) { /* $jultodate(jul) - take a julian and return date in current format */
		stat = DT_fromjul( atol( arg[0]), result );
		if( stat != 0 ) { err( 1613, "$fromjul error", arg[0] ); return( 1 ); }
		return( 0 );
		}
	
	
	if( hash == 1215 ) { /* $dategroup( mode, date ) - take a date in current format and return
	   		      * it, adjusted to either midmonth, midweek, etc. for grouping purposes. */
	
		stat = DT_group( arg[0][0], arg[1][0], arg[2], result );
		if( stat != 0 ) { err( stat, "$dategroup error", arg[2] ); return( 1 ); }
		return( 0 );
		}
	
	if( hash == 1252 ) { /* $formatdate(date,fmt) - format the date, using fmt which is any of 
			      * the supported format strings) */
		stat = DT_formatdate( arg[0], arg[1], result );
		if( stat != 0 ) { err( 1611, "$formatdate failed", arg[1] ); return( 1 ); }
		return( 0 );
		}
	
	if( hash == 1352 ) { /* $setdatefmt(fmt) - set the date format.  fmt may be any of the supported format strings. */
		stat = DT_setdatefmt( arg[0] );
		if( stat != 0 ) { err( 1609, "$setdatefmt: invalid date format", arg[0] ); return( 1 ); }
		sprintf( result, "0" );
		*typ = NUMBER;
		return( 0 );
		}
	
	
	if( hash == 1983 ) { /* $setdateparms(parm,value) - set a date parameter.
	    		      * Currently the only date parameters that should be set this way are
			      * Pivotyear, Lazydates, Strictdatelengths */
		stat = DT_setdateparms( arg[0], arg[1] );
		if( stat != 0 ) { err( 1610, "$setdateparms error ", arg[0] ); return( 1 ); }
		sprintf( result, "0" );
		*typ = NUMBER;
		return( 0 );
		}
	
	
	if( hash == 1293 ) { /* $todaysdate() - result is today's date.  alias: $today()  */
		TODAY:
		stat = DT_todaysdate( result );
		return( 0 );
		}
	
	}

return( err( 199, "unrecognized function", name ) ); /* function not found */
}
#endif


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */



/* DATETIME - datetime routines */
/* #include <stdio.h>
 * #include <string.h>
 * #include <ctype.h>
 */

extern int DT_settimefmt(), DT_tomin(), DT_frommin(), DT_formattime(), DT_frame_mins();

/* extern int DT_setdatetimefmt(), GL_getchunk(), DT_setdatefmt(), DT_settimefmt();
 * extern int DT_datetime2days(), DT_jdate(), DT_tomin(), DT_frame_mins(), TDH_err(), DT_days2datetime(), DT_fromjul(), DT_frommin();
 * extern int DT_formatdatetime(), DT_formatdate(), DT_formattime();
 * extern int atoi();
 */


static double Dtwinbegin = 0.0, Dtwinend = 1440.0, Dtwinsize = 1440.0; /* default: 24 hours */
static int Suppress_twin_warn = 0;
static char Dtsep[4] = "."; /* must be one character only */  /* added scg 9/26/03 */

/* ================================== */
int
DT_datetime_initstatic()
{
Dtwinbegin = 0.0;
Dtwinend = 1440.0;
Dtwinsize = 1440.0;
Suppress_twin_warn = 0;
strcpy( Dtsep, "." );
return( 0 );
}

/* ================================== */
/* SETDATETIMEFMT - set the format for the date and time 
   	components of datetime */
int
DT_setdatetimefmt( fmt, window )
char *fmt;
char *window; /* Specifies a 'window' of hours to be shown for each day.
		 Notation nn-mm where nn is hour to begin with and mm is 
		hour to end with (0-24) */
{
char datepart[20], timepart[20];
char beginpart[20], endpart[20];
int i;

if( fmt[0] != '\0' ) {  /* condition added scg 9/29/03 */
	i = 0;
	GL_getchunk( datepart, fmt, &i, Dtsep );
	GL_getchunk( timepart, fmt, &i, Dtsep );
	DT_setdatefmt( datepart );
	DT_settimefmt( timepart );
	}

if( window[0] != '\0' ) { /* parse 'window' specification */
	i = 0;
	GL_getchunk( beginpart, window, &i, "-" );
	GL_getchunk( endpart, window, &i, "-" );
	Dtwinbegin = atoi( beginpart ) * 60.0;
	Dtwinend = atoi( endpart ) * 60.0;
	Dtwinsize = (Dtwinend - Dtwinbegin);
	}
if( window[0] == '\0' || Dtwinsize == 0.0 ) {
	Dtwinbegin = 0.0;
	Dtwinend = 1440.0;
	Dtwinsize = 1440.0;
	}

return( 0 );
}

/* ================================== */
/* GETDTPARTS - split out the date and time portions of a datetime value */
/* note - this should only be called when w/ 'datetime' scaletype, never w/ 'date' (messes up w/ formats such as dd.mm.yy) */

int
DT_getdtparts( dt, datepart, timepart )
char *dt, *datepart, *timepart;
{
int i;
i = 0;
GL_getchunk( datepart, dt, &i, Dtsep );
GL_getchunk( timepart, dt, &i, Dtsep );
return( 0 );
}


/* ================================== */
/* DATETIME2DAYS - convert a datetime to julian date with 
	decimal portion representing the time component.
   	Returns 0 if ok, 1 on error */

int
DT_datetime2days( dt, days )
char *dt;
double *days;
{
char datepart[20], timepart[20];
long jul;
int i, stat;
double mins;

i = 0;
GL_getchunk( datepart, dt, &i, Dtsep );
GL_getchunk( timepart, dt, &i, Dtsep );
stat = DT_jdate( datepart, &jul );
if( stat != 0 ) return( 1 );

if( strcmp( timepart, "" )==0 ) mins = Dtwinbegin; /* added scg 9/26/03 */
else 	{
	stat = DT_tomin( timepart, &mins );
	if( stat != 0 ) return( 1 );
	}

stat = DT_frame_mins( &mins );
if( stat && !Suppress_twin_warn ) TDH_err( 4279, "warning: time is outside of window range- displaying it at window boundary", dt );

mins = (mins-Dtwinbegin) / Dtwinsize; /* normalize to  0.0 - 1.0 */
*days = jul+mins;
return( 0 );
}


/* ================================== */
/* DAYS2DATETIME - convert decimal julian date to datetime */

int
DT_days2datetime( days, dt )
double days;
char *dt;
{
char s[30], t[30];
double datepart, timepart, floor(), fmod();

datepart = floor( days );
timepart = days - datepart;

timepart = (timepart * Dtwinsize) + Dtwinbegin; /* convert from 0.0 - 1.0 to #min past midnite */
/* the following bug fix from treykinkead@yahoo.com...  scg 10/10/08 */
/* timepart = floor( timepart + 0.5 ); */ /* remove any rounding error introduced by above */
timepart = floor( timepart*60.0+0.5 ) / 60.0; /* remove any rounding error introduced by above */

DT_fromjul( (long)datepart, s );
DT_frommin( timepart, t );


sprintf( dt, "%s%s%s", s, Dtsep, t );
return( 0 );
}

/* ================================== */
/* FORMATDATETIME - convert a datetime from current format 
	to a new format for display.  Either the date or
	time component may be omitted for display purposes.
*/

int
DT_formatdatetime( dt, format, newdt )
char *dt, *format, *newdt;
{
char datepart[20], timepart[20];
char datefmt[20], timefmt[20];
char s[30], t[30];
int i;

i = 0;
strcpy( datefmt, "" );
strcpy( timefmt, "" );
if( format[0] != Dtsep[0] && tolower(format[0]) != 'h' ) GL_getchunk( datefmt, format, &i, Dtsep );
GL_getchunk( timefmt, format, &i, Dtsep );

i = 0;
GL_getchunk( datepart, dt, &i, Dtsep );
GL_getchunk( timepart, dt, &i, Dtsep );

if( datefmt[0] != '\0' ) {
	DT_formatdate( datepart, datefmt, s );
	}
else strcpy( s, "" );
if( timefmt[0] != '\0' ) {
	if( datefmt[0] != '\0' )strcpy( t, Dtsep );	
	else strcpy( t, "" );
	DT_formattime( timepart, timefmt, &t[strlen(t)] );
	}
else strcpy( t, "" );

sprintf( newdt, "%s%s", s, t );

return( 0 );
}

/* ================================= */
/* FRAME_MINS - if mins is outsize window, adjust mins to a window boundary */

int
DT_frame_mins( mins )
double *mins;
{
if( *mins < Dtwinbegin || *mins > Dtwinend ) {
	if( *mins < Dtwinbegin ) *mins = Dtwinbegin;
	if( *mins > Dtwinend ) *mins = Dtwinend;
	return( 1 ); /* indicates an adjustment had to be made.. */
	}
return( 0 );
}

/* ================================= */
/* SUPPRESS_TWIN_WARN - allow warning message 'value outside datetime
	window' to be suppressed */

int
DT_suppress_twin_warn( mode )
int mode;
{
Suppress_twin_warn = mode;
return( 0 );
}

/* ================================== */
/* GETWIN - return the date window size in hours */

int
DT_getwin( winsize  )
double *winsize;
{
*winsize = Dtwinsize / 60.0;
return( 0 );
}

/* =================================== */
/* BUILD_DT - build a datetime string from a date part and a time part */

int
DT_build_dt( datepart, timepart, result )
char *datepart, *timepart, *result;
{
if( timepart[0] == '\0' ) strcpy( result, datepart ); 		/* scg 1/28/05 */
else sprintf( result, "%s%s%s", datepart, Dtsep, timepart );
return( 0 );
}

/* =================================== */
/* SETDTSEP - set the datetime separator character */

int
DT_setdtsep( c )
char c;
{
sprintf( Dtsep, "%c", c );
return( 0 );
}

/* =================================== */
/* GETDTSEP - get the datetime separator character */

int
DT_getdtsep( sep )
char *sep;
{
strcpy( sep, Dtsep );
return( 0 );
}


#ifdef TESTING
/* ================================== */
main()
{
double min;
char result[80];
char result2[80];

DT_tomin( "now", &min );
DT_frommin( min, result );
DT_formattime( result, "hh:mmA", result2 );
printf( "%s\n", result2 );
}
#endif


/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
