/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PLOTICUS API */
#include "pl.h"


int PL_init_statics();

static int begin_needed = 1;

/* ================================== */
int
ploticus_init( device, outfilename )
char *device, *outfilename;
{
int stat;
begin_needed = 1;
PL_init_statics();
stat = PL_do_preliminaries();
if( stat != 0 ) return( stat );
strcpy( PLS.outfile, outfilename );
PLG_setoutfilename( PLS.outfile );
stat = PL_devnamemap( &(PLS.device), device, 1 );
if( stat != 0 ) return( stat );
return( 0 );
}


/* =================================== */
int
ploticus_arg( name, value )
char *name, *value;
{
int valused, found, stat;
stat = PL_process_arg( name, value, &valused, &found );
if( stat != 0 ) return( stat );
if( !found ) return( Eerr( 6294, "arg not recognized", name ));
return( 0 );
}

/* =================================== */
int
ploticus_begin()
{
if( ! begin_needed ) return( 0 );
begin_needed = 0;
return( PL_begin() );
}


/* ==================================== */ 
int
ploticus_execline( line )
char *line;  /* a line of script, with or without trailing newline */
{
int stat;
if( begin_needed ) { 
	stat = ploticus_begin();
	if( stat ) return( stat );
	}
return( PL_execline( line ));
}


/* ==================================== */ 
int
ploticus_execscript( scriptfile, prefab ) 
char *scriptfile;
int prefab;
{
int stat;
char filename[MAXPATH], *prefabs_dir, *getenv();
if( begin_needed ) { 
	stat = ploticus_begin();
	if( stat ) return( stat );
	}
if( ! prefab ) return( PL_exec_scriptfile( scriptfile ));
else 	{
	prefabs_dir = getenv( "PLOTICUS_PREFABS" );

	/* maybe PREFABS_DIR was set in pl.h... */
  	if( prefabs_dir == NULL ) prefabs_dir = PREFABS_DIR ;
  	else if( prefabs_dir[0] == '\0' ) prefabs_dir = PREFABS_DIR ;
  	if( prefabs_dir[0] == '\0' ) prefabs_dir = NULL;

	if( prefabs_dir == NULL ) return( Eerr( 7237, "PLOTICUS_PREFABS environment variable not found", "" ));
	sprintf( filename, "%s/%s", prefabs_dir, scriptfile );
	return( PL_exec_scriptfile( filename ));
	}
}


/* ==================================== */ 
int
ploticus_end()
{
int stat;
char buf[80];

strcpy( buf, "#endproc" );
stat = PL_execline( buf );
if( PLS.eready ) {
	Eflush();
	stat = Eendoffile();
	}
PL_free();
return( stat );
}


/* ==================================== */ 
int
ploticus_setvar( name, value )
char *name, *value;
{
return( TDH_setvar( name, value ));
}

/* ==================================== */ 
int
ploticus_getvar( name, value )
char *name, *value;
{
return( TDH_getvar( name, value ));
}



#ifdef PHASE2

PL_dataset_begin()
PL_datarow( buf )
PL_datarowp( fields )
PL_dataset_end()
a way to access gd working image

#endif

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
