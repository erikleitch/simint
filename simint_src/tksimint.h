#ifndef tksimint_h
#define tksimint_h

#ifndef image_h
#include "image.h"
#endif

#ifdef DMALLOC
#include "dmalloc.h"
#endif

#include <tk.h>
/*
 * Converts radians to degrees
 */
#define RTOD 57.2958
/*
 * Converts radians to arcsec
 */
#define RTOAS 206264.806247
/*
 * Converts radians to hours of ra
 */
#define RTOH 3.81972
/*
 * Converts radians to sec
 */
#define RTOS 13750.9870831
/*
 * Temperature of the Microwave Background (K)
 */
#define TCMB 2.726
/*
 * Number of seconds in a day.
 */
#define DAYSEC 86400
/*
 * Boltzmann constant.
 */
#define K 1.380622e-16
/*
 * Speed of light (cm/s)
 */
#define C 2.99792458e10
/*
 * Planck's constant (erg s)
 */
#define HPLANCK 6.62618e-27
/*
 * Radius of the earth (cm)
 */
#define RE 6.37103e8
/*
 * Atmospheric temp in K.
 */
#define TATM 300
/*
 * The sim supports several 2D functions that are displayed in
 * its image window. For each supported function-type there is a 
 * C function of the following declaration, that returns the
 * value of the function at a given x,y position.
 */
#define IMAGE_FN(fn) float (fn)(float x, float y)

#define Uvscale 1.0

/* Define a flag for antenna selection */

typedef enum {ANT_US, ANT_S} Antflag;

/* DEfine a flag for antenna status */

typedef enum {ANT_ADD, ANT_DEL, ANT_TOG} Antstat;

typedef struct AntType {
  double size;   /* Each antenna might have a different size */
  double apeff;  /* Each antenna might have a different aperture efficiency */
  struct AntType *next; /* A pointer to the next type of antenna present */
} AntType;
/*
 * Declare a type to encapsulate a single antenna. This includes its
 * physical position and a pointer to the type of antenna.
 */
typedef struct ant {
  float x; /* East position relative to the phase reference center. */
  float y; /* North position relative to the phase reference center. */
  double X; /* X-coordinate of the antenna (X (H=0, dec=0)) */
  double Y; /* Y-coordinate of the antenna (Y (H=-6h, dec=0)) */
  double Z; /* Z-coordinate of the antenna (Z (dec=90)) */
  AntType *type;
  Antflag flag;
  int iant; /* The sequence number of this antenna */
  struct ant *next;
} Ant;
/*
 * Define default xy axis limits (in lambda) 
 */
typedef struct {
  char *name;
  char *label;
  float xmin;
  float xmax;
  float ymin;
  float ymax;
  float size; /* Size of the dishes (cm) */
} Array;

/*
 * Declare a type to hold a single X,Y coordinate.
 */
typedef struct {
  double x, y;       /* World coordinates */
} Vertex;
/*
 * Define a container for all observation-related parameters.
 */
typedef struct {
  double lat;
  double lng;
  double srcdec;
  double hastart;
  double hastop;
  double wave;      /* Center wavelength */
  double bw;        /* Total bandwidth in GHz */
  int symm;         /* N-fold symmetry of antenna positions. */
  int nchan;        /* Number of correlator channels. */
  double antsize;   /* The current size of the antennas */
  double correff;   /* Correlator efficiency */
  double tint;      /* Integration time to use when breaking up tracks */
  double tscale;    /* A scale factor to apply to the integration time */
  double tsys0;     /* Fixed component of the Tsys */
  double tau;       /* Atmospheric opacity */
  int ngrid;        /* Multi-purpose axis size */
  double cellsize;  /* Multi-purpose axis size */
  int isouv;        /* True if writing the same UV coord. for each IF. */
  double xmax;      /* The max abs value of the XY axis of the array area */
} Observation;
/*
 * Declare the object type that is used to record the state of a
 * given sim instance command.
 */
typedef struct {
  Tcl_Interp *interp; /* The TCL interpreter of the sim */
  int image_id;       /* The PGPLOT id of the image widget */
  int xyplt_id;       /* The PGPLOT id of the xy plot widget */
  int uvplt_id;       /* The PGPLOT id of the uv plot widget */
  int slice_id;       /* The PGPLOT id of the slice widget */
  float *slice;       /* The slice compilation array */
  Observation obs;    /* The observation parameters. */
  Array *array;
  AntType *anttypes;  /* A list of all distinct antenna types present
                         in the current array */
  Ant *ants;
  Image *image;       /* An image (read in or generated) */
  float *vis;         /* The array of visibilities. */
  float scale;        /* Coversion factor pixels -> coords */
  int image_size;     /* The number of pixels along each side of the image */
  int slice_size;     /* The length of the slice array */
  float xa,xb;          /* Min and max X pixel coordinates */
  float ya,yb;          /* Min and max Y pixel coordinates */
  float xpa,xpb;          /* Min and max xyplt X pixel coordinates */
  float ypa,ypb;          /* Min and max xyplt Y pixel coordinates */
  float upa,upb;          /* Min and max uvplt U pixel coordinates */
  float vpa,vpb;          /* Min and max uvplt V pixel coordinates */
  float xpasave,xpbsave;/* Min and max xyplt X pixel coordinates */
  float ypasave,ypbsave;/* Min and max xyplt Y pixel coordinates */
  float upasave,upbsave;/* Min and max uvplt U pixel coordinates */
  float vpasave,vpbsave;/* Min and max uvplt V pixel coordinates */
  float datamin;      /* The minimum data value in image[] */
  float datamax;      /* The maximum data value in image[] */
  IMAGE_FN(*fn);      /* The function to be displayed */
  Vertex va;          /* The start of the latest slice line */
  Vertex vb;          /* The end of the latest slice line */
  int have_slice;     /* This true when va and vb contain valid slice limits */
  int monochrome;     /* True if the image colormap only contains two colors */
  int nant;
  float antsize;
  int interactive;
  /*
   * Parameters relating to image generation.
   */
  struct {
    int ngrid;      /* The number of pixels in the requested image size. */
    float size;     /* The size of the generated field, in radians */
    float cellsize; /* The resulting pixel delta, in radians */
  } imgen;
  /*
   * Parameters relating to pt src image generation
   */
  struct {
    int ngrid;
    float size;
  } ptsrc;
} Simint;
/*
 * Define a structure to contain a Tcl variable name and value.
 */
typedef struct {
  char *name;
  double val;
} TclVar;
/*
 * Enumerate the types of data output.
 */
typedef enum {
  T_BIN,
  T_DAT,
  T_UVF,
} Dtype;
/*
 * Enumerate data operators for plotim()
 */
typedef enum {
  OP_RE,
  OP_IM,
  OP_ABSRE,
  OP_ABSIM,
  OP_POW
} Optype;

int write_vis(FILE *fp, Simint *sim, Dtype type, Image *image);
int write_sepvis(FILE *fp, Simint *sim, Dtype type, Image *image);
double getsysdate(void);
int getobs(Simint *sim);
int fill_imheader(Fitshead *header, Simint *sim);
int fill_uvheader(Fitshead *header);
int copy_header(Fitshead *dest, Fitshead *src);
int plotim(Simint *sim, Image *image, Optype op);

#endif
