#ifndef imgen_h
#define imgen_h
/*
 * Enumerate the types of power spectra.
 */
typedef enum {
  S_GAUSS,
  S_POW,
  S_FILE,
} Spectype;

float *imgen(Simint *sim, int ngrid, double cellsize, Spectype type, double powind, char *fstring);
Fitshead *imgen_get_header(Simint *sim);

#endif
