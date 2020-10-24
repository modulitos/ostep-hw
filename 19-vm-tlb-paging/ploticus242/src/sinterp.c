/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/*  
 *  Call sinterp_open() or sinterp_openmem() first.  Then repeatedly call sinterp().
 *  Continue while getting a return value of SINTERP_MORE; for any other return value, stop.
 *
 *  Returns SINTERP_MORE  = normal, more results to come
 *	     SINTERP_END  = no more results - eof
 *		0-19      = user 
 *              20 & up   = error  (revised 1/30/03)
 *
 *  Notes: 
 *    Any programs that use sinterp need a stub for customtextvect() 
 *
 *    data may be passed as NULL (recordid must be "")
 */
#include "tdhkit.h"

extern int TDH_shellresultrow(), TDH_shellclose(), TDH_sqlnames(), TDH_sqlrow(), TDH_dequote(), TDH_function_call();
extern int TDH_function_listsep(), TDH_condex_listsep(), TDH_errmode(), TDH_setshellfdelim();
extern int atoi(); /* sure thing */

#ifndef TDH_NOREC
  extern int sqlbuild0(), sqlbuild1(), TDH_sqltabdef();
#endif

#ifndef PLOTICUS
  extern int customforvect();
#endif

#ifndef TDH_DB
#define TDH_DB 0
#endif

#if TDH_DB != 0
  extern int TDH_sqlrow_nullrep();
#endif

static FILE *skiptoendloop();
static char *specialincludedir = "";

/* -------------------------------------- */

int
TDH_sinterp( line, ss, recordid, data )
char *line;	/* should be length of at least SCRIPTLINELEN */
struct sinterpstate *ss;
char *recordid;
char data[][DATAMAXLEN+1];
{
int i, j;
int stat;
int ix; 	
char buf[ SCRIPTLINELEN ], tok[ DATAMAXLEN+1];
long linebegin;
char str[ DATAMAXLEN+1 ];
char varname[40];
char list[ SCRIPTLINELEN ];
int len;
char conj[40];
char delimstr[5];
int typ;

TDH_dat = (char *)data;
TDH_recid = recordid;

while( 1 ) {
	if( ss->nmemrows > 0 ) goto MEMOPS;

	/* if an #shell dump is currently in progress, handle results.. */
	if( ss->doingshellresult != 0 ) {
		int nshfields;
		char *shfields[MAXITEMS];
		int delim;

		/* get a row.. */
		stat = TDH_shellresultrow( buf, shfields, &nshfields, SCRIPTLINELEN );
		if( stat != 0 ) {
			TDH_shellclose();
			ss->doingshellresult = 0;
			TDH_setshellfdelim( 0 ); /* reset shell delimiter.. added scg 8/3/06 */
			continue;
			}

		delim = ss->doingshellresult;
		if( delim == 's' ) continue; /* silent */
	
		/* stream output.. do delimitation processing */
		for( i = 0, j = 0; i < nshfields; i++ ) {
			if( delim == 'h' ) { strcpy( &line[j], "<td>" ); j+=4; }

			strcpy( &line[j], shfields[i] ); 
			j+= strlen( shfields[i] ); 

			if( delim == 'h' )  { strcpy( &line[j], "</td>" ); j+=5; }
			else if( delim == 't' ) { strcpy( &line[j], "\t" ); j+=1; }
			}
		if( delim == 'h' ) { strcpy( &line[j], "</tr>" ); j+=5; }
		strcpy( &line[j], "\n" );
		return( SINTERP_MORE );
		}


	/* if an #sql dump is currently in progress, handle results.. */
	if( ss->doingsqlresult != 0 ) {
		int nsqlfields, nsqlnames;
		char *sqlfields[MAXITEMS], *sqlnames[MAXITEMS];
		int delim;

		if( ss->doingsqlresult == 'l' ) {  /* load from 1st retrieved row.. */
			TDH_sqlnames( ss->dbc, sqlnames, &nsqlnames );

			stat = TDH_sqlrow( ss->dbc, sqlfields, &nsqlfields );

			if( stat == 0 ) for( i = 0; i < nsqlfields; i++ ) {
				if( stricmp( sqlfields[i], TDH_dbnull )==0 && ss->nullrep ) {
					if( ss->nullrep == 1 ) TDH_setvar( sqlnames[i], "" );
					else if( ss->nullrep == 2 ) TDH_setvar( sqlnames[i], DBNULL );
					else if( ss->nullrep == 3 ) TDH_setvar( sqlnames[i], "&nbsp;" );
					}
				else TDH_setvar( sqlnames[i], sqlfields[i] );
				}
			else 	{
				for( i = 0; i < nsqlfields; i++ ) TDH_setvar( sqlnames[i], "" );
				if( stat > 20 ) {
					ss->doingsqlresult = 0;
					return( err( stat, "error on sql load", "" ) );
					}
				}

			ss->doingsqlresult = 0;
			continue;
			}

		/* get a row.. */
		stat = TDH_sqlrow( ss->dbc, sqlfields, &nsqlfields );
		if( stat != 0 ) {
			ss->doingsqlresult = 0;
			if( stat > 20 ) return( err( stat, "error on sql row", "" ));
			continue;
			}
	
		delim = ss->doingsqlresult;
		if( delim == 's' ) continue; /* silent */
	
		/* stream output.. do delimitation processing */
		for( i = 0, j = 0; i < nsqlfields; i++ ) {
			if( delim == 'h' ) { strcpy( &line[j], "<td>" ); j+=4; }
			if( stricmp( sqlfields[i], TDH_dbnull )==0 && ss->nullrep ) {
				if( ss->nullrep == 2 ) { strcpy( &line[j], DBNULL ); j+=4; }
				else if( ss->nullrep == 3 ) { strcpy( &line[j], "&nbsp;" ); j+= 6; }
				}
			else 	{ strcpy( &line[j], sqlfields[i] ); j+= strlen( sqlfields[i] ); }

			if( delim == 'h' )  { strcpy( &line[j], "</td>" ); j+=5; }
			else if( delim == 't' ) { strcpy( &line[j], "\t" ); j+=1; }
			}
		if( delim == 'h' ) { strcpy( &line[j], "</tr>" ); j+=5; }
		strcpy( &line[j], "\n" );
		return( SINTERP_MORE );
		}

#ifndef TDH_NOREC
	/* if a #sqlbuild is currently in progress, handle results.. */
	if( ss->sqlbuildi > 0 ) {
		stat = sqlbuild1( line, ss );
		if( stat != 0 ) {
			ss->sqlbuildi = 0;
			return( err( stat, "error in sqlbuild", "" ));
			}
		return( SINTERP_MORE );
		}
#endif


	/* --------------------------------------------- */

	/* otherwise, read next line of script.. */
	linebegin = ftell( ss->sfp[ ss->incnest ] );  /* remember loc before the read.. */
	if( fgets( line, SCRIPTLINELEN-1, ss->sfp[ ss->incnest ] ) == NULL ) {
		fclose( ss->sfp[ ss->incnest ] ); 
		ss->sfp[ ss->incnest ] = NULL; 
		if( ss->incnest > 0 ) {
			(ss->incnest)--;
			continue;
			}
		else return( SINTERP_END );
		}

	/* or.. get next in-memory script row.. */
	MEMOPS:
	if( ss->nmemrows > 0 ) {
		if( ss->mrow >= ss->nmemrows ) return( SINTERP_END );
		sprintf( line, "%s\n", *(ss->memrows) );
		(ss->mrow)++;
		(ss->memrows)++;
		/* fprintf( stderr, "%s", line ); */
		}
	

 	/* get first token.. */
	ix = 0;
	strcpy( tok, GL_getok( line, &ix ) );

        if( strncmp( tok, "//", 2 )==0 ) continue; /* comment */

	/* remove trailing newline.. */
        /* line[ strlen( line ) -1 ] = '\0';  */
 	len = strlen( line );
        line[ len-1 ] = '\0';  len--;
        if( line[ len-1 ] == 13 ) { line[ len-1 ] = '\0';  len--; } /* DOS LF */


	/* for conditional expressions and assignments, convert quoted strings.
	   for all other lines, evaluate items. */
	if( ss->evalvars ) {
		if( GL_smember( tok, "#if #elseif #set #call #setifnotgiven" ))  /* did include #sqlbuild */
			TDH_dequote( buf, line, "SL" ); 
		else TDH_value_subst( buf, line, data, recordid, NORMAL, 0 );  
		strcpy( line, buf );
		}

	/* quick return for non-op lines .. */
	if( tok[0] != '#' ) {
		PUT:
		/* check if-logic display flags .. if any are 0 then don't display */
		for( i = 1; i <= ss->ifnest; i++ ) if( ! ss->disp[i] ) break;
		if( i != (ss->ifnest)+1 ) continue;

		/* add trailing newline unless \c  */
		len = strlen( line );

		if( len >= 2 && line[ len-2 ] == '\\' && line[ len-1 ] == 'c' ) {
		 	line[ len-2 ] = '\0';
		 	return( SINTERP_MORE );
		 	}
		line[ len ] = '\n'; line[ len+1 ] = '\0';
		return( SINTERP_MORE );
		}


	/* ----------------------------------------------------- */
	/* if-logic operators begin here.  Each chunk must end with a 'continue' */

	if( strcmp( tok, "#if" )==0 ) {
		if( (ss->ifnest)+1 >= IFNESTMAX ) return( err( 1220, "#if nest level exceeded", "" ) ); 
		(ss->ifnest)++;
		/* if parent disp flag is 0 don't evaluate condex.. */
		if( ss->ifnest > 1 && ss->disp[ (ss->ifnest)-1 ] == 0 ) stat = 0;
		else stat = TDH_condex( &line[ix], 1 );
		ss->condmet[ ss->ifnest ] = ss->disp[ ss->ifnest ] = stat;
		continue;
		}
	else if( strcmp( tok, "#endif" )==0 ) {
		ss->condmet[ ss->ifnest ] = 0;
		ss->disp[ ss->ifnest ] = 1;
		if( ss->ifnest > 0 ) (ss->ifnest)--;
		else return( err( 1264, "extra #endif", "" ) );
		continue;
		}
	else if( strcmp( tok, "#elseif" )== 0 ) {
		/* if parent disp flag is 0 don't evaluate condex.. */
		if( ss->ifnest > 1 && ss->disp[ (ss->ifnest)-1 ] == 0 ) ss->disp[ ss->ifnest ] = 0;
		else if( !ss->condmet[ ss->ifnest ] ) {
			stat = TDH_condex( &line[ix], 1 );
			ss->condmet[ ss->ifnest ] = ss->disp[ ss->ifnest ] = stat;
			}
		else ss->disp[ ss->ifnest ] = 0;
		continue;
		}
	else if( strcmp( tok, "#else" )==0 ) {
		/* if parent disp flag is 0 don't evaluate condex.. */
		sscanf( &line[ix], "%s", tok );
		if( strcmp( tok, "if" )==0 ) err( 1219, "#else if: invalid syntax. Use #elseif", "" );
		if( ss->ifnest > 1 && ss->disp[ (ss->ifnest)-1 ] == 0 ) ss->disp[ ss->ifnest ] = 0;
		else if( !ss->condmet[ ss->ifnest ] ) ss->disp[ ss->ifnest ] = 1;
		else ss->disp[ ss->ifnest ] = 0;
		continue;
		}


	/* ----------------------------------------------------- */
	/* check if-logic display flags .. if any are 0 then continue.. */
	for( i = 1; i <= ss->ifnest; i++ ) if( ! ss->disp[i] ) break;
	if( i != (ss->ifnest)+1 ) continue;


	/* ----------------------------------------------------- */
	/* standard operators other than if-logic ones begin here.  
         * Each chunk must end with a 'continue'                 */

	if( strcmp( tok, "#+" )==0 || strcmp( tok, "#print" )==0 ) {    /* #print is used by ploticus getdata filters (only?) */
		strcpy( line, &line[ix+1] );
		goto PUT;
		}

	if( strcmp( tok, "#set" )==0 || strcmp( tok, "#setifnotgiven" )==0 ) {
		strcpy( varname, GL_getok( line, &ix ) );
		if( strlen( tok ) > 4 ) {  /* setifnotgiven */
			stat = TDH_getvar( varname, str );
			if( stat != 0 || str[0] == '\0' ) ;
			else continue;
			/* if( stat == 0 ) continue; */
			}
		GL_getok( line, &ix ); /* skip '=' */
		while( line[ix] == ' ' ) ix++; /* skip over white space.. */

		if( line[ix] == '@' ) {  /* variable(s) (as supplied or from "string").. take everything to eol */
			TDH_valuesubst_settings( "omitws", 1 );
			TDH_value_subst( buf, &line[ix], data, recordid, NORMAL, 0 );
			TDH_valuesubst_settings( "omitws", 0 );
        		stat = TDH_setvalue( varname, buf, data, recordid );
			if( stat != 0 ) return( stat );
        		}
		else if( line[ix] == '$' ) {   /* a standalone function.. */
			strcpy( buf, &line[ix] );
                	stat = TDH_function_call( buf, &typ, 1 );
			if( stat != 0 ) err( 1201, "function error", buf );
                	if( buf[ strlen(buf)-1 ] == ' ' ) buf[ strlen(buf)-1 ] = '\0'; /* forced alpha */
                	stat = TDH_setvalue( varname, buf, data, recordid );
			if( stat != 0 ) return( stat );
                	}

		else 	{   /* value, e.g. numeric (single token) */
			strcpy( tok, "" );
			sscanf( line, "%*s %*s %*s %s", tok );
			stat = TDH_setvalue( varname, tok, data, recordid );
			if( stat != 0 ) return( stat );
			}

		continue;
		}

	if( strcmp( tok, "#call" )==0 ) {
		while( line[ix] == ' ' ) ix++; /* skip over white space.. */
		strcpy( buf, &line[ix] );
                stat = TDH_function_call( buf, &typ, 1 );
		if( stat != 0 ) err( 1201, "function error", buf );
                if( buf[ strlen(buf)-1 ] == ' ' ) buf[ strlen(buf)-1 ] = '\0'; /* forced alpha */
		continue;
		}

	if( strcmp( tok, "#exit" )==0 ) {
		strcpy( tok, "" );
		sscanf( line, "%*s %s", tok );
		stat = atoi( tok );
		/* close all open script files.. */
		for( i = 0; i < ss->incnest; i++ ) {
			fclose( ss->sfp[ i ] ); 
			ss->sfp[ i ] = NULL; 
			}
		ss->ifnest = 0;
		ss->loopnest = 0;
		if( stat >= 0 && stat <= 255 ) return( atoi( tok ) ); /* should be 0 - 255 */
		else return( 0 );
		}

	if( strcmp( tok, "#declare" )==0 ) continue; /* do nothing */

	/* in-memory scripts don't support any other ops.. */
	if( ss->nmemrows > 0 ) {
		err( 1270, "directive not supported in mem mode", tok );
		continue;
		}

	if( strcmp( tok, "#include" )==0 ) {
		char incfile[ MAXPATH ];
		sscanf( line, "%*s %s", incfile );
		if( incfile[0] == '$' ) {
			strcpy( tok, incfile );
			sprintf( incfile, "%s%c%s", specialincludedir, PATH_SLASH, &tok[1] );
			}
		if( (ss->incnest)-1 < INCNESTMAX ) {
			(ss->incnest)++;
			/* first try scriptdir.. */
			sprintf( buf, "%s%c%s", TDH_scriptdir, PATH_SLASH, incfile );
			ss->incifnest[ ss->incnest ] = ss->ifnest;
			ss->incloopnest[ ss->incnest ] = ss->loopnest;
			ss->sfp[ ss->incnest ] = fopen( buf, "r" );
			if( ss->sfp[ ss->incnest ] == NULL ) {

				/* then try name as is.. */
				ss->sfp[ ss->incnest ] = fopen( incfile, "r" );
				if( ss->sfp[ ss->incnest ] == NULL ) {
					(ss->incnest)--; 
					return( err( 1221, "cannot open #include file", incfile ) ); 
					}
				}
			continue;
			}
		return( err( 1222, "#include nest level exceeded", "" ) ); 
		}


	if( strcmp( tok, "#for" )==0 ) {  /* for var in list */
		strcpy( list, "" );
		sscanf( line, "%*s %s %s %s", varname, conj, list );
		if( (ss->loopnest)+1 >= LOOPNESTMAX ) {
			return( err( 1223, "loop nest level exceeded", "" ) ); /* loop nest level exceeded */
			}

		if( conj[0] == 'a' ) {  /* "across" */
#ifndef PLOTICUS
			stat = customforvect( str, list, 1 );  /* up to the application */
			if( stat == 1 ) strcpy( list, "" ); /* no results */
			else if( stat > 1 ) return( stat );
#else
			return( err( 12230, "for .. across not supported", "" ));
#endif
			}

		if( list[0] == '\0' ) {    /* empty list.. skip directly to matching #endloop.. */
			ss->sfp[ ss->incnest ] = skiptoendloop( line, ss->sfp[ ss->incnest ] );
			continue;
			}

		(ss->loopnest)++;

		/* prepare to execute the loop body.. */
		ss->forloc[ ss->loopnest ] = linebegin;
		ss->forcount[ ss->loopnest ] = 1;
		ss->loopifnest[ ss->loopnest ] = ss->ifnest;
		if( conj[0] != 'a' ) {
			ss->forlistpos[ ss->loopnest ] = 0;
			sprintf( delimstr, "%c", ss->listdelim );
			GL_getseg( str, list, &(ss->forlistpos[ss->loopnest]), delimstr );
			}
		stat = TDH_setvar( varname, str );
		if( stat != 0 ) return( stat );
		continue;
		}

	if( strcmp( tok, "#while" )==0 ) {
		if( (ss->loopnest)+1 >= LOOPNESTMAX ) return( err( 1224, "loop nest level exceeded", "" ) ); 
		stat = TDH_condex( &line[ix], 1 );
		if( stat == 0 ) {   /* condition is false on first try.. skip to #endloop.. */
			ss->sfp[ ss->incnest ] = skiptoendloop( line, ss->sfp[ ss->incnest ] );
			continue;
			}
		(ss->loopnest)++;
		ss->forloc[ ss->loopnest ] = linebegin;
		ss->forcount[ ss->loopnest ] = -1;
		ss->loopifnest[ ss->loopnest ] = ss->ifnest;
		continue;
		}

	if( strcmp( tok, "#loop" )==0 ) {  /* basic loop */
		if( (ss->loopnest)+1 >= LOOPNESTMAX ) return( err( 1224, "loop nest level exceeded", "" ) ); 
		(ss->loopnest)++;
		ss->forloc[ ss->loopnest ] = linebegin;
		ss->forcount[ ss->loopnest ] = 0;
		ss->loopifnest[ ss->loopnest ] = ss->ifnest;
		continue;
		}

	if( strcmp( tok, "#endloop" )== 0 ) {
		if( ss->forloc[ ss->loopnest ] < 0 ) return( err( 1262, "#endloop: no loop begin", "" ) );
		/* resume = ftell( ss->sfp[ ss->incnest ] );  */

		PROCESS_ENDLOOP:

		/* seek to #loop (or whatever) statement and read line.. */
		fseek( ss->sfp[ ss->incnest ], ss->forloc[ ss->loopnest ], SEEK_SET );
		fgets( line, SCRIPTLINELEN-1, ss->sfp[ ss->incnest ] );

		if( ss->forcount[ ss->loopnest ] == 0 ) continue; /* #loop */

		else if( ss->forcount[ ss->loopnest ] == -1 ) {   /* #while */  /* added 4/26/01 */
			TDH_value_subst( buf, line, data, recordid, FOR_CONDEX, 0 ); 
			ix = 0;
			GL_getok( buf, &ix ); /* skip #while.. */
			stat = TDH_condex( &buf[ix], 1 );
			if( stat == 0 ) {
				ss->sfp[ ss->incnest ] = skiptoendloop( line, ss->sfp[ ss->incnest ] );
				ss->forloc[ ss->loopnest ] = -1;
				(ss->loopnest)--;
				}
			continue;
			}

		/* #for */
		TDH_value_subst( buf, line, data, recordid, NORMAL, 0 ); /* added 4/20/01 */
		sscanf( buf, "%*s %s %s %s", varname, conj, list );
		ss->forcount[ ss->loopnest ] ++;

		if( conj[0] == 'a' ) {
#ifndef PLOTICUS
			stat = customforvect( str, list, ss->forcount[ ss->loopnest ]  ); 
#endif
			}
		else 	{
			sprintf( delimstr, "%c", ss->listdelim );
			stat = GL_getseg( str, list, &(ss->forlistpos[ss->loopnest]), delimstr );
			}

		if( stat != 0 ) {
			/* #for loop is finished */
			ss->sfp[ ss->incnest ] = skiptoendloop( line, ss->sfp[ ss->incnest ] );
			ss->forloc[ ss->loopnest ] = -1;
			(ss->loopnest)--;
			}
		else 	{
			stat = TDH_setvar( varname, str );
			if( stat != 0 ) return( stat );
			}
		continue;
		}

	if( strcmp( tok, "#break" )==0 ) {
		if( ss->forloc[ ss->loopnest ] < 0 ) return( err( 1260, "#break not within loop", "" ) );
		/* seek to top of latest loop.. */
		fseek( ss->sfp[ ss->incnest ], ss->forloc[ ss->loopnest ], SEEK_SET );
		/* read the loop top line.. */
		fgets( line, SCRIPTLINELEN-1, ss->sfp[ ss->incnest ] ); 
		/* skip to matching #endloop.. */
		ss->sfp[ ss->incnest ] = skiptoendloop( line, ss->sfp[ ss->incnest ] );

		ss->ifnest = ss->loopifnest[ ss->loopnest ]; /* restore */
		ss->forloc[ ss->loopnest ] = -1;
		(ss->loopnest)--;
		continue;
		}


	if( strcmp( tok, "#continue" )==0 ) {
		if( ss->forloc[ ss->loopnest ] < 0 ) return( err( 1261, "#continue not within loop", "" ) );
		ss->ifnest = ss->loopifnest[ ss->loopnest ]; /* restore */
		goto PROCESS_ENDLOOP;
		}


	if( strcmp( tok, "#return" )==0 ) {
		fclose( ss->sfp[ ss->incnest ] ); 
		ss->sfp[ ss->incnest ] = NULL; 
		ss->ifnest = ss->incifnest[ ss->incnest ]; /* restore */
		ss->loopnest = ss->incloopnest[ ss->incnest ]; /* restore */
		if( ss->incnest > 0 ) {
			(ss->incnest)--;
			continue;
			}
		else return( 0 );  /* if at top level, same as #exit 0 */
		}

	if( strcmp( tok, "#musthave" )==0 ) {
		stat = 0;
		for( i = 0, len = strlen( line ); i < len; i++ ) if( line[i] == ',' ) line[i] = ' ';
		while( 1 ) {
			strcpy( varname, GL_getok( line, &ix ) );
			if( varname[0] == '\0' ) break;
			stat += TDH_getvar( varname, buf );
			}
		if( stat != 0 ) return( 1226 );  /* required variable not set - caller can check 'line' var */
		continue;
		}

#ifndef TDH_NOREC

	if( strcmp( tok, "#sqlbuild" )==0 ) {	/* #sqlbuild insert|update table quote|noquote exceptionfield1 .. N  */
		sqlbuild0( buf, ss );
		continue;
		}


	if( strcmp( tok, "#sqlblankrow" )==0 ) {
		char table[MAXPATH], *fnames[MAXITEMS];
		/* FILE *dbfp; */
		int nitems;
        	strcpy( table, GL_getok( buf, &ix ) );           /* 1st arg is tablename */
		TDH_altfmap( 1 );
        	stat = TDH_sqltabdef( table, fnames, &nitems );	 /* caution - fnames points to info with limited lifespan */
		TDH_altfmap( 0 );
        	if( stat != 0 ) return( err( stat, "sqlblankrow: no such table", table ));
        	for( i = 0; i < nitems; i++ ) {
               		stat = TDH_getvar( fnames[i], tok );
                	if( stat == 0 ) continue;
                	stat = TDH_setvar( fnames[i], "" );
                	}
		continue;
        	}
#endif



	if( strcmp( tok, "#write" )==0 ) {  /* #write outfile [outmode]  ... #endwrite */
					    /* note: calling app must actually fprintf to writefp */
		char outfile[ MAXPATH ], outmode[20];
		int nt;
		strcpy( outmode, "w" );
		nt = sscanf( buf, "%*s %s %s", outfile, outmode );
		if( nt < 1 ) return( err( 1290, "#write: no file specified", "" ) );  /* no file specified */
		if( stricmp( outfile, "stdout" )==0 ) ss->writefp = stdout;
		else if( stricmp( outfile, "stderr" )==0 ) ss->writefp = stderr;
		else ss->writefp = fopen( outfile, outmode );
		if( ss->writefp == NULL ) return( err( 1205, "#write: cannot open file", outfile ) ); 
		continue;
		}

	if( strcmp( tok, "#endwrite" )==0 ) {  
		strcpy( tok, "" );
		sscanf( buf, "%*s %s", tok );
		if( ss->writefp != NULL ) {
			if( stricmp( tok, "noclose" )!= 0 && ss->writefp != stdout && ss->writefp != stderr ) 
				{ fclose( ss->writefp ); }
			ss->writefp = NULL; 
			}
		continue;
		}

	if( strcmp( tok, "#cat" )==0 ) {
		FILE *cfp;
		int c;
		while( 1 ) {
			strcpy( tok, GL_getok( buf, &ix ));
			if( tok[0] == '\0' ) break;
			cfp = fopen( tok, "r" );
			if( cfp == NULL ) continue;
			while( ( c = getc( cfp ) ) != EOF ) printf( "%c", c );
			fclose( cfp );
			}
		continue;
		}

	if( strcmp( tok, "#mode" )==0 || strcmp( tok, "#control" )== 0 ) {
		char what[40];
		sscanf( line, "%*s %s %s", what, tok );
		if( strcmp( tok, "comma" )==0 ) strcpy( tok, "," );
		else if( strcmp( tok, "tab" )==0 ) strcpy( tok, "\t" );
		else if( strcmp( tok, "space" )==0 ) strcpy( tok, " " );

		if( strncmp( what, "listsep", 7 )==0 ) {
			TDH_function_listsep( tok[0] );  /* for function args that are lists */
			TDH_condex_listsep( tok[0] );   /* for condex list ops e.g. in, inlike, etc. */
			ss->listdelim = tok[0];    /* for lists herein */
			}
		else if( strcmp( what, "evalvars" ) == 0 ) {
			if( tok[0] == 'n' ) ss->evalvars = 0;
			else ss->evalvars = 1;
			}
#if TDH_DB != 0
		else if( strcmp( what, "nullrep" ) == 0 ) {
			if( strcmp( tok, "noconvert" )==0 ) ss->nullrep = 0;  
			else if( strcmp( tok, "blank" )==0 ) ss->nullrep = 1;
			else if( strcmp( tok, "null" )==0 ) ss->nullrep = 2;
			else if( strcmp( tok, "nbsp" )==0 ) ss->nullrep = 3;
			TDH_sqlrow_nullrep( ss->nullrep ); /* make it available to $functions */
			}
#endif

		else if( strcmp( what, "errormode" )==0 ) TDH_errmode( tok );

		else if( strcmp( what, "shellmetachars" )==0 ) strcpy( TDH_shellmetachars, tok );
			
		else if( GL_smember( what, "allowinlinecodes suppressdll shieldquotedvars dot_in_varnames" )) {
			if( tok[0] == 'y' ) TDH_valuesubst_settings( what, 1 );
			else TDH_valuesubst_settings( what, 0 );
			}
		continue;
		}


	/* other operators - add trailing newline and return - caller must implement */
	len = strlen( line );
	line[ len ] = '\n'; line[ len+1 ] = '\0';
	return( SINTERP_MORE );
	}
}

/* ---------------------------------- */
/* SINTERP_OPEN  - open a script file and prepare to interpret it.
 * filename may be "-" indicating stdin.
 * Example: sinterp_open( "myscript", &ss );  
 *
 * script may be taken from memory.  To do this, pass filename as "",
 * and, before calling TDH_sinterp_open(), set ss.nmemrows and ss.memrows.
 */

int
TDH_sinterp_open( filename, ss )
char *filename;
struct sinterpstate *ss;
{
int i;
char buf[512];  /* was 256 .. scg 3/16/06 */

ss->incnest = 0;

if( filename[0] != '\0' ) {
	/* first try scriptdir.. */
	sprintf( buf, "%s%c%s", TDH_scriptdir, PATH_SLASH, filename );
	ss->sfp[ 0 ] = fopen( buf, "r" );
	if( ss->sfp[ 0 ] == NULL ) {
		/* then try the file name as is.. */
		ss->sfp[ 0 ] = fopen( filename, "r" );
		if( ss->sfp[ 0 ] == NULL ) {
			return( 1 );
			}
		}
	ss->nmemrows = 0;
	}
else ss->mrow = 0;

/* initialize.. */
for( i = 0; i < IFNESTMAX; i++ ) ss->condmet[ i ] = 0;
for( i = 0; i < IFNESTMAX; i++ ) ss->disp[ i ] = 1;
ss->ifnest = 0;
ss->loopnest = 0;
TDH_valuesubst_settings( "hideund", 0 );
ss->listdelim = ',';
ss->nitems = 0;
ss->evalvars = 1;
ss->doingshellresult = 0;
ss->doingsqlresult = 0;
ss->sqlbuildi = 0;
ss->writefp = NULL;
ss->forloc[0] = -1;
ss->nullrep = 1;
return( 0 );
}
/* ============================== */
int
TDH_sinterp_openmem( memrows, nmemrows, ss )
char **memrows;
int nmemrows;
struct sinterpstate *ss;
{
int stat;
ss->memrows = memrows;
ss->nmemrows = nmemrows;
stat = TDH_sinterp_open( "", ss );
return( stat );
}

/* ============================== */
/* SKIPTOENDLOOP */
static FILE *
skiptoendloop( buf, fp )
char *buf;
FILE *fp;
{
int nestcount;
char tok[ DATAMAXLEN+1];
nestcount = 1;
while( fgets( buf, SCRIPTLINELEN-1, fp ) != NULL ) {
	tok[0] = '\0'; /* scg 5/1/03 */
	sscanf( buf, "%s", tok );
	if( GL_smember( tok, "#for #while #loop" )) nestcount++;
	if( GL_smember( tok, "#endloop" )) nestcount--;
	if( nestcount == 0 ) break;
	}
return( fp );
}

/* =============================== */
/* SETSPECIALINCDIR - set special include directory */
int
TDH_setspecialincdir( dir )
char *dir;
{
specialincludedir = dir;
return( 0 );
}

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
