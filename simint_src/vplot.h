#ifndef vplot_h
#define vplot_h

#ifndef color_h
#include "color_tab.h"
#endif

#ifndef tksimint_h
#include "tksimint.h"
#endif

#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define NPT 50

typedef enum {
  B_NORM, 
  B_LINE, 
  B_RECT, 
  B_YRNG, 
  B_XRNG, 
  B_YVAL, 
  B_XVAL,
  B_CROSS
} Bandmode;

int v_grey2(int ndata, float *zdata, int nx,int ny, float xmina, float xmaxa, 
	    float ymina, float ymaxa, float *flag, float z1, float z2,
	    char *xlab, char *ylab, char *title, char *unit);

int im_grey(Image *image, float *data, char *xlab, char *ylab, char *title, char *unit);

int v_hdraw(float hist[], int nbin,float xmin,float xmax,float ymin, 
		   float ymax,float dx, char *xlab, char *ylab, char *title);
int v_radplot(float data[],int nbin,float xmin,float xmax,
	      int dx, float ymin,float ymax,int dy);

#endif

