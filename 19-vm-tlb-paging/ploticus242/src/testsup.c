#include <stdio.h>

main()
{
char buf[80];

while( fgets( buf, 79, stdin ) != NULL ) {
	if( buf[0] == 'q' ) break;
	dotext( buf );
	}
}

/* ======================================== */
/* DOTEXT -  handle multi-line text. */
dotext( s )
char *s;
{
int i, slen;
char chunk[256];
double x, y;

/* superscript/subscript related.. */
int nsup;
double suppos[12];
int supi[12];
char supcon[12][20];
int clen, gettingsup;
int j, k, supj;

clen = 0;
gettingsup = 0;
nsup = 0;
chunk[0] = '\0';
for( i = 0; ; i++ ) {
	
	if( s[i] == '\n' || s[i] == '\0' ) { /* reached end of a line.. draw it */
		chunk[clen] = '\0';
		printf( "Primary: %s\n", chunk );
		if( nsup > 0 ) {
			/* render superscript */
			for( k = 0; k < nsup; k++ ) {
				printf( "Sub %d  : ", k+1 );
				for( j = 0; j < supi[k]; j++ ) printf( "." );
				printf( "%s\n", supcon[k] );
				}
			}
		
		if( s[i] == '\0' || s[i+1] == '\0' ) break;
		else clen = 0;
		}

	/* opening tag */
	else if( s[i] == '<' && s[i+1] == 's' && s[i+2] == 'u' && s[i+3] != '\0' && s[i+4] == '>' ) {
		i += 4;
		gettingsup = 1;
		/* suppos[ nsup ] = clen * Ecurtextwidth; */
		supi[ nsup ] = clen;
		supj = 0;
		}

	/* closing tag */
	else if( s[i] == '<' && s[i+1] == '/' && s[i+2] == 's' && s[i+3] == 'u' && s[i+4] != '\0' && s[i+5] == '>' ) {
		/* add a certain # of blanks to chunk (proportional to length of superscript in smaller font) */
		/* for graphic use "k < (supj+1)/2"  */
		for( k = 0; k < supj; k++ ) chunk[clen++] = ' ';
		supcon[nsup][supj] = '\0';
		i += 5;
		nsup++;
		gettingsup = 0;
		}
	else if( gettingsup ) supcon[nsup][supj++] = s[i];
	else chunk[clen++] = s[i];
	}
}
