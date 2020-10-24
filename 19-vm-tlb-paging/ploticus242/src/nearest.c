/* ======================================================= *
 * Copyright 1998-2006 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"

/* Given smin, smax, and nearest, determine minval and maxval.
   Smin and Smax are string reps of numerics, dates, times, etc.
   Pass identical smin and smax to get a $dategroup() or $numgroup() operation.

   Function returns 1 if the 'nearest' mode was recognized and a nearest range computed.
   Function returns 0 otherwise.
 */
int
PLP_findnearest( smin, smax, axis, nearest, minval, maxval )
char *smin, *smax, axis, *nearest, *minval, *maxval;
{
char datepart[40], timepart[40], unittyp[40];

Egetunits( axis, unittyp );

if( strncmp( nearest, "month", 5 )== 0 || strncmp( nearest, "quarter", 7 )==0 || strncmp( nearest, "3month", 6 )==0 ) { 
	/* nearest month boundary / quarter-year boundary.. */
	int mon, day, yr, newmon;
	long l;
	if( !GL_smember( unittyp, "date datetime" )) 
		Eerr( 2892, "autorange 'nearest=month' or 'nearest=quarter' only valid with date or datetime scaletype", unittyp );
	/* min */
	DT_jdate( smin, &l );
        DT_getmdy( &mon, &day, &yr );
	if( nearest[0] == 'q' || nearest[0] == '3' ) {
		if( mon >= 10 ) mon = 10;
		else if( mon >= 7 ) mon = 7;
		else if( mon >= 4 ) mon = 4;
		else if( mon >= 1 ) mon = 1;
		}
	DT_makedate( yr, mon, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		DT_build_dt( datepart, timepart, minval );
		}
	else strcpy( minval, datepart );

	/* max */
	DT_jdate( smax, &l );
        DT_getmdy( &mon, &day, &yr );
	if( nearest[0] == 'q' || nearest[0] == '3' ) {
		if( mon <= 3 ) mon = 4;
		else if( mon <= 6 ) mon = 7;
		else if( mon <= 9 ) mon = 10;
		else if( mon <= 12 ) mon = 13;
		}
	else mon ++;

        /* wrap around year.. */
        newmon = ((mon-1) % 12 ) +1;
        yr += ((mon-1) / 12);
        mon = newmon;
	DT_makedate( yr, mon, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		DT_build_dt( datepart, timepart, maxval );
		}
	else strcpy( maxval, datepart );
	return( 1 );
	}


else if( strncmp( nearest, "year", 4 )== 0 || strncmp( nearest, "2year", 5 )==0 || 
	strncmp( nearest, "5year", 5 )==0 || strncmp( nearest, "10year", 6 )==0 ) { 
	int mon, day, yr;
	long l;
	int yearsblock; /* 0 5 or 10 */

	if( !GL_smember( unittyp, "date datetime" )) 
		Eerr( 2892, "autorange 'nearest=year' only valid with date or datetime scaletype", unittyp );

	if( nearest[0] != 'y' ) {			/* this section scg 1/28/05 */
		yearsblock = nearest[0] - '0';
		if( yearsblock == 1 ) yearsblock = 10;
		}
	else yearsblock = 0;

	/* min */
	DT_jdate( smin, &l );
        DT_getmdy( &mon, &day, &yr );
	if( yearsblock ) yr = (yr / yearsblock) * yearsblock; 		/* scg 1/28/05 */
	DT_makedate( yr, 1, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		DT_build_dt( datepart, timepart, minval );
		}
	else strcpy( minval, datepart );
		
	/* max */
	DT_jdate( smax, &l );
        DT_getmdy( &mon, &day, &yr );
	if( yearsblock ) yr = ((yr / yearsblock)+1) * yearsblock; 		/* scg 1/28/05 */
	else yr++;
	DT_makedate( yr, 1, 1, "", datepart );
	if( strcmp( unittyp, "datetime" )==0 ) {
		DT_maketime( 0, 0, 0.0, timepart );
		DT_build_dt( datepart, timepart, maxval );
		}
	else strcpy( maxval, datepart );
	return( 1 );
	}


else if( strncmp( nearest, "day", 3 )== 0 || strcmp( nearest, "monday" )==0 || strcmp( nearest, "sunday" )==0 ) { 
	int mon, day, yr;
	double days, mins;

	if( !GL_smember( unittyp, "date datetime" )) 
		Eerr( 2892, "autorange 'nearest=day' only valid with date or datetime scaletype", unittyp );

	/* min */
	if( strcmp( unittyp, "datetime" )==0 ) DT_getdtparts( smin, datepart, timepart ); 
	else strcpy( datepart, smin ); /* if and else added scg 8/10/05 */

	if( nearest[0] == 'm' || nearest[0] == 's' ) {  /* adjust datepart back to a monday or sunday */
		int iwk;  char rbuf[40];
		DT_weekday( datepart, rbuf, &iwk ); /* rbuf not used */
		if( nearest[0] == 'm' ) { if( iwk == 1 ) iwk = 8; DT_dateadd( datepart, 2 - iwk, rbuf ); }
		else if( nearest[0] == 's' ) DT_dateadd( datepart, 1 - iwk, rbuf );
		strcpy( datepart, rbuf );
		}

	/* this is just a way to get the dt parts (?) ...
	 * DT_datetime2days( smin, &days ); 
         * DT_getmdy( &mon, &day, &yr );
	 * DT_makedate( yr, mon, day, "", datepart );
	 */

	if( strcmp( unittyp, "date" )==0 ) 
	/* check for biz day window.. scg 7/21/04 */
	mins = 0.0;
	DT_frame_mins( &mins ); /* adjust to any biz day window.. */
	DT_frommin( mins, timepart ); 
	if( strcmp( unittyp, "date" )==0 ) strcpy( minval, datepart );
	else DT_build_dt( datepart, timepart, minval );

	/* max */
	if( strcmp( unittyp, "datetime" )==0 ) DT_getdtparts( smax, datepart, timepart );
	else strcpy( datepart, smax );  /* if and else added scg 8/10/05 */

	if( nearest[0] == 'm' || nearest[0] == 's' ) {  /* adjust datepart to next monday or sunday */
		int iwk;  char rbuf[40];
		DT_weekday( datepart, rbuf, &iwk ); /* rbuf not used */
		if( nearest[0] == 'm' ) { if( iwk == 1 ) iwk = 8; DT_dateadd( datepart, 9 - iwk, rbuf ); }
		else if( nearest[0] == 's' ) DT_dateadd( datepart, 8 - iwk, rbuf );
		DT_build_dt( rbuf, timepart, smax );
		}

	DT_datetime2days( smax, &days );
	if( fabs( days - floor( days )) < 0.0001 ) ; /* avoid spurious extra day when data max is on date boundary added scg 7/21/04 */
	else days++;
	DT_days2datetime( days, smax ); 
	DT_datetime2days( smax, &days ); /* set next day's date for getmdy below */
        DT_getmdy( &mon, &day, &yr );
	DT_makedate( yr, mon, day, "", datepart );
	DT_maketime( 0, 0, 0.0, timepart );
	if( strcmp( unittyp, "date" )==0 ) strcpy( maxval, datepart );
	else DT_build_dt( datepart, timepart, maxval );
	return( 1 );
	}

else if( strncmp( nearest, "hour", 4 )== 0 || strncmp( nearest, "3hour", 5 )==0 ||
	 strncmp( nearest, "6hour", 5 )==0 || strncmp( nearest, "12hour", 6 )==0 ) {
	int hour, minute;
	double sec;
	int hoursblock; /* 0, 3, 6, or 12 */
	if( !GL_smember( unittyp, "time datetime" )) 
		Eerr( 2892, "autorange 'nearest=hour' is incompatible with scaletype", unittyp );

	if( nearest[0]!= 'h' ) {				/* this section scg 1/28/05 */
		hoursblock = nearest[0] - '0';
		if( hoursblock == 1 ) hoursblock = 12;
		}
	else hoursblock = 0;

	if( strcmp( unittyp, "time" )==0 ) {
		/* min */
		DT_tomin( smin, &sec ); /* sec not used */
		DT_gethms( &hour, &minute, &sec );
		if( hoursblock ) hour = (hour / hoursblock) * hoursblock; 		/* scg 1/28/05 */
		DT_maketime( hour, 0, 0.0, minval );
		/* max */
		DT_tomin( smax, &sec ); /* sec not used */
		DT_gethms( &hour, &minute, &sec );
		if( hoursblock ) hour = ((hour / hoursblock)+1) * hoursblock; 		/* scg 1/28/05 */
		if( minute != 0 || sec != 0.0 ) hour++; 				/* bug, scg 12/13/05 */
		DT_maketime( hour, 0, 0.0, maxval ); 
		}
	else if( strcmp( unittyp, "datetime" )==0 ) {
		double days;
		int mon, day, yr;

		/* min */
		DT_datetime2days( smin, &days );
		/* time part */
		DT_gethms( &hour, &minute, &sec );
		if( hoursblock ) hour = (hour / hoursblock) * hoursblock; 		/* scg 1/28/05 */
		DT_maketime( hour, 0, 0.0, timepart );
		/* date part */
        	DT_getmdy( &mon, &day, &yr );
		DT_makedate( yr, mon, day, "", datepart );
		DT_build_dt( datepart, timepart, minval );

		/* max */
		DT_datetime2days( smax, &days );
		/* time part */
		DT_gethms( &hour, &minute, &sec );
		if( hour == 23 ) {
			DT_days2datetime( days+1.0, smax ); /* set next day's date for getmdy below*/
			DT_datetime2days( smax, &days ); /* set next day's date for getmdy below*/
			DT_maketime( 0, 0, 0.0, timepart );					/* ok for any hoursblock */
			}
		else 	{
			if( hoursblock ) hour = ((hour / hoursblock)+1) * hoursblock; 		/* scg 1/28/05 */
			else hour++;
			DT_maketime( hour, 0, 0.0, timepart );
			}
		/* date part */
        	DT_getmdy( &mon, &day, &yr );
		DT_makedate( yr, mon, day, "", datepart );
		DT_build_dt( datepart, timepart, maxval );
		}
	return( 1 );
	}

/* else if( strcmp( nearest, "minute" )==0 || strcmp( nearest, "10minute" )==0 || 
 *	strcmp( nearest, "20minute" )==0 || strcmp( nearest, "30minute" )==0 ) {		
 */
else if( stricmp( nearest, "minute" )==0 ||	/* handles  minute  or  Nminute or NNminute, contributed by Chris Demetriou 1/9/08 */
         ( nearest[0] >= '0' && nearest[0] <= '9' && stricmp( nearest + 1, "minute" )==0 ) ||
         ( nearest[0] >= '0' && nearest[0] <= '9' && nearest[1] >= '0' && nearest[1] <= '9' && stricmp( nearest + 2, "minute" )==0 ) ) {

	int hour, minute, minblock;
	double sec;

	if( strcmp( unittyp, "time" )!= 0 ) Eerr( 2892, "autorange 'nearest=minute' is incompatible with scaletype", unittyp );

	/* if( nearest[0] != 'm' ) minblock = (nearest[0] - '0') * 10;  // changed 1/9/08 */
        if( tolower( (int) nearest[0] ) != 'm' ) {
               minblock = atoi(nearest);
               if( minblock < 0 || minblock >= 60 || ( 60 % minblock )!=0 ) {
                       Eerr( 2892 /*???*/, "invalid number of minutes in 'nearest' autorange specification", nearest );
                       }
               }
	else minblock = 0;

	/* min */
	DT_tomin( smin, &sec ); /* sec not used */
	DT_gethms( &hour, &minute, &sec );
	if( minblock ) minute = (minute / minblock) * minblock; 		
	DT_maketime( hour, minute, 0.0, minval );

	/* max */
	DT_tomin( smax, &sec ); /* sec not used */
	DT_gethms( &hour, &minute, &sec );
	if( minblock ) minute = ((minute / minblock)+1) * minblock; 	
	else minute++;
	if( minute >= 60 ) { minute = minute % 60; hour++; }
	DT_maketime( hour, minute, 0.0, maxval );
	return( 1 );
	}


else if( stricmp( nearest, "second" )==0 ||   /* handles  second or  Nsecond or NNsecond, contributed by Chris Demetriou 1/9/08 */
         ( nearest[0] >= '0' && nearest[0] <= '9' && stricmp( nearest + 1, "second" )==0 ) ||
         ( nearest[0] >= '0' && nearest[0] <= '9' && nearest[1] >= '0' && nearest[1] <= '9' && stricmp( nearest + 2, "second" )==0 ) ) {
       int hour, minute, isec, secblock;
       double sec;

       if( strcmp( unittyp, "time" )!= 0 ) Eerr( 2892, "autorange 'nearest=second' is incompatible with scaletype", unittyp );

       if( tolower( (int) nearest[0] ) != 's' ) {
               secblock = atoi(nearest);
               if( secblock < 0 || secblock >= 60 || ( 60 % secblock )!=0 ) {
                       Eerr( 2892 /*???*/, "invalid number of minutes in 'nearest' autorange specification", nearest );
                       }
               }
       else secblock = 0;

       /* min */
       DT_tomin( smin, &sec );
       DT_gethms( &hour, &minute, &sec );
       isec = (int)floor(sec);
       if( secblock ) isec = (isec / secblock) * secblock;
       DT_maketime( hour, minute, isec * 1.0, minval );

       /* max */
       DT_tomin( smax, &sec );
       DT_gethms( &hour, &minute, &sec );
       isec = (int)ceil(sec); /* will round up to "60" at most. */
       if( secblock ) isec = ((isec + secblock - 1) / secblock) * secblock;
       if( isec >= 60 ) { minute++; isec = 0; }
       if( minute >= 60 ) { minute = minute % 60; hour++; }
       DT_maketime( hour, minute, isec * 1.0, maxval );
       return( 1 );
       }


return( 0 );
}
