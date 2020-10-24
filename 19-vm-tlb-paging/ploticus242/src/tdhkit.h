/* TDHKIT.H
 * Copyright 1998-2002 Stephen C. Grubb  (ploticus.sourceforge.net) .
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

#ifndef TDHKIT
  #define TDHKIT 1

/* general includes and defines.. */
#include <stdio.h> 
#include <string.h>

extern char *GL_getok();
extern int TDH_err();

/* Data array size */
#define MAXITEMS 128    /* max number of fields per record (was 80, raised scg 2/21/13) */
#define DATAMAXLEN 256  /* max length of one field - should be same value as VARMAXLEN */

/* Variables list */
#define MAXVAR 250	/* max number of variables */ /* raised to 250 - scg 7/11/07 */
#define VARMAXLEN 256	/* max length of variable contents - should be same value as DATAMAXLEN */

/* Variable name size */
#define NAMEMAXLEN 50	/* max length of variable name */

/* Data type max size */
#define DTMAXLEN 40 	

/* Other maximums */
#define MAXRECORDLEN 3000  /* max length of an input data record */

#define SCRIPTLINELEN 3000 /* max length of a scripter text line, 
			      both before variable evaluation, and after. 
			      4/6/01 expanded from 1024 to accomodate sql result rows */

#define VARSUBLINELEN 1024 /* max length of a text line that will have
				variable substitution applied, before and after. */
#define MAXPATH 256

#ifndef PATH_SLASH
#ifdef WIN32
#define PATH_SLASH '\\'
#else
#define PATH_SLASH '/'
#endif
#endif

#define MAXSORTFIELDS 20

/* value_subst modes */
#define NORMAL 0
#define FOR_CONDEX 1
#define URL_ENCODED 2

#define DBNULL "null"		/* the word "null" */


/* ======== sinterp state ======== */
#define IFNESTMAX 20
#define INCNESTMAX 5
#define LOOPNESTMAX 20
#define SINTERP_END -1
#define SINTERP_END_BUT_PRINT -2
#define SINTERP_MORE -5
struct sinterpstate {
	int ifnest;			/* current 'if' nest level */
	char condmet[IFNESTMAX];	/* flags for condition met, one per nest level */
	char disp[IFNESTMAX];		/* flags for whether to display based on #if, 1 per nest level */

	int incnest;			/* current 'include' nest level */
	FILE *sfp[INCNESTMAX];		/* array of file pointers, one per nest level */
	int incifnest[INCNESTMAX];	/* save current ifnest to restore to in case of #return */
	int incloopnest[INCNESTMAX];	/* save current loopnest to restore to in case of #return */

	int loopnest;			/* current 'for' or 'loop' nest level */
	long forloc[ LOOPNESTMAX ];	/* seek offset for top of loop, one per nest level */
	int forcount[ LOOPNESTMAX ];	/* loop counter value, one per nest level */
	int forlistpos[ LOOPNESTMAX ];	/* loop, current position in list, one per nest level */
	int loopifnest[ LOOPNESTMAX ];	/* save current ifnest to restore to in case of #break or #continue */
	char listdelim;			/* character to be used as list delimiter */
	int nitems;			/* n data array slots filled */
	int evalvars;			/* 1 = evaluate vars  0 = don't */
	int doingshellresult;		/* >0 = in midst of getting shell command result, 0 = not */
	int doingsqlresult;		/* >0 = in midst of getting sql result, 0 = not */
	int sqlbuildi;			/* >0 = in midst of #sqlbuild op, tells next item(-1)   0 = not */
	int nullrep;			/* modes for presenting "null" fields.. 0 = no conversion, 1 = "", 2 = "null" */
	FILE *writefp;			/* fp for use during a #write   */
	int dbc;			/* db connection for sql dump */
	char **memrows;			/* in-memory script rows (optional) */
	int nmemrows;			/* number of in-memory script rows */
	int mrow;			/* current in-memory row */
	} ;

/* ==== macros ==== */
#ifdef LOCALE
 #ifndef stricmp
 #define stricmp( s, t )         stricoll( s, t )
 #endif
 #ifndef strnicmp
 #define strnicmp( s, t, n )     strnicoll( s, t, n )
 #endif
 extern int stricoll(), strnicoll();   /* added scg 5/31/06 gcc4 */
#else
 #ifndef stricmp
 #define stricmp( s, t )         strcasecmp( s, t )
 #endif
 #ifndef strnicmp
 #define strnicmp( s, t, n )     strncasecmp( s, t, n )
 #endif
#endif

#define err(a,b,c) 		TDH_err(a,b,c)

/* ==== fseek defines ==== */
#ifndef SEEK_SET
  #define SEEK_SET 0
  #define SEEK_CUR 1
  #define SEEK_END 2
#endif

/* ==== vars ==== */
extern char TDH_scriptdir[];
extern char TDH_tmpdir[];
extern char TDH_dbnull[];
extern int TDH_debugflag;
extern char TDH_decpt;
extern char *TDH_dat;
extern char *TDH_recid;
extern char *TDH_dat;                   /* points to data array for condex */
extern char *TDH_recid;                 /* points to recordid for condex */
extern char TDH_progname[];
extern int TDH_initialized;
extern char TDH_configfile[]; 	/* scg 11/11/02 */
extern char TDH_shellmetachars[]; /* scg 11/18/02 */
extern int TDH_midriff_flag;   /* scg 5/29/03 */
#ifndef TDH_NOREC
extern char TDH_fdfpath[];
#endif


extern int GL_getchunk(), GL_getseg(), GL_goodnum(), GL_member(), GL_slmember(), GL_smember(), GL_smemberi(), GL_sysdate(), GL_systime();
extern int TDH_condex(), TDH_err(), TDH_getvalue(), TDH_getvar(), TDH_readconfig(), TDH_setvalue(), TDH_setvar();
extern int TDH_value_subst(), TDH_valuesubst_settings();

extern int TDH_fieldmap(), TDH_altfmap(), TDH_loadfieldmap();
extern int TDH_errprog(), TDH_errmode();

#endif
