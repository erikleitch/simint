#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

#include "tksimint.h"
#include "fitsio.h"
#include "vis.h"
#include "rand.h"

#include "fourier.h"
#include "fftw.h"
#include "fftw-int.h"
#include "rfftw.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

/* #define DEBUG */
/*
 * If defined, we will manually add point sources to the central FWHM
 */
/* #define PTSRC */
/*
 * The sigma of the convolution function, in pixel units (corresponds
 * to HWHM of 0.7 pixels)
 */
#define CONV_SIG 0.594525
#define NMASK 2
/*
 * Define a structure to encapsulate the transform of an image.
 */
typedef struct FT {
  Image *image;
  fftw_real *data;
  rfftwnd_plan plan;
  int n_actual[2]; /* The actual size of the data array */
  int n_data[2];   /* The size of the portion of the array occupied by data */
} FT;

static VisList *add_VisList(VisList **list, int nant, AntType *type1, AntType *type2);
static VisList *del_VisList(VisList *node);
static int imvis(Image *image, FT *transform, double u, double v, double *re, double *im, int *badvis);
static int imvis_save(Image *image, FT *transform, double u, double v, double *re, double *im, int *badvis);
static int imvis2(Image *image, FT *transform, double u, double v, double *re, double *im, int *badvis);
static int obsim(Simint *sim, VisList *pair, double freq, FT **transform);
static int obsim2(Simint *sim, VisList *pair, double wave, FT **transform);
static FT *new_FT(Image *image);
static FT *new_FT2(Image *image);
static FT *del_FT(FT *trans);
static int calc_primary_beam(Simint *sim, FT *trans, VisList *pair, double freq);
static int calc_primary_beam2(Simint *sim, FT *trans, VisList *pair, double wave);
static Fitshead *get_uvheader(Fitshead *imhdr, FT *trans);
static int apfield(double r, double aperture_diameter, double *val);
static float bessj0(float x);
static int beam_envelope(Simint* sim, VisList *pair, double freq, FT *trans);

/*.......................................................................
 * This routine will also construct lists of indices in the n*(n-1)/2
 * visibilities associated with each distinct pairing of antenna types.
 * If there are nant different types of antennas present, there will be 
 * nant*(nant-1)/2 of these.
 */
VisList *construct_list(Simint *sim)
{
  Ant *ant1=NULL,*ant2=NULL;
  AntType *type1=NULL,*type2=NULL;
  VisList *list=NULL,*listnode=NULL;
  int waserr=0,ivis;
  /*
   * Contruct the list of distinct antenna pairings.
   */
  for(type1=sim->anttypes;type1!=NULL;type1=type1->next) 
    for(type2=type1;type2!=NULL;type2=type2->next) 
      waserr |= add_VisList(&list,sim->nant,type1,type2)==NULL;
  /*
   * Now read through the list of visibilities, determining which baselines refer to which
   * pairings of antennas.
   */
  for(ivis=0,ant1=sim->ants;ant1!=NULL && !waserr;ant1=ant1->next)
    for(ant2=ant1->next;ant2!=NULL;ant2=ant2->next,ivis++) {
      for(listnode=list;listnode!=NULL;listnode=listnode->next)
	if((listnode->type1==ant1->type && listnode->type2==ant2->type) ||
	   (listnode->type1==ant2->type && listnode->type2==ant1->type))
	  break;
      listnode->indices[listnode->nvis] = ivis;
      listnode->ant1[listnode->nvis] = ant1;
      listnode->ant2[listnode->nvis++] = ant2;
    }
  return list;
}
/*.......................................................................
 * Add a new node to a list of antenna pairs.
 */
static VisList *add_VisList(VisList **list, int nant, AntType *type1, AntType *type2)
{
  VisList *node=NULL;
  int nvis = nant*(nant-1)/2;

  if((node=(VisList *)malloc(sizeof(VisList)))==NULL)
    return NULL;

  node->nvis = 0;
  node->next = NULL;
  node->indices = NULL;
  node->ant1 = NULL;
  node->ant2 = NULL;

  if((node->indices=(int *)malloc(sizeof(int)*nvis))==NULL)
    return del_VisList(node);
  if((node->ant1=(Ant **)malloc(sizeof(Ant *)*nvis))==NULL)
    return del_VisList(node);
  if((node->ant2=(Ant **)malloc(sizeof(Ant *)*nvis))==NULL)
    return del_VisList(node);

  node->type1 = type1;
  node->type2 = type2;
  /*
   * Add the node to the head of the list.
   */
  if(*list!=NULL)
    node->next = *list;

  *list = node;

  return node;
}
/*.......................................................................
 * Destroy a VisList node.
 */
static VisList *del_VisList(VisList *node)
{
  if(node==NULL)
    return node;

  if(node->indices) {
    free(node->indices);
    node->indices = NULL;
  }
  if(node->ant1) {
    free(node->ant1);
    node->ant1 = NULL;
  }
  if(node->ant2) {
    free(node->ant2);
    node->ant2 = NULL;
  }
  node->next = NULL;

  free(node);
  return NULL;
}
/*.......................................................................
 * Write out the visibilities for real images.  This will behave in
 * the following manner:
 * 
 * UV points must be written in time order for difmap.  This means for
 * each consecutive integration, we have to write out visibilities for
 * each baseline and IF.  
 *
 * For real data (image != NULL), we will compute the re and im
 * visiblities by convolution with the primary beam, but this must be
 * done once for each pair of different antennas x each frequency, or
 * if nant is the number of distinct types of antennas, we require
 * 
 *      nant*(nant-1)/2 * nfreq
 *
 * FTs.  Each FT will be the transform of the appropriate mean primary
 * beam for that antenna combination at that frequency.
 *
 * We will thus compute the visibilities in order of antenna pair and
 * frequency, inserting the binary data into the output file at the
 * appropriate ordered position.  
 *
 * Input:
 *
 *  fp     *  FILE    -  The output file descriptor.
 *  sim    *  Simint  -  The current sim descriptor.
 *  type      Dtype   -  The type of output requested.
 *  image  *  Image   -  The image to observe (NULL accepted.)
 *
 * Output:
 *  waserr    int     -  True if an error was encountered.
 */
int new_write_vis(FILE *fp, Simint *sim, Dtype type)
{  
  Ant *ant1=NULL,*ant2=NULL;
  FT *transform=NULL;
  double u,v,w,sh,ch,dh,ha,base;
  int i,iint,inu,ibase,nbase,ivis,waserr=0,nint=0,nvis=0;
  double nu0,dnu,nu,wave;
  double tsys,wt,wt0,sig;
  double date0,date1,date2,date;
  double rms;
  double re=1.0,im=0.0;
  float fltbuf[9],rev[9],*tmp=NULL,fitsbuf[720];
  int nran=6,ndat=3,nbyte=0,nres=0;  /* Bookkeeping for FITS file buffering */
  VisList *list=NULL,*pair=NULL;
  long offset_abs,offset_rel,offset_current,offset_data;
  long vissize,datsize,ransize;
  int badvis,nbadvis=0; /* The number of visibilities which couldn't
                           be interpolated. */
  int donoise=1;
  /*
   * Get the Tcl variables we need to compute the visibilities.
   */
  waserr = getobs(sim);
  donoise = (sim->obs.tsys0 + sim->obs.tau) > 0.0;
  /*
   * Check that we have an array
   */
  if(sim->nant==1) {
    fprintf(stdout,"No baselines to be computed.\n");
    return 1;
  }

  /*
   * Form the list of distinct antenna pairs.
   */
  waserr |= (list=construct_list(sim))==NULL;

  /*
   * Get the date.
   */
  date0 = getsysdate();

  /*
   * Precompute the weight, modulo the aperture diameter, efficiency
   * and tsys, since these can be different for different antennas.
   * Note that this already includes the factor of pi/4 to go from D^2 to
   * area.
   */
  rms = sqrt(2.0)*K/(sim->obs.correff*sqrt(sim->obs.tscale*sim->obs.tint*
			 sim->obs.bw/sim->obs.nchan*1e9))*1e23/M_PI*4;
  wt0 = 1.0/(rms*rms);

  /*
   * Get the wavelength range corresponding to the specified bandwidth.
   */
  dnu = sim->obs.bw/sim->obs.nchan; /* Channel width */

  /* 
   * Interpret obs.wave as the center wavelength, and get the starting
   * frequency here
   */
  nu0 = C/1e9/sim->obs.wave - dnu*((double)sim->obs.nchan)/2 + dnu/2; 

  /*
   * Now proceed to computing the visibilities.  We will do these in
   * antenna pair + frequency order.
   */
  if(!waserr) {
    double sd,cd,sl,cl,cza;

    /*
     * Get the nearest number of integrations. HAs are in radians, so we need
     * convert seconds to the same.
     */
    if(sim->obs.hastart==sim->obs.hastop)
      nint = 1;
    else
      nint = (sim->obs.hastop-sim->obs.hastart)/(sim->obs.tint/RTOS);

    /*
     * Precompute sines and cosines
     */
    sd = sin(sim->obs.srcdec);
    sl = sin(sim->obs.lat);
    cd = cos(sim->obs.srcdec);
    cl = cos(sim->obs.lat);

    /*
     * Get the dh from the tint in radians.
     */
    dh = (sim->obs.hastop-sim->obs.hastart)/(nint);

    /*
     * Compute the size in bytes of the visibility header
     */
    ransize = nran*4;

    /*
     * And the size in bytes of a single visibility data record (re, im, wt)
     */
    datsize = ndat*4;

    /*
     * Size of the whole thing.
     */
    vissize = ransize + datsize*sim->obs.nchan;

    /*
     * And store the offset of the data segment from the start of the file.
     */
    waserr |= (offset_data=ftell(fp)) < 0;
    offset_current = offset_data;

    /*
     * Set the total number of baselines
     */
    nbase = sim->nant*(sim->nant-1)/2;

    /*
     * Main loop -- loop through all distinct antenna pairs and frequencies.
     */
    for(pair=list;pair!=NULL && !waserr;pair=pair->next) {

      if(pair->nvis==0)
	continue;

      /*
       * Loop through all frequencies, starting with the lowest.
       */
      for(inu=0;inu < sim->obs.nchan && !waserr;inu++) {
	nu = nu0 + inu*dnu;/* Frequency, in GHz */
	wave = C/(nu*1e9); /* Store the wavelength, in cm */

	/*
	 * Get the transform for this antenna pair, this frequnecy
	 */
	waserr |= obsim(sim, pair, nu, &transform);

	/*
	 * Now loop through all baselines involving these antennas.
	 */
	for(ivis=0;!waserr && ivis < pair->nvis;ivis++) {
	  ibase = pair->indices[ivis];

	  ant1 = pair->ant1[ivis];
	  ant2 = pair->ant2[ivis];

	  /*
	   * Get the baseline random parameter value for this baseline.
	   */
	  base = 256*(ant1->iant+1) + ant2->iant+1;

	  /*
	   * Loop over all integrations for this baseline.
	   */
	  for(iint=0;iint < nint && !waserr;iint++) {

	    /*
	     * Increment the date by tint seconds.
	     */
	    date = date0 + ((double)(sim->obs.tint)/DAYSEC)*iint;

	    /*
	     * Get the integral and fractional parts of the date.
	     */
	    date2 = date - (int)date;
	    date1 = date - date2;     /* In days */
	    date2 *= DAYSEC;           /* Convert to seconds. */

	    /*
	     * Compute the visibilities at the central HA of this integration,
	     * hence the extra dh/2
	     */
	    ha = sim->obs.hastart + dh*iint + dh/2;
	    sh = sin(ha);
	    ch = cos(ha);

	    /*
	     * Compute cos(za) from the law of cosines.
	     */
	    cza = cd*cl*ch + sd*sl;

	    /* 
	     * Compute the tsys from TATM, tau and sec(za)
	     */
	    if(donoise)	    
	      tsys = sim->obs.tsys0 + TATM*sim->obs.tau/cza;
	    else
	      tsys = 1.0;

	    u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	    v = -sd*ch*(ant1->X-ant2->X) + sd*sh*(ant1->Y-ant2->Y) + 
	      cd*(ant1->Z-ant2->Z);
	    w = cd*ch*(ant1->X-ant2->X) - cd*sh*(ant1->Y-ant2->Y) + 
	      sd*(ant1->Z-ant2->Z);

	    /*
	     * We only have to write the random parameters at the
	     * beginning of each integration.  This will be the
	     * first frequency written for this baseline, this
	     * integration.
	     */
	    if(inu==0) {
	      fltbuf[0] = u/C*Uvscale;
	      fltbuf[1] = v/C*Uvscale;
	      fltbuf[2] = w/C*Uvscale;
	      fltbuf[3] = base;
	      fltbuf[4] = date1;
	      fltbuf[5] = date2;

#ifdef linux_i486_gcc
	      cp_4r4((unsigned char *)rev, (unsigned char *)fltbuf, nran);
	      tmp = rev;
#else
	      tmp = fltbuf;
#endif
	      /*
	       * Now we just have to figure out where to write it.
	       * Each visibility consists of a set of random
	       * parameters, followed by re im wt data for each
	       * frequency.  So a single visibility comprises
	       * 
	       * vissize = (nran + nnu*3)*4 bytes 
	       *
	       * Visibilities are written sequentially by integration,
	       * so the starting point for the group which records the
	       * data for baseline ibase, integration iint will be
	       * 
	       * offset_data + vissize*(nbase*iint + ibase)
	       */
	      offset_abs = offset_data + vissize*(nbase*iint + ibase);
	      offset_rel = offset_abs - offset_current;

	      waserr |= fseek(fp, offset_rel, SEEK_CUR) < 0;

	      waserr |= fwrite(tmp,sizeof(float),nran,fp) == 0;

	      offset_current += offset_rel + ransize;

	      nbyte += nran*sizeof(float);
	    }
	    wt = wt0*(ant1->type->apeff*ant1->type->size*ant1->type->size)*
	      (ant2->type->apeff*ant2->type->size*ant2->type->size)/(tsys*tsys);

	    /*
	     * Increment the total number of visibilities.
	     */
	    ++nvis;

	    /*
	     * If no image descriptor was passed, just write bogus values
	     * for the visibilities.
	     */
	    badvis = 0;
	    waserr |= imvis(sim->image, transform, u/wave, v/wave, &re, &im, 
			    &badvis);
	    nbadvis += badvis;

	    /*
	     * At this point, the weight is an inverse variance, in
	     * Jy^-2.  Get the sigma, in Jy, and add to each
	     * visibility random noise consistent with the system
	     * temperature.
	     *
	     * Check the factor of sqrt(2) vs. the single-dish formula.
	     */
	    sig = 1.0/sqrt(wt);

	    fltbuf[0] = re + (donoise ? gauss_rand(sig) : 0.0);
	    fltbuf[1] = im + (donoise ? gauss_rand(sig) : 0.0);
	    fltbuf[2] = (badvis ? -wt : wt);

	    /*
	     * Visibilities are written sequentially by integration,
	     * so the starting point for the visibility for
	     * integration iint, baseline ibase, frequency inu will
	     * be
	     * 
	     * offset_data + vissize*(nbase*iint + ibase) + datsize*inu
	     *
	     * Where datsize is the size of one data element, or
	     * 3 * 4 bytes (re, im, wt)
	     */
	    offset_abs = offset_data + vissize*(nbase*iint + ibase) + 
	      ransize + datsize*inu;
	    offset_rel = offset_abs-offset_current;

	    waserr |= fseek(fp, offset_rel, SEEK_CUR) < 0;

	    /*
	     * If we are running on a linux OS, we need to swap the
	     * byte order of binary floats.
	     */
#ifdef linux_i486_gcc
	      cp_4r4((unsigned char *)rev, (unsigned char *)fltbuf, ndat);
	      tmp = rev;
#else
	      tmp = fltbuf;
#endif
	    waserr |= fwrite(tmp, sizeof(float),ndat,fp) == 0;
	    offset_current += offset_rel + datsize;

	    nbyte += ndat*sizeof(float);
	  } /* End looping over integrations */
	} /* End looping over concerned baselines */
      } /* End looping over frequencies. */
    } /* End looping over pairs. */
  }
  /*
   * Buffer the data array up to the next integral number of data records.
   */  
  if(nbyte%2880 > 0)
    nres = 2880 - nbyte%2880;
  else
    nres = 0;
  
  nres = nres/sizeof(float);
  
  for(i=0;i < 720;i++)
    fitsbuf[i] = 0.0;
  /*
   * At the end of the above, the file pointer could be at any
   * location within the dat segment.  Before buffering, move the
   * file pointer to the end of the data segment.
   */  
  offset_rel = offset_data + nbyte - offset_current;
  waserr |= fseek(fp, offset_rel, SEEK_CUR) < 0;

  fwrite(fitsbuf, sizeof(float), nres, fp);
  /*
   * Free any memory allocated in this function.
   */
  if(transform)
    waserr |= del_FT(transform)!=NULL;
  /*
   * Finally, print some statistics about the last write.
   */
  fprintf(stdout,"Wrote %d visibilities.\n",nvis);
  if(nbadvis > 0)
    fprintf(stdout,"Couldn't interpolate %d visibilities due to the image pixel scale.\n",nbadvis);
#ifdef DMALLOC
  dmalloc_verify(0);
#endif
  return waserr;
}
/*.......................................................................
 * Write out the visibilities for real images.  This will behave in
 * the following manner:
 * 
 * UV points must be written in time order for difmap.  This means for
 * each consecutive integration, we have to write out visibilities for
 * each baseline and IF.  
 *
 * For real data (image != NULL), we will compute the re and im
 * visiblities by convolution with the primary beam, but this must be
 * done once for each pair of different antennas x each frequency, or
 * if nant is the number of distinct types of antennas, we require
 * 
 *      nant*(nant-1)/2 * nfreq
 *
 * FTs.  Each FT will be the transform of the appropriate mean primary
 * beam for that antenna combination at that frequency.
 *
 * We will thus compute the visibilities in order of antenna pair and
 * frequency, inserting the binary data into the output file at the
 * appropriate ordered position.  
 *
 * Input:
 *
 *  fp     *  FILE    -  The output file descriptor.
 *  sim   *  Simint  -  The current sim descriptor.
 *  type      Dtype   -  The type of output requested.
 *  image  *  Image   -  The image to observe (NULL accepted.)
 *
 * Output:
 *  waserr    int     -  True if an error was encountered.
 */
int new_write_vis2(FILE *fp, Simint *sim, Dtype type)
{  
  Ant *ant1=NULL,*ant2=NULL;
  FT *transform=NULL;
  double u,v,w,sh,ch,dh,ha,base;
  int i,iint,inu,ibase,nbase,ivis,waserr=0,nint=0,nvis=0;
  double nu0,dnu,nu,wave;
  double tsys,wt,wt0;
  double date0,date1,date2,date;
  double rms;
  double re=1.0,im=0.0;
  float fltbuf[9],rev[9],*tmp=NULL,fitsbuf[720];
  int nran=6,ndat=3,nbyte=0,nres=0;  /* Bookkeeping for FITS file buffering */
  VisList *list=NULL,*pair=NULL;
  long offset_abs,offset_rel,offset_current,offset_data;
  long vissize,datsize,ransize;
  int nbadvis=0; /* The number of visibilities which couldn't be interpolated. */
  /*
   * Get the Tcl variables we need to compute the visibilities.
   */
  waserr = getobs(sim);
  /*
   * Check that we have an array
   */
  if(sim->nant==1) {
    fprintf(stdout,"No baselines to be computed.\n");
    return 1;
  }
  /*
   * Form the list of distinct antenna pairs.
   */
  waserr |= (list=construct_list(sim))==NULL;
  /*
   * Get the date.
   */
  date0 = getsysdate();
  fprintf(stdout, "Date0 is: %f\n", date0);

  /*
   * Precompute the weight, modulo the aperture diameter, efficiency
   * and tsys, since these can be different for different antennas.
   * Note that this already includes the factor of pi/4 to go from D^2 to
   * area.
   */
  rms = 2*K/sqrt(sim->obs.tscale*sim->obs.tint*
		 sim->obs.bw/sim->obs.nchan*1e9*
		 sim->obs.correff)*1e23/M_PI*4;
  wt0 = 1.0/(rms*rms);
  /*
   * Get the wavelength range corresponding to the specified bandwidth.
   */
  dnu = sim->obs.bw/sim->obs.nchan; /* Channel width */
  /* 
   * Interpret obs.wave as the center wavelength, and get the center of the
   * the starting channel here.  Note that the center is offset from the 
   * lowest detected frequency by dnu/2
   */
  nu0 = C/1e9/sim->obs.wave - dnu*((double)sim->obs.nchan)/2 + dnu/2;
  /*
   * Now proceed to computing the visiblities.  We will do these in
   * antenna pair + frequency order.
   */
  if(!waserr) {
    double sd,cd,sl,cl,cza;
    /*
     * Get the nearest number of integrations. HAs are in radians, so we need
     * convert seconds to the same.
     */
    if(sim->obs.hastart==sim->obs.hastop)
      nint = 1;
    else
      nint = (sim->obs.hastop-sim->obs.hastart)/(sim->obs.tint/RTOS);
    /*
     * Precompute sines and cosines
     */
    sd = sin(sim->obs.srcdec);
    sl = sin(sim->obs.lat);
    cd = cos(sim->obs.srcdec);
    cl = cos(sim->obs.lat);
    /*
     * Get the dh from the tint in radians.
     */
    dh = (sim->obs.hastop-sim->obs.hastart)/(nint);
    /*
     * Compute the size in bytes of the visibility header
     */
    ransize = nran*4;
    /*
     * And the size in bytes of a single visibility data record (re, im, wt)
     */
    datsize = ndat*4;
    /*
     * Of the whole thing.
     */
    vissize = ransize + datsize*sim->obs.nchan;
    /*
     * And store the offset of the data segment from the start of the file.
     */
    waserr |= (offset_data=ftell(fp)) < 0;
    offset_current = offset_data;
    /*
     * Set the total number of baselines
     */
    nbase = sim->nant*(sim->nant-1)/2;
    /*
     * Main loop -- loop through all distinct antenna pairs and frequencies.
     */
    for(pair=list;pair!=NULL && !waserr;pair=pair->next) {

      if(pair->nvis==0)
	continue;
      /*
       * Loop through all frequencies, starting with the lowest.
       */
      for(inu=0;inu < sim->obs.nchan && !waserr;inu++) {
	nu = nu0 + inu*dnu;
	wave = C/(nu*1e9); /* Store the wavelength, in cm */
	/*
	 * Get the transform for this antenna pair, this frequnecy
	 */
	waserr |= obsim2(sim, pair, wave, &transform);
	/*
	 * Now loop through all baselines involving these antennas.
	 */
	for(ivis=0;!waserr && ivis < pair->nvis;ivis++) {
	  ibase = pair->indices[ivis];

	  ant1 = pair->ant1[ivis];
	  ant2 = pair->ant2[ivis];
	  /*
	   * Get the baseline random parameter value for this baseline.
	   */
	  base = 256*(ant1->iant+1) + ant2->iant+1;
	  /*
	   * Loop over all integrations for this baseline.
	   */
	  for(iint=0;iint < nint && !waserr;iint++) {
	    /*
	     * Increment the date by tint seconds.
	     */
	    date = date0 + ((double)(sim->obs.tint)/DAYSEC)*iint;
	    /*
	     * Get the integral and fractional parts of the date.
	     */
	    date2 = date - (int)date;
	    date1 = date - date2;     /* In days */
	    date2 *= DAYSEC;           /* Convert to seconds. */
	    /*
	     * Compute the visibilities at the central HA of this integration,
	     * hence the extra dh/2
	     */
	    ha = sim->obs.hastart + dh*iint + dh/2;
	    sh = sin(ha);
	    ch = cos(ha);
	    /*
	     * Compute cos(za) from the law of cosines.
	     */
	    cza = cd*cl*ch + sd*sl;
	    /* 
	     * Compute the tsys from TATM, tau and sec(za)
	     */
	    tsys = sim->obs.tsys0 + TATM*sim->obs.tau/cza;
	    
	    u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	    v = -sd*ch*(ant1->X-ant2->X) + sd*sh*(ant1->Y-ant2->Y) + 
	      cd*(ant1->Z-ant2->Z);
	    w = cd*ch*(ant1->X-ant2->X) - cd*sh*(ant1->Y-ant2->Y) + 
	      sd*(ant1->Z-ant2->Z);
	    /*
	     * We only have to write the random parameters at the
	     * beginning of each integration.  This will be the
	     * first frequency written for this baseline, this
	     * integration.
	     */
	    if(inu==0) {
	      fltbuf[0] = u/C*Uvscale;
	      fltbuf[1] = v/C*Uvscale;
	      fltbuf[2] = w/C*Uvscale;
	      fltbuf[3] = base;
	      fltbuf[4] = date1;
	      fltbuf[5] = date2;
#ifdef linux_i486_gcc
	      cp_4r4((unsigned char *)rev, (unsigned char *)fltbuf, nran);
	      tmp = rev;
#else
	      tmp = fltbuf;
#endif
	      /*
	       * Now we just have to figure out where to write it.
	       * Each visibility consists of a set of random
	       * parameters, followed by re im wt data for each
	       * frequency.  So a single visibility comprises
	       * 
	       * vissize = (nran + nnu*3)*4 bytes 
	       *
	       * Visibilities are written sequentially by integration,
	       * so the starting point for the group which records the
	       * data for baseline ibase, integration iint will be
	       * 
	       * offset_data + vissize*(nbase*iint + ibase)
	       */
	      offset_abs = offset_data + vissize*(nbase*iint + ibase);
	      offset_rel = offset_abs - offset_current;

	      waserr |= fseek(fp, offset_rel, SEEK_CUR) < 0;

	      waserr |= fwrite(tmp,sizeof(float),nran,fp) == 0;

	      offset_current += offset_rel + ransize;

	      nbyte += nran*sizeof(float);
	    }

	    wt = wt0*(ant1->type->apeff*ant1->type->size*ant1->type->size)*
	      (ant2->type->apeff*ant2->type->size*ant2->type->size)/(tsys*tsys);
	    /*
	     * Increment the total number of visibilities.
	     */
	    ++nvis;
	    /*
	     * If no image descriptor was passed, just write bogus values
	     * for the visibilities.
	     */
	    waserr |= 
	      imvis2(sim->image, transform, u/wave, v/wave, &re, &im, &nbadvis);
	    
	    fltbuf[0] = re;
	    fltbuf[1] = im;
	    fltbuf[2] = wt;
	    /*
	     * Visibilities are written sequentially by integration,
	     * so the starting point for the visibility for
	     * integration iint, baseline ibase, frequency inu will
	     * be
	     * 
	     * offset_data + vissize*(nbase*iint + ibase) + datsize*inu
	     *
	     * Where datsize is the size of one data element, or
	     * 3 * 4 bytes (re, im, wt)
	     */
	    offset_abs = offset_data + 
	      vissize*(nbase*iint + ibase) + ransize + datsize*inu;

	    offset_rel = offset_abs-offset_current;

	    waserr |= fseek(fp, offset_rel, SEEK_CUR) < 0;

#ifdef linux_i486_gcc
	      cp_4r4((unsigned char *)rev, (unsigned char *)fltbuf, ndat);
	      tmp = rev;
#else
	      tmp = fltbuf;
#endif
	    waserr |= fwrite(tmp, sizeof(float),ndat,fp) == 0;
	    offset_current += offset_rel + datsize;

	    nbyte += ndat*sizeof(float);
	  } /* End looping over integrations */
	} /* End looping over concerned baselines */
      } /* End looping over frequencies. */
    } /* End looping over pairs. */
  }

  /*
   * Buffer the data array up to the next integral number of data records.
   */  
  if(nbyte%2880 > 0)
    nres = 2880 - nbyte%2880;
  else
    nres = 0;
  
  nres = nres/sizeof(float);
  
  for(i=0;i < 720;i++)
    fitsbuf[i] = 0.0;

  /*
   * At the end of the above, the file pointer could be at any
   * location within the dat segment.  Before buffering, move the file
   * pointer to the end of the data segment.
   */  
  offset_rel = offset_data + nbyte - offset_current;
  waserr |= fseek(fp, offset_rel, SEEK_CUR) < 0;

  fwrite(fitsbuf, sizeof(float), nres, fp);

  /*
   * Free any memory allocated in this function.
   */
  if(transform)
    waserr |= del_FT(transform)!=NULL;

  /*
   * Finally, print some statistics about the last write.
   */
  fprintf(stdout,"Wrote %d visibilities.\n",nvis);

  if(nbadvis > 0)
    fprintf(stdout,"Couldn't interpolate %d visibilities "
	    "due to the image pixel scale.\n",nbadvis);

#ifdef DMALLOC
  dmalloc_verify(0);
#endif

  return waserr;
}
/*.......................................................................
 * Compute the transform of the image.
 *
 * The passed array will only be allocated by this function, so we can
 * have only three cases:
 *
 * sim->image AND FT are NULL         --> no image to observe; return NULL
 * sim->image != NULL, and FT == NULL --> transform hasn't been allocated yet;
 *                                         allocate it
 * sim->image != NULL, and FT != NULL --> transform has been
 *                                         allocated; don't allocate it again, 
 *                                         since it will have the same dimension
 *                                         every time thie routine is called.
 */
static int obsim(Simint *sim, VisList *pair, double freq, FT **trans)
{
  int i,nx,ny;

  if(sim->image == NULL) {
    if(*trans != NULL) {
      fprintf(stderr,"obsim: received invalid arguments.\n");
      return 1;
    } 
  } else {

    /*
     * If the transform hasn't been allocated yet, do it now.
     */
    if(*trans==NULL)
      if((*trans=new_FT(sim->image))==NULL)
	return 1;

    nx=(*trans)->n_data[0];
    ny=(*trans)->n_data[1];

    /*
     * Initialize everything to 0.0/
     */
    for(i=0;i < (*trans)->n_actual[0]*(*trans)->n_actual[1];i++)
      (*trans)->data[i] = 0.0;

    /*
     * Else compute the primary beam for this pair of antennas, this frequency.
     */
    if(calc_primary_beam(sim, *trans, pair, freq))
      return 1;

    /*
     * And compute the transform.
     */
    (void) rfftwnd_one_real_to_complex((*trans)->plan, (*trans)->data, NULL);

    /*
     * 
     */
    {
      int ix,iy,ind;
      fftw_complex *complx = (fftw_complex *)(*trans)->data;

      for(ix=0;ix < (*trans)->n_actual[0]/2;ix++)
	for(iy=0;iy < (*trans)->n_actual[1];iy++) {
	  ind = ix + iy*((*trans)->n_actual[0]/2);

	  /*
	   * We multiply alternate elements by -1 to undo the effect
	   * of the shift.
	   */
	  complx[ind].re *= (iy%2==0 ? -1 : 1)*(ix%2==0 ? -1 : 1);
	  complx[ind].im *= (iy%2==0 ? -1 : 1)*(ix%2==0 ? -1 : 1);
	}

      /*
       * And set the zero-frequency (nx = 0, ny = n/2) to zero for
       * simplicity
       */
      ind = 0 + ((*trans)->n_actual[1]/2)*((*trans)->n_actual[0]/2);
      complx[ind].re = 0.0;
      complx[ind].im = 0.0;
    }

#ifdef DEBUG
  /*
   * Plot the transform
   */
  {
    int waserr=0;
    Image *image=sim->image;
    Fitshead *head=NULL,*save=NULL;
   
    if((head=get_uvheader(image->header, *trans))==NULL)
      waserr = 1;

    image->im = (*trans)->data;

    save = image->header;
    image->header = head;

    plotim(sim, image, OP_IM);

    image->header = save;
    image->im = NULL;

    del_Fitshead(head);
  }
#endif

  }
  return 0;
}
/*.......................................................................
 * Compute the transform of the image.
 *
 * The passed array will only be allocated by this function, so we can
 * have only three cases:
 *
 * sim->image AND FT are NULL         --> no image to observe; return NULL
 * sim->image != NULL, and FT == NULL --> transform hasn't been allocated yet;
 *                                         allocate it
 * sim->image != NULL, and FT != NULL --> transform has been allocated; don't 
 *                                         allocate it again, since it will have
 *                                         the same dimensions every time this 
 *                                         routine is called.
 */
static int obsim2(Simint *sim, VisList *pair, double wave, FT **trans)
{
  int i,nx,ny;

  if(sim->image == NULL) {
    if(*trans != NULL) {
      fprintf(stderr,"obsim: received invalid arguments.\n");
      return 1;
    } 
  } else {
    /*
     * If the transform hasn't been allocated yet, do it now.
     */
    if(*trans==NULL)
      if((*trans=new_FT2(sim->image))==NULL)
	return 1;

    nx=(*trans)->n_data[0];
    ny=(*trans)->n_data[1];
    /*
     * Initialize everything to 0.0/
     */
    for(i=0;i < (*trans)->n_actual[0]*(*trans)->n_actual[1];i++)
      (*trans)->data[i] = 0.0;
    /*
     * Else compute the primary beam for this pair of antennas, this frequency.
     */
    if(calc_primary_beam2(sim, *trans, pair, wave))
      return 1;
    /*
     * And compute the transform.
     */
    (void) rfftwnd_one_real_to_complex((*trans)->plan, (*trans)->data, NULL);
    /*
     * Undo the effect of the shift.
     */
    {
      int ix,iy,ind;
      fftw_complex *complx = (fftw_complex *)(*trans)->data;

      for(ix=0;ix < (*trans)->n_actual[0]/2;ix++)
	for(iy=0;iy < (*trans)->n_actual[1];iy++) {
	  ind = ix + iy*((*trans)->n_actual[0]/2);
	  /*
	   * We multiply alternate elements by -1 to undo the effect
	   * of the shift.
	   *
	   * Also normalize the convolution here by the sum over the beam.
	   */
	  complx[ind].re *= (iy%2==0 ? 1 : -1)*(ix%2==0 ? 1 : -1);
	  complx[ind].im *= (iy%2==0 ? 1 : -1)*(ix%2==0 ? 1 : -1); 
	}
    }
    /*
     * And set the zero-frequency (nx = 0, ny = n/2) to zero for simplicity

    {
      int ind;
      fftw_complex *complx = (fftw_complex *)(*trans)->data;

      ind = 0 + ((*trans)->n_actual[1]/2)*((*trans)->n_actual[0]/2);
      complx[ind].re = 0.0;
      complx[ind].im = 0.0;
    }
     */
  }
#ifdef DEBUG
  /*
   * Plot the transform
   */
  {
    int waserr=0;
    Image *image=sim->image;
    Fitshead *head=NULL,*save=NULL;
   
    if((head=get_uvheader(image->header, *trans))==NULL)
      waserr = 1;

    image->im = (*trans)->data;

    save = image->header;
    image->header = head;

    plotim(sim, image, OP_IM);

    image->header = save;
    image->im = NULL;

    del_Fitshead(head);
  }
#endif
  return 0;
}
/*.......................................................................
 * Compute the Re and Im visibility from the transform of an image.
 */
static int imvis_save(Image *image, FT *transform, double u, double v, double *re, double *im, int *badvis)
{
  int nu,nv;
  float umin,vmin;
  int iulo,iuhi,ivlo,ivhi;
  int im_sign=-1;
  /*
   * If no transform was passed, just fill with bogus values.
   */
  if(transform==NULL) {
    *re = 1.0;
    *im = 0.0;
  }
  /*
   * Else read off the visibilities from the transform.  The transform
   * contains the half-plane in u, full-plane in v.  I've shifted the
   * transform so that it is centered on the center of the array in y.
   */
  else {
    float du,dv,dx,dy;
    float xconv,yconv;
    fftw_complex *complx = (fftw_complex *)transform->data;
    /*
     * Number of elements in the complex array.
     */
    nu = transform->n_data[0]/4;/* This is just the halfplane of the
				   unpadded half of the input array */
    nv = transform->n_data[1]/2;/* This is just the unpadded half of
				   the input image array */
    /*
     * Determine the conversion between the native data axes and radians.
     */
    switch (image->header->ctypes[0]) {
    case AX_RAD:
      xconv = 1;
      break;
    case AX_DEG:
      xconv = M_PI/180;
      break;
    default:
      xconv = 1;
      break;
    }
    switch (image->header->ctypes[1]) {
    case AX_RAD:
      yconv = 1;
      break;
    case AX_DEG:
      yconv = M_PI/180;
      break;
    default:
      yconv = 1;
      break;
    }
    dx = image->header->cdelts[0]*xconv;
    dy = image->header->cdelts[1]*yconv;
    /*
     * The u-axis only goes from 0 to umax.  Resolution in u is set by
     * the full size of the input array, including padding, which is
     * trans->n_data[0] = 4*nu
     */
    du = 1.0/(4*nu*dx);
    /*
     * The v-axis goes from -vmax to vmax.  Resolution in v is set by
     * the full size of the input array, which is trans->n_data[0] =
     * 4*nv;
     */
    dv = 1.0/(2*nv*dy);
    /*
     * zero frequency is at the center of the ny/2 pixel
     */
    vmin = -1.0/(4*dy) - 0.5*dv;
    /*
     * And at the center of the first pixel, in u.
     */
    umin = -0.5;
    /*
     * Convert to the half plane for which we have data.  We'll force this to be u +ive 
     */
    if(u < 0) {
      im_sign = 1; /* We have to flip the sign of the Im component from the other half-plane,
		       to make the transform Hermitian */
      u = -u;
      v = -v;
    }
    /*
     * Convert the u and v to fractional pixel values.  umin and vmin
     * are the values of u and v at the extreme edges of the arrays,
     * so a value of u == umin is effectively at pixel -0.5.
     */
    {
      float fu,fv;

      fu = (u-umin)/du-0.5;   /* The approximate "pixel" value */
      fv = (v-vmin)/dv-0.5;

      iulo = (int)floor(fu);  /* The low bracketing pixel */
      ivlo = (int)floor(fv);
      iuhi = (int)ceil(fu);   /* The high bracketing pixel */
      ivhi = (int)ceil(fv);
      /*
       * If the point lies past the middle of the terminal pixels, but not more than one pixel
       * away, extrapolate from the two edge pixels.
       */
      if(iulo==-1) {
	iuhi = 1;
	iulo = 0;
      }
      if(ivlo==-1) {
	ivhi = 1;
	ivlo = 0;
      }
      if(iuhi==nu) {
	iulo = nu-2;
	iuhi = nu-1;
      }
      /*
       */
      if(ivhi==nv) {
	ivlo = nv-2;
	ivhi = nv-1;
      }
      /*
       * Finally, interpolate.  If no good bracketing values were
       * found, set this component to 0.0 
       */
      if(iulo < 0 || ivlo < 0 || iuhi > nu-1 || ivhi > nv) {
	*re = 0.0;
	*im = 0.0;
	*badvis = 1;
      }
      /*
       * Else interpolate from a square of bracketing pixels.
       */
      else {
	int ind_ll,ind_hl,ind_lh,ind_hh,iu,iv,ind;
	float re_ll,re_hl,re_lh,re_hh,re1,re2;
	float im_ll,im_hl,im_lh,im_hh,im1,im2;
	/*
	 * Test not interpolating.
	 */
	iu = fu-iulo < iuhi-fu ? iulo : iuhi;
	iv = fv-ivlo < ivhi-fv ? ivlo : ivhi;
	/*
	 * Get the indices in the complex array.
	 */
	iv += transform->n_actual[1]/4;
	ind = iu + (transform->n_actual[0]/2)*iv;

	/*
	 * Shift to the central quadrant
	 */
	fv += transform->n_actual[1]/4;
	ivlo += transform->n_actual[1]/4;
	ivhi += transform->n_actual[1]/4;
	/*
	 * Get the indices in the complex array.
	 */
	ind_ll = iulo + (transform->n_actual[0]/2)*ivlo;
	ind_hl = iuhi + (transform->n_actual[0]/2)*ivlo;
	ind_lh = iulo + (transform->n_actual[0]/2)*ivhi;
	ind_hh = iuhi + (transform->n_actual[0]/2)*ivhi;

	re_ll = complx[ind_ll].re;
	re_hl = complx[ind_hl].re;
	re_lh = complx[ind_lh].re;
	re_hh = complx[ind_hh].re;
	/*
	 * Interpolate in u. 
	 */
	re1 = re_hl + (re_ll-re_hl) * ((float)iuhi-fu);
	re2 = re_hh + (re_lh-re_hh) * ((float)iuhi-fu);
	/*
	 * Now interpolate in v.
	 */
	*re = re1 + (re2-re1) * ((float)ivhi-fv);

	im_ll = complx[ind_ll].im;
	im_hl = complx[ind_hl].im;
	im_lh = complx[ind_lh].im;
	im_hh = complx[ind_hh].im;

	im1 = im_hl + (im_ll-im_hl) * ((float)iuhi-fu);
	im2 = im_hh + (im_lh-im_hh) * ((float)iuhi-fu);

	*im = im_sign*(im1 + (im2-im1) * ((float)ivhi-fv));
      }
    }
  }
  return 0;
}
/*.......................................................................
 * Compute the Re and Im visibility from the transform of an image.
 *
 * We do this by convolving with a gaussian function over a finite
 * pixel mask.  The effect of the convolution will be taken care of in
 * the image plane by dividing by the appropriate gaussian.
 */
static int imvis(Image *image, FT *transform, double u, double v, double *re, 
		      double *im, int *badvis)
{
  int nu,nv;
  float umin,vmin;
  int iulo,iuhi,ivlo,ivhi;
  int im_sign=-1;
  /*
   * If no transform was passed, just fill with bogus values.
   */
  if(transform==NULL) {
    *re = 0.0;
    *im = 0.0;
  }
  /*
   * Else read off the visibilities from the transform.  The transform
   * contains the half-plane in u, full-plane in v.  I've shifted the
   * transform so that it is centered on the center of the array in y.
   * (But on the left (zero-frequency) edge in u)
   */
  else {
    float du,dv,dx,dy;
    fftw_complex *complx = (fftw_complex *)transform->data;
    /*
     * Number of elements in the complex array.
     */
    nu = transform->n_data[0]/4;/* This is just the halfplane of the
				   unpadded half of the input array */
    nv = transform->n_data[1]/2;/* This is just the unpadded half of
				   the input image array */
    /*
     * Determine the conversion between the native data axes and radians.
     */
    if(native_to_radian(image, 0, &dx))
      return 1;
    if(native_to_radian(image, 0, &dy))
      return 1;
    /*
     * The u-axis only goes from 0 to umax.  Resolution in u is set by
     * the full size of the input array, including padding, which is
     * trans->n_data[0] = 4*nu
     */
    du = 1.0/(4*nu*dx);
    /*
     * The v-axis goes from -vmax to vmax.  Resolution in v is set by
     * the full size of the input array, which is trans->n_data[0] =
     * 4*nv;
     */
    dv = 1.0/(2*nv*dy);
    /*
     * zero frequency is at the center of the ny/2 pixel
     */
    vmin = -1.0/(4*dy) - 0.5*dv;
    /*
     * And at the center of the first pixel, in u.
     */
    umin = -0.5*du;

    /*
     * Convert to the half plane for which we have data.  We'll force
     * this to be u +ive
     */
    if(u < 0) {
      im_sign = 1; /* We have to flip the sign of the Im component
		       from the other half-plane, to make the
		       transform Hermitian */
      u = -u;
      v = -v;
    }

    /*
     * Convert the u and v to fractional pixel values.  umin and vmin
     * are the values of u and v at the extreme edges of the arrays,
     * so a value of u == umin is effectively at pixel -0.5.
     */
    {
      float fu,fv;
      int iu,iv;

      fu = (u-umin)/du-0.5;   /* The approximate "pixel" value */
      fv = (v-vmin)/dv-0.5;

      iu = (int)floor(fu);
      iv = (int)floor(fv);

      iulo = iu-NMASK;  /* The low bracketing pixel */
      ivlo = iv-NMASK;
      iuhi = iu+NMASK;  /* The high bracketing pixel */
      ivhi = iv+NMASK;

      /*
       * If the point lies past the middle of the terminal pixels, but
       * not more than one pixel away, extrapolate from the two edge
       * pixels.
       */
      if(iulo < 0) 
	iulo = 0;
      if(ivlo < 0) 
	ivlo = 0;

      if(iuhi > nu-1) 
	iuhi = nu-1;
      if(ivhi > nv-1) 
	ivhi = nv-1;

      /*
       * Finally, convolve.  If no good bracketing values were found,
       * set this component to 0.0
       */
      if(iu < 0 || iv < 0 || iu > nu-1 || iv > nv) {
	*re = 0.0;
	*im = 0.0;
	*badvis = 1;
      }
      /*
       * Else interpolate from a square of bracketing pixels.
       */
      else {
	int i,j,ind;
	float s2=2*CONV_SIG*CONV_SIG;
	double arg,wt,wtsum=0.0;

	/*
	 * Convolve this value off of the grid, iterating over the
	 * pixel mask
	 */
	*re = 0.0;
	*im = 0.0;
	
	for(i=iulo;i <= iuhi;i++)
	  for(j=ivlo;j <= ivhi;j++) {
	    /*
	     * Get the value of the convolution function at this pixel.
	     */
	    arg = ((fu-i)*(fu-i)+(fv-j)*(fv-j))/s2;
	    wt = exp(-arg);
	    /*
	     * Shift to the center half in v.
	     */
	    iu = i;
	    iv = j+transform->n_actual[1]/4;
	    /*
	     * Get the index in the complex array.
	     */
	    ind = iu + (transform->n_actual[0]/2)*iv;
	    /*
	     * Store running means
	     */
	    *re += (complx[ind].re - *re)*wt/(wtsum+wt); 
	    *im += (complx[ind].im - *im)*wt/(wtsum+wt); 
	    wtsum += wt;
	  }
	*im *= im_sign;
      }
    }
  }
  return 0;
}

/*.......................................................................
 * Compute the Re and Im visibility from the transform of an image.
 */
static int imvis2(Image *image, FT *transform, double u, double v, 
		  double *re, double *im, int *badvis)
{
  int nu,nv;
  float umin,vmin;
  int iulo,iuhi,ivlo,ivhi;
  int im_sign=-1;
  /*
   * If no transform was passed, just fill with bogus values.
   */
  if(transform==NULL) {
    *re = 1.0;
    *im = 0.0;
  }
  /*
   * Else read off the visibilities from the transform.  The transform
   * contains the half-plane in u, full-plane in v.  I've shifted the
   * transform so that it is centered on the center of the array in y.
   */
  else {
    float du,dv,dx,dy;
    float xconv,yconv;
    fftw_complex *complx = (fftw_complex *)transform->data;
    /*
     * Number of elements in the complex array.
     */
    nu = transform->n_data[0]/2;/* This is just the halfplane of the
				   unpadded half of the input array */
    nv = transform->n_data[1];  /* This is just the unpadded half of
				   the input image array */
    /*
     * Determine the conversion between the native data axes and radians.
     */
    switch (image->header->ctypes[0]) {
    case AX_RAD:
      xconv = 1;
      break;
    case AX_DEG:
      xconv = M_PI/180;
      break;
    default:
      xconv = 1;
      break;
    }
    switch (image->header->ctypes[1]) {
    case AX_RAD:
      yconv = 1;
      break;
    case AX_DEG:
      yconv = M_PI/180;
      break;
    default:
      yconv = 1;
      break;
    }

    dx = image->header->cdelts[0]*xconv;
    dy = image->header->cdelts[1]*yconv;

    /*
     * The u-axis only goes from 0 to umax.  Resolution in u is set by
     * the full size of the input array, including padding, which is
     * trans->n_data[0] = 2*nu
     */
    du = 1.0/(2*nu*dx);

    /*
     * The v-axis goes from -vmax to vmax.  Resolution in v is set by
     * the full size of the input array, which is trans->n_data[0] =
     * 2*nv;
     */
    dv = 1.0/(nv*dy);

    /*
     * zero frequency is at the center of the ny/2 pixel
     */
    vmin = -1.0/(2*dy)  - 0.5*dv;

    /*
     * And at the center of the first pixel, in u.
     */
    umin = -0.5;

    /*
     * Convert to the half plane for which we have data.  We'll force
     * this to be u +ive
     */
    if(u < 0) {
      im_sign = 1; /* We have to flip the sign of the Im component
		       from the other half-plane, to make the
		       transform Hermitian */
      u = -u;
      v = -v;
    }
    /*
     * Convert the u and v to fractional pixel values.  umin and vmin
     * are the values of u and v at the extreme edges of the arrays,
     * so a value of u == umin is effectively at pixel -0.5.
     */
    {
      float fu,fv;

      fu = (u-umin)/du-0.5;   /* The approximate "pixel" value */
      fv = (v-vmin)/dv-0.5;

      iulo = (int)floor(fu);  /* The low bracketing pixel */
      ivlo = (int)floor(fv);
      iuhi = (int)ceil(fu);   /* The high bracketing pixel */
      ivhi = (int)ceil(fv);

      /*
       * If the point lies past the middle of the terminal pixels, but
       * not more than one pixel away, extrapolate from the two edge
       * pixels.
       */
      if(iulo==-1) {
	iuhi = 1;
	iulo = 0;
      }
      if(ivlo==-1) {
	ivhi = 1;
	ivlo = 0;
      }
      if(iuhi==nu) {
	iulo = nu-2;
	iuhi = nu-1;
      }

      /*
       * Adjust for the fact that we've duplicated the critical
       * frequency into the row above 3n/4-1
       */
      if(ivhi==nv+1) {
	ivlo = nv-1;
	ivhi = nv;
      }

      /*
       * Finally, interpolate.  If no good bracketing values were
       * found, set this component to 0.0
       */
      if(iulo < 0 || ivlo < 0 || iuhi > nu-1 || ivhi > nv) {
	*re = 0.0;
	*im = 0.0;
	*badvis=1;
      }
      /*
       * Else interpolate from a square of bracketing pixels.
       */
      else {
	int ind_ll,ind_hl,ind_lh,ind_hh;
	float re_ll,re_hl,re_lh,re_hh,re1,re2;
	float im_ll,im_hl,im_lh,im_hh,im1,im2;
	/*
	 * Get the indices in the complex array.
	 */
	ind_ll = iulo + (transform->n_actual[0]/2)*ivlo;
	ind_hl = iuhi + (transform->n_actual[0]/2)*ivlo;
	ind_lh = iulo + (transform->n_actual[0]/2)*ivhi;
	ind_hh = iuhi + (transform->n_actual[0]/2)*ivhi;

	re_ll = complx[ind_ll].re;
	re_hl = complx[ind_hl].re;
	re_lh = complx[ind_lh].re;
	re_hh = complx[ind_hh].re;
	/*
	 * Interpolate in u. 
	 */
	re1 = re_hl + (re_ll-re_hl) * ((float)iuhi-fu);
	re2 = re_hh + (re_lh-re_hh) * ((float)iuhi-fu);
	/*
	 * Now interpolate in v.
	 */
	*re = re1 + (re2-re1) * ((float)ivhi-fv);

	im_ll = complx[ind_ll].im;
	im_hl = complx[ind_hl].im;
	im_lh = complx[ind_lh].im;
	im_hh = complx[ind_hh].im;

	im1 = im_hl + (im_ll-im_hl) * ((float)iuhi-fu);
	im2 = im_hh + (im_lh-im_hh) * ((float)iuhi-fu);

	*im = im_sign*(im1 + (im2-im1) * ((float)ivhi-fv));
      }
    }
  }
  return 0;
}
/*.......................................................................
 * Allocate a new FT container
 */
static FT *new_FT(Image *image)
{
  int nx,ny,i;
  FT *ft=NULL;

  if(image==NULL) {
    fprintf(stderr,"new_FT: received NULL image.\n");
    return NULL;
  }
  /*
   * Attempt to allocate the container.
   */
  if((ft=(FT *)malloc(sizeof(FT)))==NULL) {
    fprintf(stderr,"new_FT: Unable to allocate FT container.\n");
    return NULL;
  }
  nx = image->header->naxes_data[0];
  ny = image->header->naxes_data[1];
  /*
   * Store a pointer to the parent image.
   */
  ft->image =  image;
  /*
   * Check that the axes are powers of 2.
   *
   * Make the transform array twice as big as the image, to avoid aliasing.
   *
   */
  ft->n_data[0] = 2*pow(2.0f,floor((log((double)nx)/log(2.0)+0.5)));
  ft->n_data[1] = 2*pow(2.0f,floor((log((double)ny)/log(2.0)+0.5)));
  /*
   * Also expand the x-array size by 2m as required by the fftw
   * algorithm for real transforms.
   */
  ft->n_actual[0] = ft->n_data[0]+2;
  ft->n_actual[1] = ft->n_data[1];
  /*
   * Allocate the data array itself.
   */
  ft->data = NULL;
  if((ft->data=(fftw_real *)malloc(ft->n_actual[0]*ft->n_actual[1]*sizeof(fftw_real)))==NULL) {
    fprintf(stderr,"new_FT: Insufficient memory to allocate transform data array.\n");
    return del_FT(ft);
  }
  for(i=0;i < ft->n_actual[0]*ft->n_actual[1];i++)
    ft->data[i] = 0.0;
  /*
   * And create the plan.
   */
  ft->plan = NULL;
  if((ft->plan=rfftwnd_create_plan(2, ft->n_data, FFTW_REAL_TO_COMPLEX, 
				 FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL) {
    fprintf(stderr,"new_FT: Insufficient memory to allocate transform plan.\n");
    return del_FT(ft);
  }
  return ft;
}
/*.......................................................................
 * Allocate a new FT container
 */
static FT *new_FT2(Image *image)
{
  int nx,ny,i;
  FT *ft=NULL;

  if(image==NULL) {
    fprintf(stderr,"new_FT: received NULL image.\n");
    return NULL;
  }
  /*
   * Attempt to allocate the container.
   */
  if((ft=(FT *)malloc(sizeof(FT)))==NULL) {
    fprintf(stderr,"new_FT: Unable to allocate FT container.\n");
    return NULL;
  }
  nx = image->header->naxes_data[0];
  ny = image->header->naxes_data[1];
  /*
   * Check that the axes are powers of 2.
   *
   */
  ft->n_data[0] = pow(2.0f,floor((log((double)nx)/log(2.0)+0.5)));
  ft->n_data[1] = pow(2.0f,floor((log((double)ny)/log(2.0)+0.5)));
  /*
   * Also expand the x-array size by 2m as required by the fftw
   * algorithm for real transforms.
   */
  ft->n_actual[0] = ft->n_data[0]+2;
  ft->n_actual[1] = ft->n_data[1];
  /*
   * Allocate the data array itself.
   */
  ft->data = NULL;
  if((ft->data=(fftw_real *)malloc(ft->n_actual[0]*ft->n_actual[1]*sizeof(fftw_real)))==NULL) {
    fprintf(stderr,"new_FT: Insufficient memory to allocate transform data array.\n");
    return del_FT(ft);
  }
  for(i=0;i < ft->n_actual[0]*ft->n_actual[1];i++)
    ft->data[i] = 0.0;
  /*
   * And create the plan.
   */
  ft->plan = NULL;
  if((ft->plan=rfftwnd_create_plan(2, ft->n_data, FFTW_REAL_TO_COMPLEX, 
				 FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL) {
    fprintf(stderr,"new_FT: Insufficient memory to allocate transform plan.\n");
    return del_FT(ft);
  }
  return ft;
}
/*.......................................................................
 * Delete a single FT descriptor.
 */
static FT *del_FT(FT *ft)
{
  if(ft==NULL)
    return NULL;

  ft->image = NULL;

  if(ft->data)
    free(ft->data);
  /*
   * And free the plan
   */
  if(ft->plan)
    rfftwnd_destroy_plan(ft->plan);

  free(ft);
  ft = NULL;
  return ft;
}
/*.......................................................................
 * Install the image, multiplied by the primary beam for a given pair of 
 * antennas, into the transform array.
 */
static int calc_primary_beam(Simint *sim, FT *trans, VisList *pair, double freq)
{
  /*
   * All calculations will proceed in radians.  Check the units of the
   * input image.
   */
  int ix,iy,imind,ftind,xind,yind;
  int ixmid,iymid;
  float data_conv;

#ifdef PTSRC
  static int first=1,counter=0;
  // The i pixel index of the source 
  static int isrc[8] =      {108,  0, 120, 0,0, 150,0,0}; 
  // The j pixel index of the source 
  static int jsrc[8] =      {118,  0, 140, 0,0, 110,0,0}; 
  // The flux, in Jy of the source 
  static float fluxsrc[8] = {0.5,  0,   1, 0,0, 0.4,0,0}; 
  // The spectral index of the source 
  static float asrc[8] =    {-0.7, 0, 0.0, 0,0, 0.3,0,0}; 
  float freq0 = 30.0;
  int srcind,i;
#endif

  /*
   * Get the width of the Gaussian function we will divide the image
   * by to correct for the convolution in the UV plane.
   */
  double sigx = trans->n_data[0]/(2*M_PI)/CONV_SIG;
  double sigy = trans->n_data[1]/(2*M_PI)/CONV_SIG;
  double arg,wt;

  Image *image = sim->image;
#ifdef PTSRC
  /*
   * What source is this?
   */
  srcind = counter/10;
  /*
   * If adding point sources was desired, allocate an array for
   * storing the point source fluxes 
   */
  if(counter%10==0) {
    if((image->im=(float *)malloc(sizeof(float)*image->header->n))==NULL) {
      fprintf(stderr,"Unable to allocate the array of point source fluxes\n");
      return 1;
    }
  }
  /*
   * Initialize the array to zero after each image.
   */
  if(counter%10==0) {
    for(i=0;i < image->header->n;i++)
      image->im[i] = 0.0;
  }
  /*
   * Now add the appropriate point source 
   */
  xind = isrc[srcind] + image->header->data_start[0];
  yind = jsrc[srcind] + image->header->data_start[1];
  /*
   * Get the corresponding image pixel
   */
  imind = yind*image->header->naxes_actual[0] + xind;
  image->im[imind] = fluxsrc[srcind]*pow(freq/freq0,asrc[srcind]);
  counter++;

#endif
  /*
   * Get the conversion factor between image units and Jy.
   */
  if(native_to_jansky(image, freq, &data_conv))
    return 1;

  fprintf(stdout, "Conversion to Jy at %g GHz is: %g\n", freq, data_conv);

  /*
   * Store the indices of the center pixel
   */
  ixmid = image->header->naxes_data[0]/2;
  iymid = image->header->naxes_data[1]/2;
  /*
   * Iterate over all image pixels which contain data.
   */
  for(iy=0;iy < image->header->naxes_data[1];iy++)  
    for(ix=0;ix < image->header->naxes_data[0];ix++) {
      /*
       * This pixel will be installed in the center of the transform
       * array at a location modified by the extra 2 elements in the
       * x-axis, modulo any buffering we may also have to do to make
       * the array a power of 2 in each dimension.
       */
      ftind = ((iy+image->header->naxes_data[1]/2)*trans->n_actual[0] + 
	       (ix+image->header->naxes_data[0]/2));

      xind = ix + image->header->data_start[0];
      yind = iy + image->header->data_start[1];

      /*
       * Get the value of the function which will correct for the UV
       * convolution
       */
      arg = (xind-ixmid)*(xind-ixmid)/(2*sigx*sigx)+(yind-iymid)*
	(yind-iymid)/(2*sigy*sigy);
      wt = exp(arg);

      imind = yind*image->header->naxes_actual[0] + xind;

      /*
       * Install the image in the transform array, shifting in y
       */
#ifdef PTSRC
      /*
       * If point sources are being added, add the array which
       * contains the appropriate flux for this frequency.
       */
      trans->data[ftind] = (data_conv*image->re[imind]+image->im[imind])*wt*(iy%2==0 ? 1 : -1);  
#else
      trans->data[ftind] = data_conv*image->re[imind]*wt*(iy%2==0 ? 1 : -1);  
#endif
    }

  /*
   * Now modulate with the effective primary beam of this antenna
   * pair.
   */
  if(beam_envelope(sim, pair, freq, trans))
    return 1;

#ifdef DEBUG
  /*
   * Plot the image.
   */
  {
    Fitshead *head=NULL,*save=NULL;
    /*
     * Initialize a header.
     */
    if((head=new_Fitshead(""))==NULL)
      return 1;
    
    head->naxis = image->header->naxis;
    /*
     * Allocate memory for the FITS axes of this header.
     */
    if(new_Axes(head, image->header->naxis))
      return del_Fitshead(head)!=NULL;
    
    if(copy_Fitshead(image->header, head))
      return del_Fitshead(head)!=NULL;
    
    head->naxes_actual[0] = trans->n_actual[0];
    head->naxes_actual[1] = trans->n_actual[1];

    head->imin = 0;
    head->imax = trans->n_actual[0]-1;
    	
    head->jmin = 0;
    head->jmax = trans->n_actual[1]-1;
    	
    head->n = trans->n_actual[1]*trans->n_actual[0];

    image->im = trans->data;
    head->type = T_CMPLX;

    save = image->header;
    image->header = head;

    plotim(sim, image, OP_IM);

    image->header = save;
    image->im = NULL;

    head = del_Fitshead(head);
  }
#endif

#ifdef PTSRC
  /*
   * If generating point sources, free the temprary array.
   */
  if(counter%10==0) {
    if(image->im)
      free(image->im);
    image->im = NULL;
  }
#endif
  return 0;
}
/*.......................................................................
 * Install the image, multiplied by the primary beam for a given pair of 
 * antennas, into the transform array.
 */
static int calc_primary_beam2(Simint *sim, FT *trans, VisList *pair, 
			      double wave)
{
  /*
   * All calculations will proceed in radians.  Check the units of the
   * input image.  
   */
  double xconv,yconv,sig1,sig2,sig,prefac;
  int ix,iy,imind,ftind,ixim,iyim;
  double x,y,bm;
  Image *image = sim->image;

  switch (image->header->ctypes[0]) {
  case AX_RAD:
    xconv = 1;
    break;
  case AX_DEG:
    xconv = M_PI/180;
    break;
  default:
    xconv = 1;
    break;
  }
  switch (image->header->ctypes[1]) {
  case AX_RAD:
    yconv = 1;
    break;
  case AX_DEG:
    yconv = M_PI/180;
    break;
  default:
    yconv = 1;
    break;
  }
  /*
   * Primary beam will be (roughly) a gaussian whose width is given by:
   *
   * sqrt((4 ln2 wave^2)/(pi * A_p * apeff))
   */
  prefac = 4*sqrt(log(2.0))/M_PI;
  sig1 = prefac*(wave/pair->type1->size)/sqrt(pair->type1->apeff);
  sig2 = prefac*(wave/pair->type2->size)/sqrt(pair->type2->apeff);
  /*
   * Primary beam will be the mean of the two if the antennas are different.
   */
  sig = sqrt(sig1*sig2);
  /*
   * Iterate over all image pixels which contain data.
   */
  for(iy=0;iy < image->header->naxes_data[1];iy++)  
    for(ix=0;ix < image->header->naxes_data[0];ix++) {
      /*
       * The start of the image data.
       */
      ixim = ix+image->header->data_start[0];
      iyim = iy+image->header->data_start[1];
      /*
       * Place this pixel in the same position as in the image.
       */
      ftind = ix + iy*trans->n_actual[0];
      imind = ixim + iyim*image->header->naxes_actual[0];
      /*
       * Get the x and y values at this pixel location.
       */
      x = image->header->crvals[0] + (ixim-image->header->crpixs[0])*
	image->header->cdelts[0];
      y = image->header->crvals[1] + (iyim-image->header->crpixs[1])*
	image->header->cdelts[1];

      x *= xconv;
      y *= yconv;
      /*
       * And get the amplitude of the primary beam at this location.
       */
      bm = gauss2d(x,0,y,0,sig,sig,0.0);  
      /*
       * Compute the transform, shifting in y
       */
      trans->data[ftind] = image->re[imind]*bm*(iy%2==0 ? 1 : -1)*image->header->cdelts[0]*image->header->cdelts[1];
    }
#ifdef DEBUG
  /*
   * Plot the image.
   */
  {
    Fitshead *head=NULL,*save=NULL;
    /*
     * Initialize a header.
     */
    if((head=new_Fitshead(""))==NULL)
      return 1;
    
    head->naxis = image->header->naxis;
    /*
     * Allocate memory for the FITS axes of this header.
     */
    if(new_Axes(head, image->header->naxis))
      return del_Fitshead(head)!=NULL;
    
    if(copy_Fitshead(image->header, head))
      return del_Fitshead(head)!=NULL;
    
    head->naxes_actual[0] = trans->n_actual[0];
    head->naxes_actual[1] = trans->n_actual[1];

    head->imin = 0;
    head->imax = trans->n_actual[0]-1;
    	
    head->jmin = 0;
    head->jmax = trans->n_actual[1]-1;
    	
    head->n = trans->n_actual[1]*trans->n_actual[0];

    image->im = trans->data;
    head->type = T_CMPLX;

    save = image->header;
    image->header = head;

    plotim(sim, image, OP_IM);

    image->header = save;
    image->im = NULL;

    head = del_Fitshead(head);
  }
#endif

  return 0;
}
/*.......................................................................
 * Given an image, return an appropriate header for the transform.
 */
static Fitshead *get_uvheader(Fitshead *imhdr, FT *trans)
{
  Fitshead *head=NULL;
  /*
   * Initialize a header.
   */
  if((head=new_Fitshead(""))==NULL)
    return NULL;
  head->naxis = imhdr->naxis;
  /*
   * Allocate memory for the FITS axes of this header.
   */
  if(new_Axes(head, imhdr->naxis))
    return del_Fitshead(head);
  
  if(copy_Fitshead(imhdr, head))
    return del_Fitshead(head);

  head->n = trans->n_actual[0]*trans->n_actual[1];
  head->naxes_actual[0] = trans->n_actual[0];
  head->naxes_actual[1] = trans->n_actual[1];
  /*
   * By now, we've copied the duplicated critical frequency into the 3n/4 row of the
   * transform.  So increase the size of the data array by 1 in the y-direction
   */
  head->naxes_data[0] = trans->n_data[0];
  head->naxes_data[1] = trans->n_data[1]+1;
  /*
   * Set the display to include only the unaliased quadrant of the transform.
   */
#ifdef TRUE
  head->imin = 0;
  head->imax = (trans->n_actual[0]-2)/2-1;
  head->jmin = trans->n_actual[1]/4;
  /*
   * Likewise here we have to increase the size of the yarray.
   */
  head->jmax = 3*(trans->n_actual[1])/4;
#else
  head->imin = 0;
  head->imax = trans->n_actual[0]-1;
  head->jmin = 0;
  head->jmax = trans->n_actual[1]-1;
#endif

  head->type = T_CMPLX;
  
  head->ctypes[0] = AX_U;
  head->ctypes[1] = AX_V;
  {
    int nu,nv;
    float dx,dy,du,dv;
    float xconv,yconv;
    /*
     * Number of unpadded elements in the complex array.
     */
    nu = trans->n_data[0]/2;
    nv = trans->n_data[1]/2;
    /*
     * Determine the conversion between the native data axes and radians.
     */
    switch (imhdr->ctypes[0]) {
    case AX_RAD:
      xconv = 1;
      break;
    case AX_DEG:
      xconv = M_PI/180;
      break;
    default:
      xconv = 1;
      break;
    }
    switch (imhdr->ctypes[1]) {
    case AX_RAD:
      yconv = 1;
      break;
    case AX_DEG:
      yconv = M_PI/180;
      break;
    default:
      yconv = 1;
      break;
    }
    dx = imhdr->cdelts[0]*xconv;
    dy = imhdr->cdelts[1]*yconv;
    /*
     * The u-axis only goes from 0 to umax.  
     */
    du = 1.0/(2*nu*dx);
    /*
     * The v-axis goes from -vmax to vmax
     */
    dv = 1.0/(2*nv*dy);

    head->crpixs[0] = -0.5;
    /*
     * Now 0 frequency is at the N/2 pixel.
     */
    head->crpixs[1] = ((float)(trans->n_actual[1]+1))/2;

    head->crvals[0] = 0.0;
    head->crvals[1] = 0.0;
    /* 
     * For purposes of plotting, the du will be
     * half the complex du.
     */
#ifdef TRUE
    head->cdelts[0] = du/2;
    head->cdelts[1] = dv;
#else
    head->cdelts[0] = du/2;
    head->cdelts[1] = dv;
#endif
    /*
     * Get the effective min and max of the full-size array.
     */
#ifdef TRUE
    head->xmin = 0.0;
    head->xmax = head->cdelts[0]*head->naxes_actual[0];

    head->ymin = -head->cdelts[1]*(head->naxes_actual[1]/2+0.5);
    head->ymax =  head->cdelts[1]*(head->naxes_actual[1]/2-0.5);
#else
    head->xmin = 0.0;
    head->xmax = 1.0;

    head->ymin = 0.0;
    head->ymax = 1.0;
#endif
  }
  return head;
}

/*.......................................................................
 * Calculate a generic aperture function, with the same taper as DASI.
 *
 * Input:
 *
 *    r       double     Radius, in cm
 *    diam    double     The diameter of the aperture, in cm.
 *
 * Input/Output:
 *
 *    val     double  *  The value of the aperture field at the specified radius
 *
 * Output:
 *
 *    waserr  int        0 -- OK
 *                     1 -- error
 */
static int apfield(double r, double aperture_diameter, double *val) 
{
  double g, n, r0, theta, dr_dtheta, theta0;
  double ct, st;
  static const double alpha_01=2.4048255;     // first zero of Bessel fcn J0 
  static const double lens_index=1.527;       // DASI lens design
					      // index of refraction
					      // (for T_lens = 273 K).
  static const double theta_0=30.0;           // DASI horn design
					      // half-angle, degrees.

  int waserr=0;

  if (r < 0) {
    fprintf(stderr,"apfield: Input argument is negative.\n");
    return 1;
  }
  if(r > aperture_diameter/2) {
    *val = 0;
    return 0;
  }
  theta0 = theta_0*M_PI/180.0;
  r0 = aperture_diameter/(2*sin(theta0));
  n = lens_index;
  g = r0*(n-cos(theta0));
  /* 
   * Calculate cos(theta) as a function of r 
   */
  ct = (r*r*n + g*sqrt(g*g + (1-n*n)*r*r))/(g*g + r*r);
  theta = acos(ct);
  /* 
   * Calculate sin(theta) (always positive or zero) 
   */ 
  st = sqrt(1-ct*ct);
  /* 
   * Derivative dr/dtheta. 
   */ 
  dr_dtheta = g*(n*ct-1)/((n-ct)*(n-ct));
  /* 
   * Eqn. for field amplitude. 
   */
  if(r==0) 
    *val = bessj0((theta/theta0)*alpha_01)/dr_dtheta;
  else if(r < aperture_diameter/2.0)
    *val = bessj0((theta/theta0)*alpha_01)*sqrt(st/(r*dr_dtheta));
  else
    *val = 0; 

  return waserr;
}
/*.......................................................................
 * Compute the zero-th order Bessel function (NR routine)
 */
static float bessj0(float x)
{
  float ax,z;
  double xx,y,ans,ans1,ans2;
  
  if ((ax = fabs(x)) < 8.0) {
    y = x*x;
    ans1 = 57568490574.0 + y*(-13362590354.0 + y*(651619640.7 
		  + y*(-11214424.18 + y*(77392.33017 + y*(-184.9052456)))));
    ans2 = 57568490411.0 + y*(1029532985.0 + y*(9494680.718
		  + y*(59272.64853 + y*(267.8532712 + y*1.0))));
    ans = ans1/ans2;
  } else {

    z = 8.0/ax;
    y = z*z;
    xx=ax-0.785398164;
    ans1 = 1.0 + y*(-0.1098628627e-2 + y*(0.2734510407e-4
	    + y*(-0.2073370639e-5 + y*0.2093887211e-6)));
    ans2 = -0.1562499995e-1 + y*(0.1430488765e-3
	       + y*(-0.6911147651e-5 + y*(0.7621095161e-6 - y*0.934935152e-7)));
    ans=sqrt(0.636619772/ax)*(cos(xx)*ans1-z*sin(xx)*ans2);
  }
  return ans;
}
/*.......................................................................
 * Take an input transform array, and multiply it by the effective
 * primary beam for a given antenna pair.  The effective primary beam
 * is the multiplication of the transforms of the aperture current
 * grading.  For non-identical antenna pairs, this is in general not
 * guaranteed to be everywhere positive.
 *
 * Input:
 *
 * pair   VisList  *  The pair of antennas for which the primary beam
 *                    envelope is desired.
 * freq   double      Frequency, in GHz
 * trans  FT       *  The parent transform container
 *
 * Output:
 *
 *  0 -- ok
 *  1 -- error
 */
static int beam_envelope(Simint* sim, VisList *pair, double freq, FT *trans)
{
  rfftwnd_plan plan_inv=NULL;
  float *beam=NULL;
  float dx,dy,drx,dry,rymin;
  int n[2],i,j,waserr=0,iant;
  double wave = C/(freq*1e9);
  /*
   * Get the delta x and delta y of the image in radians.
   */
  if(native_to_radian(trans->image, 0, &dx))
    return 1;
  if(native_to_radian(trans->image, 1, &dy))
    return 1;

  /*
   * The critical sampling frequency for the aperture function is set
   * by the angular resolution of the desired image, in this case dx
   * and dy.
   */
  drx = wave/(trans->n_data[0]*dx);
  dry = wave/(trans->n_data[1]*dy);
  rymin = -trans->n_data[1]/2*dry;

  /*
   * Now allocate the array we'll need to compute the aperture.  This should be 
   * of size (ngridx+2)*ngridy.
   */ 
  if((beam=(float *)malloc((trans->n_actual[0]*trans->n_actual[1]*sizeof(float))))==NULL) {
    fprintf(stderr,"Unable to allocate transform array\n");
    return -1;
  }

  /*
   * Zero the array.
   */
  for(i=0;i < trans->n_actual[0]*trans->n_actual[1];i++)
    beam[i] = 0.0;

  /*
   * Allocate the inverse plan.
   */
  n[0] = trans->n_data[0];
  n[1] = trans->n_data[1];

  if((plan_inv=rfftwnd_create_plan(2, n, FFTW_COMPLEX_TO_REAL,
				   FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL)
    waserr = 1;

  /*
   * If no errors were encountered, proceed to calculate the aperture.
   * We will place this in the center of the transform array.
   */
  if(!waserr) {
    int ind;
    fftw_complex *complx = (fftw_complex *)beam;
    double rx,ry,r,val,diam;

    /*
     * Loop over both antennas (they might be different).  We will
     * treat the beam pattern as being the product of the far field
     * responses from the separate antennas.  In the case of the same
     * type of antenna, this is just the square of the ff response.
     * In the case of distinct antennas, this is the geometric mean of
     * the two power patterns.
     */
    for(iant=0;iant < 2;iant++) {
      diam = iant==0 ? pair->type1->size : pair->type2->size;

      /*
       * Now iterate over the whole array.  We only recompute the
       * transform of the aperture field if the antennas are not
       * identical.  Otherwise we will just compute the transform of
       * the first, and re-use it for the second.
       */
      if(iant < 1 || (pair->type1 != pair->type2)) {
	for(j=0;!waserr && j < trans->n_actual[1];j++)
	  for(i=0;!waserr && i < trans->n_actual[0]/2;i++) {

	    /*
	     * Note that rymin is the value of y at the edge of the
	     * full transform array, including zero-buffering.
	     */
	    rx = drx*i;
	    ry = rymin + dry*j;
	    r = sqrt(rx*rx+ry*ry);
	    ind = j*trans->n_actual[0]/2+i;

	    /*
	     * Install the appropriate value in the transform array,
	     * multiplying every other element by -1 to shift the
	     * image to the center.
	     */
	    //	    fprintf(stdout,"Calculating apfield with r = %f
	    //	    diam = %f\n", r, diam);

	    waserr=apfield(r, diam, &val);
	    
	    if(!waserr) {
	      complx[ind].re = (float)val*(j%2==0 ? 1 : -1)*(i%2==0 ? 1 : -1);
	      complx[ind].im = 0.0;
	    }
	  }

	/*
	 * Now compute the inverse transform to recover the far field
	 * response corresponding to this aperture distribution.
	 */
	if(!waserr) 
	  (void)rfftwnd_one_complex_to_real(plan_inv, (fftw_complex *)beam, NULL); 
      }

      /*
       * Now we have the far field response in an array equal in size
       * and dimensions to the input image.  All we have to do is
       * multiply them.
       */
      if(!waserr) {
	/*
	 * Normalize by the peak of the far-field response.
	 */
	ind = trans->n_data[1]/2*trans->n_actual[0] + trans->n_data[0]/2;
	/* 
	 * Sequentially multiply the two patterns together here.
	 */
	for(i=0;i < trans->n_actual[0]*trans->n_actual[1];i++)
	  trans->data[i] *= beam[i]/beam[ind];
      }
    }
  }

#ifdef DEBUG
  /*
   * Plot the transform
   */
  {
    int waserr=0;
    Image* image=sim->image;
    Fitshead *head=NULL,*save=NULL;
   
    fprintf(stdout, "Plotting beam for this pair:\n");

    if((head=get_uvheader(image->header, trans))==NULL)
      waserr = 1;

    image->im = beam;

    save = image->header;
    image->header = head;

    plotim(sim, image, OP_IM);

    image->header = save;
    image->im = NULL;

    del_Fitshead(head);
  }
#endif

  /*
   * Free any memory allocated in this routine.
   */
  if(beam)
    free(beam);
  if(plan_inv)
    rfftwnd_destroy_plan(plan_inv);

  return waserr;
}
