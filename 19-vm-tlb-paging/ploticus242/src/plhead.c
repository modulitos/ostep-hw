/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

#include "plg.h"
#include "pl.h"


struct plstate PLS;
struct pldata PLD;
struct proclines PLL;
double *PLV;
int PLVsize, PLVhalfsize, PLVthirdsize;
char PL_bigbuf[ MAXBIGBUF ];	/* general purpose large buffer - don't rely 
				on content integrity across procs */

/* ======================================================= *
 * Copyright 1998-2005 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */
