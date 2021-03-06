#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "image.h"
#include "fitsio.h"

static float dplanck(float freq, float T);
static float dplanckRJ(float freq, float T);
static double planckX(double freq, double T);
static double comptonYToDtemp(double freq);

/* 
 * Create a container for a dynamic array of known images
 */
struct {
  int n;               /* Number of images in array */
  char **alias;        /* Array of aliases for the created images */
  Image **f;           /* Array of 'images.n' known images */
} images={0,NULL};
/*
 * Enumerate valid axis types. The last card is the default and should 
 * always be UNKNOWN.
 */
Axiscard fitsaxes[] = {
  {"RA" ,     "\\gh (\\(0718))", AX_DEG},
  {"DEC",     "\\gh (\\(0718))", AX_DEG},
  {"RA_R",    "\\gh (rad)",      AX_RAD},
  {"DEC_R",   "\\gh (rad)",      AX_RAD},
  {"U",       "U (\\gl)",        AX_U},
  {"V",       "V (\\gl)",        AX_V},
  {"UNKNOWN", "Unknown",         AX_UNKNOWN},
};
/*
 * And the number of valid types
 */
int nfitsaxes = sizeof(fitsaxes)/sizeof(Axiscard);
/*
 * Enumerate valid bunit types.  The last card is the default and should 
 * always be UNKNOWN.
 */
Bunitcard fitsunits[] = {
  {"MJY/SR", "MJy/sr",   BU_MJYSR},
  {"JY",     "Jy",       BU_JY},
  {"JY/BEAM","Jy/Beam",  BU_JYBEAM},
  {"MUK",    "\\gmK",    BU_MUK},
  {"K",      "K",        BU_K},
  {"Y",      "Y",        BU_Y},
  {"UNKNOWN", "Unknown", BU_UNKNOWN},

  {"MJy/sr", "MJy/sr",   BU_MJYSR},
  {"Jy",     "Jy",       BU_JY},
  {"Jy/Beam","Jy/Beam",  BU_JYBEAM},
  {"muK",    "\\gmK",    BU_MUK},
  {"K",      "K",        BU_K},
  {"Y",      "Y",        BU_Y},
  {"Unknown", "Unknown", BU_UNKNOWN},
};
/*
 * And the number of valid types
 */
int nfitsunits = sizeof(fitsunits)/sizeof(Bunitcard);

// Local functions.

Bunit parseUnits(char* units);

/*.......................................................................
 * Return a given image by its (1-relative) index in the array of known
 * images.
 *
 * Input:
 *  findex int      The 1-relative index into the array of images.
 * Output:
 *  return Image *  The image at the requested position or NULL if
 *                  findex is out of range.
 */
Image *get_image(int findex)
{
  if(findex < 1 || findex > images.n) {
    fprintf(stderr, "get_image: No image numbered: %d\n", findex);
    return NULL;
  };
  return images.f[findex-1];
}
/*.......................................................................
 * Add a image to the list of known images.
 *
 * Input:
 *  image   Image *  The Image to be appended to the list.
 * Output:
 *  return      int    0 - OK.
 *                     1 - Error.
 */
int add_image(Image *image)
{
  Image **farray;   /* Pointer to expanded array of images */
  if(image==NULL) {
    fprintf(stderr, "add_image: NULL image received\n");
    return 1;
  };
/*
 * Attempt to expand the array of known images.
 */
  if(images.f==NULL || images.n==0)
    farray = (Image **) malloc(sizeof(Image *));
  else
    farray = (Image **) realloc(images.f, (images.n+1) * sizeof(Image *));
/*
 * Did the resize operation fail?
 */
  if(farray==NULL) {
    fprintf(stderr, "Insufficient memory to resize array of known images\n");
    return 1;
  };
/*
 * Append the new image to the end of the array.
 */
  farray[images.n++] = image;
  images.f = farray;
  return 0;
}
/*.......................................................................
 * Locate the appropriate image from an array of images.
 */
Image *imfind(char *imnam)
{
  Image *image=NULL;
  int i;

  for(i=0; i<images.n; i++) {
    image = images.f[i];
    if(strcmp(image->name,imnam)==0)
      return image;
  }
  return NULL;
}
/*.......................................................................
 * Remove an image from the established list of images.
 */
Image *rem_image(int findex)
{
  Image *image=NULL;   /* The removed image */
  int i;
/*
 * Extract the image.
 */
  image = get_image(findex);
  if(image==NULL)
    return image;
/*
 * Shuffle the images above the removed image to cover its slot.
 */
  for(i=findex-1; i<images.n-1; i++)
    images.f[i] = images.f[i+1];
  images.n--;
  return image;
}
/*.......................................................................
 * Delete all images in the list.
 *
 */
void zap_images(void)
{
  int i,nrem;
  Image *image=NULL;
  
  do {
    nrem = 0;
    for(i=1;i <= images.n;i++) {
      image = rem_image(i);
      if(image) {
	del_Image(image);
	nrem += 1;
      }
    }
  } while(nrem > 0);
  return;
}
/*.......................................................................
 * Return the index of the requested image.
 *
 * Returns 1-relative index on success, 0 on failure. 
 */
int imindex(Image *image)
{
  int i;
  /*
   * Extract the image.
   */
  for(i=0;i < images.n;i++) 
    if(images.f[i]==image)
      return i+1;
  return 0;
}
/*.......................................................................
 * Delete a single image descriptor.
 *
 * Input:
 *  image     Image *  The image descriptor to be deleted.
 * Output:
 *  return  Image *  Allways NULL. Use like  image=del_Image(image);
 */
Image *del_Image(Image *image)
{
  /*
   * Delete the name string.
   */
  if(image->name)
    free(image->name);
  /*
   * Delete the data.
   */
  if(image->re)
    free(image->re);
  if(image->im)
    free(image->im);
  /*
   * Delete the header
   */
  del_Fitshead(image->header);
  /*
   * Delete the container.
   */
  free(image);
  return (Image *) 0;
}
/*.......................................................................
 * Create a new image container.
 *
 * Input:
 *  name         char *  The name to identify the image by. If the
 *                       image named already exists, the existing
 *                       image is returned.
 * Output:
 *  return  Image *  The pointer to the new image, or
 *                       NULL on error.
 */
Image *new_Image(char *name, Fitshead *header)
{
  Image *image=NULL;/* Pointer to the new image descriptor. */
  int i;
  /*
   * Has this image already been created?
   */
  image = imfind(name);

  if(image)
    return image;
  /*
   * Allocate memory for the new image.
   */
  image = (Image *) malloc(sizeof(Image));

  if(image==NULL) {
    fprintf(stderr, "Insufficient memory for new image:\'%s\'\n",name);
    return image;
  };
  /*
   * Initialize image up to the point where it's safe to delete it.
   */
  image->header = header;
  image->re     = NULL;
  image->im     = NULL;
  image->name   = NULL;
  /* 
   * Allocate a copy of name 
   */
  image->name = (char *) malloc(strlen(name)+1);
  if(image->name==NULL) {
    fprintf(stderr, "Insufficient memory for image name:\'%s\'\n",name);
    return del_Image(image);
  };
  strcpy(image->name, name);
  /*
   * Allocate a data array.
   */
  if(header->type==T_RE) {

    // If this is a real image, just allocate the real part of the array

    image->re = (float *) malloc(header->naxes_actual[0]*
				 header->naxes_actual[1]*sizeof(float));
    if(image->re==NULL) {
      fprintf(stderr, "Insufficient memory for image data:\'%s\'\n",name);
      return del_Image(image);
    };

    for(i=0;i < header->naxes_actual[0]*header->naxes_actual[1];i++)
      image->re[i] = 0.0;

  } else if(header->type==T_CMPLX) {

    // If this is a complex image, allocate both parts

    image->re = (float *) malloc(header->naxes_actual[0]*
				 header->naxes_actual[1]*sizeof(float));
    image->im = (float *) malloc(header->naxes_actual[0]*
				 header->naxes_actual[1]*sizeof(float));

    if(image->re==NULL || image->im==NULL) {
      fprintf(stderr, "Insufficient memory for image data:\'%s\'\n",name);
      return del_Image(image);
    };

    // And zero the array

    for(i=0;i < header->naxes_actual[0]*header->naxes_actual[1];i++) {
      image->re[i] = 0.0;
      image->im[i] = 0.0;
    }
  } else {
    fprintf(stderr,"Unrecognized image type.\n");
    return del_Image(image);
  }
  
  return image;
}
/*.......................................................................
 * Initialize a FITS header.
 */
Fitshead *new_Fitshead(char* units)
{
  Fitshead *header=NULL;/* Pointer to the new header descriptor. */
  
  // Allocate memory for the new header.

  header = (Fitshead *) malloc(sizeof(Fitshead));
  if(header==NULL) {
    fprintf(stderr, "Insufficient memory for new header.\n");
    return header;
  };
  
  // And intialize the header.

  header->bitpix = 0;
  header->naxis =  0;
  header->naxes_data = NULL;
  header->data_start = NULL;
  header->naxes_actual = NULL;
  header->bscale = 1.0;

  header->bunit  = parseUnits(units);

  header->bzero  =  0.0;
  header->crvals = NULL;
  header->crpixs = NULL;
  header->cdelts = NULL;
  header->ctypes = NULL;

  header->datamin = 0.0;
  header->datamax = 0.0;

  header->telescope = NULL;
  header->instrument = NULL;

  header->n =  0;
  header->xmin = 0.0;
  header->xmax = 0.0;
  header->ymin = 0.0;
  header->ymax = 0.0;

  header->type = T_RE;

  return header;
}

/**.......................................................................
 * Return the unit enumerator associated with the passed string
 */
Bunit parseUnits(char* units)
{
  int i;

  if(units != NULL)
    for(i=0;i < nfitsunits;i++)
      if(strncmp(units, fitsunits[i].string, strlen(units))==0)
	return fitsunits[i].bunit;
  
  return BU_UNKNOWN;
}

/**.......................................................................
 * Return the unit enumerator associated with the passed string
 */
char* printUnits(Bunit unit)	       
{
  int i;

  for(i=0;i < nfitsunits;i++)
    if(unit == fitsunits[i].bunit)
      return fitsunits[i].string;
  
  return NULL;
}

/*.......................................................................
 * Copy a FITS header
 */
int copy_Fitshead(Fitshead *orig, Fitshead *dest)
{
  int i;
  /*
   * And intialize the header.
   */
  dest->bitpix = orig->bitpix;
  dest->naxis = orig->naxis;

  if(rd_str(&(dest->telescope), orig->telescope))
    return 1;

  if(rd_str(&(dest->instrument), orig->instrument))
    return 1;

  for(i=0;i < orig->naxis;i++) {
    dest->naxes_data[i] = orig->naxes_data[i];
    dest->data_start[i] = orig->data_start[i];
    dest->naxes_actual[i] = orig->naxes_actual[i];
    dest->crvals[i] = orig->crvals[i];
    dest->crpixs[i] = orig->crpixs[i];
    dest->cdelts[i] = orig->cdelts[i];
    dest->ctypes[i] = orig->ctypes[i];
  }

  dest->bscale = orig->bscale;
  dest->bunit = orig->bunit;
  dest->bzero = orig->bzero;

  dest->datamin = orig->datamin;
  dest->datamax = orig->datamax;

  dest->n = orig->n;
  dest->xmin = orig->xmin;
  dest->xmax = orig->xmax;
  dest->ymin = orig->ymin;
  dest->ymax = orig->ymax;

  dest->imin = orig->imin;
  dest->imax = orig->imax;
  dest->jmin = orig->jmin;
  dest->jmax = orig->jmax;

  dest->type = orig->type;

  return 0;
}
/*.......................................................................
 * Delete a FITS header.
 */
Fitshead *del_Fitshead(Fitshead *header)
{
  /*
   * Delete dynamically allocated arrays.
   */
  if(header->naxes_data)
    free(header->naxes_data);
  if(header->naxes_actual)
    free(header->naxes_actual);
  if(header->crvals)
    free(header->crvals);
  if(header->crpixs)
    free(header->crpixs);
  if(header->cdelts)
    free(header->cdelts);
  if(header->crotas)
    free(header->crotas);
  /*
   * Delete various strings
   */
  if(header->instrument)
    free(header->instrument);
  if(header->telescope)
    free(header->telescope);
  /*
   * Delete the container.
   */
  free(header);
  return (Fitshead *) 0;
}
/*.......................................................................
 * Return the delta of an image in radians.
 *
 * Output:
 *   delta  *  float  The factor to convert  pixels to radians
 */
int native_to_radian(Image *image, int iaxis, float *delta)
{
  float conv;

  if(iaxis > image->header->naxis-1) {
    fprintf(stderr,"native_to_radians: Requested invalid axis index: %d\n",
	    iaxis);
    return 1;
  }
  /*
   * Now get the delta and convert to radians.
   */
  switch (image->header->ctypes[iaxis]) {
    case AX_RAD:
      conv = 1;
      break;
    case AX_DEG:
      conv = M_PI/180;
      break;
    default:
      conv = 1;
      break;
    }

  // Conv is now in units of radian/native, multiply by native/pixel
  // to get radian/pixel

  *delta =  conv*image->header->cdelts[iaxis];

  return 0;
}
/*.......................................................................
 * Return the conversion between native image units and Janskys.
 *
 * Input:
 * 
 *  image  Image  *  The parent image.
 *  freq   float     The frequency, in GHz
 * 
 * Input/Output:
 * 
 *  conv   float  *  The conversion factor, in (Jy/native)
 *
 * Output:
 *
 *  0 -- ok
 *  1 -- error
 */
int native_to_jansky(Image *image, float freq, float *conv)
{
  float dx,dy;
  /*
   * Get the pixel deltas
   */
  if(native_to_radian(image, 0, &dx))
    return 1;
  if(native_to_radian(image, 1, &dy))
    return 1;
  /*
   * Now convert the units to janskys
   */
  switch (image->header->bunit) {
  case BU_JY:
    *conv = 1;
    break;
  case BU_MJYSR:
    *conv = 1e6*dx*dy;
    break;
  case BU_MUK:
    *conv = dplanck(freq, TCMB)*dx*dy*1e-6;
    break;
  case BU_K:
    *conv = dplanck(freq, TCMB)*dx*dy;
    break;
  case BU_Y:
    *conv = comptonYToDtemp(freq) * dplanck(freq, TCMB) * dx * dy * TCMB;
    fprintf(stdout, "Conversion from Y to Jy/sr at %g GHz is: %g\n", freq, *conv/(dx*dy));
    break;
  default:
    fprintf(stderr,"native_to_jansky: Received unrecognized unit\n");
    return 1;
    break;
  }
  return 0;
}
/*.......................................................................
 * Evaluate the derivative of the Planck function wrt to T at given T
 * and freq.
 *
 * Input:
 *
 *  freq  float  Frequency, in GHz
 *  T     float  Brightness temperature, in Kelvin
 *
 * Output:
 * 
 *  The conversion, in Jy/K.
 */
static float dplanck(float freq, float T)
{
  double prefac = 2*K*(freq*freq*1e18)/(C*C);
  double x = HPLANCK*freq*1e9/(K*T);
  double expx = exp(x);

  return prefac*x*x*expx/((expx-1)*(expx-1))*1e23;
}

static float dplanckRJ(float freq, float T)
{
  double prefac = 2*K*(freq*freq*1e18)/(C*C) * 1e23;

  return prefac;
}

/**.......................................................................
 * Get the Planck x-factor
 *
 * Input:
 *
 *  freq  double Frequency, in GHz
 *  T     double Brightness temperature, in Kelvin
 *
 * Output:
 * 
 *  The dimensionless x-factor for Planck function calcs.
 */
static double planckX(double freq, double T)
{
  return HPLANCK*freq*1e9/(K*T);
}

/**.......................................................................
 * Return the factor by which the compton Y factor should be
 * multiplied to convert to temperature decrement
 *
 * Input:
 *
 *  freq  double Frequency, in GHz
 *  T     double Brightness temperature, in Kelvin
 *
 * Output:
 *
 *  The factor by which compton Y should be multiplied to convert to
 *  CMB temperature decrement/increment:
 *
 *  can be derived from Peebles (24.48), p. 585 for dN/N, with chain
 *  rule applied to N = 1 / (ex - 1) to convert to dT/T.

 *  Should tend to:
 *    
 *         -2 at x << 1
 *        x*x at x >> 1
 */
static double comptonYToDtemp(double freq)
{
  double x        = planckX(freq, TCMB);
  double ex       = exp(x);

  return x * (ex+1) / (ex-1) - 4;
}


