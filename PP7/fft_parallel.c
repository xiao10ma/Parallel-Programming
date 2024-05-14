#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <ctime>
#include "mpi.h"
using namespace std;

int main();
void ccopy(int n, double x[], double y[]);
void cfft2(int n, double x[], double y[], double w[], double sgn);
void cffti(int n, double w[]);
double cpu_time(void);
double ggl(double *ds);
void step(int n, int mj, double a[], double b[], double c[], double d[],
          double w[], double sgn);
void timestamp();

//****************************************************************************80

int comm_sz;
int my_rank;

int main()
//    The complex data in an N vector is stored as pairs of values in a
//    real vector of length 2*N.
{
  double ctime;
  double ctime1;
  double ctime2;
  double error;
  int first;
  double flops;
  double fnm1;
  int i;
  int icase;
  int it;
  int ln2;
  double mflops;
  int n;
  int nits = 10000;
  static double seed;
  double sgn;
  double *w;
  double *x;
  double *y;
  double *z;
  double z0;
  double z1;

  // initialize MPI
  MPI_Init(NULL, NULL);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

  if (my_rank == 0)
  {
    timestamp();
    cout << "\n";
    cout << "FFT_SERIAL\n";
    cout << "  C++ version\n";
    cout << "\n";
    cout << "  Demonstrate an implementation of the Fast Fourier Transform\n";
    cout << "  of a complex data vector.\n";
    //
    //  Prepare for tests.
    //
    cout << "\n";
    cout << "  Accuracy check:\n";
    cout << "\n";
    cout << "    FFT ( FFT ( X(1:N) ) ) == N * X(1:N)\n";
    cout << "\n";
    cout << "             N      NITS    Error         Time          Time/Call     MFLOPS\n";
    cout << "\n";
  }

  seed = 331.0;
  n = 1;
  //
  //  LN2 is the log base 2 of N.  Each increase of LN2 doubles N.
  //

  for (ln2 = 1; ln2 <= 20; ln2++)
  {
    n = 2 * n;
    //
    //  Allocate storage for the complex arrays W, X, Y, Z.
    //
    //  We handle the complex arithmetic,
    //  and store a complex number as a pair of doubles, a complex vector as a doubly
    //  dimensioned array whose second dimension is 2.
    //
    w = new double[n];
    x = new double[2 * n];
    y = new double[2 * n];
    z = new double[2 * n];

    first = 1;

    for (icase = 0; icase < 2; icase++)
    {
      if (first)
      {
        for (i = 0; i < 2 * n; i = i + 2)
        {
          z0 = ggl(&seed);
          z1 = ggl(&seed);
          x[i] = z0;
          z[i] = z0;
          x[i + 1] = z1;
          z[i + 1] = z1;
        }
      }
      else
      {
        for (i = 0; i < 2 * n; i = i + 2)
        {
          z0 = 0.0;
          z1 = 0.0;
          x[i] = z0;
          z[i] = z0;
          x[i + 1] = z1;
          z[i + 1] = z1;
        }
      }
      //
      //  Initialize the sine and cosine tables.
      //
      cffti(n, w);
      //
      //  Transform forward, back
      //
      if (first)
      {
        sgn = +1.0;
        cfft2(n, x, y, w, sgn);
        sgn = -1.0;
        cfft2(n, y, x, w, sgn);
        //
        //  Results should be same as initial multiplied by N.
        //
        fnm1 = 1.0 / (double)n;
        error = 0.0;
        for (i = 0; i < 2 * n; i = i + 2)
        {
          error = error + pow(z[i] - fnm1 * x[i], 2) + pow(z[i + 1] - fnm1 * x[i + 1], 2);
        }
        error = sqrt(fnm1 * error);
        if (my_rank == 0)
        {
          cout << "  " << setw(12) << n
               << "  " << setw(8) << nits
               << "  " << setw(12) << error;
        }
        first = 0;
      }
      else
      {
        // record time
        ctime1 = MPI_Wtime();
        // ctime1 = cpu_time ( );
        for (it = 0; it < nits; it++)
        {
          sgn = +1.0;
          cfft2(n, x, y, w, sgn);
          sgn = -1.0;
          cfft2(n, y, x, w, sgn);
        }
        ctime2 = MPI_Wtime();
        // ctime2 = cpu_time ( );
        ctime = ctime2 - ctime1;

        flops = 2.0 * (double)nits * (5.0 * (double)n * (double)ln2);

        mflops = flops / 1.0E+06 / ctime;
        if (my_rank == 0)
        {
          cout << "  " << setw(12) << ctime
               << "  " << setw(12) << ctime / (double)(2 * nits)
               << "  " << setw(12) << mflops << "\n";
        }
      }
    }
    if ((ln2 % 4) == 0)
    {
      nits = nits / 10;
    }
    if (nits < 1)
    {
      nits = 1;
    }

    //cout << "error:" << endl;

    delete[] w;
    delete[] x;
    delete[] y;
    delete[] z;
  }

  if (my_rank == 0)
  {
    cout << "\n";
    cout << "FFT_SERIAL:\n";
    cout << "  Normal end of execution.\n";
    cout << "\n";
    timestamp();
  }

  MPI_Finalize();

  return 0;
}

void ccopy(int n, double x[], double y[])
//    CCOPY copies a complex vector.
//    The "complex" vector A[N] is actually stored as a double vector B[2*N].
//
//    The "complex" vector entry A[I] is stored as:
//
//      B[I*2+0], the real part,
//      B[I*2+1], the imaginary part.
//  Parameters:
//
//    Input, int N, the length of the "complex" array.
//
//    Input, double X[2*N], the array to be copied.
//
//    Output, double Y[2*N], a copy of X.
//
{
  int i;

  for (i = 0; i < n; i++)
  {
    y[i * 2 + 0] = x[i * 2 + 0];
    y[i * 2 + 1] = x[i * 2 + 1];
  }
  return;
}

void cfft2(int n, double x[], double y[], double w[], double sgn)
//    CFFT2 performs a complex Fast Fourier Transform.
//  Parameters:
//
//    Input, int N, the size of the array to be transformed.
//
//    Input/output, double X[2*N], the data to be transformed.
//    On output, the contents of X have been overwritten by work information.
//
//    Output, double Y[2*N], the forward or backward FFT of X.
//
//    Input, double W[N], a table of sines and cosines.
//
//    Input, double SGN, is +1 for a "forward" FFT and -1 for a "backward" FFT.
{
  int j;
  int m;
  int mj;
  int tgle;

  m = (int)(log((double)n) / log(1.99));
  mj = 1;
  //
  //  Toggling switch for work array.
  //  切换工作数组
  //
  tgle = 1;
  step(n, mj, &x[0 * 2 + 0], &x[(n / 2) * 2 + 0], &y[0 * 2 + 0], &y[mj * 2 + 0], w, sgn);

  if (n == 2)
  {
    return;
  }

  for (j = 0; j < m - 2; j++)
  {
    mj = mj * 2;
    if (tgle)
    {
      step(n, mj, &y[0 * 2 + 0], &y[(n / 2) * 2 + 0], &x[0 * 2 + 0], &x[mj * 2 + 0], w, sgn);
      tgle = 0;
    }
    else
    {
      step(n, mj, &x[0 * 2 + 0], &x[(n / 2) * 2 + 0], &y[0 * 2 + 0], &y[mj * 2 + 0], w, sgn);
      tgle = 1;
    }
  }
  //
  //  Last pass thru data: move y to x if needed
  //
  if (tgle)
  {
    ccopy(n, y, x);
  }

  mj = n / 2;
  step(n, mj, &x[0 * 2 + 0], &x[(n / 2) * 2 + 0], &y[0 * 2 + 0], &y[mj * 2 + 0], w, sgn);

  return;
}

void cffti(int n, double w[])
//  Purpose:
//
//    CFFTI sets up sine and cosine tables needed for the FFT calculation.
//  Parameters:
//
//    Input, int N, the size of the array to be transformed.
//
//    Output, double W[N], a table of sines and cosines.
//
{
  double arg;
  double aw;
  int i;
  int n2;
  const double pi = 3.141592653589793;

  n2 = n / 2;
  aw = 2.0 * pi / ((double)n);

  for (i = 0; i < n2; i++)
  {
    arg = aw * ((double)i);
    w[i * 2 + 0] = cos(arg);
    w[i * 2 + 1] = sin(arg);
  }
  return;
}

double cpu_time(void)
//  Purpose:
//
//    CPU_TIME reports the elapsed CPU time.
//
//  Modified:
//
//    27 September 2005
//
//  Author:
//
//    John Burkardt
//
//  Parameters:
//
//    Output, double CPU_TIME, the current total elapsed CPU time in second.
//
{
  double value;

  value = (double)clock() / (double)CLOCKS_PER_SEC;

  return value;
}

double ggl(double *seed)
//****************************************************************************80
//
//  Purpose:
//
//    GGL generates uniformly distributed pseudorandom numbers.
//  Parameters:
//
//    Input/output, double *SEED, used as a seed for the sequence.
//
//    Output, double GGL, the next pseudorandom value.
//
{
  double d2 = 0.2147483647e10;
  double t;
  double value;

  t = *seed;
  t = fmod(16807.0 * t, d2);
  *seed = t;
  value = (t - 1.0) / (d2 - 1.0);

  return value;
}

// x和y的长度是2n，n个复数
void step(int n, int mj, double a[], double b[], double c[], double d[], double w[], double sgn) {
  double ambr;
  double ambu;
  int j;
  int ja;
  int jb;
  int jc;
  int jd;
  int jw;
  int k;
  int lj;
  int mj2;
  double wjw[2];

  mj2 = 2 * mj;
  lj = n / mj2;

  // avg complex number
  int local_size = (lj % comm_sz == 0) ? (lj / comm_sz) : (lj / comm_sz + 1);
  // complex start position
  int startlocal = (my_rank * local_size > lj) ? lj : my_rank * local_size;
  // 每个进程结束处理的复数的位置（取整余数）
  int endlocal = ((my_rank + 1) * local_size > lj) ? lj : (my_rank + 1) * local_size;

  int count = 0;  // 用于计算传输大小

  if (my_rank == 0) {
    for (int i = 0; i < comm_sz; i++) {
      int start = (i * local_size > lj) ? lj : i * local_size;
      int end = ((i + 1) * local_size) > lj ? lj : (i + 1) * local_size;
      MPI_Send(&a[start * mj2], (end - start) * mj2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
      MPI_Send(&b[start * mj2], (end - start) * mj2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
      MPI_Send(&w[start * mj2], (end - start) * mj2, MPI_DOUBLE, i, 0, MPI_COMM_WORLD);
      // count = (end - start) * mj2;
      // double sendbuf[count * 3];
      // int position = 0;
      // MPI_Pack(&a[start * mj2], count, MPI_DOUBLE, sendbuf, count * 3 * sizeof(MPI_DOUBLE), &position, MPI_COMM_WORLD);
      // MPI_Pack(&b[start * mj2], count, MPI_DOUBLE, sendbuf, count * 3 * sizeof(MPI_DOUBLE), &position, MPI_COMM_WORLD);
      // MPI_Pack(&w[start * mj2], count, MPI_DOUBLE, sendbuf, count * 3 * sizeof(MPI_DOUBLE), &position, MPI_COMM_WORLD);
      // MPI_Send(sendbuf, position, MPI_PACKED, i, 0, MPI_COMM_WORLD);
    }
  } 
  MPI_Recv(&a[startlocal * mj2], (endlocal - startlocal) * mj2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_Recv(&b[startlocal * mj2], (endlocal - startlocal) * mj2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  MPI_Recv(&w[startlocal * mj2], (endlocal - startlocal) * mj2, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  // count = (endlocal - startlocal) * mj2;
  // double recvbuf[count * 3];
  // MPI_Recv(recvbuf, count * 3 * sizeof(MPI_DOUBLE), MPI_PACKED, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  // int position = 0;
  // MPI_Unpack(recvbuf, count * 3 * sizeof(MPI_DOUBLE), &position, &a[startlocal * mj2], count, MPI_DOUBLE, MPI_COMM_WORLD);
  // MPI_Unpack(recvbuf, count * 3 * sizeof(MPI_DOUBLE), &position, &b[startlocal * mj2], count, MPI_DOUBLE, MPI_COMM_WORLD);
  // MPI_Unpack(recvbuf, count * 3 * sizeof(MPI_DOUBLE), &position, &w[startlocal * mj2], count, MPI_DOUBLE, MPI_COMM_WORLD);

  // 计算
  for (j = startlocal; j < endlocal; j++) {
    jw = j * mj;
    ja = jw;
    jb = ja;
    jc = j * mj2;
    jd = jc;

    wjw[0] = w[jw * 2 + 0];
    wjw[1] = w[jw * 2 + 1];

    if (sgn < 0.0)
    {
      wjw[1] = -wjw[1];
    }

    for (k = 0; k < mj; k++)
    {
      c[(jc + k) * 2 + 0] = a[(ja + k) * 2 + 0] + b[(jb + k) * 2 + 0];
      c[(jc + k) * 2 + 1] = a[(ja + k) * 2 + 1] + b[(jb + k) * 2 + 1];
      ambr = a[(ja + k) * 2 + 0] - b[(jb + k) * 2 + 0];
      ambu = a[(ja + k) * 2 + 1] - b[(jb + k) * 2 + 1];
      d[(jd + k) * 2 + 0] = wjw[0] * ambr - wjw[1] * ambu;
      d[(jd + k) * 2 + 1] = wjw[1] * ambr + wjw[0] * ambu;
    }
  }

  if(my_rank == 0) {
    for (int i = 0; i < comm_sz; i++) {
      int start = (i * local_size > lj) ? lj : i * local_size;
      int end = ((i + 1) * local_size) > lj ? lj : (i + 1) * local_size;
      if(start == end) continue;
      MPI_Recv(&c[start * mj2 * 2], (end - start - 1) * mj2 * 2 + 2 * mj, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      MPI_Recv(&d[start * mj2 * 2], (end - start - 1) * mj2 * 2 + 2 * mj, MPI_DOUBLE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // count = (end - start - 1) * mj2 * 2 + 2 * mj;
      // double recvbuf[count * 2];
      // MPI_Recv(recvbuf, count * 2 * sizeof(MPI_DOUBLE), MPI_PACKED, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      // int position = 0;
      // MPI_Unpack(recvbuf, count * 2 * sizeof(MPI_DOUBLE), &position, &c[start * mj2 * 2], count, MPI_DOUBLE, MPI_COMM_WORLD);
      // MPI_Unpack(recvbuf, count * 2 * sizeof(MPI_DOUBLE), &position, &d[start * mj2 * 2], count, MPI_DOUBLE, MPI_COMM_WORLD);
    }
  } 
  else if(endlocal - startlocal > 0) {
    MPI_Send(&c[startlocal * mj2 * 2], (endlocal - startlocal - 1) * mj2 * 2 + 2 * mj, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    MPI_Send(&d[startlocal * mj2 * 2], (endlocal - startlocal - 1) * mj2 * 2 + 2 * mj, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    // count = (endlocal - startlocal - 1) * mj2 * 2 + 2 * mj;
    // double sendbuf[count * 2];
    // int position = 0;
    // MPI_Pack(&c[startlocal * mj2 * 2], count, MPI_DOUBLE, sendbuf, count * 2 * sizeof(MPI_DOUBLE), &position, MPI_COMM_WORLD);
    // MPI_Pack(&d[startlocal * mj2 * 2], count, MPI_DOUBLE, sendbuf, count * 2 * sizeof(MPI_DOUBLE), &position, MPI_COMM_WORLD);
    // MPI_Send(sendbuf, position, MPI_PACKED, 0, 0, MPI_COMM_WORLD);
  } 
  
  return;
}

void timestamp()
//  TIMESTAMP prints the current YMDHMS date as a time stamp.
//
//  Example:
//    31 May 2001 09:45:54 AM
{
#define TIME_SIZE 40

  static char time_buffer[TIME_SIZE];
  const struct tm *tm;
  time_t now;

  now = time(NULL);
  tm = localtime(&now);

  strftime(time_buffer, TIME_SIZE, "%d %B %Y %I:%M:%S %p", tm);

  cout << time_buffer << "\n";

  return;
#undef TIME_SIZE
}
