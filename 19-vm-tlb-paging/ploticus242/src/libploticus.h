/* libploticus.h
 * Copyright 1998-2007 Stephen C. Grubb  (ploticus.sourceforge.net)
 * and Colin Tuckley.
 * This code is covered under the GNU General Public License (GPL);
 * see the file ./Copyright for details. */

#ifndef LIBPLOTICUS
  #define LIBPLOTICUS 1

/* function defines.. */

int ploticus_init( char *device, char *outfilename );

int ploticus_arg( char *name, char *value );

int ploticus_execline( char *line );

int ploticus_execscript( char *scriptfile, int prefabflag );

int ploticus_end();

#endif
