/* simple example using ploticus API to run 2 jobs */

/* compile and load w/ libploticus, libpng, libz, and -lm */
/* scg uses:  gcc apitest.c /home/scg/ploticus/src/libploticus.a  /home/scg/lib/libpng.a  /home/scg/lib/libz.a -lm -o apitest  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define RESULT_TYPE "png"

main( argc, argv )
int argc;
char **argv;
{
int stat;
char buf[256], outfile[80], memcommand[80];

if( argc != 2 ) {
	fprintf( stderr, "usage: api_examp   plscriptfile\n" );
	exit( 1 );
	}

printf( "This test will run two tests, with -debug on.  Memory footprint will be shown before, after, and between tests.\n" );

sprintf( memcommand, "ps -p %d -o vsz -o rss >&2", getpid() );

/* show preliminary mem usage */
system( memcommand );

/* test 1.  executing a script file... */

sprintf( outfile, "apitest1.%s", RESULT_TYPE );

stat = ploticus_init( RESULT_TYPE, outfile );
if( stat ) { fprintf( stderr, "error %d on ploticus_init\n", stat ); exit(1); }

stat = ploticus_arg( "-debug", "" );
if( stat )  { fprintf( stderr, "error %d on ploticus_arg\n", stat ); exit(1); }

stat = ploticus_execscript( argv[1], 0 );
if( stat ) { fprintf( stderr, "error %d on ploticus_execscript\n", stat ); exit(1); }

stat = ploticus_end();
if( stat ) { fprintf( stderr, "error %d on ploticus_end\n", stat ); exit(1); }

/* show another mem usage */
system( memcommand );


/* test 2.  embedded script lines... */

sprintf( outfile, "apitest2.%s", RESULT_TYPE );

ploticus_init( RESULT_TYPE, outfile );
stat = ploticus_arg( "-debug", "" );

strcpy( buf, "#proc annotate" ); ploticus_execline( buf );
strcpy( buf, "location: 2 2" ); ploticus_execline( buf );
strcpy( buf, "text: test of embedded script lines" ); ploticus_execline( buf );
strcpy( buf, "and multiple lines" ); ploticus_execline( buf );
strcpy( buf, "such as this..." ); ploticus_execline( buf );

ploticus_end();

/* show another mem usage */
system( memcommand );


}

