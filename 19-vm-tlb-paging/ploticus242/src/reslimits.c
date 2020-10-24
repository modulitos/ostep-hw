/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* RESLIMITS - set system resource limits for current process and all child processes.  
   cpu sets max cpu time in seconds
   filesize sets max size in bytes of created files

   offending processes will dump core; debugger gives explanation..
 */

#ifndef WIN32

#include <stdio.h>
#include <sys/time.h>
#include <sys/resource.h>

int
TDH_reslimits( type, value )
char *type;
int value;
{
struct rlimit rl, *rlp;

rlp = &rl;
rlp->rlim_cur = (rlim_t)value;
rlp->rlim_max = (rlim_t)RLIM_INFINITY;
if( type[0] == 'c' ) setrlimit( RLIMIT_CPU, rlp );
/* else if( type[0] == 'f' ) setrlimit( RLIMIT_FSIZE, rlp ); */

return( 0 );
}
#endif
