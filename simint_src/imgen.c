#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tksimint.h"
#include "image.h"
#include "fitsio.h"
#include "imgen.h"
#include "rand.h"

#include "fourier.h"
#include "fftw.h"
#include "fftw-int.h"
#include "rfftw.h"

/*.......................................................................
 * Generate an image with a 2D gaussian or arbitrary power-spectrum.
 * For now, assume that the gaussian width applies to the UV-plane.
 * 
 * Include zero-padding.
 *
 * Input:
 *
 *  sim      Simint   *  The parent sim container
 *  ngrid     int         The number of pixels in the image.
 *  cellsize  double      The size of an image pixel, in radians.
 *  type      Spectype    The enumerated power spectrum type.
 *  ind       double      The spectral index/gaussian width
 *  fp        FILE     *  Pointer to the power spectrum file.
 *
 * Output:
 *  plarr     float    *  An array containging the power spectrum.
 */
float *imgen(Simint *sim, int ngrid, double cellsize, Spectype type, double powind, char *fstring)
{
  float *px=NULL,*py=NULL;   /* Arrays for reading a spectrum from a file. */
  float norm=1;                 /* A normalization for the power spectrum. */
  int waserr=0,badpt=0,nl=0,i,j;
  char line[200];               /* A work buffer for reading from a file. */
  FILE *fp=NULL;
  double u,v,du,dv,l,dl,fl,fac;
  int il,ih;
  int n[2];
  rfftwnd_plan plan_inv=NULL;
  static int first=1, nseed=1000, seeds[1000], iseed;
  long seed;
  int nneg=0;
  double sq2 = sqrt(2.0);
  float *trans=NULL,vmin,sigma;
  /*
   * I'm going to normalize non-cmb models for now to the peak of the CMB spectrum, around l=200.
   */
  static float l_norm = 200,T_norm = 100;

  if(first) {
    for(i=0;i < 1000;i++)
      seeds[i] = rand();
    first = 0;
  }
  seed = seeds[(iseed++)%nseed];

  n[0] = ngrid;
  n[1] = ngrid;
  /*
   * The resolution is set by the angular resolution and # of pixels of the desired image.
   */
  du = 1.0/(ngrid*cellsize);
  dv = 1.0/(ngrid*cellsize);
  /*
   * The critical sampling frequency in the UV plane is set by the angular resolution of the
   * desired image.  This is the value of v corresponding to the edge of the array.
   */
  vmin = -1.0/(2*cellsize);
  /*
   * Try to allocate the array we will use for computing the transform.
   */
  if((trans=((float *)malloc(sizeof(float)*(ngrid*(ngrid+2)))))==NULL) {
    fprintf(stderr,"imgen_fn: Unable to allocate transform array\n");
    return NULL;
  }
  /*
   * Zero the data array.
   */
  for(i=0;i < ngrid*(ngrid+2);i++)
    trans[i] = 0.0;
  /*
   * Generate a plan for the inverse transform.
   */
  if((plan_inv=rfftwnd_create_plan(2, n, FFTW_COMPLEX_TO_REAL,
				   FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL)
    waserr = 1;

  if(!waserr) {
    /*
     * If the spectrum is in a file, try to read it now.
     */
    switch(type) {
    case S_FILE:
      if((fp = fopen(fstring,"r"))==NULL) {
	fprintf(stderr,"Unable to open file: %s.\n",fstring);
	return NULL;
      }
      /*
       * Try to allocate the arrays needed to read the file. First count the
       * lines in the file.
       */
      for(nl=0;fgets(line,200,fp)!=NULL;nl++);
      
      waserr |= (px=(float *)malloc(nl*sizeof(float)))==NULL;
      waserr |= (py=(float *)malloc(nl*sizeof(float)))==NULL;
      
      if(!waserr) {
	rewind(fp);
	for(nl=0;fgets(line,200,fp)!=NULL;nl++) {	  
	  /*
	   * Ignore all but the first two columns of the file (l and scalar amplitude)
	   */
	  px[nl] = atof(strtok(line," "));
	  py[nl] = atof(strtok(NULL," "));
	  /*
	   * Take the square root and convert to muK (assuming this is 
	   * output from cmbfast).
	   */
	  if(py[nl] < 0) {
	    py[nl] = 0.0; /* Set to zero if this was negative. */
	    ++nneg;
	  }
	  /*
	   * Else scale to muK.  Cmbfast output is l(l+1)C_l/(2*pi), in units of 
	   * T_cmb^2
	   * 
	   * We want to convert this to sqrt(C_l), in uK/angular freq.
	   */
	  else
	    py[nl] = sqrt(2*M_PI*py[nl]/(px[nl]*(px[nl]+1)))*1e6*2.726;
	}
	fclose(fp);
      }
      break;
    case S_GAUSS:
      /*
       * Convert FWHM to sigma_l
       */
      sigma = powind/sqrt(8*log(2.0));
      norm = sqrt(T_norm*T_norm*2*M_PI/(l_norm*(l_norm+1)))*exp(l_norm*l_norm/(4*sigma*sigma));
      break;
    case S_POW:
      /*
       * Compute the prefactor required to normalize a power-law spectrum.
       */
      norm = sqrt(T_norm*T_norm*2*M_PI/(l_norm*(l_norm+1)))*pow(l_norm,-powind/2);
      break;
    }
    /*
     * Seed the random number generator.
     */
    srand(seed);
    /*
     * Now compute the power spectrum
     */
    if(!waserr) {
      /*
       * Since the array is conjugate symmetric, we are only filling
       * the half plane; UV radius is 0 at the left center of the
       * array, and increases towards the right.
       *
       * In the complex array, i ranges from 1 to N/2 + 1, and j
       * ranges from 1 to N/2.
       *
       * For simplicity, I'm phase-shifting the transform so that we can treat the
       * ix = 0, iy = N/2 pixel as the center (0 frequency).  iy = 1 --> N/2-1 will
       * be negative frequency, and iy = N/2+1 --> N-1 will be positive frequency.
       * iy = 0 is both - and + the critical frequency.
       */
      fftw_complex *complx = (fftw_complex *)trans;
      int ind;
      /*
       * Fill the whole thing
       */
      for(j=0;j < ngrid;j++)
	for(i=0;i < ngrid/2+1;i++) {
	  /*
	   * Compute u,v and l for this pixel.
	   * Zero frequency occurs at the center of the 0th pixel.
	   * Positive frequency go from   1     --> n/2-1
	   * +- Critical frequency 1/(2*dx) is n/2
	   * Negative frequency goes from n/2+1 --> n-1
	   */
	  u = du*i;
	  v = vmin + dv*j;
	  l = 2*M_PI*sqrt(u*u + v*v)-0.5;
	  ind = j*(ngrid/2+1) + i;
	  /*
	   * In all cases, normalize the amplitude of the power spectrum
	   * to be T_norm (assumed in uK) at l = l_norm
	   */
	  switch (type) {
	  case S_POW:
	    fac = pow(l, powind/2)*norm;
	    break;
	  case S_GAUSS:
	    fac = sqrt(gauss2d(u,0,v,0,powind,powind,0))*norm;
	    break;
	  case S_FILE:
	    /*
	     * Else do a look-up of the value of the power spectrum 
	     * in the py array.
	     */		
	    dl = (px[nl-1]-px[0])/(nl-1);
	    
	    fl = (l-px[0])/dl-0.5; /* The approximate "pixel" value */
	    il = (int)floor(fl);   /* The low bracketing value. */
	    ih = (int)ceil(fl);    /* The high bracketing value. */
	    
	    if(il >= 0 && ih < nl) 
	      fac = py[il] + (py[ih] - py[il])/dl*(l-px[il]);
	    else {
	      /*
	       * Only increment the out-of-range counter if this is not the zero-frequency bin.
	       */
	      if(!(i==0 && j==0))
		badpt++;
	      fac = 0.0;
	    }
	    break;
	  }
	  /*
	   * Scale by the resolution.
	   */
	  fac *= sqrt(du*dv);   
	  /* 
	   * Correct for the fact that each of the Re and Im
	   * components only contributes half of the total
	   * variance in each mode.
	   */
	  fac /= sq2; 
	  /*
	   * Generate gaussian random deviates, multiplying every other complex
	   * point by -1 to shift the image to the center.
	   */
	  complx[ind].re = gauss_rand(fac)*(j%2==0 ? 1 : -1)*(i%2==0 ? 1 : -1);
	  complx[ind].im = gauss_rand(fac)*(j%2==0 ? 1 : -1)*(i%2==0 ? 1 : -1);
	}
      /* 
       * Set the zero-frequency point to 0 so that the sum of the image pixels will be 0
       */
      ind = ngrid/2*(ngrid/2+1) + 0;
      complx[ind].re = 0.0;
      complx[ind].im = 0.0;
    }
    /*
     * Now transform back to the image plane
     */
    if(!waserr)
      (void)rfftwnd_one_complex_to_real(plan_inv, (fftw_complex *)trans, NULL); 
  }
  /*
   * Emit any relevant error messages.
   */
  if(nneg > 0)
    fprintf(stderr,"Warning: File %s contained %d negative points\n\n",fstring, nneg);
  if(badpt > 0) {
    fprintf(stderr,"Warning: Power spectrum %s extended from l = %.0f to l = %.0f.\n",
	    fstring, px[0], px[nl-1]);
    fprintf(stderr,"         %d visibilities were out of range and were set to 0.0\n",badpt);
  }
  /*
   * Free any memory allocated in this function.
   */
  if(px)
    free(px);
  if(py)
    free(py);
  
  return trans;
}
/*.......................................................................
 * Fill an image header for the image returned from imgen()
 */
Fitshead *imgen_get_header(Simint *sim) 
{
  int waserr=0;
  Fitshead *header=NULL;

  if((header = new_Fitshead("MUK"))==NULL)
    return NULL;
  
  if(!waserr) {
    header->naxis = 2;
    /*
     * Allocate memory for the FITS axes of this header.
     */
    waserr |= new_Axes(header, 2);
    /*
     * Set up fits header parameters for this image.
     */
    if(!waserr) {
      /*
       * Install the telescope name.
       */
      waserr |= rd_str(&header->telescope,"SIMINT");

      if(!waserr) {

	header->naxes_actual[0] = sim->imgen.ngrid;
	header->naxes_actual[1] = sim->imgen.ngrid;
	
	header->naxes_data[0] = sim->imgen.ngrid;
	header->naxes_data[1] = sim->imgen.ngrid;

	header->imin = 0;
	header->jmin = 0;
	header->imax = sim->imgen.ngrid-1;
	header->jmax = sim->imgen.ngrid-1;
	
	header->data_start[0] = header->data_start[1] = 0;
	
	header->n = header->naxes_actual[0] * header->naxes_actual[1];
	
	header->type = T_RE;
	/*
	 * Set the units to microKelvin.
	 */
	header->bunit = BU_MUK;
	/*
	 * And the axes to degrees
	 */
	header->ctypes[0] = AX_DEG;
	header->ctypes[1] = AX_DEG;
	/*
	 * Compute dx and dy from the specified cellsize.
	 */
	header->cdelts[0] = sim->imgen.cellsize;
	header->cdelts[1] = sim->imgen.cellsize;
	/*
	 * Fill the axis information.
	 */
	header->crvals[0] = 0.0;
	header->crvals[1] = 0.0;
	/*
	 * I'm asserting that 0 occurs at the center of the *array*, not the
	 * center pixel.  Ie, if this is an image with an odd number of
	 * pixels, it will be the center pixel, but if it has an even number
	 * of pixels, the center will occur at ngrid/2-1+0.5 pixel
	 */
	if(sim->imgen.ngrid%2==0) {
	  header->crpixs[0] = sim->imgen.ngrid/2;
	  header->crpixs[1] = sim->imgen.ngrid/2;
	}
	else {
	  header->crpixs[0] = sim->imgen.ngrid/2;
	  header->crpixs[1] = sim->imgen.ngrid/2;
	}
	header->xmin = (-sim->imgen.ngrid/2)*header->cdelts[0];
	header->xmax = ( sim->imgen.ngrid/2)*header->cdelts[0];
	header->ymin = (-sim->imgen.ngrid/2)*header->cdelts[1];
	header->ymax = ( sim->imgen.ngrid/2)*header->cdelts[1];
      }
    }
  }
  return waserr ? del_Fitshead(header) : header;
}
