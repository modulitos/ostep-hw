/* ======================================================= *
 * Copyright 1998-2008 Stephen C. Grubb                    *
 * http://ploticus.sourceforge.net                         *
 * Covered by GPL; see the file ./Copyright for details.   *
 * ======================================================= */

/* PROC TREE - render a tree diagram from a newick file */

#include "pl.h"
#define MAXNEWICKTOKENS 1000  /* max # tokens in input newick file.. indiv. tokens include: labels, commas, colons, distances */
#define HALFPI  1.5707963

/* static int angpos(); */

int
PLP_tree()
{
char attr[NAMEMAXLEN], *line, *lineval;
int lvp, first;

char *newickfile, *symbol, *linedetails, *textdetails, *rootsym;
double adjx, adjy;
int i, j, align, c, nrows, ntok, newstring;

char *label[MAXNEWICKTOKENS];
int nest[MAXNEWICKTOKENS];
char func[MAXNEWICKTOKENS];
double xval[MAXNEWICKTOKENS], yval[MAXNEWICKTOKENS];
double dist[MAXNEWICKTOKENS];
char *buf;

double accum;
char *toklist[MAXNEWICKTOKENS];
int nestlevel, tagcount, naccum;
FILE *fp;

char symcode[80];
double radius, x, y, y1, y2, yofs;
int squaredoff, mustparen, debug;

newickfile = "";
symbol = ""; 
linedetails = "color=blue width=0.5";
textdetails = "";
rootsym = "shape=pixcircle color=black radius=0.02 style=filled";
squaredoff = 1;
mustparen = 1;   /* if 1, the 1st char in newick file must be an open paren  */
debug = 0;
TDH_setvar( "TREEDONE", "0" );

first = 1;
while( 1 ) {
        line = getnextattr( first, attr, &lvp );
        if( line == NULL ) break;
        first = 0;
        lineval = &line[lvp];

        if( strcmp( attr, "newickfile" )==0 ) newickfile = lineval;
        else if( strcmp( attr, "symbol" )==0 ) symbol = lineval;
	else if( strcmp( attr, "linedetails" )==0 ) linedetails = lineval;
	else if( strcmp( attr, "textdetails" )==0 ) textdetails = lineval;
	else if( strcmp( attr, "rootsym" )==0 ) rootsym = lineval;
	else if( strcmp( attr, "squaredoff" )==0 ) squaredoff = getyn( lineval );
	else if( strcmp( attr, "mustparen" )==0 ) mustparen = getyn( lineval );
	else if( strcmp( attr, "debug" )==0 )  debug = getyn( lineval );
	else Eerr( 1, "attribute not recognized", attr );
	}

if( newickfile[0] == '\0' ) return( Eerr( 673, "newickfile not specified", "" ));
if( !scalebeenset() ) return( Eerr( 51, "No scaled plotting area has been defined yet w/ proc areadef", "" ) );


/* read in the newick file.. 
 *
 * Newick grammar taken from:  http://evolution.genetics.washington.edu/phylip/newick_doc.html ....
 * ... exception: to print a ' in a label use eg. 'steve\'s kludge'
 *
 * Example:
 * (NOD/LtJ:0.00000,129S1SvImJ:0.00000,((KK/HlJ:0.40085,((CAST/EiJ:0.70121,(MOLF/EiJ:0.18484,PWD/PhJ:0.03214):0.55524):0.49231,
 * (WSB/EiJ:0.73653):0.21783):0.21668,AKR/J:0.01357,(NZW/LacJ:0.00000,BTBRT+tf/J:0.02142,FVB/NJ:0.00000,C3H/HeJ:0.00000,
 * BALB/cByJ:0.00000,A/J:0.00000):0.01392):0.01188,DBA/2J:0.01071)[0.3333]; 
 */

fp = fopen( newickfile, "r" );
if( fp == NULL ) return( Eerr( 674, "cannot open newickfile", newickfile ));

buf = PL_bigbuf;

/* read in the newick format data and parse into tokens.. */
ntok = 0;
newstring = 0;
for( i = 0; i < (MAXBIGBUF-2); i++ ) {
	if( ntok > MAXNEWICKTOKENS-2 ) { fclose(fp); return( Eerr( 675, "sorry, too many tokens in newickfile, no tree produced", "MAXNEWICKTOKENS" )); }
	c = getc( fp );
	if( i == 0 && c != '(' && mustparen ) { fclose(fp); return( Eerr( 678, "first char in newickfile not a paren, quitting", "" )); }
	if( c == EOF ) break;
	else if( isspace( c )) { i--; continue; }
	else if( c == ')' || c == '('  || c == ',' || c == ';' ) { 
		buf[i++] = '\0'; buf[i++] = c; buf[i] = '\0'; 
		toklist[ntok++] = &buf[i-1]; 
		newstring = 1; 
		}
	else if( c == ':' ) { 
		buf[i++] = '\0'; 
		buf[i] = c; 
		toklist[ntok++] = &buf[i]; 
		newstring = 0;
		}
	else if( c == '[' ) {
		i--;
		while( 1 ) { /* skip [comments] ... */
			c = getc( fp );
			if( c == ']' || c == EOF ) break;
			}
		}
	else if( c == '\'' ) {  /* handle 'quoted labels' ... */
		toklist[ntok++] = &buf[i];
		for( j = 0; ; j++, i++ ) {
			c = getc( fp );
			if( ( c == '\'' && buf[i] != '\\' ) || c == EOF ) break;
			else if( c == ' ' ) buf[i] = '_';
			else if( j == 0 && GL_member( c, ":()" ) ) buf[i] = '_';
			else buf[i] = c;
			}
		i--;
		}
	else 	{
		if( newstring ) toklist[ntok++] = &buf[i];
		newstring = 0;
		buf[i] = c;
		}
	}

if( debug ) {
	fprintf( PLS.diagfp, "\nTOKENS: " );
	for( i = 0; i < ntok; i++ ) fprintf( PLS.diagfp, "[%s]", toklist[i] ); 
	fprintf( PLS.diagfp, "\n(N=%d)\n\n", ntok );
	}

fclose( fp );


/* now do some processing on the token list.. */
nestlevel = 0;
nrows = 0;
tagcount = 0;
for( i = 0; i < ntok; i++ ) {
	if( debug ) fprintf( PLS.diagfp, "[%s]", toklist[i] ); 

	if( strcmp( toklist[i], ";" )==0 ) break;

	else if( strcmp( toklist[i], "(" )==0 ) { 
		label[nrows] = "";   /* was: = toklist[i]; */
		func[nrows] = '+';
		nestlevel++; 
		xval[nrows] = 0.0;
		yval[nrows] = 0.0;
		nest[nrows++] = nestlevel; 
		continue; 
		}
	else if( strcmp( toklist[i], ")" )==0 ) { 
		if( nestlevel <= 0 ) return( Eerr( 677, "too many closing parens in newickfile, no tree produced", "" ));
		nest[nrows] = nestlevel; 
		func[nrows] = '-';
		nestlevel--; 
		label[nrows] = "";   /* was: = toklist[i]; */
		xval[nrows] = 0.0;
		yval[nrows] = 0.0;
		/* now look ahead to next toks.. there could be an internal node label and/or a distance.. */
		if( GL_smember( toklist[i+1], "( ) , ;" )); 
		else if( toklist[i+1][0] != ':' ) { label[nrows] = toklist[i+1]; i++; } /* capture internal node label.. */
		if( GL_smember( toklist[i+1], "( ) , ;" )); 
		else if( toklist[i+1][0] == ':' ) {  
			dist[nrows] = atof( &toklist[i+1][1] );
			for( j = nrows; j >= 0; j-- ) { 
				/* if( nest[j] < nest[nrows] ) break; */
				if( nest[j] == nest[nrows] && func[j] == '+' ) break;
				xval[j] += dist[nrows]; 
				if( func[j] == 'T' && nest[j] == nest[nrows] ) { accum += yval[j]; naccum++; }
				}
			i++;
			}
		
		/* also compute midpoint for fork */
		accum = 0.0; naccum = 0;
		for( j = nrows; j >= 0; j-- ) { 
			/* if( nest[j] < nest[nrows] ) break; */
			if( nest[j] == nest[nrows] && func[j] == '+' ) break;
			else if( nest[j] == nest[nrows] && func[j] == 'T' ) { accum += yval[j]; naccum++; }
			else if( nest[j] == nest[nrows]+1 && func[j] == '-' ) { accum += yval[j]; naccum++; }
			}
		if( naccum > 0 ) yval[nrows] = accum / (double) naccum;  
		
		nrows++;
		continue; 
		}

	else if( strcmp( toklist[i], "," )==0 ) continue;

	else if( toklist[i][0] != ':' ) {
		nest[nrows] = nestlevel;
		label[nrows] = toklist[i];
		func[nrows] = 'T';
		xval[nrows] = 0.0;
		yval[nrows] = 0.0;
		tagcount++;
		yval[nrows] = tagcount;  /* rooted */
		/* now look ahead to next tok.. there could be a distance.. */
		if( toklist[i+1][0] == ':' ) {
			xval[nrows] = atof( &toklist[i+1][1] );
			i++;
			}
		nrows++;
		}

	else if( toklist[i][0] == ':' ) {    /* no label, only a distance.. */
		nest[nrows] = nestlevel;
		label[nrows] = "<nolabel>";
		func[nrows] = 'D';
		xval[nrows] = atof( &toklist[i+1][1] );
		tagcount++;
		yval[nrows] = tagcount;
		nrows++;
		}
	}

if( nestlevel > 0 ) return( Eerr( 676, "too many open parens in newickfile, no tree produced", "" ));



if( debug ) {
	fprintf( PLS.diagfp, "\n\n" );
	for( i = 0; i < nrows; i++ ) fprintf( PLS.diagfp, "%2d. %c %d   %s    %g %g\n", i, func[i], nest[i], label[i], xval[i], yval[i] );  
	}







/* now draw the tree.. */
yofs = Ecurtextheight * 0.3;

linedet( "linedetails", linedetails, 1.0 );

	
/* lines connecting leafs to forks.. */
for( i = 0; i < nrows; i++ ) {
	if( func[i] == 'T' ) {
		for( j = i; j < nrows; j++ ) {
			if( func[j] == '-' && nest[j] == nest[i] ) {
				y1 = Eay( EDYhi - yval[i] ) + yofs; /* flip & adjust upward a bit */
				y2 = Eay( EDYhi - yval[j] );
				Emov( Eax( xval[i] ), y1 ); 
				if( squaredoff ) Elin( Eax( xval[j] ), y1 ); 
				Elin( Eax( xval[j] ), y2 ); 
				break;
				}
			}
		}
	}

/* lines connecting forks to parent forks.. */
for( i = 0; i < nrows; i++ ) {
	if( func[i] == '-' && nest[i] > 1 ) {
		for( j = i; j < nrows; j++ ) {
			if( func[j] == '-' && nest[j] == nest[i]-1 ) {
				y1 = Eay( EDYhi - yval[i] ); /* flip */
				y2 = Eay( EDYhi - yval[j] );
				Emov( Eax( xval[i] ), y1 ); 
				if( squaredoff ) Elin( Eax( xval[j] ), y1 ); 
				Elin( Eax( xval[j] ), y2 );
				break;
				}
			}
		}
	}

/* dots at forks.. */
radius = 0.05;
if( strcmp( symbol, "" ) != 0 ) {     
	symdet( "symbol", symbol, symcode, &radius );
	for( i = 0; i < nrows; i++ ) {
	    if( func[i] == '-' ) {
		x = Eax( xval[i] );
		y = Eay( EDYhi - yval[i] ); /* flip */
		Emark( x, y, symcode, radius ); 
		}
	    }
	}

/* leaf labels.. */
textdet( "textdetails", textdetails, &align, &adjx, &adjy, -1, "R", 1.0 );
for( i = 0; i < nrows; i++ ) {
	if( label[i][0] != '\0' ) {
		x = Eax( xval[i] );
		y = Eay( EDYhi - yval[i] ); /* flip */
		Emov( x+0.01, y );
		Etext( label[i] );
		}
	}

/* do root dot */
if( GL_smember( rootsym, "no none" ) == 0 && squaredoff ) {
	i = nrows-1;
	symdet( "root", rootsym, symcode, &radius );
	/* Emark( Eax( xval[i] ), Eay( EDYhi-yval[i] ), symcode, radius ); */
	Emark( Eax( xval[i] ), Eay( EDYhi-((double)tagcount/2.0) ), symcode, radius ); 
	}

TDH_setvar( "TREEDONE", "1" );
return( 0 );
}


#ifdef HOLD
/* ========================== */
/* ANGPOS - get new position based on current position & distance and angle in rads */
static int
angpos( oldx, oldy, dist, ang, newx, newy )
double oldx, oldy, dist, ang, *newx, *newy;
{
ang = (ang-HALFPI) * -1.0;
*newx = oldx + (dist * cos( ang ));
*newy = oldy + (dist * sin( ang ));
return( 0 );
}

#endif
