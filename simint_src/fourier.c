#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fourier.h"
#include "fitsio.h"
#include "tksimint.h"
#include "vplot.h"
#include "rand.h"

#include "fftw.h"
#include "fftw-int.h"
#include "rfftw.h"

#define DEBUG

float *invert(float *data, Fitshead *header);

extern int Grid[2];
extern Gridparms Gridwdth;
extern void fourn(float data[], unsigned long nn[], int ndim, int isign);

/*.......................................................................
 * Zero-pad a real array into a complex array.
 *
 * Input:
 *  datarr  float  *  The input data array.
 *  nx        int     The number x-axis elements.
 *  ny        int     The number y-axis elements.
 *  order     int     A flag specifying the type of shifting to perform
 *                    on the data: 0 centers the data in the zero-padded
 *                    array, 1 shifts the data to the upper left quadrant,
 *                    and anything else shifts the data in wrap-around order.
 *  value   float  *  The value to insert in the alias region (usually this
 *                    is 0, but the DC level can be substituted to avoid
 *                    edge effects).
 *
 * Note that the x and y dimensions are the actual lengths of the axes of
 * the array, NOT the number of complex data points.
 *
 * Output:
 *  newdata float  *  A pointer to the zero-padded complex array of
 *                    2*(2*nx)*(2*ny) elements.
 */
float *zeropad_complex(float *datarr, int nx, int ny, int order, float value)
{
  float *newdata=NULL;
  int npad=8*nx*ny;
  int ymid,xmid;
  int i,j, k;

/* Allocate a complex zero-padded array */

  if((newdata = (float *)malloc(npad*sizeof(float)))==NULL) {
    fprintf(stderr,"Can't allocate the pad array.\n");
    return newdata;
  }
/*
 * First fill the array with 0 (or the passed value).
 */
  for(i=0;i < npad/2;i++) {
    newdata[2*i] = value;
    newdata[2*i+1] = 0.0;
  }
  ymid = ny/2;
  xmid = nx/2;
/*
 * Now insert the old data into the appropriate slots.
 */
  for(i=0;i < nx;i++)
    for(j=0;j < ny;j++) {
/*
 * In the middle of the data array
 */     
      if(order == 0) {
	k = i*2 + nx + (j+ymid)*4*nx;
	if(k >= npad)
	  fprintf(stderr,"Bad index: %d.\n",k);
      }
/*
 * In the upper left quadrant.
 */
      else if(order == 1) 
	k = i*2 + (j+ny)*4*nx;
/*
 * Wrap-around order.
 */   
      else {
	if(i < xmid) {
	  if(j < ymid) 
	    k = 3*nx + i*2 + (j + 3*ymid)*4*nx;
	  else
	    k = 3*nx + i*2 + (j - ymid)*4*nx;
	}
	else {
	  if(j < ymid) 
	    k = (i-xmid)*2 + (j + 3*ymid)*4*nx;
	  else
	    k = (i-xmid)*2 + (j - ymid)*4*nx;	
	}
      }
      newdata[k] = datarr[i + j*nx];
    }
  return newdata;
}
/*.......................................................................
 * Zero-pad a real array into a real array.
 *
 * Input:
 *  datarr  float  *  The input data array.
 *  nx        int     The number x-axis elements.
 *  ny        int     The number y-axis elements.
 *  order     int     A flag specifying the type of shifting to perform
 *                    on the data: 0 centers the data in the zero-padded
 *                    array, 1 shifts the data to the upper left quadrant,
 *                    and anything else shifts the data in wrap-around order.
 *
 * Note that the x and y dimensions are the actual lengths of the axes of
 * the array, NOT the number of complex data points.
 *
 * Output:
 *  newdata float  *  A pointer to the zero-padded complex array of
 *                    (2*nx)*(2*ny) elements.
 */
float *zeropad_real(float *datarr, int nx, int ny, int order)
{
  float *newdata=NULL;
/* Allocate a complex zero-padded array */
  int npad=4*nx*ny;
  int ymid,xmid;
  int i,j, k;

  if((newdata = (float *)malloc(npad*sizeof(float)))==NULL) {
    fprintf(stderr,"Can't allocate the pad array.\n");
    return newdata;
  }
  
/*
 * First fill the array with 0's.
 */
  for(i=0;i < npad;i++)
    newdata[i] = 0.0;

  ymid = ny/2;
  xmid = nx/2;

/*
 * Now insert the old data in the appropriate slots.
 */
  for(i=0;i < nx;i++)
    for(j=0;j < ny;j++) {
/*
 * In the middle of the data array
 */     
      if(order == 0) 
	k = i + xmid + (j+ymid)*2*nx;
/*
 * In the upper left quadrant.
 */
      else if(order == 1) 
	k = i + (j+ny)*2*nx;
/*
 * Wrap-around order.
 */   
      else {
	if(i < xmid) {
	  if(j < ymid) 
	    k = 3*nx + i*2 + (j + 3*ymid)*4*nx;
	  else
	    k = 3*nx + i*2 + (j - ymid)*4*nx;
	}
	else {
	  if(j < ymid) 
	    k = (i-xmid)*2 + (j + 3*ymid)*4*nx;
	  else
	    k = (i-xmid)*2 + (j - ymid)*4*nx;	
	}
      }
      newdata[k] = datarr[i + j*nx];
    }
  return newdata;
}
/*.......................................................................
 * Zero-pad a real 1d array into a complex array.
 *
 * Input:
 *  datarr  float  *  The input data array.
 *  nx        int     The number x-axis elements.
 *  ny        int     The number y-axis elements.
 *  order     int     A flag specifying the type of shifting to perform
 *                    on the data: 0 centers the data in the zero-padded
 *                    array, 1 shifts the data to the upper left quadrant,
 *                    and anything else shifts the data in wrap-around order.
 *
 * Note that the x and y dimensions are the actual lengths of the axes of
 * the array, NOT the number of complex data points.
 *
 * Output:
 *  newdata float  *  A pointer to the zero-padded complex array of
 *                    2*(2*nx) elements.
 */
float *zeropad1d(float *datarr, int nx)
{
  float *newdata=NULL;
/* Allocate a complex zero-padded array */
  int npad=4*nx;
  int i,k;

  if((newdata = (float *)malloc(npad*sizeof(float)))==NULL) {
    fprintf(stderr,"Can't allocate the pad array.\n");
    return newdata;
  }
/*
 * First fill the array with 0's.
 */
  for(i=0;i < npad;i++)
    newdata[i] = 0.0;
/*
 * Now insert the old data in the appropriate slots.
 */
  for(i=0;i < nx;i++) {
/*
 * In the middle of the data array
 */     
    k = i*2+nx;
    newdata[k] = datarr[i];
  }
  return newdata;
}
/*.......................................................................
 * Grid randomly sampled 2D data onto a regular grid.  Uses gaussian 
 * with Gridwdth.(x/y)width sig in pixel units and convolving mask of
 * +- Gridwdth.(x/y)pix  pixels.
 */
float *grid2d(int ndata, float *xdata,float *ydata, float *zdata, 
      float xmin, float ymin, int nx, int ny, float xpix, float ypix, int dogauss)
{
  int i,n=nx*ny;
  float *plarr=NULL;
  float *sarr=NULL;
  int indx,indy;
  float sigx,sigy;
  int ix,iy,ind;
  float weight;
  int xoff,yoff;

  sigx = xpix*Gridwdth.xwidth;
  sigy = ypix*Gridwdth.ywidth;
  xoff = Gridwdth.xpix;
  yoff = Gridwdth.ypix;

  plarr = (float *)malloc(nx*ny*sizeof(float));
  sarr = (float *)malloc(nx*ny*sizeof(float));
  
  if(plarr==NULL || sarr==NULL) {
    fprintf(stderr, "Unable to allocate arrays for gridding.\n");
    return plarr;
  };

  for(i=0;i < n;i++) {
    plarr[i] = 0.0f;
    sarr[i] = 0.0f;
  };
  
  for(i=0;i < ndata;i++) {
    indx = floor((xdata[i]-xmin)/xpix+0.5);
    indy = floor((ydata[i]-ymin)/ypix+0.5);
/* 
 * Now we have the indices of the closest grid point: need to convolve with 
 * closest +- xoff/yoff pixels.
 */
    for(iy=indy-yoff;iy <= indy+yoff;iy++)
      for(ix=indx-xoff;ix <= indx+xoff;ix++) {
	ind = iy*nx+ix;
	if(ind >= 0 && ind < n) {
	  if(dogauss) 
	    weight = gauss2d((xmin+ix*xpix),xdata[i],(ymin+iy*ypix),ydata[i],sigx,sigy,0.0);
	  else
	    weight = 1.0;

	  plarr[ind] += zdata[i]*weight;
	  sarr[ind] += weight;
	}
      };
  };
/*
 * Normalise the sums.
 */
  for(i=0;i < n;i++) {
    if(sarr[i] != 0.0f)
      plarr[i] /= sarr[i];
  };
  if(sarr)
    free(sarr);
  return plarr;
}
/*.......................................................................
 * Grid randomly sampled 1D data onto a regular grid.  Uses gaussian 
 * with Gridwdth.xwidth FWHM in pixel units and convolving mask of
 * +- Gridwdth.xpix pixels.
 */
float *grid(int ndata, float *xdata, float *ydata, float xmin, 
	    int n, float xpix)
{
  int i;
  float *plarr=NULL;
  float *sarr=NULL;
  float sigx;
  int ix;
  float weight;
  int ind;
  int xoff;

  sigx = xpix*Gridwdth.xwidth;
  xoff = Gridwdth.xpix;

  plarr = (float *)malloc(n*sizeof(float));
  sarr = (float *)malloc(n*sizeof(float));
  
  if(plarr==NULL || sarr==NULL) {
    fprintf(stderr, "Unable to allocate arrays for gridding.\n");
    return plarr;
  };

  for(i=0;i < n;i++) {
    plarr[i] = 0.0f;
    sarr[i] = 0.0f;
  };
  
  for(i=0;i < ndata;i++) {
    ind = floor((xdata[i]-xmin)/xpix+0.5);
/* 
 * Now we have the indices of the closest grid point: need to convolve with 
 * closest +- xoff pixels.
 */
    for(ix=ind-xoff;ix <= ind+xoff;ix++) {
      if(ix >= 0 && ix < n) {
	weight = gauss((xmin+ix*xpix),xdata[i],sigx);
	plarr[ind] += ydata[i]*weight;
	sarr[ind] += weight;
      };
    };
  };
/*
 * Normalise the sums.
 */
  for(i=0;i < n;i++) {
    if(sarr[i] > 0)
      plarr[i] /= sarr[i];
    else
      plarr[i] = 0.0;
  }
  if(sarr)
    free(sarr);
  return plarr;
}
/*.......................................................................
 * Ungrid regularly-sampled 1D data onto an irregular grid,
 * with Gridwdth.xwidth FWHM in pixel units and convolving mask of
 * +- Gridwdth.xpix pixels.
 */
void rvgrid(int ndata, float *xdata, float *ydata, float *edata, float xmin, 
	    int n, float xpix, float *plarr)
{
  int i;
  float *sarr=NULL;
  float sigx,mean;
  int ix;
  float weight;
  int indx;
  int xoff;
  int ng;
  
  sigx = xpix*Gridwdth.xwidth;
  xoff = Gridwdth.xpix;

  sarr = (float *)malloc(ndata*sizeof(float));
  
  if(sarr==NULL) {
    fprintf(stderr, "Unable to allocate error array.\n");
    return;
  };

  for(i=0;i < ndata;i++) {
    indx = floor((xdata[i]-xmin)/xpix+0.5);
/* 
 * Now we have the index of the closest grid point: need to convolve with 
 * closest +- xoff pixels.
 */
    ng = 0;
    edata[i] = 0.0;
    ydata[i] = 0.0;
    sarr[i] = 0.0;
    mean = 0.0;
    for(ix=indx-xoff;ix <= indx+xoff;ix++) {
      if(ix >= 0 && ix < n) {
	weight = gauss((xmin+ix*xpix),xdata[i],sigx);
	ydata[i] += plarr[ix]*weight;
	sarr[i] += weight;
/*
 * Store the second moment.
 */
	edata[i] += (plarr[ix]*plarr[ix]-edata[i])/(ng+1);
	mean += (plarr[ix]-mean)/(ng+1);
	++ng;
      };
    };
  };
/*
 * Normalise the sums.
 */
  for(i=0;i < ndata;i++) {
    if(sarr[i] != 0) {
      ydata[i] /= sarr[i];
      edata[i] = sqrt(edata[i]/sarr[i]);
    }
  }
  if(sarr)
    free(sarr);
}

/*.......................................................................
 * Grid randomly sampled 2D data onto a regular grid.  Uses gaussian 
 * with 0.7 FWHM in pixel units and convolving mask of
 * +- 2 pixels.
 */
void rvgrid2d(int ndata, float *xdata,float *ydata, float *zdata, float *edata, float *plarr, float xmin, float ymin, int nx, int ny, float xpix, float ypix)
{
  int i,n=nx*ny,ng;
  float *sarr=NULL;
  int indx,indy;
  float sigx,sigy;
  int ix,iy,ind;
  float weight;
  float mean;
  int xoff,yoff;

  sigx = xpix*Gridwdth.xwidth;
  sigy = ypix*Gridwdth.ywidth;;
  xoff = Gridwdth.xpix;
  yoff = Gridwdth.ypix;

  sarr = (float *)malloc(ndata*sizeof(float));
  
  if(sarr==NULL) {
    fprintf(stderr, "Unable to allocate arrays for gridding.\n");
    return;
  };

  for(i=0;i < ndata;i++) {
    indx = floor((xdata[i]-xmin)/xpix+0.5);
    indy = floor((ydata[i]-ymin)/ypix+0.5);
/* 
 * Now we have the indices of the closest grid point: need to convolve with 
 * closest +- xoff/yoff pixels.
 */
    ng = 0;
    edata[i] = 0.0;
    zdata[i] = 0.0;
    sarr[i] = 0.0;
    mean = 0.0;
    for(iy=indy-yoff;iy <= indy+yoff;iy++)
      for(ix=indx-xoff;ix <= indx+xoff;ix++) {
	ind = iy*nx+ix;
	if(ind >= 0 && ind < n) {
	  weight = gauss2d((xmin+ix*xpix),xdata[i],(ymin+iy*ypix),ydata[i],sigx,sigy,0.0);
	  zdata[i] += plarr[ind]*weight;
	  sarr[i] += weight;
/* 
 * Store the second moment.
 */	
	  edata[i] += (plarr[ind]*plarr[ind]-edata[i])/(ng+1);
	  mean += (plarr[ind]-mean)/(ng+1);
	  ++ng;
	}
      };
    if(ng > 1) 
      edata[i] = (ng*edata[i]-mean*mean)/(ng-1);
    else
      edata[i] = 0.0;
  };
/*
 * Normalise the sums.
 */
  for(i=0;i < ndata;i++)
    if(sarr[i] != 0) {
      zdata[i] /= sarr[i];
      edata[i] = sqrt(edata[i]/sarr[i]);
    }
  if(sarr)
    free(sarr);

  return;
}
/*.......................................................................
 * Given an input image of dimensions (adim,bdim) apply the phase shift
 * specified in the shift theorem, necessary to move the centre of the
 * fourier transform of the input image either from the array centre
 * to 0,0 or from 0,0 to the array centre. The array centre is taken as
 * being element adim/2,bdim/2. The phase shift for an adim/2,bdim/2
 * coordinate shift is exp(-pi*i*(adim*u+bdim*v)), which is simply
 * equal to -1 when (adim*u+bdim*v) is odd and +1 when it is even.
 *
 * Input/Output:
 *  image  float *   A 1D array with adim*bdim*2 elements, where adim
 *                   and bdim denote the dimensions of the equivalent
 *                   2D array and must be integral powers of two and
 *                   the factor of 2 indicates that each element of this
 *                   array is made up of two consecutive floats, the
 *                   first is the real part and the second, the imaginary
 *                   part.
 * Input:
 *  adim   int       The first dimension (fastest changing index) of the
 *                   2D array equivalent of 'image'.
 *  bdim   int       The second dimension of the 2D array equivalent of
 *                   'image'.
 */
void fft_shift(float *image, int adim, int bdim)
{
  int ia;            /* Step count along 1st dimension */
  int ib;            /* Step count along 2nd dimension */
  float *fptr=image; /* Pointer to next element to be scaled by -1 */
/*
 * Loop over pairs of even and odd rows.
 */
  for(ib=0; ib<bdim; ib+=2) {
/*
 * Even rows.
 */
    fptr += 2;
    for(ia=0; ia<adim; ia+=2, fptr += 4) {
      *fptr *= -1.0f;     /* Real part */
      *(fptr+1) *= -1.0f; /* Imaginary part */
    };
/*
 * The odd row following the above even row -- don't do if this is a 1-D
 * array
 */
    if(bdim > 1) {
      fptr -= 2;
      for(ia=0; ia<adim; ia+=2, fptr += 4) {
	*fptr *= -1.0f;
	*(fptr+1) *= -1.0f;
      };
    };
  };
  return;
}
/*.......................................................................
 * Calculate a gaussian
 */
float gauss(float x, float x0, float sigma)
{
  float fac;
  float expnt;
  float val;
  float xdiff=x-x0;

  fac = 1.0/(sqrt(2*M_PI)*sigma);
  expnt = -xdiff*xdiff/(2*sigma*sigma);

  val = fac*exp(expnt);

  return val;
}
/*.......................................................................
 * Calculate a 2d elliptical gaussian
 */
float gauss2d(float x, float x0, float y, float y0, float sigmax, float sigmay,
float phi)
{
  float val;
  float a,b,atmp,btmp,cphi=cos(phi),sphi=sin(phi),gamma;
  float xdiff=x-x0;
  float ydiff=y-y0;
  
  a = 2.35482*sigmay;
  b = 2.35482*sigmax;

  atmp = (xdiff*cphi - ydiff*sphi)/b;
  btmp = (xdiff*sphi + ydiff*cphi)/a;

  gamma = atmp*atmp+btmp*btmp;

  val = exp(-2.772588722*gamma);

  return val;
}
/*.......................................................................
 * Take an image and transform into the UV-plane.
 * 
 * Include zero-padding.
 *
 * Input:
 *  sim      Simint   *  The observation container.
 *  type      Spectype    The enumerated power spectrum type.
 *  ind       double      The spectral index/gaussian width
 *  fp        FILE     *  Pointer to the power spectrum file.
 *
 * Output:
 *  plarr     float    *  An array containing the power spectrum.
 */
Image *uvtrans(Simint *sim, Image *image)
{
  Fitshead *header=NULL;
  Image *uvimage=NULL;
  float *tmparr=NULL;
  int waserr=0,i;
  /*
   * Initialize a header.
   */
  waserr |= (header = new_Fitshead(""))==NULL;
  if(!waserr) {
    header->naxis = image->header->naxis;
  }
  /*
   * Allocate memory for the FITS axes of this header.
   */
  if(!waserr)
    waserr |= new_Axes(header, header->naxis);
  /*
   * Copy the header information.
   */
  if(!waserr)
    waserr |= copy_header(header, image->header);
  /*
   * Set the type to complex
   */
  header->type = T_CMPLX;
  /*
   * Finally, get the transform of the array, and copy it to the newly
   * created image buffer.  Be sure to pass the uvimage header as the second
   * argument, as header->naxes will be modified if the array sizes are not
   * powers of two.
   */
  if(!waserr)
    waserr = (tmparr = invert(image->re, header))==NULL;
  /*
   * Finally, allocate a new Image descriptor and copy the data into it.
   */
  if(!waserr)
    waserr = (uvimage=new_Image("temp", header))==NULL;
  if(!waserr) 
    for(i=0;i < uvimage->header->naxes_actual[0]*uvimage->header->naxes_actual[1];i++) {
	uvimage->re[i] = tmparr[2*i];
	uvimage->im[i] = tmparr[2*i+1];
    }
  /*
   * And fill in the header information -- we assume only that the axis
   * information is correct.
   */
  if(!waserr)
    waserr |= fill_uvheader(header);
  /*
   * Free tmparr if successfully allocated.
   */
  if(tmparr)
    free(tmparr);
  /*
   * And return the image.
   */
  return uvimage;
}
/*.......................................................................
 * Routine to transform a 2D data array and return its (complex)
 * transform.  This routine will modify the header of the passed image if 
 * the data array sizes are not powers of two.
 */
float *invert(float *data, Fitshead *header)
{
  float *datarr=NULL,*work=NULL; /* Work & return Arrays */
  float *xdata=NULL,*ydata=NULL;
  float dx,dy;
  int waserr=0,i,j,nx,ny,ire,iim;
  int ind;
  unsigned long nn[3];          /* An index array needed for fourn(). */
  /*
   * Ensure that the dimensions are powers of 2.  If not, use the nearest
   * power of 2.
   */
  nx = pow(2.0f,floor((log((double)header->naxes_actual[0])/log(2.0)+0.5)));
  ny = pow(2.0f,floor((log((double)header->naxes_actual[1])/log(2.0)+0.5)));
  if(nx != header->naxes_actual[0] || ny != header->naxes_actual[1]) {
    /*
     * Modify header->naxes, so that the calling function will get the
     * number of array elements right.
     */
    waserr |= (xdata = (float *)malloc(header->naxes_actual[0]*header->naxes_actual[1]*
				       sizeof(float)))==NULL;
    waserr |= (ydata = (float *)malloc(header->naxes_actual[0]*header->naxes_actual[1]*
				       sizeof(float)))==NULL;
    /*
     * This is stupid for a regular grid array, but I didn't feel like
     * rewriting the grid2d routine...
     */
    if(!waserr) {
      for(j=0;j < header->naxes_actual[1];j++)
	for(i=0;i < header->naxes_actual[0];i++) {
	  ind = i + header->naxes_actual[0]*j;
	  xdata[ind] = header->crvals[0] + (i - header->crpixs[0])*
	    header->cdelts[0];
	  ydata[ind] = header->crvals[1] + (j - header->crpixs[1])*
	    header->cdelts[1];
	}
      fprintf(stdout,"Gridding data into %d x %d bins.\n",nx,ny);
      dx = (header->xmax - header->xmin)/(nx-1);
      dy = (header->ymax - header->ymin)/(ny-1);
      waserr |= (datarr = grid2d(header->naxes_actual[0]*header->naxes_actual[1], xdata, 
				 ydata, data, header->xmin, 
				 header->ymin, nx, ny, dx, dy, 1))==NULL;
      if(xdata)
	free(xdata);
      if(ydata)
	free(ydata);
      /*
       * Update the relevant header parameters.
       */
      header->naxes_actual[0] = nx;
      header->naxes_actual[1] = ny;
    }
  }
  else {
    /*
     * Allocate a complex array, and copy the real 
     * data to it.
     */
    waserr |= ((datarr = (float *)malloc(2*nx*ny*sizeof(float)))==NULL);
    if(!waserr)
      for(i=0;i < nx*ny;i++) {
#ifdef DEBUG
	datarr[i] = 1.0;
#endif
	datarr[i] = data[i];
      }
  }
  /*
   * And allocate a zero-padded complex version of the array.  This will
   * have size 4*2*ngrid*ngrid.
   */
  waserr |= (work = zeropad_complex(datarr, nx, ny, 0, 0.0))==NULL; 

  if(!waserr) {

    nn[0] = 2*nx;
    nn[1] = 2*ny;


    /*    fft_shift(work,2*nx,2*ny); */
    for(j=0;j < 2*ny;j++)
      for(i=0;i < 4*nx;i++) {
	if((i+j)%2 > 0)
	  work[j*4*nx + 2*i] *= -1;
      }
	
    fourn(work-1,nn-1,2,1); 
    /*    fft_shift(work,2*nx,2*ny); */

    /*
     * And extract the central quadrant of the complex transform.
     */
    if(!waserr) {
      if((datarr = (float *)malloc(2*nx*ny*sizeof(float)))==NULL) {
	fprintf(stderr,"Unable to allocate float array.\n");
	waserr = 1;
      }
      if(!waserr) {
	for(j=0;j < ny;j++)
	  for(i=0;i < nx;i++) {
	    ire = (j+ny/2)*4*nx + nx + 2*i;
	    iim = ire + 1;
	    /*
	     * Normalize by the size of the non-zero transform
	     */
	    datarr[j*2*nx + 2*i] = work[ire]/(nx*ny);
	    datarr[j*2*nx + 2*i + 1] = work[iim]/(nx*ny);
	  }
      }
    }
  }
  /*
   * free any allocated memory. 
   */
  if(work)
    free(work);
    
  return datarr;
}
