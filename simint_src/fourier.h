#ifndef fourier_h
#define fourier_h

#ifndef tksimint_h
#include "tksimint.h"
#endif

/*
 * The gridding mask.
 */
typedef struct {
  float xwidth;
  float ywidth;
  int xpix;
  int ypix;
  int dogrid;
} Gridparms;

float *zeropad_complex(float *datarr, int nx, int ny, int order, float value);
float *zeropad_real(float *datarr, int nx, int ny, int order);
float *zeropad1d(float *datarr, int nx);
float *grid2d(int ndata, float *xdata,float *ydata, float *zdata, 
	 float xmins,float ymins, int nx, int ny, float xpix, float ypix, int dogauss);
float *grid(int ndata, float *xdata, float *ydata, float xmin, 
	    int n, float xpix);
void rvgrid(int ndata, float *xdata, float *ydata, float *edata, float xmin, 
	    int n, float xpix, float *plarr);
void rvgrid2d(int ndata, float *xdata,float *ydata, float *zdata, float *edata, float *plarr, float xmin, float ymin, int nx, int ny, float xpix, float ypix);
void fft_shift(float *image, int adim, int bdim);
float gauss(float x, float x0, float sigma);
float gauss2d(float x, float x0, float y, float y0, float sigmax, float sigmay,
float phi);
Image *uvtrans(Simint *sim, Image *image);

#endif

