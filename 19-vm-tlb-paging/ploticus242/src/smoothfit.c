/*      Given a set of data points, generate a spline curve that passes thru all points.
 *     
 *      Author - Marlow etc.   Modified by - P. Ward     Date -  3.10.1973
 * 
 *      Extracted from the ROOT package by O.Couet.
 * 
 *      Re-structured to remove most gotos by Dino Ferrero Merlino
 *      (the numerical part has not been touched)
 * 
 *      Changed to be ANSI-C compliant by Oliver Koch.
 * 
 *      Integrated into ploticus by Steve Grubb  10/3/03
 * 
 *   Usage:
 * 
 *       Helps to compute a smooth representation of a curve.
 *    The history of this code is quite long...
 *   
 *    This routine draws a smooth tangentially continuous curve through
 *    the sequence of data points P(I) I=1,N where P(I)=(X(I),Y(I))
 *    the curve is approximated by a polygonal arc of short vectors .
 *    the data points can represent open curves, P(1) != P(N) or closed
 *    curves P(2) == P(N) . If a tangential discontinuity at P(I) is
 *    required , then set P(I)=P(I+1) . loops are also allowed .
 *   
 *    Reference Marlow and Powell,Harwell report No.R.7092.1972
 *    MCCONALOGUE,Computer Journal VOL.13,NO4,NOV1970PP392 6
 *   
 *    The routine was then adapted as the IGRAP1 routine of HIGZ.
 *    The FORTRAN code was translated with f2c and adapted to ROOT.
 * 
 *   
 *   TODO:
 * 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define false 0;
#define true  1;

  static int npoints, k, kp, NpointsMax, banksize, npt;
  static double xratio, yratio, sxmin, symin;
  static int closed, flgis, iw;
  static double P1, P2, P3, P4, P5, P6, W1, W2, W3, A, B, C, R, S, T, Z;
  static double CO, SO, CT, ST, CTU, STU, XNT, DX1, DY1, DX2, DY2, XO, YO, DX, DY, XT, YT;
  static double XA, XB, YA, YB, U1, U2, U3, TJ, SB, STH;
  static double wsign, tsquare, tcube, *qlx, *qly, *x, *y;
  static const int maxiterations = 20;
  static const double delta = 0.00055;

static int QpSmootherInit( int nPoints, double in[][2] );
static int QpSmootherDone();
static int continousTangentAtEndPoints() ;
static int scaleRatio() ;
static int skipConsecutiveEqualPoints(int start) ;
static void computeDirectionCosine (int kMinus1, int kP, int kPlus1) ;
static int resizeSmoothArrays() ;
static void computeStraightLine(int kP) ;
static void newPoint() ;
static void dirCosL150() ;
static void dirCosL130() ;
static void computeCubicCoeffs() ;
static int smooth() ;
static void zero(int *kCode, double AZ, double BZ, double E2, double *X, double *Y, int maxiterations);
static void checkMaxIterations(int *kCode, int *IT, int J1, double *X, double X2, int maxiterations);
/* static int getNPoint() ; */
static double* getX ();
static double* getY ();
static double min( double f, double g );
static double max( double f, double g );


/* --------------------------------------------------- */
/* SMOOTHFIT - ploticus entry point  */
int
PL_smoothfit( in, ninp, out, noutp )
double in[][2]; /* input vector */
int ninp; 	/* # input points */
double out[][2]; /* output vector */
int *noutp; 	/* # output points */
{
int i, stat;
double *resultx, *resulty;

stat = QpSmootherInit( ninp, in );
if( stat ) return( stat );

*noutp = smooth();

/* *noutp = getNPoint(); */

resultx = getX();
resulty = getY();

for( i = 0; i < *noutp; i++ ) {
	out[i][0] = resultx[i];
	out[i][1] = resulty[i];
	}

stat = QpSmootherDone();

return( 0 );
}


/* ------------------------------- */

/* void QpSmootherInit(int nPoints, double *_x, double *_y) */

static int QpSmootherInit( nPoints, in )
int nPoints;
double in[][2];
{
int i;

C = T = CO = SO = CT = ST = CTU = STU = DX1 = DY1 = DX2 = DY2 = 0;
XT = YT = XA = XB = YA = YB = U1 = U2 = U3 = TJ = SB = 0;
/* make a local copy of the original polylines (smoother will scale them) */
npoints = nPoints;
x = (double *) malloc(sizeof(double) * npoints);
y = (double *) malloc(sizeof(double) * npoints);
if (x == NULL || y == NULL) return( 5720 ); /* Not enough memory for qpsmoother */
else 	{
    	for (i=0;i<npoints;i++) {
      		x[i] = in[i][0];
      		y[i] = in[i][1];
		}

	NpointsMax  = npoints*10;
	banksize    = NpointsMax;
	
	qlx = (double *) malloc(sizeof(double) * NpointsMax);
	qly = (double *) malloc(sizeof(double) * NpointsMax);
	if (qlx == NULL || qly == NULL) return( 5720 );  /* Not enough memory for qpsmoother */
	else 	{
	    	qlx[0] = x[0];
	    	qly[0] = y[0];
	    	/* Scale data and check whether endpoints collide */
	    	closed = scaleRatio();
		}
	}
return( 0 );
}

/* ------------------------------- */
static int QpSmootherDone() {
  free(x);
  free(y);
  free(qlx);
  free(qly);  
  return( 0 );
}

/* ------------------------------- */
static int continousTangentAtEndPoints() {
  int tangent = false;

  if (x[0] != x[npoints-1] || y[0] != y[npoints-1]) 
    tangent = true;    
  if (x[npoints-2] == x[npoints-1] && y[npoints-2] == y[npoints-1])
    tangent = true;
  if (x[0] == x[1] && y[0] == y[1])  
    tangent = true;
  return tangent;
}

/* ------------------------------- */
/** Scale data to the range 0-ratio_signs in X, 0-1 in Y
  where ratio_signs is the ratio between the number of changes
  of sign in Y divided by the number of changes of sign in X.
  Returns whether the endpoints are overlapping or not.
*/

static int scaleRatio() {
   double ratio_signs;
   double sxmax, symax;
   int i;
   double six;
   double siy;
   double dx1n;
   double dy1n;

   closed = false;
   sxmin = x[0];
   sxmax = x[0];
   symin = y[0];
   symax = y[0];
   six   = 1;
   siy   = 1;
   for (i=1;i<npoints;i++) {
      if (i > 1) {
         if ((x[i]-x[i-1])*(x[i-1]-x[i-2]) < 0) six++;
         if ((y[i]-y[i-1])*(y[i-1]-y[i-2]) < 0) siy++;
      }
      if (x[i] < sxmin) sxmin = x[i];
      if (x[i] > sxmax) sxmax = x[i];
      if (y[i] < symin) symin = y[i];
      if (y[i] > symax) symax = y[i];
   }
   dx1n = fabs(x[npoints-1]-x[0]);
   dy1n = fabs(y[npoints-1]-y[0]);
   if (dx1n < 0.01*(sxmax-sxmin) && dy1n < 0.01*(symax-symin))  
     closed = true;
   if (sxmin == sxmax)
     xratio = 1;
   else {
      if (six > 1) 
	ratio_signs = siy/six;
      else         
	ratio_signs = 20;
      xratio = ratio_signs/(sxmax-sxmin);
   }
   if (symin == symax) 
     yratio = 1;
   else                
     yratio = 1/(symax-symin);

   for (i=0;i<npoints;i++) {
      x[i] = (x[i]-sxmin)*xratio;
      y[i] = (y[i]-symin)*yratio;
   }

   return closed;
}

/* ------------------------------- */
static int skipConsecutiveEqualPoints(int start) {
  k = start;
  while((k < npoints) &&  (x[k] == x[k-1] && y[k] == y[k-1])) {
      k++;
  }
  return k;
}

/* ------------------------------- */
/* Calculate the direction cosines at P(k) obtained from P(kMinus1),P(k),P(kPlus1).
 * If k=1 then N-1 is used for kMinus1. If k=N then 2 is used for kPlus1
 */

static void computeDirectionCosine (int kMinus1, int kP, int kPlus1) {
  double DK1,DK2;
  DX1 = x[kP]  - x[kMinus1];
  DY1 = y[kP]  - y[kMinus1];
  DX2 = x[kPlus1] - x[kP];
  DY2 = y[kPlus1] - y[kP];
  DK1 = DX1*DX1 + DY1*DY1;
  DK2 = DX2*DX2 + DY2*DY2;
  CTU = DX1*DK2 + DX2*DK1;
  STU = DY1*DK2 + DY2*DK1;
  XNT = CTU*CTU + STU*STU;
  /* If both ctu and stu are zero,then default.This can
   * occur when P(k)=P(k+1). I.E. A loop.
   */
  if (XNT < 1.E-25) {
    CTU = DY1;
    STU =-DX1;
    XNT = DK1;
  }
  /*  Normalise direction cosines. */
  CT = CTU/sqrt(XNT);
  ST = STU/sqrt(XNT);
}


/* ------------------------------- */
static int resizeSmoothArrays() {
  int i;
  int newsize = banksize + NpointsMax;
  double *qt1=qlx;
  double *qt2=qly;
  qlx = (double *) malloc(sizeof(double) * newsize);
  qly = (double *) malloc(sizeof(double) * newsize);
  for (i=0;i<banksize;i++) {
    qlx[i] = qt1[i];
    qly[i] = qt2[i];
  }
  free(qt1);
  free(qt2);
  banksize = newsize;
  return newsize;
}

/* ------------------------------- */
static void computeStraightLine(int kP) {
  /* Compute a straight line between P(k-1) and P(k) in Fortran
   * (XT,YT) will contain the coordinate of the endpoint
   */
  XT = x[kP] ;
  YT = y[kP] ;
}

/* ------------------------------- */
static void newPoint() {
  /* Convert coordinates to original system */
  qlx[npt] = sxmin + XT/xratio;
  qly[npt] = symin + YT/yratio;
  npt++;
  /* If the arrays are filled up, enlarge them. */
  if (npt >=  banksize) 
    banksize = resizeSmoothArrays();
}

/* ------------------------------- */
/*  Direction cosines at P(k) obtained from P(k-2),P(k-1),P(k). */
static void dirCosL150() {
   W3    = 2*(DX1*DY2-DX2*DY1);
   CT    = CTU-W3*DY2;
   ST    = STU+W3*DX2;
   XNT   = 1/sqrt(CT*CT+ST*ST);
   CT    = CT*XNT;
   ST    = ST*XNT;
   flgis = 0;
}

/* ------------------------------- */
/* Direction cosines at P(k-1) obtained from P(k-1),P(k),P(k+1). */
static void dirCosL130() {
  W3    = 2*(DX1*DY2-DX2*DY1);
  CO    = CTU+W3*DY1;
  SO    = STU-W3*DX1;
  XNT   = 1/sqrt(CO*CO+SO*SO);
  CO    = CO*XNT;
  SO    = SO*XNT;
  flgis = 1;
}


/* ------------------------------- */

/*  For the arc between P(k-1) and P(k) with direction cosines CO,
 *  SO and CT,ST respectively, calculate the coefficients of the
 *  parametric cubic represented by X(T) and Y(T) where
 *  X(T)=XA*T**3 + XB*T**2 + CO*T + XO
 *  Y(T)=YA*T**3 + YB*T**2 + SO*T + YO
 */

static void computeCubicCoeffs() {
  double CC ;
  double ERR ;
  int zeroLoop;

  XO = x[k-2];
  YO = y[k-2];
  DX = x[k-1] - XO;
  DY = y[k-1] - YO;

  /*  Initialise the values of X(TI),Y(TI) in XT and YT respectively. */

  XT = XO;
  YT = YO;
  C  = DX*DX+DY*DY;
  A  = CO+CT;
  B  = SO+ST;
  R  = DX*A+DY*B;
  T  = C*6/(sqrt(R*R+2*(7-CO*CT-SO*ST)*C)+R);
  tsquare = T*T;
  tcube   = T*tsquare;
  XA = (A*T-2*DX)/tcube;
  XB = (3*DX-(CO+A)*T)/tsquare;
  YA = (B*T-2*DY)/tcube;
  YB = (3*DY-(SO+B)*T)/tsquare;

  /*  If the curve is close to a straight line then use a straight
   *  line between (XO,YO) and (XT,YT).
   */

  if (.75*max(fabs(DX*SO-DY*CO),fabs(DX*ST-DY*CT)) <= delta) {
    computeStraightLine(k-1);
    newPoint();
  } else {

    /*   Calculate a set of values 0 == T(0).LTCT(1) <  ...  < T(M)=TC
     *  such that polygonal arc joining X(T(J)),Y(T(J)) (J=0,1,..M)
     *  is within the required accuracy of the curve 
     */

    TJ = 0;
    U1 = YA*XB-YB*XA;
    U2 = YB*CO-XB*SO;
    U3 = SO*XA-YA*CO;

    /*  Given T(J), calculate T(J+1). The values of X(T(J)), */
    /*  Y(T(J)) T(J) are contained in XT,YT and TJ respectively. */

    do {
      S  = T - TJ;
      iw = -2;

      /*  Define iw here later. */

      P1 = (2*U1)*TJ-U3;
      P2 = (U1*TJ-U3)*3*TJ+U2;
      P3 = 3*TJ*YA+YB;
      P4 = (P3+YB)*TJ+SO;
      P5 = 3*TJ*XA+XB;
      P6 = (P5+XB)*TJ+CO;
      CC  = 0.8209285;
      ERR = 0.1209835;
      iw -= 2;


    L200:
      /*  Test D(TJ,THETA). A is set to (Y(TJ+S)-Y(TJ))/S.B is
       *  set to (X(TJ+S)-X(TJ))/S.
       */
      A   = (S*YA+P3)*S+P4;
      B   = (S*XA+P5)*S+P6;

      /*  Set Z to PSI(D/delta)-CC. */

      W1 = -S*(S*U1+P1);
      W2 = S*S*U1-P2;
      W3 = 1.5*W1+W2;

      /*  Set the estimate of (THETA-TJ)/S.Then set the numerator
       *  of the expression (EQUATION 4.4)/S. Then set the square 
       *  of D(TJ,TJ+S)/delta. Then replace Z by PSI(D/delta)-CC. 
       */

      if (W3 > 0) 
        wsign = fabs(W1);
      else        
        wsign = -fabs(W1);
      STH = 0.5+wsign/(3.4*fabs(W1)+5.2*fabs(W3));
      Z   = S*STH*(S-S*STH)*(W1*STH+W1+W2);
      Z   = Z*Z/((A*A+B*B)*(delta*delta));
      Z   = (Z+2.642937)*Z/((.3715652*Z+3.063444)*Z+.2441889)-CC;

      /*  Branch if Z has been calculated */
      zeroLoop=true;
      if (iw <= 0) {
        if (Z > ERR) {
          kp = 0;
          C  = Z;
          SB = S;
        } else {
    L220:
          if (iw+2 == 0) {
            /* goto L190; */
            iw -= 2;
            goto L200;
          }

          if (iw+2 >  0) {
            /* Update TJ,XT and YT. */
            XT = XT + S*B;
            YT = YT + S*A;
            TJ = S  + TJ;
          } else {
            /* Last part of arc. */
            XT = x[k-1];
            YT = y[k-1];
            S  = 0;
          }
          newPoint();
          zeroLoop=false; /* passato qui. S>0  */
        }
      }

      /*  Z(S). find a value of S where 0 <= S <= SB such that */
      /*  fabs(Z(S)) < ERR */

      while(zeroLoop) {
        zero(&kp,0,SB,ERR,&S,&Z,maxiterations);
        if (kp == 2) {
          /* goto L210; */
          iw -= 2;
          goto L220;
        }
        if (kp > 2) {
          fprintf( stderr, "Attempt to plot outside plot limits\n" );
          XT = x[k-1];
          YT = y[k-1];
          newPoint();
          /* goto L310; */
          break;
        }
        if (iw > 0) goto L200;

        /* Set Z=Z(S) for S=0. */

        if (iw < 0) {
          Z  = -CC;
          iw = 0;
        } else {
          /* Set Z=Z(S) for S=SB. */
          Z  = C;
          iw = 1;
        }
      } /*  End while (zeroLoop) */
    } while ( S > 0 );
  }
}

/* ------------------------------- */
static int smooth() {
  npt      = 1;
  k        = 1;

/*   flgis is true in the main loop, but is false if there is
 *  a deviation from the main loop (cusp or endpoint? DinoFM) 
 */

   /* The curve is open if :
    *     the two endpoints are not overlapping (closed == false)
    *     AND there is a continuous tangent at the endpoints */

   if (!closed && continousTangentAtEndPoints()) {
     closed = true;
     flgis = 0;
     k++;
   } else {
     closed = false;
     flgis = 1;
     /*  Calculate direction cosines at P(0) using P(N-1),P(0),P(1). */
     computeDirectionCosine(npoints-2,0,1);
     /* Carry forward the direction cosines from the previous arc. */
     CO = CT;
     SO = ST;
     k++;
   }

   while (k <= npoints) {
     k=skipConsecutiveEqualPoints(k);
     /* More arcs to compute */
     if (k < npoints) {
       /*  Test whether P(k) is a cusp. */
       if (x[k-1] == x[k] && y[k-1] == y[k]) {
	 if (flgis) {
	   dirCosL150();
	   computeCubicCoeffs();
	 } else {
	   /* Make a straight line from P(k-1) to P(k). */
	   computeStraightLine(k-1);
	   newPoint();
	 }
       } else {
	 /*  k is always >= 2 ! */
	 computeDirectionCosine(k-2,k-1,k);
	 if (!flgis)
	   dirCosL130();
	   computeCubicCoeffs();
       }
     } else {
       /*  k == npoints. Last arc ! */
       if (!closed) {
	 /*  k is always >= 2 ! */
	 computeDirectionCosine(k-2,k-1,1);     
	 if (!flgis)
	   dirCosL130();
	 computeCubicCoeffs();
       } else {
	 if (flgis) {
	   dirCosL150();
	   computeCubicCoeffs();
	 } else {
	   /*  Make a straight line from P(k-1) to P(k). */
	   computeStraightLine(k-1);
	   newPoint();
	 }
       }
     }
     /* Branch if the next section of the curve begins at a cusp. */
     if (flgis) {
       /* Carry forward the direction cosines from the previous arc. */
       CO = CT;
       SO = ST;
     }
     k++;
   }
   return npt;
}

/* ------------------------------- */
static void zero(int *kCode, double AZ, double BZ, double E2, double *X, double *Y, int maxiterations)
{
   static double AA, BB, YAA, ytest, Y1, X1, H;
   static int J1, IT, J3, J2;
   double YBB, X2;
   int exitLoop;
   YBB = 0;
   exitLoop=false;

/*       Calculate Y(X) at X=AZ. */
  if ((*kCode) <= 0) {
     AA  = AZ;
     BB  = BZ;
     (*X)  = A;
     J1 = 1;
     IT = 1;
     (*kCode)  = J1;
     return;
  }

/*       Test whether Y(X) is sufficiently small. */

  if (fabs((*Y)) <= E2) { (*kCode) = 2; return; }

/*       Calculate Y(X) at X=BZ. */

  if (J1 == 1) {
     YAA = (*Y);
     (*X)  = BB;
     J1 = 2;
     return;
  }

/*       Test whether the signs of Y(AZ) and Y(BZ) are different.
 *       if not, begin the binary subdivision.
 */

  if ((J1 == 2) && (YAA*(*Y) >= 0)) {
    X1 = AA;
    Y1 = YAA;
    J1 = 3;
    H  = BB - AA;
    J2 = 1;
    X2 = AA + 0.5*H;
    J3 = 1;
    checkMaxIterations(&k,&IT,J1,X,X2,maxiterations);
    return;
  }

/*      Test whether a bracket has been found .
 *      If not,continue the search
 */

  if ((J1 != 2) || (YAA*(*Y) >= 0)) {
    if (J1 > 3) goto L170;
    if (YAA*(*Y) >= 0) {
      if (J3 >= J2) {
        H  = 0.5*H; J2 = 2*J2;
        AA  = X1;  YAA = Y1;  X2 = AA + 0.5*H; J3 = 1;
      }
      else {
        AA  = (*X);   YAA = (*Y);   X2 = (*X) + H;     J3++;
      }
      checkMaxIterations(&k,&IT,J1,X,X2,maxiterations);
      return;
    }
  }

/*       The first bracket has been found.calculate the next X by the
 *       secant method based on the bracket.
 */

  BB  = (*X);
  YBB = (*Y);
  J1 = 4;

  if (fabs(YAA) > fabs(YBB)) { 
     X1 = AA; Y1 = YAA; (*X)  = BB; (*Y)  = YBB;
  } else {
     X1 = BB; Y1 = YBB; (*X)  = AA; (*Y)  = YAA;
  }

/*       Use the secant method based on the function values Y1 and Y.
 *       check that X2 is inside the interval (A,B).
 */


  do {
    X2    = (*X)-(*Y)*((*X)-X1)/((*Y)-Y1);
    X1    = (*X);
    Y1    = (*Y);
    ytest = 0.5*min(fabs(YAA),fabs(YBB));
    if ((X2-AA)*(X2-BB) < 0) {
      checkMaxIterations(kCode,&IT,J1,X,X2,maxiterations);
      return;
    }

    /*       Calculate the next value of X by bisection . Check whether
     *       the maximum accuracy has been achieved.
     */

    X2    = 0.5*(AA+BB);
    ytest = 0;
    if ((X2-AA)*(X2-BB) >= 0) { 
      (*kCode) = 2;  return; 
    }
    checkMaxIterations(kCode,&IT,J1,X,X2,maxiterations);
    return;

    /*       Revise the bracket (A,B). */

  L170:
    if (J1 != 4) return;
    if (YAA*(*Y) < 0) { 
      BB  = (*X); YBB = (*Y);
    } else {
      AA  = (*X); YAA = (*Y);
    }

    /*       Use ytest to decide the method for the next value of X. */

    if (ytest <= 0) {
      if (fabs(YAA) > fabs(YBB)) { 
        X1 = AA; Y1 = YAA; (*X)  = BB; (*Y)  = YBB;
      } else {
        X1 = BB; Y1 = YBB; (*X)  = AA; (*Y)  = YAA;
      }
    } else {
      if (fabs((*Y))-ytest > 0)
        exitLoop=true;
    }
  } while (!exitLoop);
  
  X2 = 0.5*(AA+BB);
  ytest = 0;
  if ((X2-AA)*(X2-BB) >= 0) { 
     k = 2;
     return; 
  }
  checkMaxIterations(&k,&IT,J1,X,X2,maxiterations);
}

/* ------------------------------- */
/* Check whether (maxiterations) function values have been calculated. */

static void checkMaxIterations(int *kCode, int *IT, int J1, double *X, double X2, int maxiterations)
{
  (*IT)++;
  if ((*IT) >= maxiterations) {
    (*kCode) = J1;
  } else {
    (*X) = X2;
  }
}

/* ------------------------------- */
/* 
 * static int getNPoint() { 
 *  return(npoints);
 * } 
 */

/* ------------------------------- */
static double* getX () {
  return(qlx);
}

/* ------------------------------- */
static double* getY () {
  return(qly);
}

/* ------------------------------- */
static double min( f, g )
double f, g;
{
if( f < g ) return( f );
else return( g );
}

/* ------------------------------- */
static double max( f, g )
double f, g;
{
if( f > g ) return( f );
else return( g );
}
