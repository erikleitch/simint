#ifndef fitsio_h
#define fitsio_h

#include <stdio.h>

#ifndef tksimint_h
#include "tksimint.h"
#endif

#ifndef image_h
#include "image.h"
#endif

/*
 * Length of a data record
 */
#define NDATREC 2880
/*
 * Length of an ASCII header
 */
#define NHEAD 80
/*
 * Position of the logical value 'T' or 'F' for logical keywords.
 */
#define NLOG 30
/*
 * Length of the header keyword.
 */
#define NKEY 8
/*
 * Declare a header structure to encapsulate relevant FITS information to be
 * passed to writevis
 */
typedef struct {
  double obsra;
  double obsdec;
  double freq;
  double dfreq;
  int nchan; /* Number of IFs. */
  int nvis;
  char *date;
} Header;
/*
 * Declare a header struct for binary data format files.
 */
typedef struct {
  int samples; /* Nbaseline * nintegration */
  float freq;  /* Baseline freq in GHz */
  float diam1; /* 1st dish diameter (in cm) */
  float diam2; /* 2nd dish diameter (in cm) */
  float diam3; /* 3rd dish diameter (in cm) */
} Binhead;
/*
 * Declare a type to store a single visibility datum.
 */
typedef struct {
  double u;
  double v;
  double re;
  double im;
  double wt;
  short code;
} Datum;

Header *new_Header(Simint *sim);
Header *del_Header(Header *hdr);

int writevis(FILE *fp, Header *hdr, Simint *sim);
int new_writevis(FILE *fp, Header *hdr, Simint *sim);

int writedat(FILE *fp, Header *hdr, Simint *sim, Dtype type);

int write_phdu(FILE *fp, Simint *sim, Header *hdr);
int write_antable(FILE *fp, Simint *sim);
int write_fqtable(FILE *fp, Simint *sim);

int put_phdu(FILE *fp, char *name, char *val, char *comment);
int null_str(char *buf, char *val);
int log_str(char *buf, char *val);
int int_str(char *buf, char *val);
int flt_str(char *buf, char *val);
int str_str(char *buf, char *val);
int rd_null(void *ptr, char *string);
int rd_log(void *ptr, char *string);
int rd_str(void *ptr, char *string);
int rd_int(void *ptr, char *string);
int rd_flt(void *ptr, char *string);
int rd_bunit(void *ptr, char *string);
int rd_axis(void *ptr, char *string);

char *decstring(double dec, char *string);
char *rastring(double fra, char *string);

int new_Axes(Fitshead *header, int naxis);
int getheader(FILE *fp, Fitshead *header, Simint *sim);
int readfits(FILE *fp, Image *image, int realcoord);

void cp_4r4(unsigned char *dest, unsigned char *orig, size_t nitem);
void cp_8r8(unsigned char *dest, unsigned char *orig, size_t nitem);

#endif
