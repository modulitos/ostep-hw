/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "pl.h"
#include "tdhkit.h"

#define NONDRAWINGPROCS "page print originaldata usedata tabulate getdata boxplot catslide processdata settings endproc"

#define MAXMALLOCATTRS 30   /* max # of multiline attributes per proc including clones */

#define GETTING 0
#define SCANNING 1


static int execline_init = 0;
static int lastbs; 	      /* indicates that previous line ended with a backslash, indicating continuation.. */
static char procname[NAMEMAXLEN];
static char saveas_name[NAMEMAXLEN];
static char last_proctok[20]; /* either #proc or #procdef */
static char clone_name[NAMEMAXLEN];
static int nlhold;
static char clonelist[200];
static int holdmemflag = 0;
static int prevlineblank = 0;	/* prevent -echo from spitting out lots of adjacent blank lines */
static char *mem; 
static int nfl = 0;
static char *malloclist[MAXMALLOCATTRS]; /* list of malloced multiline items */
static int procstop; 		/* communicates end of proc w/ getmultiline  */


static int proc_call(), initproc();

/* ================================================================= */
int
PL_execline_initstatic()
{
execline_init = 0;
holdmemflag = 0;
prevlineblank = 0;
/* don't reset nfl here.. let new job free the list */
return( 0 );
}

/* ================================================================= */
/* EXECLINE - execute one ploticus script line.
 * Returns 0 if ok, or a non-zero error code 
 */

int
PL_execline( line )
char *line;	/* line of script file.. */   /* if const, no trailing newline and no #ifspec! (these modify line) */
{
int i, ix, stat;
char firsttok[50], buf2[50];
int buflen, endproc, procstat;


if( !execline_init ) {
	PLS.npages = 0;
	lastbs = 0;
	strcpy( saveas_name, "" );
	strcpy( last_proctok, "" );
	strcpy( procname, "" );
	strcpy( clonelist, "" );

	/* proc line initializations.. */
	PLL.nobj = 0;
	PLL.nlines = nlhold = 0;

	execline_init = 1;
	}

if( PLS.skipout ) return( 1 ); /* a major error has occured;  don't process any more lines.. just return */

buflen = strlen( line );


/* remove newlines cr/lf and trailing whitespace from every line... added scg 11/9/07 */
for( i = buflen-1; i >= 0; i-- ) if ( !isspace( (int) line[i] )) break;   
line[i+1] = '\0';
buflen = i+1;
/* was... */
/* if( line[ buflen-1 ] == '\n' ) { line[ buflen-1 ] = '\0'; buflen--; } */ /* don't keep trailing newline.. */
/* if( line[ buflen-1 ] == 13 ) { line[ buflen-1] = '\0'; buflen--; } */ /* DOS LF */

ix = 0; 

/* get first token in line.. changed to handle potentially long lines having no whitespace eg. long CSV data lines .. scg 10/25/07 */
strncpy( buf2, line, 40 ); buf2[40] = '\0'; 
strcpy( firsttok, GL_getok( buf2, &ix ) );

if( PLS.echolines && strcmp( firsttok, "#ifspec" )!= 0 ) {
	if( prevlineblank && firsttok[0] == '\0' ); /* multiple blank lines.. don't output */
	else if( PLS.echolines == 1 ) { printf( "%s\n", line ); fflush( stdout ); }
        else if( PLS.echolines == 2 ) { fprintf( PLS.diagfp, "%s\n", line ); fflush( PLS.diagfp ); }
	if( firsttok[0] == '\0' ) prevlineblank = 1;
	else prevlineblank = 0;
	}



/* intercept #endproc.. */
endproc = 0;
if( strcmp( firsttok, "#endproc" )==0 ) { 
	strcpy( firsttok, "#proc" ); 
	endproc = 1; 

	/* and add an additional blank line to terminate any last multline item.. */
	/* nlines>0 was added .. this caused seg fault on degenerate api case of 0 script lines 5/29/03 */
	if( PLL.nlines > 0 && PLL.nlines < PLL.maxproclines-1 ) {
		PLL.procline[ PLL.nlines ] = (char *) malloc( 5 ); 
		strcpy( PLL.procline[ PLL.nlines ], "\n" );
        	(PLL.nlines)++;
		}
	}


/*** #proc(def): get ready to capture next proc, and execute the proc that has just been read... */
if( strncmp( firsttok, "#proc", 5 )==0 && !lastbs ) {  /* #proc or #procdef break */

	procstat = 0;

	/* if #saveas was used, record the name.. */
	if( saveas_name[0] != '\0' )  strcpy( PLL.objname[ PLL.nobj ], saveas_name );

	/* get line count.. */
	PLL.objlen[ PLL.nobj ] = PLL.nlines - nlhold;

	/* if not first time around, and #proc was used (as opposed to #procdef), execute proc.. */
	if( strcmp( last_proctok, "#proc" )==0 ) {  

		/* proc page can do the Einit, or we can do it here.. */
		if( strcmp( procname, "page" )==0 ) PLS.eready = 1;
		if( !PLS.eready && !GL_slmember( procname, NONDRAWINGPROCS )) {
			stat = Einit( PLS.device );
			if( stat ) { PLS.skipout = 1; return( stat ); }
			Epaper( PLS.landscape );
			PLS.eready = 1;

			/* we are ready to draw.. safe to say page 1.. scg 11/28/00 */
			if( strcmp( procname, "page" )!=0 ) PLS.npages = 1;
			if( PLS.bkcolorgiven ) {
				/* EPS color=transparent - best to do nothing.. scg 1/10/00*/
				if( PLS.device == 'e' && strcmp( Ecurbkcolor, "transparent" )==0 ) ;
				else Eclr(); 
				}
			}


		/* execute the appropriate plotting procedure... */
		procstat = proc_call( procname );

		if( PLS.eready ) Eflush();  /* get output onto screen.. */
		}

	if( endproc ) strcpy( procname, "endproc" );
	else if( sscanf( line, "%*s %s", procname ) < 1 ) {
		Eerr( 24, "#proc must be followed by a procedure name", "" );
		procstat = 24;
		}
		


	/* if we're not told to hold on to the memory (used by getdata), and if
	   there were no #saveas.., then free the proc lines now..*/
	if( !holdmemflag && saveas_name[0] == '\0' ) {
		for( i = nlhold; i < PLL.nlines; i++ ) free( PLL.procline[i] ); 
	 	PLL.nlines = nlhold; 
		}
	else 	{
		if( PLL.nobj >= MAXOBJ-1 ) {
			Eerr( 25, "too many active procs - see limitations page MAXOBJ", "" );
			procstat = 25;
			}
		else (PLL.nobj)++;
		}
	holdmemflag = 0;

	strcpy( last_proctok, firsttok );

	if( procname[ strlen( procname ) - 1 ] == ':' ) procname[ strlen( procname ) - 1 ] = '\0';


	/* initialize to capture next proc */
	strcpy( saveas_name, "" );
	strcpy( clonelist, "" );
	strcpy( PLL.objname[ PLL.nobj ], "" );
	PLL.objstart[ PLL.nobj ] = PLL.nlines;
	nlhold = PLL.nlines;

	return( procstat );
	}


/****** for all other lines, get them, and add them to proc line list, looking for
 ****** special cases such as #clone and #saveas..  */

else 	{

	if( firsttok[0] == '#' && firsttok[1] != '#' ) {
		if( strncmp( firsttok, "#clone", 6 ) != 0 && 
		    strncmp( firsttok, "#saveas", 7 ) != 0 && 
		    strncmp( firsttok, "#ifspec", 7 ) != 0 ) Eerr( 57468, "unrecognized operator", firsttok );
		}

	if( procname[0] == '\0' ) return( 0 ); /* ? */
	else 	{
		/* add lines to proc line list.. */
		/* also look for exceptions such as "#clone" */

		if( !lastbs && strncmp( firsttok, "#clone", 6 )==0 ) {
			strcpy( clone_name, "" );
			sscanf( line, "%*s %s", clone_name );
			if( clone_name[0] == '\0' ) {
				Eerr( 27, "#clone object name is missing", procname );
				return( 1 );
				}
			strcat( clonelist, clone_name );
			strcat( clonelist, " " );
			}

		else if( !lastbs && strncmp( firsttok, "#saveas", 7 )==0 ) sscanf( line, "%*s %s", saveas_name );


		else 	{

			/* #ifspec   scg 10/16/03 */
			if( !lastbs && strncmp( firsttok, "#ifspec", 7 )==0 ) {  /* #ifspec varname [attrname] */
				int nt;
				char varname[50], attrname[50], val[DATAMAXLEN+1];
				nt = sscanf( line, "%*s %s %s", varname, attrname );
				if( nt == 1 ) strcpy( attrname, varname );
				stat = TDH_getvar( varname, val );
				if( stat == 0 && val[0] != '\0' ) {
					sprintf( line, " %s: %s", attrname, val );
					if( PLS.echolines == 1 ) printf( "%s\n", line );
        				else if( PLS.echolines == 2 ) fprintf( PLS.diagfp, "%s\n", line );
					}
				else strcpy( line, "" );
				buflen = strlen( line );
				}

			PLL.procline[ PLL.nlines ] = (char *) malloc(  buflen+1 );
			strncpy( PLL.procline[ PLL.nlines ], line, buflen );
			PLL.procline[ PLL.nlines ][ buflen ] = '\0';

			if( PLL.nlines >= PLL.maxproclines-1 ) {
				PLS.skipout = 1; /* this is severe enough to abort mission.. */
				return( err( 28, "Script file - too many lines in current proc plus saved procs; try raising -maxproclines", "" ));
				}
			(PLL.nlines)++;
			if( line[ buflen - 2 ] == '\\' ) lastbs = 1;
			else lastbs = 0;
			}
		}

	return( 0 );
	}

}

/* ========================= */
/* HOLDMEM - allow other modules to tell execline() to not free the lines 
   for the current proc.. Used by getdata.
 */
int
PL_holdmem( stat )
int stat;
{
holdmemflag = stat;
return( 0 );
}

/* ========================= */
/* PROC_CALL - call the appropriate proc routine */
static int proc_call( procname )
char *procname;
{
int stat;
int n;

stat = 0;

initproc(); /* initialize attribute malloc stuff for this proc  (see below) */

if( PLS.debug ) { 
	if( strcmp( procname, "endproc" )==0 ) { fprintf( PLS.diagfp, "(endproc)\n" ); fflush( PLS.diagfp ); }
	else fprintf( PLS.diagfp, "Executing %s\n", procname ); fflush( PLS.diagfp ); 
	}

if( strcmp( procname, "areadef" )==0 ) {
	stat = PLP_areadef();
	if( stat != 0 ) {
		PLS.skipout = 1;
		return( Eerr( 10, "cannot set up plotting area .. likely culprits: bad xrange or yrange, or bad area rectangle", "" ));
		}
	}
else if( strcmp( procname, "page" )==0 ) {
	stat = PLP_page();
	if( stat ) { PLS.skipout = 1; return( stat ); }
	}
else if( strcmp( procname, "xaxis" )==0 ) stat = PLP_axis( 'x', 0 );
else if( strcmp( procname, "yaxis" )==0 ) stat = PLP_axis( 'y', 0 );
else if( strcmp( procname, "getdata" )==0 ) stat = PLP_getdata();
else if( strcmp( procname, "categories" )==0 ) stat = PLP_categories( 0 );
else if( strcmp( procname, "legend" )==0 ) stat = PLP_legend();
else if( strcmp( procname, "bars" )==0 ) stat = PLP_bars();
else if( strcmp( procname, "scatterplot" )==0 ) stat = PLP_scatterplot();
else if( strcmp( procname, "pie" )==0 ) stat = PLP_pie();
else if( strcmp( procname, "lineplot" )==0 ) stat = PLP_lineplot();
else if( strcmp( procname, "rangesweep" )==0 ) stat = PLP_rangesweep();
else if( strcmp( procname, "boxplot" )==0 ) stat = PLP_boxplot();
else if( strcmp( procname, "annotate" )==0 ) stat = PLP_annotate();
else if( strcmp( procname, "processdata" )==0 ) stat = PLP_processdata();
else if( strcmp( procname, "catlines" )==0 ) stat = PLP_catlines();
else if( strcmp( procname, "curvefit" )==0 ) stat = PLP_curvefit();
else if( strcmp( procname, "vector" )==0 ) stat = PLP_vector();
else if( strcmp( procname, "usedata" )==0 ) stat = PLP_usedata(); 
else if( strcmp( procname, "legendentry" )==0 ) stat = PLP_legendentry();
else if( strcmp( procname, "line" )==0 ) stat = PLP_line();
else if( strcmp( procname, "rect" )==0 ) stat = PLP_rect();
else if( strcmp( procname, "tree" )==0 ) stat = PLP_tree();
else if( strcmp( procname, "venndisk" )==0 ) stat = PLP_venndisk();
else if( strcmp( procname, "pvalue" )==0 ) stat = PLP_pvalue();
else if( strcmp( procname, "settings" )==0 ) stat = PLP_settings();
else if( strcmp( procname, "breakaxis" )==0 ) stat = PLP_breakaxis();
else if( strcmp( procname, "image" )==0 ) stat = PLP_image();
else if( strcmp( procname, "drawcommands" )==0 ) stat = PLP_drawcommands();
else if( strcmp( procname, "tabulate" )==0 ) stat = PLP_tabulate();
else if( strcmp( procname, "symbol" )==0 ) stat = PLP_symbol();
else if( strcmp( procname, "print" )==0 ) stat = PLP_print(); 

else if( strcmp( procname, "trailer" )==0 ) ; /* do nothing */
else if( strcmp( procname, "endproc" )==0 ) ; /* do nothing */

else if( strcmp( procname, "catslide" )==0 ) stat = PLP_categories( 0 ); /* maps to: proc categories */
else if( strcmp( procname, "transform" )==0 ) stat = PLP_processdata(); /* maps to: proc processdata */
else if( strcmp( procname, "originaldata" )==0 ) stat = PLP_usedata();  /* maps to: proc usedata */
else if( strcmp( procname, "bevelrect" )==0 ) stat = PLP_rect();        /* maps to: proc rect */
else if( strcmp( procname, "import" )==0 ) stat = PLP_image(); 	/* maps to: proc image */
else if( strcmp( procname, "datesettings" )==0 ) stat = PLP_settings(); /* maps to: proc settings */

else if( strcmp( procname, "rangebar" )==0 ) return( Eerr( 27925, "proc rangebar has been replaced with proc boxplot", "" ) );
else if( strcmp( procname, "defineunits" )==0 ) return( Eerr( 27926, "proc defineunits discontinued; use proc areadef", "xnewunits and ynewunits" ));

else return( Eerr( 101, "procedure name unrecognized", procname ) );


TDH_errprog( "pl" );

if( PLS.eready ) Eflush();
n = report_convmsgcount();
if( PLS.debug && n > 0 ) {
	fprintf( PLS.diagfp, "note: pl proc %s encountered %d unplottable data values\n", procname, n );
	zero_convmsgcount();
	}
return( stat );
}

/* ================================================================= */
/* GETNEXTATTR - serve up the next proc line, or NULL if no more */
/* This function returns a pointer to the proc line, and returns some values in the parameters. */

char *
PL_getnextattr( firsttime, attr, valpos )
int firsttime;	/* 1 = first call for proc */
char *attr;	/* returned: attribute name */
int *valpos;    /* returned: char position in the string returned by this function, where value content begins */
{
static int cloneix, state;
static char *line;
int j, ix, alen;
char clone_name[NAMEMAXLEN];

/* states:  0 = init   1 = getting clone  2 = getting proc  3 = done */

if( firsttime ) { state = 0; cloneix = 0; }

if( state == 3 ) {
	line = NULL;
	return( line );
	}

if( state == 0 ) {  
	RETRY:
	strcpy( clone_name, GL_getok( clonelist, &cloneix ));
	if( clone_name[0] != '\0' ) {
		/* look up obj in list, starting with latest entry and working backward.. */
		for( j = (PLL.nobj)-1; j >= 0; j-- ) if( strcmp( PLL.objname[j], clone_name )==0 ) break;
		if( j < 0 ) {
			Eerr( 2506, "#clone object not found", clone_name );
			goto RETRY;
			}
		PLL.curline = PLL.objstart[j];
		procstop = PLL.objstart[j] + PLL.objlen[j];
		state = 1;
		}
	else 	{
		PLL.curline = PLL.objstart[ PLL.nobj ];
		procstop = PLL.nlines;
		state = 2;
		}
	}

if( state == 1 || state == 2 ) {
	RETRY2:
	if( PLL.curline >= PLL.nlines ) return( NULL );
	line = PLL.procline[ PLL.curline ];
	ix = 0;

	strncpy( attr, GL_getok( line, &ix ), 38 );   /* get 1st token (truncate at 38 chars) */
	attr[38] = '\0'; 

	if( attr[0] == '\0' ) {   /* blank line.. skip */
		(PLL.curline)++;
		if( PLL.curline >= procstop && state == 1 ) { state = 0; goto RETRY; }
		else if( PLL.curline >= procstop && state == 2 ) { state = 3; return( NULL ); }
		else goto RETRY2;
		}
	alen = strlen( attr );
	if( attr[ alen-1 ] == ':' ) attr[ alen-1 ] = '\0';
	if( attr[0] != '\0' ) while( isspace( (int) line[ix] )) ix++; /* skip over ws */
	*valpos = ix;

	PLL.curline++;
	if( PLL.curline >= procstop ) {
		if( state == 1 ) state = 0;
		else state = 3;
		}

	return( line );
	}
return( NULL );
}

/* ================================================================= */
/* GETMULTILINE - get a multi-line text item from script file.  Terminates when first empty line is encountered.
	If mode == "get", sufficient memory is malloc'ed, the text is copied into it, and function returns pointer to text.
	If mode == "skip", we simply advance to the end of the multiline text (see proc_getdata)
*/
   
char *
PL_getmultiline( firstline, mode )
char *firstline; /* first row of data, without attribute name */ 
char *mode;  /* either "get" or "skip" */
{
char *line;
int i, iline;
int txtlen, txtstartline, txtstopline, memlen, emptyline;

txtstartline = PLL.curline;

/* first, scan thru all rows to get count of total # chars... */
txtlen = strlen( firstline );

/* go until we hit an empty line, or reach end of proc.. */
for( iline = txtstartline; iline <= procstop ; iline++ ) {
	line = PLL.procline[ iline ];
	for( i = 0, emptyline = 1; line[i] != '\0'; i++ ) if( !isspace( (int) line[i] )) { emptyline = 0; break; }
	if( emptyline ) break;
	if( mode[0] == 'g' ) txtlen += (strlen( &line[i] ) + 2);  /* mode = "get", accumulate length sans leading ws */
	}

/* remember where we stopped.. */
txtstopline = iline;  
PLL.curline = iline;  /* so scanner can resume at the right place.. */

if( mode[0] == 's' ) return( 0 );  /* mode = "skip" */

mem = malloc( txtlen+2 * sizeof( char *) );
if( mem == (char *)NULL ) { PLS.skipout = 1; Eerr( 27509, "multiline malloc failed", "" ); return( "" ); }
malloclist[nfl++] = mem;

memlen = 0;

/* copy first line content.. */
for( i = 0; firstline[i] != '\0'; i++ ) if( !isspace( (int) firstline[i] )) break; /* skip leading ws */
if( firstline[i] != '\0' ) {
	sprintf( mem, "%s\n", &firstline[i] );
	memlen = strlen( &firstline[i] ) + 1;
	}


/* now fill mem.. */
for( iline = txtstartline; iline < txtstopline && iline <= procstop; iline++ ) {
	line = PLL.procline[ iline ];
	
	/* skip over leading whitespace as well as any leading backslash.. */
	for( i = 0; line[i] != '\0'; i++ ) if( !isspace( (int) line[i] )) break;
	if( line[i] == '\\' ) i++;

	strcpy( &mem[memlen], &line[i] );
	memlen += strlen( &line[i] );
	mem[ memlen++ ] = '\n';
	mem[ memlen ] = '\0';
	}
return( mem );
}



/* ========================================= */
/* TOKNCPY - copy 1st token of lineval into val, up to maxlen (maxlen should be same as var declaration size) */
int
PL_tokncpy( val, lineval, maxlen )
char *val, *lineval;
int maxlen;
{
int i;
for( i = 0; i < maxlen-1; i++ ) {
	if( (lineval[i]=='\0') || isspace( (int)lineval[i] )) break;
	/* was: if isspace( (int)lineval[i] )) break; */
	val[i] = lineval[i];
	}
val[i] = '\0';
if( i == (maxlen-1) ) return( 1 );
else return( 0 );
}

/* ======================================== */
/* ITOKNCOPY - do tokncpy and convert to integer using atoi() */
int
PL_itokncpy( lineval )
char *lineval;
{
char val[80];
tokncpy( val, lineval, 80 );
return( atoi( val ) );
}

/* ======================================== */
/* FTOKNCOPY - do tokncpy and convert to float using atof() */
double 
PL_ftokncpy( lineval )
char *lineval;
{
char val[80];
tokncpy( val, lineval, 80 );
return( atof( val ) );
}



#ifdef HOLD
/* ========================================= */
/* NEWATTR - malloc some memory for an attribute value, and copy the attribute value into it. */
char *
PL_newattr( lineval, len )
char *lineval;
int len;
{
if( nfl >= MAXMALLOCATTRS-1 ) { PLS.skipout = 1; Eerr( 29, "too many malloced attributes in this proc", "" ); return( "" ); }
if( len < 1 ) len = strlen( lineval );
mem = malloc( len+2 * sizeof( char *) );
if( mem == (char *)NULL ) { PLS.skipout = 1; Eerr( 27508, "newattr malloc failed", "" ); return( "" ); }
malloclist[nfl++] = mem;
strncpy( mem, lineval, len );
mem[len] = '\0';
return( mem );
} 
#endif


/* =========================================== */
/* INITPROC - free all currently malloc'ed attr memory (if any) and initialize for next proc  */
static int
initproc()
{
int i;
for( i = 0; i < nfl; i++ ) free( malloclist[i] );
nfl = 0;
return( 0 );
}




/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
