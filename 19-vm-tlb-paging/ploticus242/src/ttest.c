/* ttest - take two measurements (mean, n, sd) 
   and return the p value associated with the T-test.

   Used code from gnu scientific library and wctb.
*/

#define PI 3.141592653589

double sqrt(), atan(), fabs();


double GL_ttest( mean1, sd1, n1, mean2, sd2, n2 )
double mean1, sd1, mean2, sd2;
int n1, n2;
{
int i;
double tvalue, pooled_variance, var1, var2, df;
double t1, t2, t3, cdf;

var1 = sd1 * sd1;
var2 = sd2 * sd2;

pooled_variance = (((n1 - 1 ) * var1 ) + ((n2 - 1) * var2)) / (n1 + n2 - 2);

tvalue = (mean1 - mean2) / (sqrt (pooled_variance * ((1.0 / n1) + (1.0 / n2))));

df = (n1 + n2) - 2;

/* printf( "var1=%g  var2=%g  pooledvar=%g  tvalue=%g  df=%g\n", var1, var2, pooled_variance, tvalue, df ); */

/* now compute the pvalue on the tvalue... (from wctb/statlibx/ttest_p.c) */

t1 = df / ( df + (tvalue * tvalue) );

if( df/2 != (double)(int)( df/2 ) ) { /* odd df */
	t3 = 0;
	if( df != 1.0 ) {
		t2 = t3 = 2 * sqrt( t1 * ( 1 - t1 )) / PI;
		for( i = 1; i <= (int) (df/2) -1; i++ )
			{
			t2 = t2 * (double)(i) / ( (double)(i) + 0.5 ) * t1 ;
			t3 = t3 + t2;
			}
		}
	t3 = 1 - 2 / PI * atan( sqrt ( t1 / (1-t1) )) + t3;
	}

else {  /* even df */
	t2 = t3 = sqrt( 1 - t1 );
	for( i = 1; i <= df/2 -1; i++ )
		{
		t2 = t2 * ( (double)i - 0.5 ) / (double)i * t1;
		t3 = t3 + t2;
		}
	}

if( tvalue > 0 ) cdf = ( 1 + t3 / 2 );
else cdf = ( 1 - t3 / 2 );

if( tvalue >= 0 ) return( 1- fabs( ( 1-cdf ) * 2 ) );
else return( fabs( 1- ( 2 * cdf ) ));
}

/* main()
* {
* double ttest(), p;
*
* p = ttest( 35.0, 5.0, 5, 38.0, 7.0, 6 );
* printf( "p=%g\n", p );
* }
*/



/* ================================= */
/* ================================= */

#ifdef CUT
/* source code from GSL */
double 
FUNCTION(gsl_stats,ttest) (const BASE data1[], 
                           const size_t stride1, const size_t n1, 
                           const BASE data2[],
                           const size_t stride2, const size_t n2)
{
  /* runs a t-test between two datasets representing independent
     samples. Tests to see if the difference between means of the
     samples is different from zero */

  /* find means for the two samples */
  const double mean1 = FUNCTION(gsl_stats,mean) (data1, stride1, n1);
  const double mean2 = FUNCTION(gsl_stats,mean) (data2, stride2, n2);

  /* find pooled variance for the two samples */
  const double pv = FUNCTION(gsl_stats,pvariance) (data1, stride1, n1, data2, stride2, n2);

  /* calculate the t statistic */
  const double t = (mean1 - mean2) / (sqrt (pv * ((1.0 / n1) + (1.0 / n2))));

  return t;
}


double 
FUNCTION(gsl_stats,pvariance) (const BASE data1[], 
                               const size_t stride1, const size_t n1, 
                               const BASE data2[], 
                               const size_t stride2, const size_t n2)
{
  /* Find the pooled variance of two datasets */

  const double var1 = FUNCTION(gsl_stats,variance) (data1, stride1, n1);
  const double var2 = FUNCTION(gsl_stats,variance) (data2, stride2, n2);

  /* calculate the pooled variance */

  const double pooled_variance = 
    (((n1 - 1) * var1) + ((n2 - 1) * var2)) / (n1 + n2 - 2);

  return pooled_variance;
}

static double
FUNCTION(compute,variance) (const BASE data[], const size_t stride, const size_t n, const double mean)
{
  /* takes a dataset and finds the variance */

  long double variance = 0 ;

  size_t i;

  /* find the sum of the squares */
  for (i = 0; i < n; i++)
    {
      const long double delta = (data[i * stride] - mean);
      variance += (delta * delta - variance) / (i + 1);
    }

  return variance ;
}
#endif
