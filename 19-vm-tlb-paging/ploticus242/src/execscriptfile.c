/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"
#include "tdhkit.h"

/* =================================================== */
/* EXEC_SCRIPTFILE - execute a script file using midriff script interpreter.
 * Returns 0 if ok, or a non-zero error code 
 */

int
PL_exec_scriptfile( scriptfile )
char *scriptfile;
{
int ix, stat, j;
struct sinterpstate ss; 
char buf[ SCRIPTLINELEN ];
char firsttok[80];
char tok[80];
long loc;
char tmpchar;

/* open spec file.. */
stat = TDH_sinterp_open( scriptfile, &ss );
if( stat != 0 ) return( Eerr( 22, "Cannot open specified scriptfile", scriptfile ) );
	
if( PLS.debug ) { fprintf( PLS.diagfp, "Script file successfully opened\n" ); fflush( PLS.diagfp ); }

/* read through script file using sinterp script interpreter.. */
while( 1 ) {
	stat = TDH_sinterp( buf, &ss, "", NULL );
	if( stat == 1226 ) {
        	err( stat, "required variable(s) missing", buf );
        	break;
        	}

        if( stat > 255 ) return( Eerr( 23, "Fatal script processing error.", "" ) );
	else if( PLS.skipout ) { /* error that is fatal for this script */
		PLS.skipout = 0;
		return( 1 );
		}
        else if( stat != SINTERP_MORE ) break;

	ix = 0; 

	tmpchar = buf[40];  buf[40] = '\0';  /* careful.. there could be long lines w/ no whitespace, eg. long csv lines .. scg 10/25/07 */
	strcpy( firsttok, GL_getok( buf, &ix ) ); /* check first token on line.. */
	buf[40] = tmpchar; /* restore */

	/* check for #proc trailer.. if encountered we're done.. */
	if( strcmp( firsttok, "#proc" )==0 && strcmp( GL_getok( buf, &ix ), "trailer" )==0 ) break; 

	/* handle midriff "secondary" ops such as #shell, #sql, #write... */
	if( ss.doingshellresult == 0 && ss.doingsqlresult == 0 && 
	    firsttok[0] == '#' && firsttok[1] != '#' && !GL_slmember( firsttok, "#proc* #endproc #clone* #ifspec* #saveas* #intrailer" ) ) {
            	while( 1 ) {
			stat = TDH_secondaryops( buf, &ss, "", NULL ); /* may call sinterp() to get lines*/
			if( stat > 255 ) return( Eerr( 24, "Fatal script processing error.", "" ) );
        		else if( stat != SINTERP_MORE ) break;
			if( ss.writefp != NULL ) fprintf( ss.writefp, "%s", buf );
			}
		if( stat == SINTERP_END_BUT_PRINT ) {
			if( ss.writefp != NULL ) fprintf( ss.writefp, "%s", buf );
			}
		continue;
		}
	if( ss.writefp != NULL ) {
		fprintf( ss.writefp, "%s", buf ); 
		continue;
		}

	if( strncmp( firsttok, "#intrailer", 10 )==0 ) {
		/* remember current location in control file, then scan forward until we reach 
		   #proc trailer.  Then get the lines there.  Then seek back to where we left off. 
		   This cannot be used from within an #include.  */
		loc = ftell( ss.sfp[0] );
		while( fgets( buf, SCRIPTLINELEN-1, ss.sfp[0] ) != NULL ) {
			buf[78] = '\0';  /* careful ... long csv lines possible - scg 10/25/07 */
			strcpy( tok, "" );
			sscanf( buf, "%s %s", firsttok, tok );
			if( strcmp( firsttok, "#proc" ) ==0 && strcmp( tok, "trailer" )==0 ) break;
			}
		while( fgets( buf, SCRIPTLINELEN-1, ss.sfp[0] ) != NULL ) {
			/* check for comment lines.. if found skip them.. */
			for( j = 0; !isspace((int)buf[j] ); j++ ); /* skip leading ws */
			if( strncmp( &buf[j], "//", 2 )==0 ) continue; /* skip comments */

			PL_execline( buf );
			/* no use checking return status.. */
			}

		fseek( ss.sfp[0], loc, 0 /*SEEK_SET BSD*/ );  /* now go back.. */
		continue;
		}

	PL_execline( buf );
	/* no use checking return status.. */

	if( PLS.skipout ) break;
	}

strcpy( buf, "#endproc" );
PL_execline( buf ); /* this causes last-encountered proc to be executed.. */
/* no use checking return status.. */

if( ss.sfp[0] != NULL ) fclose( ss.sfp[0] );
return( 0 );
}


/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
