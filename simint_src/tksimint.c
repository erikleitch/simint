#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <tcl.h>
#include <tk.h>

#include "tksimint.h"

#include "tkpgplot.h"
#include "cpgplot.h"

#include "color_tab.h"
#include "vplot.h"
#include "fourier.h"
#include "fitsio.h"
#include "image.h"
#include "vis.h"
#include "imgen.h"

#include "fftw.h"
#include "fftw-int.h"
#include "rfftw.h"
#include "slalib.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif
/* 
 * Un-comment the following to print debugging information:
 */ 
#define ATI

/* #define TEST */
/* Set the default image size */

enum {IMAGE_SIZE=129};

/* Set the number of points plotted per slice */

enum {SLICE_SIZE=100};

/*
 * List the prototypes of the available 2D-function functions.
 */
static IMAGE_FN(sinc_fn);
static IMAGE_FN(gaus_fn);
static IMAGE_FN(ring_fn);
static IMAGE_FN(sin_angle_fn);
static IMAGE_FN(cos_radius_fn);
static IMAGE_FN(star_fn);
/*
 * List the association between image function name and the functions
 * that evaluate them.
 */
static struct {
  char *name;               /* The TCL name for the function */
  IMAGE_FN(*fn);            /* The C function that evaluates the function */
} image_functions[] = {
  {"cos(R)sin(A)",             ring_fn},
  {"sinc(R)",                  sinc_fn},
  {"exp(-R^2/20.0)",           gaus_fn},
  {"sin(A)",                   sin_angle_fn},
  {"cos(R)",                   cos_radius_fn},
  {"(1+sin(6A))exp(-R^2/100)", star_fn}
};

static Simint *new_Simint(Tcl_Interp *interp, char *caller, char *cmd,
			  char *image_device, char *slice_device);
static Simint *del_Simint(Simint *sim);
static void Simint_DeleteProc(ClientData data);
static int simint_instance_command(ClientData data, Tcl_Interp *interp,
				   int argc, char *argv[]);
static int simint_antarray_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[]);
static int simint_antpos_command(Simint *sim, Tcl_Interp *interp, int argc,
				 char *argv[]);
static int simint_addant_command(Simint *sim, Tcl_Interp *interp, int argc,
				 char *argv[]);
static int simint_markant_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[]);
static int simint_markvis_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[]);
static int simint_compvis_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[]);
static int simint_radplot_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[]);
static int simint_compbeam_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[]);
static int simint_compwin_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[]);
static int simint_recolor_image_command(Simint *sim, Tcl_Interp *interp,
					int argc, char *argv[]);
static int simint_writeant_command(Simint *sim, Tcl_Interp *interp,
				   int argc, char *argv[]);
static int simint_zapant_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[]);
static int simint_drawframes_command(Simint *sim, Tcl_Interp *interp,
				     int argc, char *argv[]);
static int simint_writevis_command(Simint *sim, Tcl_Interp *interp,
				   int argc, char *argv[]);
static int simint_newwritevis_command(Simint *sim, Tcl_Interp *interp,
				      int argc, char *argv[]);
static int simint_writedat_command(Simint *sim, Tcl_Interp *interp,
				   int argc, char *argv[]);
static int simint_uvzoom_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[]);
static int simint_xyzoom_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[]);
static int simint_powspec_command(Simint *sim, Tcl_Interp *interp,
				  int argc, char *argv[]);
static int simint_drawframes_command(Simint *sim, Tcl_Interp *interp,
				     int argc, char *argv[]);
static int simint_xyframe_command(Simint *sim, Tcl_Interp *interp,
				  int argc, char *argv[]);
static int simint_uvframe_command(Simint *sim, Tcl_Interp *interp,
				  int argc, char *argv[]);
static int simint_rfits_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[]);
static int simint_imslct_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[]);
static int simint_obsim_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[]);
static int simint_redraw_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[]);
static int simint_ptsrc_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[]);
static void Simint_DeleteProc(ClientData data);
static int create_simint(ClientData data, Tcl_Interp *interp, int argc,
			 char *argv[]);

static int valid_sim_script(char *name);
static int Demo_AppInit(Tcl_Interp *interp);

static int addant(Simint *sim, double x, double y, double antsize, double wave, double obslat, double obslong, int symm, int add);
static int compvis(Simint *sim, double wave, double lat, double lng, double hastart, double hastop, double dec);
static int radplot(Simint *sim, int nbin);
static int markvis(Simint *sim, float x, float y, float antsize, double wave, double obslat, double obslong, double hastart, double hastop, double dec, int symm, Antstat stat);
static int plotbeam(Simint *sim, double wave, double lat, double lng, double hastart, double hastop, double dec, double cellsize, double uvrad);
static int plotwin(Simint *sim, double antsize,double wave, double lat, double lng, double hastart, double hastop, double dec);
static int draw_frames(Simint *sim);
static int draw_xyframe(Simint *sim);
static int draw_uvframe(Simint *sim);
static int draw_ants(Simint *sim, Array *array, float antsize, float wave, double obslat, double obslong);

static int add_Ant(Simint *sim, double x, double y, double obslat, double obslong, double altitude, double antsize, double apeff);
Ant *rem_Ant(Simint *sim, Ant *node);
Ant *new_Ant(double x, double y, double obslat, double obslong, double altitude);
Ant *del_Ant(Ant *node);
static int zap_ant(Simint *sim);
static int zap_AntTypes(Simint *sim);
double decfloat(char *dec);
double rafloat(char *ra);
void getXYZ(Ant *ant, double obslat, double obslong, double altitude);
static int markant(Simint *sim, float x, float y, float antsize, int symm);
static int newfft(float *u, float *v, float *vis, int nvis, double cellsize);
static int getpar(Tcl_Interp *interp);
int tcl_fltval(Tcl_Interp *interp, char *name, double *val);
int tcl_intval(Tcl_Interp *interp, char *name, int *val);
int tcl_raval(Tcl_Interp *interp, char *name, double *val);
int tcl_decval(Tcl_Interp *interp, char *name, double *val);
static int draw_xyframe(Simint *sim);
static int drawants(Simint *sim);
double cluster_model(double k);
int writeant(Simint *sim, FILE *fp);
float *checkop(Image *image, Optype op);
static int obsim(Simint *sim, Image *image);
static int imvis(Image *image, Ant *ant1, Ant *ant2, Datum *dat);
static AntType *add_AntType(Simint *sim, double size, double apeff); 
int MAIN__(void);
/*
 * Declare external functions here (numerical recipes)
 */
extern void fourn(float data[], unsigned long nn[], int ndim, int isign);
extern float gasdev(long *idum);
/*
 * And global variables defined elsewhere.
 */
extern Axiscard fitsaxes[];
extern Bunitcard fitsunits[];
extern int nfitsaxes;
extern int nfitsunits;
/*
 * Define a macro that returns the number of elements in a static array.
 */
#ifdef COUNT
#undef COUNT
#endif
#define COUNT(lev) sizeof(lev)/sizeof(lev[0])
/*
 * Define the bounds in cm, so these will need to be divided by the wavelength
 * to convert to lambda.
 */
Array arrays[] = {
  {"user",        "User",          -500,   500,   -500,   500,   25},
  {"dasi",        "DASI",          -100,   100,   -100,   100,   25},
  {"jca",         "JCA",          -5000,  5000,  -5000,  5000,  250},
  {"sza",         "SZA",          -6000,  6000,  -6000,  6000,  350},
  {"ati_compact", "ATI Compact",  -5000,  5000,  -5000,  5000,  350},
  {"ati_hetero",  "ATI Hetero",   -5000,  5000,  -5000,  5000,  350},
  {"a",           "VLA A Array", -2.4e6, 2.4e6, -2.4e6, 2.4e6, 2500},
  {"b",           "VLA B Array", -6.7e5, 6.7e5, -6.7e5, 6.7e5, 2500},
  {"c",           "VLA C Array", -2.4e5, 2.4e5, -2.4e5, 2.4e5, 2500},
  {"d",           "VLA D Array", -6.7e4, 6.7e4, -6.7e4, 6.7e4, 2500},
};
int narray = sizeof(arrays)/sizeof(arrays[0]);
/*
 * Define a structure to encapsulate all Tcl Variables of interest.
 */
TclVar Wt[] = {
  {"apeff",    0.7},
  {"bw",       1.0},
  {"correff",  0.88},
  {"tint",     50},
  {"tsys",     20},
  {"cellsize", 20}, /* Cellsize, in radians */
};

int nwt = sizeof(Wt)/sizeof(Wt[0]);

/*
 * Define a grid size.
 */
int Grid[]={256,256};
Gridparms Gridwdth = {1.645, 1.645, 2, 2, 1};

/*
 * Define the default DASI antenna positions.
 */
float Dasix[] = {
  57.4491, 
  32.3477, 
  00.8355, 
  -11.5934, 
  23.4172, 
  -44.2504, 
  -62.8229, 
  -33.1831, 
  -45.8557, 
  -9.0637, 
  39.4057, 
  53.3142, 
  00.0000
};

float Dasiy[] = {
  19.7813,
  19.6407,
  -37.8342,
  -59.6430,
  -59.0217,
  -36.0139,
  9.2309,
  18.1936,
  39.8617,
  56.3289,
  49.7907,
  -20.3150,
  00.0000
};

/*
 * Define the default SZA antenna positions.
 */
float Szax[] = {
   496.0,
   161.0,
     0.0,
  -567.0,
    50.0,
  -581.0,
 -2121.0,
  3879.0
};

float Szay[] = {
  -20.0,
  533.0, 
    0.0, 
  206.0, 
 -598.0, 
 -247.0, 
 5595.0, 
 3095.0 
};

/*.......................................................................
 * After presenting a warning if the first argument is not the name
 * of the sim Tcl script, main() simply calls the standard Tk_Main()
 * to initialize Tcl/Tk and the sim package.
 * Input:
 *  argc     int    The number of command line arguments.
 *  argv    char*[] The array of command-line argument strings.
 * Output:
 *  return   int    0 - OK.
 *                  1 - Error.
 */
int main(int argc, char *argv[])
{
  char *usage = "Usage: tksimint simint.tcl [tk-options].\n";
  /*
   * Check whether the first argument names a valid simint
   * script file.
   */
  /*  if(argc < 2 || *argv[1] == '-' || !valid_sim_script(argv[1])) { */
  if(argc < 2 || *argv[1] == '-') { 
    fprintf(stderr, usage);
    return 1;
  };
  /*
   * Start the application.
   */
  Tk_Main(argc, argv, Demo_AppInit);
  
  return 0;
}

/*.......................................................................
 * This dummy fortran main allows simint to be linked with the
 * f2c-compiled pgplot library.
 */
int MAIN__(void)
{
  return 0;
}
/*.......................................................................
 * This is the application initialization file that is called by Tk_Main().
 */
static int Demo_AppInit(Tcl_Interp *interp)
{
#ifdef DEBUG_STARTUP
  fprintf(stdout,"Inside Demo_AppInit\n");
#endif
  /*
   * Create the standard Tcl and Tk packages, plus the TkPgplot package.
   */
  if(Tcl_Init(interp)    == TCL_ERROR ||
     Tk_Init(interp)     == TCL_ERROR ||
     Tkpgplot_Init(interp) == TCL_ERROR)
    return 1;
  
#ifdef DEBUG_STARTUP
  fprintf(stdout,"Just before Tcl_CreateCommand\n");
#endif
  
  /*
   * Create the TCL command used to initialize the sim.
   */
  Tcl_CreateCommand(interp, "create_simint", (Tcl_CmdProc* )create_simint,
		    (ClientData) Tk_MainWindow(interp), 0);
#ifdef DEBUG_STARTUP
  fprintf(stdout,"Just after Tcl_CreateCommand\n");
#endif
  
  return 0;
}

/*.......................................................................
 * This function provides the TCL command that creates a simint
 * manipulation command. This opens the two given PGPLOT widgets to
 * PGPLOT, establishes a cursor handler and records the state of the
 * sim in a dynamically allocated container.
 *
 * Input:
 *  data      ClientData    The main window cast to ClientData.
 *  interp    Tcl_Interp *  The TCL intrepreter of the sim.
 *  argc             int    The number of command arguments.
 *  argv            char ** The array of 'argc' command arguments.
 *                          argv[0] = "create_simint"
 *                          argv[1] = The name to give the new command.
 *                          argv[2] = The name of the image widget.
 *                          argv[3] = The name of the slice widget.
 * Output:
 *  return           int    TCL_OK    - Success.
 *                          TCL_ERROR - Failure.
 */
static int create_simint(ClientData data, Tcl_Interp *interp, int argc,
			 char *argv[])
{
  Simint *sim;      /* The new widget instance object */
  /*
   * Check that the right number of arguments was provided.
   */
  if(argc != 4) {
    Tcl_AppendResult(interp,
		     argv[0], ": Wrong number of arguments - should be \'",
		     argv[0], " new_command_name image_widget slice_widget\'", NULL);
    return TCL_ERROR;
  };
  /*
   * Allocate a context object for the command.
   */
  sim = new_Simint(interp, argv[0], argv[1], argv[2], argv[3]);
  if(!sim)
    return TCL_ERROR;
  return TCL_OK;
}

/*.......................................................................
 * Create a new PGPLOT sim instance command and its associated context
 * object.
 *
 * Input:
 *  interp   Tcl_Interp *  The TCL interpreter object.
 *  caller         char *  The name of the calling TCL command.
 *  cmd            char *  The name to give the new sim-instance command.
 *  image_device   char *  The PGPLOT device specification to use to open
 *                         the image-display device.
 *  slice_device   char *  The PGPLOT device specification to use to open
 *                         the slice-display device.
 * Output:
 *  return       Simint *  The new sim object, or NULL on error.
 *                         If NULL is returned then the context of the
 *                         error will have been recorded in the result
 *                         field of the interpreter.
 */
static Simint *new_Simint(Tcl_Interp *interp, char *caller, char *cmd,
			  char *xyplt_device, char *uvplt_device)
{
  Simint *sim;        /* The new widget object */
  /*
   * Allocate the container.
   */
  sim = (Simint *) malloc(sizeof(Simint));
  if(!sim) {
    Tcl_AppendResult(interp, "Insufficient memory to create ", cmd, NULL);
    return NULL;
  };
  /*
   * Before attempting any operation that might fail, initialize the container
   * at least up to the point at which it can safely be passed to
   * del_Simint().
   */
  sim->interp = interp;
  sim->image_id = -1;
  sim->slice_id = -1;
  sim->xyplt_id = -1;
  sim->uvplt_id = -1;
  sim->ants = NULL;
  sim->array=NULL;
  sim->anttypes = NULL;
  sim->vis=NULL;
  sim->image_size = IMAGE_SIZE;
  sim->slice_size = SLICE_SIZE;
  sim->scale = 40.0f/sim->image_size;
  sim->xpa = 0.0;
  sim->xpb = 0.0;
  sim->ypa = 0.0;
  sim->ypb = 0.0;
  sim->upa = 0.0;
  sim->upb = 0.0;
  sim->vpa = 0.0;
  sim->vpb = 0.0;
  sim->fn = sin_angle_fn;
  sim->have_slice = 0;
  sim->monochrome = 0;
  sim->nant=0;
  sim->antsize=0.0;
  
  // Attempt to open the image and uvplt widgets: the special argument
  // "none" will signify that this application was called in
  // non-interactive mode
  
  sim->interactive = !(strcmp(xyplt_device, "none")==0 || 
		       strcmp(uvplt_device, "none")==0);
  
  if(sim->interactive)
    if((sim->xyplt_id = cpgopen(xyplt_device)) <= 0 ||
       (sim->uvplt_id = cpgopen(uvplt_device)) <= 0) {
      Tcl_AppendResult(interp, "Unable to open widgets: ", xyplt_device, ", ",
		       uvplt_device, NULL);
      return del_Simint(sim);
    };
  /*
   * Create the instance command.
   */
  Tcl_CreateCommand(interp, cmd, (Tcl_CmdProc*)simint_instance_command,
		    (ClientData)sim, Simint_DeleteProc);
  /*
   * Return the command name.
   */
  Tcl_AppendResult(interp, cmd, NULL);
  return sim;
}

/*.......................................................................
 * Delete the context of a Simint instance command.
 *
 * Input:
 *  sim     Simint *   The widget to be deleted.
 * Output:
 *  return  Simint *   Always NULL.
 */
static Simint *del_Simint(Simint *sim)
{
  if(sim) {
    sim->interp = NULL;
    /*
     * Close the PGPLOT widgets.
     
     if(sim->xyplt_id > 0) {
     cpgslct(sim->xyplt_id);
     cpgclos();
     sim->xyplt_id = -1;
     };
     if(sim->uvplt_id > 0) {
     cpgslct(sim->uvplt_id);
     cpgclos();
     sim->uvplt_id = -1;
     };
     if(sim->image_id > 0) {
     cpgslct(sim->image_id);
     cpgclos();
     sim->image_id = -1;
     };
     if(sim->slice_id > 0) {
     cpgslct(sim->slice_id);
     cpgclos();
     sim->slice_id = -1;
     };
    */
    /*
     * Delete the antenna array and associated antenna types.
     */
    zap_ant(sim);
    /*
     * And delete the container.
     */
    free(sim);
  };
  return NULL;
}

/*.......................................................................
 * This is a wrapper around del_Simint() suitable to be registered as
 * a DeleteProc callback for Tcl_CreateCommand().
 *
 * Input:
 *  data  ClientData   The (Simint *) object cast to ClientData.
 */
static void Simint_DeleteProc(ClientData data)
{
  (void) del_Simint((Simint *) data);
}

/*.......................................................................
 * This function implements a given Tcl PGPLOT sim instance command.
 *
 * Input:
 *  data      ClientData    The sim context object cast to (ClientData).
 *  interp    Tcl_Interp *  The TCL intrepreter.
 *  argc             int    The number of command arguments.
 *  argv            char ** The array of 'argc' command arguments.
 *                          argv[0] - the name of the sim command.
 *                          argv[1..] - One of:
 *                           save device_spec
 *                           function image_function
 *                           slice x1 y1 x2 y2
 * Output:
 *  return           int    TCL_OK    - Success.
 *                          TCL_ERROR - Failure.
 */
static int simint_instance_command(ClientData data, Tcl_Interp *interp,
				   int argc, char *argv[])
{
  Simint *sim = (Simint *) data;
  char *command;    /* The name of the command */
  /*
   * We must have at least one command argument.
   */
  if(argc < 2) {
    Tcl_SetResult(interp, "Wrong number of arguments.", TCL_STATIC);
    return TCL_ERROR;
  };
  /*
   * Get the command-name argument.
   */
  command = argv[1];
  if(strcmp(command, "antarray") == 0)
    return simint_antarray_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "antpos") == 0)
    return simint_antpos_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "addant") == 0)
    return simint_addant_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "markant") == 0)
    return simint_markant_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "markvis") == 0)
    return simint_markvis_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "compvis") == 0)
    return simint_compvis_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "radplot") == 0)
    return simint_radplot_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "compbeam") == 0)
    return simint_compbeam_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "compwin") == 0)
    return simint_compwin_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "recolor_image") == 0)
    return simint_recolor_image_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "zapant") == 0)
    return simint_zapant_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "drawframes") == 0)
    return simint_drawframes_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "xyframe") == 0)
    return simint_xyframe_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "uvframe") == 0)
    return simint_uvframe_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "writeant") == 0)
    return simint_writeant_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "writevis") == 0)
    return simint_writevis_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "newwritevis") == 0)
    return simint_newwritevis_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "writedat") == 0)
    return simint_writedat_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "uvzoom") == 0)
    return simint_uvzoom_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "xyzoom") == 0)
    return simint_xyzoom_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "powspec") == 0)
    return simint_powspec_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "rfits") == 0)
    return simint_rfits_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "imslct") == 0)
    return simint_imslct_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "obsim") == 0)
    return simint_obsim_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "redraw") == 0)
    return simint_redraw_command(sim, interp, argc - 2, argv + 2);
  else if(strcmp(command, "ptsrc") == 0)
    return simint_ptsrc_command(sim, interp, argc - 2, argv + 2);
  /*
   * Unknown command name.
   */
  Tcl_AppendResult(interp, argv[0], ": Unknown sim command \"",
		   argv[1], "\"", NULL);
  return TCL_ERROR;
}

/*.......................................................................
 * Implement the sim "antpos" command. This draws the frames around the
 * X-Y and U-V plot windows, draws any associated antennas and the 
 * corresponding * U-V visibilities.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A function designation chosen from:
 *                                   "cos(R)sin(A)"
 *                                   "sinc(R)"
 *                                   "exp(-R^2/20.0)"
 *                                   "sin(A)"
 *                                   "cos(A)"
 *                                   "(1+sin(6A))exp(-R^2/100)"
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_antpos_command(Simint *sim, Tcl_Interp *interp, int argc,
				 char *argv[])
{
  double obslat,obslong,altitude;
  int waserr=0;
  Ant *ant=NULL;
  /*
   * There should be three arguments, specifying the array, antenna size and
   * wavelength.
   */
  if(argc != 3) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the update_antpos command.\n",
		     "Should be: obslat obslong altitude", NULL);
    
    return TCL_ERROR;
  }
  /*
   * Read the two double values.
   */
  obslat = decfloat(argv[0]);
  obslong = decfloat(argv[1]);
  if(Tcl_GetDouble(interp, argv[2], &altitude) == TCL_ERROR)
    return TCL_ERROR;
  
  
  for(ant = sim->ants;ant != NULL;ant = ant->next)
    getXYZ(ant,obslat,obslong,altitude);
  
  if(waserr)
    return TCL_ERROR;
  return TCL_OK;
}
/*.......................................................................
 * Implement the sim "antarray" command. This draws the frames around the
 * X-Y and U-V plot windows, draws any associated antennas and the 
 * corresponding * U-V visibilities.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A function designation chosen from:
 *                                   "cos(R)sin(A)"
 *                                   "sinc(R)"
 *                                   "exp(-R^2/20.0)"
 *                                   "sin(A)"
 *                                   "cos(A)"
 *                                   "(1+sin(6A))exp(-R^2/100)"
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_antarray_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[])
{
  int i;
  double antsize,wave,lat,lng,hastart,hastop,dec=0.0,obslat,obslong;
  int waserr=0;
  Array *antarr=NULL;
  /*
   * There should be three arguments, specifying the array, antenna size and
   * wavelength.
   */
  if(argc != 8) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the draw_antarray command.\n",
		     "Should be: array antsize wavelength obslat obslong hastart hastop dec", NULL);
    
    return TCL_ERROR;
  }
  /*
   * Read the two double values.
   */
  if(Tcl_GetDouble(interp, argv[1], &antsize) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[2], &wave) == TCL_ERROR)
    
    return TCL_ERROR;
  /*
   * Convert the other strings into radians.
   */
  obslat = decfloat(argv[3]);
  obslong = decfloat(argv[4]);
  hastart = rafloat(argv[5]);
  hastop = rafloat(argv[6]);
  
  for(i=0;i < narray;i++)
    if(strcmp(argv[0],arrays[i].name)==0) {
      antarr = &arrays[i];
      break;
    }
  
  if(i==narray) {
    fprintf(stderr,"Unrecognized array: %s.\n",argv[0]);
    return TCL_ERROR;
  }
  
  if(strcmp(argv[7],"Zenith")!=0)
    dec = decfloat(argv[7]);
  else {
    hastart = 0;
    hastop = 0;
  }
  
  for(i=0;i < narray;i++)
    if(strcmp(argv[0],arrays[i].name)==0) {
      antarr = &arrays[i];
      sim->array = antarr;
      break;
    }
  
  if(i==narray) {
    fprintf(stderr,"Unrecognized array: %s.\n",argv[0]);
    return TCL_ERROR;
  }
  /*
   * First, zap the existing antenna array.
   */
#ifdef TEST
  fprintf(stdout,"Before zap_ant()\n");
#endif
  if(zap_ant(sim)) 
    return TCL_ERROR;
#ifdef TEST
  fprintf(stdout,"Before draw_frames()\n");
#endif
  
  waserr = draw_frames(sim);
#ifdef TEST
  fprintf(stdout,"Before draw_ants()\n");
#endif
  
  waserr |= draw_ants(sim,antarr,(float) antsize,(float) wave, obslat, obslong);
#ifdef TEST
  fprintf(stdout,"Before compvis()\n");
#endif
  
  waserr |= compvis(sim,wave,lat,lng,hastart,hastop,dec);
  
  if(waserr)
    return TCL_ERROR;
  return TCL_OK;
}
/*.......................................................................
 * Draw the default antennas for the chosen array.
 */
static int draw_ants(Simint *sim, Array *array, float antsize, float wave, double obslat, double obslong)
{
  int nant, i, waserr = 0;
  /*
   * Re-initialize the antenna arrays to NULL
   */
  zap_ant(sim);
  sim->nant=0;
  /*
   * And parse the new array
   */
  if(strcmp(array->name,"user")==0) {
    return 0;
  }
  else if(strcmp(array->name,"dasi")==0) {
    nant = sizeof(Dasix)/sizeof(float);
    for(i=0;i < nant;i++) {
      waserr |= addant(sim, Dasix[i], Dasiy[i], antsize, wave, obslat, obslong, 1, 1);
#ifdef DEBUG
      {
	char tmp[10];
	sprintf(tmp,"%d",i);
	cpgtext(Dasix[i],Dasiy[i],tmp);
      }
#endif
    }
    return waserr;
  }
  else if(strcmp(array->name,"sza")==0) {
    nant = sizeof(Szax)/sizeof(float);

    for(i=0;i < nant;i++) 
      waserr |= addant(sim, Szax[i], Szay[i], antsize, wave, obslat, obslong, 1, 1);
    return waserr;
  }
  /*
   * If this is the JCA, add in jca dishes, OVRO and BIMA dishes.
   */
  else if(strcmp(array->name,"jca")==0) {
    double r,eps=1,d1,d2,x,y;
    waserr |= addant(sim, 0,0,antsize,wave, obslat, obslong, 1, 1);
    for(i=0;i < 2;i++) {
      r = antsize+eps;
      waserr |= addant(sim, r*sin(i*60.0/180*M_PI),r*cos(i*60.0/180*M_PI),antsize,wave, obslat, obslong, 3, 1);
    }
    /*
     * add in the two outliers
     */
    waserr |= addant(sim, 5000*sin(60.0/180*M_PI),5000*cos(60.0/180*M_PI),antsize,wave, obslat, obslong, 1, 1);
    waserr |= addant(sim, 5000*sin((60.0+180.0)/180*M_PI),5000*cos((60.0+180.0)/180*M_PI),antsize,wave, obslat, obslong, 1, 1);
    /*
     * Now add in 6 BIMA dishes.
     */
    d1 = antsize;
    d2 = antsize/2+600.0/2;
    
    r = sqrt(3)/2*d1 + d2*cos(asin(d1/(2.0*d2))) + eps;
    
    x = (r-eps)*sin(30.0/180*M_PI);
    y = (r-eps)*cos(30.0/180*M_PI);
    
    antsize = 600;
    for(i=0;i < 2;i++) {
      waserr |= addant(sim, r*sin((30+i*60.0)/180*M_PI),r*cos((30+i*60.0)/180*M_PI),antsize,wave, obslat, obslong, 3, 1);
    }
    /*
     * Now add in the 6 OVRO dishes.
     */
    d1 = r;
    d2 = antsize/2 + 1040/2;
    r = sqrt(3)/2*d1 + d2*cos(asin(d1/(2.0*d2))) + eps;
    
    antsize = 1040;
    for(i=0;i < 2;i++) {
      waserr |= addant(sim, r*sin(i*60.0/180*M_PI),r*cos(i*60.0/180*M_PI),antsize,wave, obslat, obslong, 3, 1);
    }
    /*
     * And add in the last three BIMA dishes.
     */
    d1 = r;
    d2 = antsize/2 + 600/2;
    r = sqrt(3)/2*d1 + d2*cos(asin(d1/(2.0*d2))) + eps;
    
    antsize = 600;
    waserr |= addant(sim, r*sin((30+i*60.0)/180*M_PI),r*cos((30+i*60.0)/180*M_PI),antsize,wave, obslat, obslong, 3, 1);
    
    return waserr;
  }
  /*
   * If this is the ATI, add in ati dishes, OVRO and BIMA dishes.
   */
  else if(strcmp(array->name,"ati_hetero")==0 || strcmp(array->name,"ati_compact")==0) {
    double r,eps=1,alpha;
    r = 2.0/sqrt(3.0)*antsize/2+eps;
    waserr |= addant(sim, r*sin(60.0/180*M_PI),r*cos(60.0/180*M_PI),antsize,wave, obslat, obslong, 3, 1);
    /*
     * Now add in the next 3 jca dishes
     */
    r = r/2 + antsize*(sqrt(3.0)/2) + eps;
    waserr |= addant(sim, 0,r,antsize,wave, obslat, obslong, 3, 1);
    if(strcmp(array->name,"ati_hetero")==0) {
      /*
       * Now add in 3 OVRO dishes
       */
      r = 2.0/sqrt(3.0)*antsize/2+ + antsize/2 + 1040.0/2 + 2*eps;
      antsize = 1040;
      waserr |= addant(sim, r*sin(60.0/180*M_PI),r*cos(60.0/180*M_PI),antsize,wave, obslat, obslong, 3, 1);
      /*
       * Add in the last 3 OVRO dishes
       */
      alpha = asin(r*sqrt(3)/2/antsize);
      r  = r/2 + antsize*cos(alpha) + eps;
      waserr |= addant(sim, 0,r,antsize,wave, obslat, obslong, 3, 1);
    }
    return waserr;
  }
  else {
    double r,norm;
    /*
     * VLA -- largest distance from the array center is 21 km, ie 
     * N*9^1.7 = 21 km => N = 50119.5; each array scales by 2^1.7 = 3.24901 
     * from the previous (cf. TMS, p. 133-134), so minimum spacing in D conf.
     * is 50119.5/(2^1.7)^3 = 1461.35 cm.
     */
    if(strcmp(array->name,"a")==0)
      norm = 1461.35*3.24901*3.24901*3.24901;
    else if(strcmp(array->name,"b")==0) 
      norm = 1461.35*3.24901*3.24901;
    else if(strcmp(array->name,"c")==0) 
      norm = 1461.35*3.24901;
    else if (strcmp(array->name,"d")==0)
      norm = 1461.35;
    
    for(i=0;i < 9;i++) {
      r = norm*pow((double) i+1,1.7);
      waserr |= addant(sim, r*sin(5.0/180*M_PI),r*cos(5.0/180*M_PI),antsize,wave, obslat, obslong, 1, 1);
      waserr |= addant(sim, r*sin(125.0/180*M_PI),r*cos(125.0/180*M_PI),antsize,wave, obslat, obslong, 1, 1);
      waserr |= addant(sim, r*sin(-115.0/180*M_PI),r*cos(-115.0/180*M_PI),antsize,wave, obslat, obslong, 1, 1);
    }
    return 0;
  }
}
/*.......................................................................
 * Draw the appropriate frame around the xy and uv plots.
 */
static int draw_frames(Simint *sim)
{
  float urange;
  double wave0,nu0,numin,dnu;
  char *label=NULL;
  int waserr = 0;
  
  waserr = getobs(sim);
  if(!waserr) {
    
    if(sim->interactive) {
      // Use the image space defined by the array argument.
      
      sim->xpa = sim->xpasave = sim->array->xmin;
      sim->xpb = sim->xpbsave = sim->array->xmax;
      sim->ypa = sim->ypasave = sim->array->ymin;
      sim->ypb = sim->ypbsave = sim->array->ymax;
      /*
       * Select the xyplt device.
       */
      cpgslct(sim->xyplt_id);
      cpgsci(1);
      cpgask(0);
      cpgpage();
      cpgsch(1.0f);
      cpgsvp(0,1,0,1);
      cpgswin(0,1,0,1);
      cpgarro(0.01,0.01,0.01,0.1);
      cpgtext(0.11,0.005,"E");
      cpgarro(0.01,0.01,0.1,0.01);
      cpgtext(0.005,0.11,"N");
      cpgvstd();
      cpgwnad(0,1,0,1);
      cpgswin(sim->xpa,sim->xpb,sim->ypa,sim->ypb);
      cpgbox("BCNST",0,0,"BCNST",0,0);
      
      if((label = (char *)malloc(sizeof(char)*(strlen("Antenna locations")+strlen(sim->array->label)+4)))==NULL) {
	fprintf(stderr,"Unable to allocate plot label.\n");
	return 1;
      }
      strcpy(label,"Antenna locations (");
      strcat(label,sim->array->label);
      strcat(label,")\0");
      cpglab("X (cm)","Y (cm)",label);
      if(label)
	free(label);
      /*
       * Now draw the frame around the uv-plot.
       */
      cpgslct(sim->uvplt_id);
      cpgsci(1);
      cpgask(0);
      cpgpage();
      cpgvstd();
      cpgwnad(0,1,0,1);
      /* 
       * Get the maximum U-V size needed for the smallest wavelength.
       */
      wave0 = sim->obs.wave;
      dnu = sim->obs.bw/sim->obs.nchan;
      nu0 = C/1e9/wave0;
      numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
      wave0 = C/1e9/numin;
      
      urange = sqrt(2.0)*(sim->xpb-sim->xpa)/wave0;
      cpgswin(-urange,urange,-urange,urange);
      sim->upa = sim->upasave = -urange;  
      sim->vpa = sim->vpasave = -urange; 
      sim->upb = sim->upbsave = urange;
      sim->vpb = sim->vpbsave = urange;
      cpgbox("BCNST",0,0,"BCNST",0,0);
      cpglab("U (\\gl)","V (\\gl)","U-V plane");
    }
  }
  return waserr;
}
/*.......................................................................
 * Draw the appropriate frame around the uv plot alone.
 */
static int draw_uvframe(Simint *sim)
{
  double wave0,nu0,numin,dnu;
  int waserr = 0;
  
  waserr = getobs(sim);
  if(!waserr) {

    if(sim->interactive) {
      /*
       * Now draw the frame around the uv-plot.
       */
      cpgslct(sim->uvplt_id);
      cpgsci(1);
      cpgask(0);
      cpgpage();
      cpgvstd();
      cpgwnad(0,1,0,1);
      /* 
       * Get the maximum U-V size needed for the smallest wavelength.
       */
      wave0 = sim->obs.wave;
      dnu = sim->obs.bw/sim->obs.nchan;
      nu0 = C/1e9/wave0;
      numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
      wave0 = C/1e9/numin;
      
      cpgswin(sim->upa,sim->upb,sim->vpa,sim->vpb);
      cpgbox("BCNST",0,0,"BCNST",0,0);
      cpglab("U (\\gl)","V (\\gl)","U-V plane");
    }
  }
  return waserr;
}
/*.......................................................................
 * Elemental function to redraw the antenna array.
 */
static int drawants(Simint *sim)
{  
  int waserr=0;
  Ant *ant=NULL;
  
  if(sim->ants==NULL) {
    fprintf(stderr,"No antennas to draw.\n");
    return 0;
  }
  /*
   * Select the xyplt plot
   */
  if(sim->interactive) {
    cpgslct(sim->xyplt_id);
  
    for(ant = sim->ants;ant != NULL;ant = ant->next) {
      /*
       * Draw a point marking the center of the dish, and a circle indicating 
       * its size (in lambda).
       */
      cpgsci(ant->flag==ANT_US?1:2);
      cpgpt1(ant->x,ant->y,1);
      cpgsfs(2);
      cpgcirc(ant->x,ant->y,ant->type->size/2);
      cpgsci(1);
    }
  }
  return waserr;
}
/*.......................................................................
 * Draw the frame around the xy plot alone
 */
static int draw_xyframe(Simint *sim)
{
  char *label=NULL;
  /*
   * Select the xyplt device.
   */
  if(sim->interactive) {
    cpgslct(sim->xyplt_id);
    cpgsci(1);
    cpgask(0);
    cpgpage();
    cpgsch(1.0f);
    cpgsvp(0,1,0,1);
    cpgswin(0,1,0,1);
    cpgarro(0.01,0.01,0.01,0.1);
    cpgtext(0.11,0.005,"E");
    cpgarro(0.01,0.01,0.1,0.01);
    cpgtext(0.005,0.11,"N");
    cpgvstd();
    cpgwnad(0,1,0,1);
    cpgswin(sim->xpa,sim->xpb,sim->ypa,sim->ypb);
    cpgwnad(sim->xpa,sim->xpb,sim->ypa,sim->ypb);
    cpgbox("BCNST",0,0,"BCNST",0,0);
    
    if((label = (char *)malloc(sizeof(char)*(strlen("Antenna locations")+strlen(sim->array->label)+4)))==NULL) {
      fprintf(stderr,"Unable to allocate plot label.\n");
      return 1;
    }
    strcpy(label,"Antenna locations (");
    strcat(label,sim->array->label);
    strcat(label,")\0");
    cpglab("X (cm)","Y (cm)",label);
    if(label)
      free(label);
  }
  return 0;
}
/*.......................................................................
 * Implement the sim "addant" command. This takes a cursor location and
 * draws and antenna at the specified location.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A function designation chosen from:
 *                                   "cos(R)sin(A)"
 *                                   "sinc(R)"
 *                                   "exp(-R^2/20.0)"
 *                                   "sin(A)"
 *                                   "cos(A)"
 *                                   "(1+sin(6A))exp(-R^2/100)"
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_addant_command(Simint *sim, Tcl_Interp *interp, int argc,
				 char *argv[])
{
  double x, y;
  double antsize,wave;
  double obslat, obslong;
  int add,symm;
  /*
   * There should be 11 arguments.
   */
  if(argc != 8) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the addant command.\n",
		     "Should be: x y antsize wave obslat obslong symm add", NULL);
    return TCL_ERROR;
  };
  /*
   * Read the parameter values.
   */
  if(Tcl_GetDouble(interp, argv[0], &x) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[1], &y) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[2], &antsize) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[3], &wave) == TCL_ERROR ||
     Tcl_GetInt(interp, argv[6], &symm) == TCL_ERROR ||
     Tcl_GetInt(interp, argv[7], &add) == TCL_ERROR)
    return TCL_ERROR;
  /*
   * Add an antenna at the requested coordinate.
   */
  obslat = decfloat(argv[4]);
  obslong = decfloat(argv[5]);
  /*
   * Add or remove (add==0) the requested antenna
   */
  if(addant(sim, x,  y,  antsize,  wave, obslat, 
	    obslong, symm, add))
    return TCL_ERROR;
  
  return TCL_OK;
}
/*.......................................................................
 * Implement the sim "markvis" command. This takes a cursor location and
 * draws visibilities associated with it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A function designation chosen from:
 *                                   "cos(R)sin(A)"
 *                                   "sinc(R)"
 *                                   "exp(-R^2/20.0)"
 *                                   "sin(A)"
 *                                   "cos(A)"
 *                                   "(1+sin(6A))exp(-R^2/100)"
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_markvis_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[])
{
  double x, y;
  double antsize,wave;
  double obslat, obslong, hastart, hastop, srcdec;
  int add,symm;
  /*
   * There should be 10 arguments.
   */
  if(argc != 11) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the markvis command.\n",
		     "Should be: x y antsize wave obslat obslong hastart hastop srcdec symm add", NULL);
    return TCL_ERROR;
  };
  /*
   * Read the two coordinate values.
   */
  if(Tcl_GetDouble(interp, argv[0], &x) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[1], &y) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[2], &antsize) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[3], &wave) == TCL_ERROR ||
     Tcl_GetInt(interp, argv[9], &symm) == TCL_ERROR ||
     Tcl_GetInt(interp, argv[10], &add) == TCL_ERROR)
    return TCL_ERROR;
  /*
   * Add an antenna at the requested coordinate.
   */
  obslat = decfloat(argv[4]);
  obslong = decfloat(argv[5]);
  hastart = rafloat(argv[6]);
  hastop = rafloat(argv[7]);
  srcdec = decfloat(argv[8]);
  /*
   * And update the visibilities associated with this antenna.
   */
  if(markvis(sim, (float) x, (float) y, (float) antsize, wave, obslat, 
	     obslong, hastart, hastop, srcdec, symm, add?ANT_ADD:ANT_DEL))
    return TCL_ERROR;
  
  return TCL_OK;
}
/*.......................................................................
 * Implement the sim "marlvis" command. This takes a cursor location and
 * draws and antenna at the specified location.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A function designation chosen from:
 *                                   "cos(R)sin(A)"
 *                                   "sinc(R)"
 *                                   "exp(-R^2/20.0)"
 *                                   "sin(A)"
 *                                   "cos(A)"
 *                                   "(1+sin(6A))exp(-R^2/100)"
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_markant_command(Simint *sim, Tcl_Interp *interp, int argc,
				  char *argv[])
{
  double x, y;
  double antsize,obslat,obslong,hastart,hastop,srcdec,wave;
  int symm,disp;
  /*
   * There should be two arguments.
   */
  if(argc != 11) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the markant command.\n",
		     "Should be: x y antsize wave obslat obslong hastart hastop srcdec symm", NULL);
    return TCL_ERROR;
  };
  /*
   * Read the two coordinate values.
   */
  if(Tcl_GetDouble(interp, argv[0], &x) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[1], &y) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[2], &antsize) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[3], &wave) == TCL_ERROR ||
     Tcl_GetInt(interp, argv[9], &symm) == TCL_ERROR ||
     Tcl_GetInt(interp, argv[10], &disp) == TCL_ERROR)
    return TCL_ERROR;
  /*
   * Get the rest of the numbers in radians
   */
  obslat = decfloat(argv[4]);
  obslong = decfloat(argv[5]);
  hastart = rafloat(argv[6]);
  hastop = rafloat(argv[7]);
  srcdec = decfloat(argv[8]);
  /*
   * Toggle the antenna nearest to the requested coordinate.
   */
  if(markant(sim, (float) x, (float) y, (float) antsize, symm))
    return TCL_ERROR;
  /*
   * And mark the visibilities, but only if we are not displaying something else/
   */
  if(disp) 
    if(markvis(sim, (float) x, (float) y, (float) antsize, wave, obslat, 
	       obslong, hastart, hastop, srcdec, symm, ANT_TOG))
      return TCL_ERROR;
  
  return TCL_OK;
}
/*.......................................................................
 * Add an antenna to the array of antenna positions.
 */
static int addant(Simint *sim, double xa, double ya, double antsize, double wave, double obslat, double obslong, int symm, int add)
{  
  float rad,r;
  int waserr=0,i;
  Ant *ant=NULL,*near=NULL;
  double x,y;
  double theta;
  int nant;
  double altitude, apeff;
  
  waserr |= tcl_fltval(sim->interp,"apeff",&apeff);
  waserr |= tcl_fltval(sim->interp,"obsalt",&altitude);
  
  /*
   * See if 3-fold symmetry was requested.
   */
  nant = symm;
  theta = 2*M_PI/nant;
  /*
   * If requested to add an antenna, see if (any of) the requested position(s) 
   * is too close to another antenna.
   */
  if(add) {
    for(i=0;i < nant;i++) {
      x = xa*cos(i*theta) + ya*sin(i*theta);
      y = ya*cos(i*theta) - xa*sin(i*theta);
      for(ant = sim->ants;ant != NULL;ant = ant->next) {
	if(sqrt((x-ant->x)*(x-ant->x) + (y-ant->y)*(y-ant->y)) < (antsize+ant->type->size)/2) {
	  fprintf(stderr,"Antennas are too close.\n");
	  return 1;
	}
      }
      /*
       * If the requested position is too close from the origin for the given symmetry, 
       * place a single dish at the origin.
       */
      if(symm > 1)
	if(sqrt(x*x + y*y) < antsize/2/sin(M_PI/nant)) {
	  xa = 0.0;
	  ya = 0.0;
	  nant = 1;
	}
    }
  }
  /*
   * Now add or delete the requested antenna.
   */
  for(i=0;i < nant;i++) {
    
    x = xa*cos(i*theta) + ya*sin(i*theta);
    y = ya*cos(i*theta) - xa*sin(i*theta);
    
    if(!add) {
      /*
       * Use the exact n-fold symmetric location of the previous antenna
       * for the next n-1 deletions.
       */
      if(i != 0) {
	x = near->x*cos(theta) + near->y*sin(theta);
	y = near->y*cos(theta) - near->x*sin(theta);
      }
      /*
       * If deleting, find the antenna nearest to the selected point.
       */
      if(sim->nant==0) {
	fprintf(stderr,"No more antennas to delete.\n");
	return 0;
      };
      
      rad = sqrt((x-sim->ants->x)*(x-sim->ants->x) + 
		 (y-sim->ants->y)*(y-sim->ants->y));
      near = sim->ants;
      
      for(ant = sim->ants;ant != NULL;ant = ant->next) 
	if((r=sqrt((x-ant->x)*(x-ant->x) + 
		   (y-ant->y)*(y-ant->y))) < rad) {
	  rad = r;
	  near = ant;
	}
      x = near->x;
      y = near->y;
      /*
       * If symmetry was requested, only delete n-1 more dishes if
       * they are close "enough" to the exact n-fold symmetric locations.
       */
      if(i > 0 && rad > antsize/20)
	return 0;
    }
    
    // Select the xyplt plot
    
    if(sim->interactive) {
      cpgslct(sim->xyplt_id);
      /*
       * Draw a point marking the center of the dish, and a circle indicating 
       * its size (in lambda).
       */
      cpgsci(add?1:0);
      cpgpt1(x,y,1);
      cpgsfs(2);
      cpgcirc(x,y,add?antsize/2:near->type->size/2);
      cpgsci(1);
    }
    /*
     * Increment/decrement the antenna array.
     */
    if(add) 
      waserr|=add_Ant(sim,x,y,obslat,obslong,altitude,antsize,apeff);
    else 
      waserr|=(rem_Ant(sim,near)!=NULL);
  }
  return waserr;
}
/*.......................................................................
 * Implement the sim "writevis" command. This takes a file name and
 * outputs the current visibilities to it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_writevis_command(Simint *sim, Tcl_Interp *interp,
				   int argc, char *argv[])
{
  char *name;   /* The name of the output file. */
  int waserr=0;
  FILE *fp=NULL;
  Header *hdr=NULL;
  /*
   * There should only be two argumnts.
   */
  if(argc != 1) {
    Tcl_AppendResult(interp, "Usage: filename.\n", NULL);
    return TCL_ERROR;
  };
  /*
   * Get the file name.
   */
  name = argv[0];
  if((fp = fopen(name,"w"))==NULL) {
    fprintf(stderr,"Unable to open file %s.\n",name);
    Tcl_AppendResult(interp, "Unable to open file.\n", NULL);
    return TCL_ERROR;
  }
  
  waserr |= (hdr = new_Header(sim))==NULL;
  
  if(!waserr)
    writevis(fp, hdr, sim);
  
  del_Header(hdr);
  
  if(fp)
    fclose(fp);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "newwritevis" command. This takes a file name and
 * outputs the current visibilities to it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_newwritevis_command(Simint *sim, Tcl_Interp *interp,
				      int argc, char *argv[])
{
  char *name;   /* The name of the output file. */
  int waserr=0;
  FILE *fp=NULL;
  Header *hdr=NULL;
#ifdef DMALLOC
  dmalloc_verify(0);
#endif
  /*
   * There should only be two argumnts.
   */
  if(argc != 1) {
    Tcl_AppendResult(interp, "Usage: filename.\n", NULL);
    return TCL_ERROR;
  };
  /*
   * Get the file name.
   */
  name = argv[0];
  /*
   * Open the new data file for reading and writing. 
   */
  
  if((fp=fopen(name,"w"))==NULL) {
    fprintf(stderr,"Unable to open file %s.\n",name);
    Tcl_AppendResult(interp, "Unable to open file.\n", NULL);
    return TCL_ERROR;
  }
  
  waserr |= (hdr = new_Header(sim))==NULL;
  
  if(!waserr)
    new_writevis(fp, hdr, sim);
  
  del_Header(hdr);
  
  if(fp != NULL)
    fclose(fp);
#ifdef DMALLOC
  dmalloc_verify(0);
#endif
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "writeant" command. This takes a file name and
 * outputs the current visibilities to it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_writeant_command(Simint *sim, Tcl_Interp *interp,
				   int argc, char *argv[])
{
  char *name;   /* The name of the output file. */
  int waserr=0;
  FILE *fp=NULL;
  /*
   * There should only be a single argument.
   */
  if(argc != 1) {
    Tcl_AppendResult(interp, "Missing file name.\n", NULL);
    return TCL_ERROR;
  };
  /*
   * Get the file name.
   */
  name = argv[0];
  if((fp = fopen(name,"w"))==NULL) {
    fprintf(stderr,"Unable to open file %s.\n",name);
    Tcl_AppendResult(interp, "Unable to open file.\n", NULL);
    return TCL_ERROR;
  }
  
  writeant(sim,fp);
  
  if(fp)
    fclose(fp);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "zapant" command. This deletes the current antenna
 * array.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_zapant_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[])
{
  int waserr=0;
  
  waserr |= zap_ant(sim);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "xyframe" command. This deletes the current antenna
 * array.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_xyframe_command(Simint *sim, Tcl_Interp *interp,
				  int argc, char *argv[])
{
  int waserr=0;
  
  waserr |= draw_xyframe(sim);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "uvframe" command. This deletes the current antenna
 * array.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_uvframe_command(Simint *sim, Tcl_Interp *interp,
				  int argc, char *argv[])
{
  int waserr=0;
  
  waserr |= draw_uvframe(sim);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "drawframes" command. This deletes the current antenna
 * array.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_drawframes_command(Simint *sim, Tcl_Interp *interp,
				     int argc, char *argv[])
{
  int waserr=0;
  double wave;
  
  if(argc != 1) {
    Tcl_AppendResult(interp, "Missing wavelength.\n", NULL);
    return TCL_ERROR;
  };
  
  if(Tcl_GetDouble(interp, argv[0], &wave) == TCL_ERROR)
    return TCL_ERROR;
  
  waserr |= draw_frames(sim);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "uvzoom" command. This zooms the uv plot
 * outputs the current visibilities to it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_uvzoom_command(Simint *sim, Tcl_Interp *interp,
				 int argc, char *argv[])
{
  int waserr=0;
  double x1,x2,y1,y2;
  double wave,lat,lng,hastart,hastop,dec;
  
  waserr = tcl_fltval(sim->interp,"wave",&wave);
  waserr |= tcl_raval(sim->interp,"hastart",&hastart);
  waserr |= tcl_raval(sim->interp,"hastop",&hastop);
  waserr |= tcl_raval(sim->interp,"obslong",&lng);
  waserr |= tcl_decval(sim->interp,"srcdec",&dec);
  waserr |= tcl_decval(sim->interp,"obslat",&lat);
  
  /*
   * There should only be 4 arguments.
   */
  if(argc != 4) {
    Tcl_AppendResult(interp, "Missing file name.\n", NULL);
    return TCL_ERROR;
  };
  if(Tcl_GetDouble(interp, argv[0], &x1) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[1], &y1) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[2], &x2) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[3], &y2) == TCL_ERROR)
    return TCL_ERROR;
  
  if(x1==x2 && y1==y2) {
    sim->upa = 0.0;
    sim->vpa = 0.0;
    sim->upb = 0.0;
    sim->vpb = 0.0;
  }
  else {
    if(x1 < x2) {
      sim->upa = x1;
      sim->upb = x2;
    }
    else {
      sim->upa = x2;
      sim->upb = x1;
    }
    if(y1 < y2) {
      sim->vpa = y1;
      sim->vpb = y2;
    }
    else {
      sim->vpa = y2;
      sim->vpb = y1;
    }
  }
  if(compvis(sim,wave,lat,lng,hastart,hastop,dec))
    return TCL_ERROR;
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "xyzoom" command. This zooms the xy plot
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_xyzoom_command(Simint *sim, Tcl_Interp *interp,
					int argc, char *argv[])
{
  int waserr=0;
  double x1,x2,y1,y2;
  double wave,lat,lng,hastart,hastop,dec;

  waserr = tcl_fltval(sim->interp,"wave",&wave);
  waserr |= tcl_raval(sim->interp,"hastart",&hastart);
  waserr |= tcl_raval(sim->interp,"hastop",&hastop);
  waserr |= tcl_raval(sim->interp,"obslong",&lng);
  waserr |= tcl_decval(sim->interp,"srcdec",&dec);
  waserr |= tcl_decval(sim->interp,"obslat",&lat);
/*
 * There should only be 4 arguments.
 */
  if(argc != 4) {
    Tcl_AppendResult(interp, "Missing file name.\n", NULL);
    return TCL_ERROR;
  };
  if(Tcl_GetDouble(interp, argv[0], &x1) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[1], &y1) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[2], &x2) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[3], &y2) == TCL_ERROR)
    return TCL_ERROR;

  if(x1==x2 && y1==y2) {
#ifdef DEBUG_PG
    fprintf(stdout,"GOt same limits");
#endif
    sim->xpa = sim->xpasave;
    sim->ypa = sim->ypasave;
    sim->xpb = sim->xpbsave;
    sim->ypb = sim->ypbsave;
  }
  else {
    if(x1 < x2) {
      sim->xpa = x1;
      sim->xpb = x2;
    }
    else {
      sim->xpa = x2;
      sim->xpb = x1;
    }
    if(y1 < y2) {
      sim->ypa = y1;
      sim->ypb = y2;
    }
    else {
      sim->ypa = y2;
      sim->ypb = y1;
    }
  }

  if(draw_xyframe(sim))
    return TCL_ERROR;
  if(drawants(sim))
    return TCL_ERROR;

  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "powspec" command. This enters a power spectral index
 * and plots a random field generated according to it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_powspec_command(Simint *sim, Tcl_Interp *interp,
					int argc, char *argv[])
{
  int waserr=0;
  double ind;
  char *fstring=NULL;
  Spectype type;
  Image *image=NULL;
  Fitshead *header=NULL;
  char *imname=NULL;
  int ngrid,ndata;
  int zeropad=1;
  int istart=0,jstart=0;
  float *trans=NULL;
  double cellsize;
/*
 * There should be 5 arguments.
 */
  if(argc != 5) {
    Tcl_AppendResult(interp, "Usage: powspec type imagename size npix (option)\n", NULL);
    return TCL_ERROR;
  };
  switch (argc) {
  case 5:
    if(strcmp(argv[0],"pow")==0) {
      type = S_POW;
      ind = atof(argv[4]);
    }
    else if(strcmp(argv[0],"gauss")==0) {
      type = S_GAUSS;
      ind = atof(argv[4]);
    }
    else if(strcmp(argv[0],"file")==0) {
      type = S_FILE;
      fstring = argv[4];
    }
    else {
      fprintf(stderr,"Unrecognized power spectrum type: %s.\n",argv[0]);
      return TCL_ERROR;
    }
  case 4:
    sim->imgen.ngrid = atoi(argv[3]);
  case 3:
    sim->imgen.size = atof(argv[2]);
  case 2:
    imname = argv[1];
    break;
  }
  /*
   * And extract any other variables we might need from the Tcl interface.
   */
  if(getobs(sim))
    return TCL_ERROR;

  ngrid = sim->imgen.ngrid;  
  if(ngrid <= 0) {
    fprintf(stderr,"Received bad image size: %d pixels.\n",ngrid);
    return TCL_ERROR;
  }
  /*
   * Ensure that the dimension is a power of 2.  If not, use the nearest power
   * of 2.  
   *
   * Also, if zeropadding, we will generate an image which will be
   * twice as large as the requested image size.
   */
  ndata = (int)pow(2.0f,floor((log((double)ngrid)/log(2.0)+0.5)));
  fprintf(stdout,"Using %d pixels\n",ndata);
  ngrid=ndata;

  if(zeropad) {
    ngrid = 2*ndata;
    istart = ngrid/4;
    jstart = ngrid/4;
  }
  /*
   * The cellsize will be the requested image size divided by the
   * requested image pixels.
   */
  sim->imgen.cellsize = sim->imgen.size/ndata; /* Leave this one in degrees */
  cellsize = sim->imgen.size/ndata*M_PI/180;    /* This will be in radians */
  /*
   * Get the power spectrum image.
   */
  waserr |= (trans=imgen(sim, ngrid, cellsize, type, ind, fstring))==NULL;
  /*
   * Look for the requested image in the list. If it doesn't yet exist create a
   * new image.
   */
  if(!waserr) {
    image = imfind(imname);
    if(image != NULL) {
      image = rem_image(imindex(image));
      image = del_Image(image);
    }
    if((header=imgen_get_header(sim))==NULL)
      waserr = 1;
  }
  /*
   * Now allocate a new image container.
   */
  if(!waserr)
    waserr=(image=new_Image(imname, header))==NULL;
  /*
   * Copy out the central quadrant, undoing the effect of the shift as we go along
   */
  if(!waserr) {
    int i,j,imind,ftind;
    for(i=0;i < ndata;i++)
      for(j=0;j < ndata;j++) {
	imind = i + j*ndata;
	ftind = i+istart + (j+jstart)*(ngrid+2);
	image->re[imind] = trans[ftind]*(j%2==0 ? 1 : -1);
      }
  }
  /* 
   * On success, add the image to the list of known images.
   */
  if(!waserr)
    add_image(image);
  else
    del_Image(image);
  /*
   * And install the image as the default.
   */
  if(!waserr)
    sim->image = image;
  /*
   * Finally, plot the image.
   */
  if(!waserr) 
    waserr |= plotim(sim, image, OP_RE);
  /*
   * And delete the array returned from imgen()
   */
  if(trans);
    free(trans);

    return waserr ? TCL_ERROR : TCL_OK;
  }
/*.......................................................................
 * Implement the sim "writedat" command. This takes a file name and
 * outputs the current visibilities to it.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_writedat_command(Simint *sim, Tcl_Interp *interp,
					int argc, char *argv[])
{
  char *name=NULL,*type=NULL;   /* The name of the output file. */
  int waserr=0;
  FILE *fp=NULL;
  Header *hdr=NULL;
/*
 * There should only be a single argument.
 */
  if(argc != 2) {
    Tcl_AppendResult(interp, "Missing file name.\n", NULL);
    return TCL_ERROR;
  };
/*
 * Get the file name.
 */
  name = argv[0];
  type = argv[1];
  if((fp = fopen(name,"w"))==NULL) {
    fprintf(stderr,"Unable to open file %s.\n",name);
    Tcl_AppendResult(interp, "Unable to open file.\n", NULL);
    return TCL_ERROR;
  }

  waserr |= (hdr = new_Header(sim))==NULL;

  if(!waserr) {
    if(strcmp(type,"dat")==0)
      writedat(fp, hdr, sim, T_DAT);
    else if(strcmp(type,"bin")==0)
      writedat(fp, hdr, sim, T_BIN);
    else
      fprintf(stderr,"Unrecognized output type: %s.\n",type);
  }
  del_Header(hdr);
  
  if(fp)
    fclose(fp);

  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the sim "recolor_image" command. This takes one of a set of
 * supported color-table names and redisplays the current image with the
 * specified color table.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *                        argv[0] - A color table name chosen from:
 *                                   "aips"    -  AIPS tvfiddle color table.
 *                                   "blue"    -  A blue color table.
 *                                   "green"   -  A green color table.
 *                                   "grey"    -  A grey-scale color table.
 *                                   "heat"    -  The IRAF "heat" color table.
 *                                   "rainbow" -  A red color table.
 *                                   "red"     -  A red color table.
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_recolor_image_command(Simint *sim, Tcl_Interp *interp,
					int argc, char *argv[])
{
  char *name;   /* The name of the desired color table */
  int i;

  // There should only be a single argument.

  if(argc != 1) {
    Tcl_AppendResult(interp, "Missing color-table name.\n", NULL);
    return TCL_ERROR;
  };

  // Get the color-table name.

  name = argv[0];

  // Look up the name in our list of supported color tables.

  for(i=0; i<n_std_cmap; i++) {
    Cmap *cmap = std_cmaps + i;
    /*
     * If the color table is found, install it and return.
     */
    if(strcmp(cmap->name, name) == 0) {
      if(sim->interactive) {
	cpgslct(sim->image_id);
	cpgctab(cmap->l, cmap->r, cmap->g, cmap->b, cmap->n, 1.0, 0.5);
      }
      return TCL_OK;
    };
  };
  Tcl_AppendResult(interp, "Unknown color map name \"", name, "\"", NULL);
  return TCL_ERROR;
}

/*.......................................................................
 * A sinc(radius) function.
 *
 * Input:
 *  x,y     float   The coordinates to evaluate the function at.
 * Output:
 *  return  float   The function value at the specified coordinates.
 */
static IMAGE_FN(sinc_fn)
{
  const float tiny = 1.0e-6f;
  float radius = sqrt(x*x + y*y);
  return (fabs(radius) < tiny) ? 1.0f : sin(radius)/radius;
}

/*.......................................................................
 * A exp(-(x^2+y^2)/20) function.
 *
 * Input:
 *  x,y     float   The coordinates to evaluate the function at.
 * Output:
 *  return  float   The function value at the specified coordinates.
 */
static IMAGE_FN(gaus_fn)
{
  return exp(-((x*x)+(y*y))/20.0f);
}

/*.......................................................................
 * A cos(radius)*sin(angle) function.
 *
 * Input:
 *  x,y     float   The coordinates to evaluate the function at.
 * Output:
 *  return  float   The function value at the specified coordinates.
 */
static IMAGE_FN(ring_fn)
{
  return cos(sqrt(x*x + y*y)) * sin(x==0.0f && y==0.0f ? 0.0f : atan2(x,y));
}

/*.......................................................................
 * A sin(angle) function.
 *
 * Input:
 *  x,y     float   The coordinates to evaluate the function at.
 * Output:
 *  return  float   The function value at the specified coordinates.
 */
static IMAGE_FN(sin_angle_fn)
{
  return sin(x==0.0f && y==0.0f ? 0.0f : atan2(x,y));
}

/*.......................................................................
 * A cos(radius) function.
 *
 * Input:
 *  x,y     float   The coordinates to evaluate the function at.
 * Output:
 *  return  float   The function value at the specified coordinates.
 */
static IMAGE_FN(cos_radius_fn)
{
  return cos(sqrt(x*x + y*y));
}

/*.......................................................................
 * A (1+sin(6*angle))*exp(-radius^2 / 100)function.
 *
 * Input:
 *  x,y     float   The coordinates to evaluate the function at.
 * Output:
 *  return  float   The function value at the specified coordinates.
 */
static IMAGE_FN(star_fn)
{
  return (1.0 + sin(x==0.0f && y==0.0f ? 0.0f : 6.0*atan2(x,y)))
    * exp(-((x*x)+(y*y))/100.0f);
}

/*.......................................................................
 * Check that the specified command-line argument names a simint
 * script file. A simint script file is defined as being a readable
 * text file that contains the string "#!simint.tcl" at its start.
 *
 * Input:
 *  name    char *   The command-line argument to be checked.
 * Output:
 *  return   int     0 - Not valid.
 *                   1 - Valid.
 */
static int valid_sim_script(char *name)
{
#define REQUIRED_HEADER "#!tksimint"
  char header[sizeof(REQUIRED_HEADER)];
/*
 * Attempt to open the file for reading.
 */
  FILE *fp = fopen(name, "r");
  if(!fp) {
    fprintf(stderr, "Unable to open file: %s\n", name);
    return 0;
  };
/*
 * Read the first line and compare it to the required header.
 */
  if(fgets(header, sizeof(header), fp) == NULL ||
     strcmp(header, REQUIRED_HEADER)!=0 || getc(fp) != '\n') {
    fprintf(stderr, "File '%s' is not a simint Tcl script.\n", name);
    fclose(fp);
    return 0;
  };
  fclose(fp);
  return 1;
}
/*.......................................................................
 * Implement the "compvis" command. This takes a list of user-specified antenna
 * locations and draws the corresponding visibilities in the UVplot window.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_compvis_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[])
{
  double lat,lng,hastart,hastop,dec,wave;

  if(argc != 6) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the compvis command.\n",
		     "Should be: array antsize wavelength lat long hastart hastop dec", NULL);
    
    return TCL_ERROR;
  }
  /*
   * Convert the strings into radians.
   */
  if(Tcl_GetDouble(interp, argv[0], &wave) == TCL_ERROR)
    return TCL_ERROR;
  lat = decfloat(argv[1]);
  lng = decfloat(argv[2]);
  hastart = rafloat(argv[3]);
  hastop = rafloat(argv[4]);
  if(strcmp(argv[5],"Zenith")!=0)
    dec = decfloat(argv[5]);
  else {
    hastart = 0;
    hastop = 0;
  }
  /*
   * Now get additional Tcl variables we will need.
   */
  if(getpar(interp))
    return TCL_ERROR;

  if(compvis(sim,wave,lat,lng,hastart,hastop,dec))
    return TCL_ERROR;

  return TCL_OK;
}
/*.......................................................................
 * Implement the "radplot" command. This takes a list of user-specified antenna
 * locations and draws the corresponding visibilities in the UVplot window.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_radplot_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[])
{
  int nbin;

  if(argc != 1) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the radplot command.\n",
		     "Should be: nbin", NULL);
    
    return TCL_ERROR;
  }

  /*
   * Convert the strings into radians.
   */
  if(Tcl_GetInt(interp, argv[0], &nbin) == TCL_ERROR)
    return TCL_ERROR;

  if(radplot(sim, nbin))
    return TCL_ERROR;

  return TCL_OK;
}
/*.......................................................................
 * Implement the "compbeam" command. This takes a list of user-specified antenna
 * locations and draws the corresponding visibilities in the UVplot window.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_compbeam_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[])
{
  double lat,lng,hastart,hastop,dec,wave,cellsize,uvrad;
  int ngrid;

  if(argc != 9) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the compbeam command.\n",
		     "Should be: array antsize wavelength lat long hastart hastop dec cellsize", NULL);
    
    return TCL_ERROR;
  }
  /*
   * Convert the strings into radians.
   */
  if(Tcl_GetDouble(interp, argv[0], &wave) == TCL_ERROR ||
       Tcl_GetDouble(interp, argv[6], &cellsize) == TCL_ERROR ||
       Tcl_GetInt(interp, argv[7], &ngrid) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[8], &uvrad) == TCL_ERROR)
    return TCL_ERROR;

  Grid[0] = ngrid;
  Grid[1] = ngrid;

  lat = decfloat(argv[1]);
  lng = decfloat(argv[2]);
  hastart = rafloat(argv[3]);
  hastop = rafloat(argv[4]);
  if(strcmp(argv[5],"Zenith")!=0)
    dec = decfloat(argv[5]);
  else {
    hastart = 0;
    hastop = 0;
  }

  if(plotbeam(sim,wave,lat,lng,hastart,hastop,dec,cellsize,uvrad))
    return TCL_ERROR;

  return TCL_OK;
}
/*.......................................................................
 * Recompute visibilities and update the UV plot window.
 */
static int compvis(Simint *sim, double wave, double lat, double lng, double hastart, double hastop, double dec)
{  
  Ant *ant1=NULL,*ant2=NULL;
  float u,v,sh,ch,dh,ha,urange;
  int i,j,waserr=0,nint;
  float us[2],vs[2];
  double wave0,dwave;
  double wt;
  double nu0,numin,numax,dnu;
  double rms;
  
  waserr = getobs(sim);
  if(!waserr) {

    if(sim->interactive) {
      /*
       * Precompute the weight, modulo the aperture diameter and efficiency.
       * Note that this already includes the factor of pi/4 to go from D^2 to
       * area.
       */
      rms = sqrt(2)*sim->obs.tsys0*K/sqrt(sim->obs.tscale*sim->obs.tint*
					  sim->obs.bw/sim->obs.nchan*1e9*
					  sim->obs.correff)*1e23/M_PI*4;
      wt = 1.0/(rms*rms);
      /*
       * Get the wavelength range corresponding to the specified bandwidth.
       */
      wave0 = sim->obs.wave;
      dnu = sim->obs.bw/sim->obs.nchan;
      /*
       * nu0 in GHz (C is in cm/s)
       */
      nu0 = C/1e9/wave0;
      numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
      numax = nu0 + (double)(sim->obs.nchan)/2*dnu;
      dwave = C/1e9*(1.0/numin-1.0/numax)/sim->obs.nchan;
      wave0 = C/1e9/numax;
      /*
       * Attempt to increment the array of uv data points.  There will be 
       * nant*(nant-1)/2 baselines x 2 conjugate points
       */
      if(sim->nant==1) {
	fprintf(stdout,"No baselines to be computed.\n");
	return 1;
      }
      /*
       * Redraw the UV plot.
       */
      cpgslct(sim->uvplt_id);
      cpgsci(1);
      cpgask(0);
      cpgpage();
      cpgvstd();
      if(sim->upa==sim->upb) {
	urange = sqrt(2.0)*(sim->xpb-sim->xpa)/wave0;
	sim->upa = -urange;  sim->upb = urange;
	sim->vpa = -urange;  sim->vpb = urange;
      }
      cpgswin(sim->upa,sim->upb,sim->vpa,sim->vpb);
      cpgwnad(sim->upa,sim->upb,sim->vpa,sim->vpb);
      cpgbox("BCNST",0,0,"BCNST",0,0);
      cpglab("U (\\gl)","V (\\gl)","U-V plane");
      /*
       * Allocate an array to hold the maximum number of visibilities.
       */
      if(!waserr) {
	cpgbbuf();
	/*
	 * Get the nearest number of integrations. HAs are in radians, so we need
	 * convert seconds to the same.
	 */
	if(sim->obs.hastart==sim->obs.hastop)
	  nint = 1;
	else
	  nint = (sim->obs.hastop-sim->obs.hastart)/(sim->obs.tint/RTOS);
	/*
	 * Get the dh from the tint in radians.
	 */
	dh = (sim->obs.hastop-sim->obs.hastart)/(nint);
	for(ant1 = sim->ants;ant1 != NULL;ant1 = ant1->next)
	  for(ant2 = ant1->next;ant2 != NULL;ant2 = ant2->next) {
	    /*
	     * Assume that the aperture efficiency does not change with 
	     * frequency across the bandpass.
	     */
	    wt *= (ant1->type->apeff*ant1->type->size*ant1->type->size)*(ant2->type->apeff*ant2->type->size*ant2->type->size);
	    cpgsci((ant1->flag==ANT_S||ant2->flag==ANT_S)?2:10);
	    
	    
	    if(sim->obs.hastart == sim->obs.hastop) {
	      sh = sin(sim->obs.hastart);
	      ch = cos(sim->obs.hastart);
	      for(j=0;j < sim->obs.nchan;j++) {
		wave = wave0 + j*dwave;
		u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		v = -sin(sim->obs.srcdec)*ch*(ant1->X-ant2->X) + sin(sim->obs.srcdec)*sh*(ant1->Y-ant2->Y) + cos(sim->obs.srcdec)*(ant1->Z-ant2->Z);
		u = u/wave;
		v = v/wave;
		us[0] = u;
		vs[0] = v;
		us[1] = -u;
		vs[1] = -v;
		cpgpt(2,us,vs,1);
	      }
	    }
	    else {
	      for(i=0;i < nint;i++) {
		/*
		 * Compute the visibilities at the central HA of this integration,
		 * hence the extra dh/2
		 */
		ha = sim->obs.hastart + dh*i + dh/2;
		sh = sin(ha);
		ch = cos(ha);
		for(j=0;j < sim->obs.nchan;j++) {
		  wave = wave0 + j*dwave;
		  
		  
		  u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		  v = -sin(sim->obs.srcdec)*ch*(ant1->X-ant2->X) + sin(sim->obs.srcdec)*sh*(ant1->Y-ant2->Y) + cos(sim->obs.srcdec)*(ant1->Z-ant2->Z);
		  u = u/wave;
		  v = v/wave;
		  
		  us[0] = u;
		  vs[0] = v;
		  us[1] = -u;
		  vs[1] = -v;
		  cpgpt(2,us,vs,-1);
		}
	      }
	    }
	  }
	cpgebuf();
      }
    }; // End if(sim->interactive) {
  }; // End if(!waserr) {

  return waserr;
}
/*.......................................................................
 * Make a radial plot of the visibilities.
 */
static int radplot(Simint *sim, int nbin)
{  
  Ant *ant1=NULL,*ant2=NULL;
  float u,v,sh,ch,dh,ha,urange;
  int i,j,waserr=0,nint;
  float *hist=NULL;
  int nsum,ind;
  double wave0,dwave=0.0;
  double tsys,wt;
  double rms,umax,vmax,r,dr,ymax,rmax;
  double wave;

  if(sim->interactive) {
    
    waserr = getobs(sim);

    /*
     * Precompute the weight, modulo the aperture diameter and efficiency.
     * Note that this already includes the factor of pi/4 to go from D^2 to
     * area.
     */
    rms = 2*tsys*K/sqrt(sim->obs.tscale*sim->obs.tint*sim->obs.bw*1e9*sim->obs.correff)*1e23/M_PI*4;
    wt = 1.0/(rms*rms);
    
    {
      double nu0, numin, numax, dnu;

      wave0 = sim->obs.wave;
      nu0 = C/1e9/wave0;
      numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
      numax = nu0 + (double)(sim->obs.nchan)/2*dnu;
      dwave = C/1e9*(1.0/numin-1.0/numax)/sim->obs.nchan;
    }
    /*
     * Attempt to increment the array of uv data points.  There will be 
     * nant*(nant-1)/2 baselines x 2 conjugate points
     */
    if(nbin < 1) {
      fprintf(stderr,"Bin number < 1.\n");
      return 1;
    }
    if(sim->nant==1) {
      fprintf(stdout,"No baselines to be computed.\n");
      return 1;
    }
    /*
     * Allocate an zero the histogram array.
     */
    if((hist=(float *)malloc(nbin*sizeof(float)))==NULL) {
      fprintf(stderr,"Unable to allocate histogram array.\n");
      return 1;
    };
    for(i=0;i < nbin;i++)
      hist[i] = 0.0;
    /*
     * Figure out the max displayed uv-radius
     */
    if(sim->upa==sim->upb) {
      urange = sqrt(2.0)*(sim->xpb-sim->xpa)/wave0;
      sim->upa = -urange;  sim->upb = urange;
      sim->vpa = -urange;  sim->vpb = urange;
    }

    umax = (fabs(sim->upa)>sim->upb)?fabs(sim->upa):sim->upb;
    vmax = (fabs(sim->vpa)>sim->vpb)?fabs(sim->vpa):sim->vpb;
    rmax = sqrt(umax*umax+vmax*vmax);
    dr = rmax/(nbin-1);

    /*
     * Allocate an array to hold the maximum number of visibilities.
     */
    if(!waserr) {
      /*
       * Get the nearest number of integrations. HAs are in radians, so we need
       * convert seconds to the same.
       */
      nint = (sim->obs.hastop-sim->obs.hastart)/(sim->obs.tint/RTOS);
      /*
       * Get the dh from the tint in radians.
       */
      dh = (sim->obs.hastop-sim->obs.hastart)/(nint);

      for(ant1 = sim->ants;ant1 != NULL;ant1 = ant1->next)
	for(ant2 = ant1->next;ant2 != NULL;ant2 = ant2->next) {

	  /*
	   * Assume that the aperture efficiency does not change with 
	   * frequency across the bandpass.
	   */
	  wt *= (ant1->type->apeff*ant1->type->size*ant1->type->size)*(ant2->type->apeff*ant2->type->size*ant2->type->size);

	  cpgsci((ant1->flag==ANT_S||ant2->flag==ANT_S)?2:10);

	  if(sim->obs.hastart == sim->obs.hastop) {

	    sh = sin(sim->obs.hastart);
	    ch = cos(sim->obs.hastart);

	    for(j=0;j < sim->obs.nchan;j++) {

	      wave = wave0 + j*dwave;
	      u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	      v = -sin(sim->obs.srcdec)*ch*(ant1->X-ant2->X) + sin(sim->obs.srcdec)*sh*(ant1->Y-ant2->Y) + cos(sim->obs.srcdec)*(ant1->Z-ant2->Z);
	      u /= wave;
	      v /= wave;
	      r = sqrt(u*u+v*v);
	      ind = (int)floor(r/dr + 0.5);
	      hist[ind] += 1.0;
	      nsum += 1.0;
	    }
	  } else {
	    for(i=0;i < nint;i++) {

	      /*
	       * Compute the visibilities at the central HA of this integration,
	       * hence the extra dh/2
	       */
	      ha = sim->obs.hastart + dh*i + dh/2;

	      sh = sin(ha);
	      ch = cos(ha);

	      for(j=0;j < sim->obs.nchan;j++) {

		wave = wave0 + j*dwave;

		u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		v = -sin(sim->obs.srcdec)*ch*(ant1->X-ant2->X) + sin(sim->obs.srcdec)*sh*(ant1->Y-ant2->Y) + cos(sim->obs.srcdec)*(ant1->Z-ant2->Z);
		u /= wave;
		v /= wave;
		r = sqrt(u*u+v*v);
		ind = (int)floor(r/dr + 0.5);
		hist[ind] += 1.0;
		nsum += 1.0;
	      }
	    }
	  }
	}
    }

    /*
     * Find the maximum of the histogram array.
     */
    ymax = hist[0];
    for(i=0;i < nbin;i++)
      ymax = (ymax > hist[i]) ? ymax : hist[i];

    /*
     * Redraw the UV plot.
     */
    cpgslct(sim->uvplt_id);

    v_hdraw(hist,nbin,0,rmax,0,ymax,dr,"UV Radius (\\gl)","Nvis","Radplot");
  }

  return waserr;
}
/*.......................................................................
 * Write out the visibilities.
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
int write_vis(FILE *fp, Simint *sim, Dtype type, Image *image)
{  
  Ant *ant1=NULL,*ant2=NULL;
  double u,v,w,sh,ch,dh,ha,base;
  int i,j,k,waserr=0,nint=0,nvis=0;
  int ntot=0;
  int a1,a2;            /* Antenna numbers (#s in the antenna table) */
  double wave0,dwave,wave;
  double tsys,wt,wt0;
  double date0,date1,date2,date;
  double rms;
  double re=1.0,im=0.0;
  Binhead hdr;
  Datum dat;
  static double diams[] = {0,0,0}; /* Allow a maximum of three. */
  int ndiam=0;
  short code,c1,c2;
  double nu0,numin,numax,dnu;
  float fltbuf[9],fitsbuf[720];
  int nran=6,ndat=3,nbyte=0,nres=0;  /* Bookkeeping for FITS file buffering */
  /*
   * Get the Tcl variables we need to compute the visibilities.
   */
  waserr = getobs(sim);
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
  rms = 2*K/sqrt(sim->obs.tscale*sim->obs.tint*sim->obs.bw/sim->obs.nchan*
		 1e9*sim->obs.correff)*1e23/M_PI*4;
  wt0 = 1.0/(rms*rms);
  /*
   * Get the wavelength range corresponding to the specified bandwidth.
   */
  wave0 = sim->obs.wave;
  dnu = sim->obs.bw/sim->obs.nchan;
  /*
   * nu0 in GHz (C is in cm/s)
   */
  nu0 = C/1e9/wave0;
  numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
  numax = nu0 + (double)(sim->obs.nchan)/2*dnu;
  dwave = C/1e9*(1.0/numin-1.0/numax)/sim->obs.nchan;
  wave0 = C/1e9/numax;
  /*
   * Attempt to increment the array of uv data points.  There will be 
   * nant*(nant-1)/2 baselines x 2 conjugate points
   */
  if(sim->nant==1) {
    fprintf(stdout,"No baselines to be computed.\n");
    return 1;
  }
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
     * Encapsulate information about this observation in a Binhead struct.
     */
    hdr.samples = sim->nant*(sim->nant-1)/2*nint*sim->obs.nchan;
    hdr.freq = 30.0/sim->obs.wave;
    /*
     * Find out which different antenna sizes are present.
     */
    diams[ndiam] = sim->ants->type->size;
    for(ant1 = sim->ants;ant1 != NULL;ant1 = ant1->next) {
      if(ant1->type->size != diams[ndiam]) {
	if(ndiam == 2) {
	  break;
	}
	else
	  diams[++ndiam] = ant1->type->size;
      }
    }
    ndiam++;

    hdr.diam1 = diams[0];
    hdr.diam2 = diams[1];
    hdr.diam3 = diams[2];

    if(type==T_BIN) {
      fwrite((void *)(&hdr),sizeof(Binhead),1,fp);
    }
    else if(type==T_DAT)
      fprintf(fp,"FREQUENCY:  %2.2f  DIAMS: %3.2f %3.2f %3.2f\n",
	      hdr.freq,hdr.diam1,hdr.diam2,hdr.diam3);
    /*
     * Get the dh from the tint in radians.
     */
    dh = (sim->obs.hastop-sim->obs.hastart)/(nint);
    /*
     * Main Loop -- loop through all integrations.
     */
    for(i=0;i < nint && !waserr;i++) {
      /*
       * Increment the date by tint seconds.
       */
      date = date0 + ((double)(sim->obs.tint)/DAYSEC)*i;
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
      ha = sim->obs.hastart + dh*i + dh/2;
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
      /*
       * For each integration, loop through all non-redundant baselines.
       */
      for(a1=1,ant1 = sim->ants;ant1 != NULL && !waserr;
	  ant1 = ant1->next,a1++)
	for(a2=a1+1,ant2 = ant1->next;ant2 != NULL && !waserr;
	    ant2 = ant2->next,a2++) {
	  /*
	   * Get the baseline random parameter value for this baseline.
	   */
	  base = 256*a1 + a2;
	  /*
	   * Get the telescope code for this observation.
	   */
	  for(k=0;k < ndiam;k++) {
	    if(ant1->type->size==diams[k])
	      c1 = (short)(k+1);
	    if(ant2->type->size==diams[k])
	      c2 = (short)(k+1);
	  }
	  /* 
	   * Construct a decimal 2-byte version of the code.
	   */
	  code = c1*10 + c2;
	  /*
	   * We can write the UVF file two different ways.  If we want separate
	   * IFs for each correlator channel, we currently have to give each
	   * baseline the same UV coordinates, since the random parameters 
	   * apply to each axis of the groups stucture (stupidly enough!)
	   * Otherwise, we can give each channel the right UV coordinates, but 
	   * the same frequency.  We'll denote the first with the isouv 
	   * variable.
	   *
	   * If isouv was requested, write the random parameters for this 
	   * group now.
	   */
	  if(type==T_UVF && sim->obs.isouv) {
	    u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	    v = -sd*ch*(ant1->X-ant2->X) + sd*sh*(ant1->Y-ant2->Y) + 
	      cd*(ant1->Z-ant2->Z);
	    w = cd*ch*(ant1->X-ant2->X) - cd*sh*(ant1->Y-ant2->Y) + 
	      sd*(ant1->Z-ant2->Z);
	    wave = sim->obs.wave;
	    u /= wave;
	    v /= wave;
	    w /= wave;

	    fltbuf[0] = u*wave/C*Uvscale;
	    fltbuf[1] = v*wave/C*Uvscale;
	    fltbuf[2] = w*wave/C*Uvscale;
	    fltbuf[3] = base;
	    fltbuf[4] = date1;
	    fltbuf[5] = date2;
	    
	    waserr |= fwrite(fltbuf,sizeof(float),nran,fp) == 0;
	    nbyte += nran*sizeof(float);
	  }

	  wt = wt0*(ant1->type->apeff*ant1->type->size*ant1->type->size)*
	    (ant2->type->apeff*ant2->type->size*ant2->type->size)/(tsys*tsys);
	  /*
	   * Now loop through all correlator channels.
	   */
	  for(j=0;j < sim->obs.nchan && !waserr;j++) {
	    wave = wave0 + j*dwave;
	    /*
	     * Increment the total number of visibilities.
	     */
	    ntot++;
	    
	    u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	    v = -sd*ch*(ant1->X-ant2->X) + sd*sh*(ant1->Y-ant2->Y) + 
	      cd*(ant1->Z-ant2->Z);
	    w = cd*ch*(ant1->X-ant2->X) - cd*sh*(ant1->Y-ant2->Y) + 
	      sd*(ant1->Z-ant2->Z);
	    u = u/wave;
	    v = v/wave;
	    w = w/wave;
	    
	    ++nvis;
	    if(type==T_BIN) {
	      dat.u = u;
	      dat.v = v;
	      dat.wt = wt;
	      dat.code = code;
	      /*
	       * If no image descriptor was passed, just write bogus values
	       * for the visibilities.
	       */
	      dat.re = re;
	      dat.im = im;
	      /*
	       * Only write this visibility if the uv point is not shadowed.
	       */
#ifdef ATI
	      if(sqrt(dat.u*dat.u + dat.v*dat.v) > ant1->type->size*1.05)
		waserr |= fwrite((const void *)(&dat),sizeof(Datum),1,fp) == 0;
#else
	      waserr |= fwrite((const void *)(&dat),sizeof(Datum),1,fp) == 0;
#endif
	    }
	    else if(type==T_DAT) {
#ifdef ATI
	      if(sqrt(u*u + v*v) > ant1->type->size*1.05)
		fprintf(fp,"   %13.7f   %13.7f      %10.7f      %10.7f    %13.7f %2d\n",u,v,re,im,wt,code);
	      fprintf(stdout,"ATI is defined.\n");
#else
	      
	      fprintf(fp,"   %13.7f   %13.7f      %10.7f      %10.7f    %13.7f %2d\n",u,v,re,im,wt,code);
	      fprintf(stdout,"ATI is not defined.\n");
#endif
	    }
	    else if(type==T_UVF) {
	      /*
	       * Pack the data into a floating point buffer and write it 
	       * to the UVF file.
	       *
	       * Store the UVW as light travel time in seconds across the
	       * projected baseline.
	       */
	      if(!sim->obs.isouv) {
		/*
		 * Multiply through by the center wavelength, so that the
		 * UV points will be scaled correctly when difmap rescales
		 * by the bogus IF.
		 */
		fltbuf[0] = u*sim->obs.wave/C*Uvscale;
		fltbuf[1] = v*sim->obs.wave/C*Uvscale;
		fltbuf[2] = w*sim->obs.wave/C*Uvscale;
		fltbuf[3] = base;
		fltbuf[4] = date1;
		fltbuf[5] = date2;

		waserr |= fwrite(fltbuf,sizeof(float),nran,fp) == 0;
		nbyte += nran*sizeof(float);
	      }
	      /*
	       * If no image descriptor was passed, just write bogus values
	       * for the visibilities.
	       */
	      if(image==NULL) {
		fltbuf[0] = re;
		fltbuf[1] = im;
	      }
	      else {
		dat.u = u;
		dat.v = v;
		dat.wt = wt;
		fltbuf[0] = dat.re;
		fltbuf[1] = dat.im;
	      }
	      fltbuf[2] = wt;
	      /*
	       * And just write the data in this pass.
	       */
	      waserr |= fwrite(fltbuf,sizeof(float),ndat,fp) == 0;
	      nbyte += ndat*sizeof(float);
	    }
	  }
	}
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

    fwrite(fitsbuf,sizeof(float),nres,fp);
  }
#ifdef DEBUG
printf("Total visibilities: %d\n",nvis);
#endif
  return waserr;
}
/*.......................................................................
 * Write out the visibilities, with separate UV points for each IF
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
int write_sepvis(FILE *fp, Simint *sim, Dtype type, Image *image)
{  
  Ant *ant1=NULL,*ant2=NULL;
  double u,v,w,sh,ch,dh,ha,base;
  int i,j,waserr=0,nint=0,nvis=0;
  int ntot=0;
  int a1,a2;            /* Antenna numbers (#s in the antenna table) */
  double wave0,dwave,wave;
  double tsys,wt,wt0;
  double date0,date1,date2,date;
  double rms;
  double re=1.0,im=0.0;
  Binhead hdr;
  Datum dat;
  static double diams[] = {0,0,0}; /* Allow a maximum of three. */
  int ndiam=0;
  double nu0,numin,numax,dnu;
  float fltbuf[9],fitsbuf[720];
  int nran=6,ndat=3,nbyte=0,nres=0;  /* Bookkeeping for FITS file buffering */
  double ifinc = 0.01; /* A tiny amount by which to increment the date for each
			 IF (seconds) */
  /*
   * Get the Tcl variables we need to compute the visibilities.
   */
  waserr = getobs(sim);
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
  rms = 2*K/sqrt(sim->obs.tscale*sim->obs.tint*sim->obs.bw/sim->obs.nchan*
		 1e9*sim->obs.correff)*1e23/M_PI*4;
  wt0 = 1.0/(rms*rms);
  /*
   * Get the wavelength range corresponding to the specified bandwidth.
   */
  wave0 = sim->obs.wave;
  dnu = sim->obs.bw/sim->obs.nchan;
  /*
   * nu0 in GHz (C is in cm/s)
   */
  nu0 = C/1e9/wave0;
  numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
  numax = nu0 + (double)(sim->obs.nchan)/2*dnu;
  dwave = C/1e9*(1.0/numin-1.0/numax)/sim->obs.nchan;
  wave0 = C/1e9/numax;
  /*
   * Attempt to increment the array of uv data points.  There will be 
   * nant*(nant-1)/2 baselines x 2 conjugate points
   */
  if(sim->nant==1) {
    fprintf(stdout,"No baselines to be computed.\n");
    return 1;
  }
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
     * Encapsulate information about this observation in a Binhead struct.
     */
    hdr.samples = sim->nant*(sim->nant-1)/2*nint*sim->obs.nchan;
    hdr.freq = 30.0/sim->obs.wave;
    /*
     * Find out which different antenna sizes are present.
     */
    diams[ndiam] = sim->ants->type->size;
    for(ant1 = sim->ants;ant1 != NULL;ant1 = ant1->next) {
      if(ant1->type->size != diams[ndiam]) {
	if(ndiam == 2) {
	  break;
	}
	else
	  diams[++ndiam] = ant1->type->size;
      }
    }
    ndiam++;

    hdr.diam1 = diams[0];
    hdr.diam2 = diams[1];
    hdr.diam3 = diams[2];

    /*
     * Get the dh from the tint in radians.
     */
    dh = (sim->obs.hastop-sim->obs.hastart)/(nint);
      /*
       * Loop through real integrations
       */
      for(i=0;i < nint && !waserr;i++) {
	/*
	 * Compute the visibilities at the central HA of this integration,
	 * hence the extra dh/2
	 */
	ha = sim->obs.hastart + dh*i + dh/2;
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
	/*
	 * To write the separate IFs as distinct UV points, we need to treat 
	 * each IF as a separate integration, thus we have two "integration" 
	 * loops per se.
	 *
	 * Loop through "IF integrations"
	 */
	for(j=0;j < sim->obs.nchan && !waserr;j++) {
	  wave = wave0 + j*dwave;
	  /*
	   * Increment the date by tint + ifinc seconds.
	   */
	  date = date0 + ((double)(sim->obs.tint)/DAYSEC)*i + j*ifinc/DAYSEC;
	  /*
	   * Get the integral and fractional parts of the date.
	   */
	  date2 = date - (int)date;
	  date1 = date - date2;     /* In days */
	  date2 *= DAYSEC;           /* Convert to seconds. */
	  /*
	   * For each integration, loop through all non-redundant baselines.
	   */
	  for(a1=1,ant1 = sim->ants;ant1 != NULL && !waserr;
	      ant1 = ant1->next,a1++)
	    for(a2=a1+1,ant2 = ant1->next;ant2 != NULL && !waserr;
		ant2 = ant2->next,a2++) {
	      /*
	       * Get the baseline random parameter value for this baseline.
	       */
	      base = 256*a1 + a2;
	      
	      u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	      v = -sd*ch*(ant1->X-ant2->X) + sd*sh*(ant1->Y-ant2->Y) + 
		cd*(ant1->Z-ant2->Z);
	      w = cd*ch*(ant1->X-ant2->X) - cd*sh*(ant1->Y-ant2->Y) + 
		sd*(ant1->Z-ant2->Z);
	      u /= wave;
	      v /= wave;
	      w /= wave;

	      wt = wt0*(ant1->type->apeff*ant1->type->size*ant1->type->size)*
		(ant2->type->apeff*ant2->type->size*ant2->type->size)/(tsys*tsys);
	      /*
	       * Increment the total number of visibilities.
	       */
	      ntot++;
	      /*
	       * Pack the data into a floating point buffer and write it 
	       * to the UVF file.
	       *
	       * Store the UVW as light travel time in seconds across the
	       * projected baseline.
	       */
	      fltbuf[0] = u*sim->obs.wave/C*Uvscale;
	      fltbuf[1] = v*sim->obs.wave/C*Uvscale;
	      fltbuf[2] = w*sim->obs.wave/C*Uvscale;
	      fltbuf[3] = base;
	      fltbuf[4] = date1;
	      fltbuf[5] = date2;
	      
	      waserr |= fwrite(fltbuf,sizeof(float),nran,fp) == 0;
	      nbyte += nran*sizeof(float);
	      /*
	       * If no image descriptor was passed, just write bogus values
	       * for the visibilities.
	       */
	      if(image==NULL) {
		fltbuf[0] = re;
		fltbuf[1] = im;
	      }
	      else {
		dat.u = u;
		dat.v = v;
		dat.wt = wt;
		dat.re = re;
		dat.im = im;
		fltbuf[0] = dat.re;
		fltbuf[1] = dat.im;
	      }
	      fltbuf[2] = wt;
	      /*
	       * And just write the data in this pass.
	       */
	      waserr |= fwrite(fltbuf,sizeof(float),ndat,fp) == 0;
	      nvis++;
	      nbyte += ndat*sizeof(float);
	    }
	}
      }
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
  
  fwrite(fitsbuf,sizeof(float),nres,fp);

#ifdef DEBUG
  printf("Total visibilities: %d\n",nvis);
#endif
  return waserr;
}
/*.......................................................................
 * Recompute a beam and display in the uvplt window.
 */
static int plotbeam(Simint *sim, double wave, double lat, double lng, double hastart, double hastop, double dec, double cellsize,double uvrad)
{  
  Ant *ant1=NULL,*ant2=NULL;
  float u,v,sh,ch,dh,ha;
  int i,nvis,waserr=0,j,nchan,nint;
  float *us=NULL,*vs=NULL,*vis=NULL;
  double wave0,dwave,tint,bw;
  double nu0,numin,numax,dnu;

  if(sim->interactive) {
    
    waserr = tcl_fltval(sim->interp,"tint",&tint);
    waserr |= tcl_fltval(sim->interp,"bw",&bw);
    waserr |= tcl_intval(sim->interp,"nchan",&nchan);
    
    uvrad /= 2.35;
    /*
     * Get the wavelength range corresponding to the specified bandwidth.
     */
    wave0 = wave;
    dnu = bw/nchan;
    /*
     * nu0 in GHz (C is in cm/s)
     */
    nu0 = C/1e9/wave0;
    numin = nu0 - (double)(nchan)/2*dnu;
    numax = nu0 + (double)(nchan)/2*dnu;
    dwave = C/1e9*(1.0/numin-1.0/numax)/nchan;
    wave0 = C/1e9/numax;
    /*
     * Attempt to increment the array of uv data points.  There will be 
     * nant*(nant-1)/2 baselines x 2 conjugate points
     */
    if(sim->nant==1) {
      fprintf(stdout,"No baselines to be computed.\n");
      return 1;
    }
    /*
     * Get the nearest number of integrations. HAs are in radians, so we need
     * convert seconds to the same.
     */
    if(hastart==hastop)
      nint = 1;
    else
      nint = (hastop-hastart)/(tint/RTOS);
    /*
     * Get the dh from the tint in radians.
     */
    dh = (hastop-hastart)/(nint);
    /*
     * Redraw the UV plot.
     */
    cpgslct(sim->uvplt_id);
    cpgsci(1);
    cpgask(0);
    cpgpage();
    cpgvstd();
    /*
     * Allocate an array to hold the maximum number of visibilities.
     */
    waserr = (us = malloc(sim->nant*(sim->nant-1)*2*nint*nchan*sizeof(float)))==NULL;
    waserr |= (vs = malloc(sim->nant*(sim->nant-1)*2*nint*nchan*sizeof(float)))==NULL;
    waserr |= (vis = malloc(sim->nant*(sim->nant-1)*2*nint*nchan*sizeof(float)))==NULL;
    nvis = 0;
    if(!waserr) {
      for(ant1 = sim->ants;ant1 != NULL;ant1 = ant1->next)
	for(ant2 = sim->ants;ant2 != NULL;ant2 = ant2->next) {
	  if(ant1 != ant2) {
	    cpgsci((ant1->flag==ANT_S||ant2->flag==ANT_S)?2:10);
	    if(hastart == hastop) {
	      sh = sin(hastart);
	      ch = cos(hastart);
	      for(j=0;j < nchan;j++) {
		wave = wave0 + j*dwave;
		u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		v = -sin(dec)*ch*(ant1->X-ant2->X) + sin(dec)*sh*(ant1->Y-ant2->Y) + cos(dec)*(ant1->Z-ant2->Z);
		u = u/wave;
		v = v/wave;
		us[nvis] = u;
		vs[nvis] = v;
		us[nvis+1] = -u;
		vs[nvis+1] = -v;
		/* Apply a uv taper, if requested. */
		vis[nvis] = vis[nvis+1] = uvrad==0.0 ? 1.0 : exp(-(u*u+v*v)/(2*uvrad*uvrad));
		nvis += 2;
	      }
	    }
	    else {
	      for(i=0;i < nint;i++) {
		ha = hastart + dh*i;
		sh = sin(ha);
		ch = cos(ha);
		for(j=0;j < nchan;j++) {
		  wave = wave0 + j*dwave;
		  u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		  v = -sin(dec)*ch*(ant1->X-ant2->X) + sin(dec)*sh*(ant1->Y-ant2->Y) + cos(dec)*(ant1->Z-ant2->Z);
		  u = u/wave;
		  v = v/wave;
		  us[nvis] = u;
		  vs[nvis] = v;
		  us[nvis+1] = -u;
		  vs[nvis+1] = -v;
		  vis[nvis] = vis[nvis+1] = 1.0;
		  nvis += 2;
		}
	      }
	    }
	  }
	}
      waserr |= newfft(us,vs,vis,nvis,cellsize);
    }
    if(us)
      free(us);
    if(vs)
      free(vs);
    if(vis)
      free(vis);
  }

  return waserr;
}
/*.......................................................................
 * Compute the window functions corresponding to the various baselines.
 */
static int plotwin(Simint *sim, double antsize,double wave, double lat, double lng, double hastart, double hastop, double dec)
{  
  Ant *ant1=NULL,*ant2=NULL;
  float u,v,sh,ch,dh,ha;
  int i,j,k,nvis,waserr=0,ntheta=20;
  float *us=NULL,*vs=NULL,*vis=NULL,ymax;
  float xs[100],ys[100],dl,dt,Dl,theta,dtheta,theta0,ut,vt;

  if(sim->interactive) {
    
    dl = 999.0/99;
    for(i=0;i < 100;i++) {
      xs[i] = 1+ i*dl;
      ys[i] = 0.0;
    }
    dt = M_PI/(ntheta-1);
    
    /*
     * Attempt to increment the array of uv data points.  There will be 
     * nant*(nant-1)/2 baselines x 2 conjugate points
     */
    if(sim->nant==1) {
      fprintf(stdout,"No baselines to be computed.\n");
      return 1;
    }
    /*
     * Redraw the UV plot.
     */
    cpgslct(sim->uvplt_id);
    cpgsci(1);
    cpgask(0);
    cpgpage();
    cpgvstd();
    cpgwnad(0,1,0,1);
    /*
     * Allocate an array to hold the maximum number of visibilities.
     */
    waserr = (us = malloc(sim->nant*(sim->nant-1)*40*sizeof(float)))==NULL;
    waserr |= (vs = malloc(sim->nant*(sim->nant-1)*40*sizeof(float)))==NULL;
    waserr |= (vis = malloc(sim->nant*(sim->nant-1)*40*sizeof(float)))==NULL;
    nvis = 0;
    cpgswin(0,1000,0,1.2);
    if(!waserr) {
      dh = (hastop-hastart)/20;
      for(ant1 = sim->ants;ant1 != NULL;ant1 = ant1->next)
	for(ant2 = ant1->next;ant2 != NULL;ant2 = ant2->next) {
	  cpgsci((ant1->flag==ANT_S||ant2->flag==ANT_S)?2:10);
	  Dl = sqrt(ant1->type->size*ant2->type->size);
	  if(hastart == hastop) {
	    sh = sin(hastart);
	    ch = cos(hastart);
	    u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	    v = -sin(dec)*ch*(ant1->X-ant2->X) + sin(dec)*sh*(ant1->Y-ant2->Y) + cos(dec)*(ant1->Z-ant2->Z);
	    ymax = 0.0;
	    for(j=0;j < 100;j++) {
	      ys[j] = 0.0;
	      /*
	       * Figure out what angle corresponds to the u-v point.
	       */
	      theta0 = atan2(v,u);
	      /*
	       * Only compute window for +- ~4.5 sigma of the appropriate angle.
	       */
	      dtheta = 2*M_PI*Dl/xs[j]*4.5;
	      theta0 -= dtheta;
	      dtheta = (2*2*M_PI*Dl/xs[j]*4.5)/(ntheta-1);
	      for(k=0;k < ntheta;k++) {
		theta = theta0 + dtheta*k;
		ut = xs[j]/(2*M_PI)*cos(theta);
		vt = xs[j]/(2*M_PI)*sin(theta);
		ys[j] += exp(-((u-ut)*(u-ut) + (v-vt)*(v-vt))/Dl)*dt;
		ymax = ys[j] > ymax ? ys[j] : ymax;
	      }
	    }
	    for(j=0;j < 100;j++)
	      ys[j] /= ymax;
	    cpgline(100,xs,ys); 
	  }
	  else {
	    for(i=0;i < 20;i++) {
	      ha = hastart + dh*i;
	      sh = sin(ha);
	      ch = cos(ha);
	      u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
	      v = -sin(dec)*ch*(ant1->X-ant2->X) + sin(dec)*sh*(ant1->Y-ant2->Y) + cos(dec)*(ant1->Z-ant2->Z);
	      u = u/wave;
	      v = v/wave;
	      for(j=0;j < 100;j++) {
		ys[j] = 0.0;
		/*
		 * Figure out what angle corresponds to the u-v point.
		 */
		theta0 = atan2(v,u);
		/*
		 * Only compute window for +- ~4.5 sigma of the appropriate angle.
		 */
		dtheta = 2*M_PI*Dl/xs[j]*4.5;
		theta0 -= dtheta;
		dtheta = (2*2*M_PI*Dl/xs[j]*4.5)/99;
		for(k=0;k < 100;k++) {
		  theta = theta0 + dtheta*k;
		  ut = xs[j]/(2*M_PI)*cos(theta);
		  vt = xs[j]/(2*M_PI)*sin(theta);
		  ys[j] += exp(-((u-ut)*(u-ut) + (v-vt)*(v-vt))/Dl)*xs[j]*dt;
		  ymax = ys[j] > ymax ? ys[j] : ymax;
		}
	      }
	      
	      cpgline(100,xs,ys); 
	    }
	  }
	}
    }
    for(j=1;j < 100;j++)
      ys[j] /= xs[j]*2*M_PI;
    
    cpgsci(1);
    
    cpgbox("BCNST",0,0,"BCNST",0,0);
    cpglab("l","W\\dii\\u(l)","");
    if(us)
      free(us);
    if(vs)
      free(vs);
    if(vis)
      free(vis);
  }
  return waserr;
}
/*.......................................................................
 * Mark a particular antenna with a particular color
 */
static int markant(Simint *sim, float xa, float ya, float antsize, int symm)
{  
  Ant *ant1=NULL,*ant=NULL;
  float rad,r;
  int oldci,i,nant;
  double theta;
  float x,y;

  if(sim->interactive) {
    
    nant = symm;
    theta=2*M_PI/symm;
    /*
     * Find the antenna nearest to the selected point.
     */
    if(sim->nant==0) {
      fprintf(stderr,"No antennas to select.\n");
      return 0;
    };
    
    x = xa;
    y = ya;
    
    for(i=0;i < nant;i++) {
      /*
       * Use the next n-fold symmetric position of the previous antenna.
       */
      if(i > 0) {
	x = ant1->x*cos(theta) + ant1->y*sin(theta);
	y = ant1->y*cos(theta) - ant1->x*sin(theta);
      }
      rad = sqrt((x-sim->ants->x)*(x-sim->ants->x) + 
		 (y-sim->ants->y)*(y-sim->ants->y));
      ant1 = sim->ants;
      
      for(ant = sim->ants;ant != NULL;ant = ant->next) 
	if((r=sqrt((x-ant->x)*(x-ant->x) + 
		   (y-ant->y)*(y-ant->y))) < rad) {
	  rad = r;
	  ant1 = ant;
	}
      if(i > 0 && rad > antsize/20)
	return 0;
      /*
       * Toggle the flag.
       */
      ant1->flag = !ant1->flag;
      /*
       * Select the xyplt plot
       */
      cpgslct(sim->xyplt_id);
      /*
       * Mark the relevant antenna in red (selected) or green (unselected)
       */
      cpgqci(&oldci);
      cpgsci(ant1->flag==ANT_US?1:2);
      cpgpt1(ant1->x,ant1->y,1);
      cpgsfs(2);
      cpgcirc(ant1->x,ant1->y,ant1->type->size/2);
    }
  }
  return 0;
}
/*.......................................................................
 * Mark visibilities Associated with a particular antenna
 */
static int markvis(Simint *sim, float xa, float ya, float antsize, double wave, double obslat, double obslong, double hastart, double hastop, double dec, int symm, Antstat stat)
{  
  Ant *ant1=NULL,*ant2=NULL,*ant=NULL;
  float rad,r,theta;
  int oldci,i,j,n,waserr=0,nint;
  double sh,ch,ha,dh;
  float u,v,x,y;
  int nant;
  double wave0,dwave;
  double nu0,numin,numax,dnu;
  /*
   * Get the necessary parameters.
   */
  waserr = getobs(sim);
  if(!waserr) {

    if(sim->interactive) {
      /*
       * Store the old color index
       */
      cpgqci(&oldci);
      /*
       * Get the wavelength range corresponding to the specified bandwidth.
       */
      wave0 = sim->obs.wave;
      dnu = sim->obs.bw/sim->obs.nchan;
      /*
       * nu0 in GHz (C is in cm/s)
       */
      nu0 = C/1e9/wave0;
      numin = nu0 - (double)(sim->obs.nchan)/2*dnu;
      numax = nu0 + (double)(sim->obs.nchan)/2*dnu;
      dwave = C/1e9*(1.0/numin-1.0/numax)/sim->obs.nchan;
      wave0 = C/1e9/numax;
      /*
       * Check if positional symmetry was requested.
       */
      nant = sim->obs.symm;
      theta = 2*M_PI/nant;
      /*
       * Find the antenna nearest to the selected point.
       */
      if(sim->nant==0) {
	fprintf(stderr,"No antennas to select.\n");
	return 0;
      };
      
      x = xa;
      y = ya;
      
      for(n=0;n < nant;n++) {
	/*
	 * Use the exact n-fold symmetric location of the previous antenna
	 * for the next n-1 deletions.
	 */
	if(n > 0) {
	  x = ant1->x*cos(theta) + ant1->y*sin(theta);
	  y = ant1->y*cos(theta) - ant1->x*sin(theta);
	}
	rad = sqrt((x-sim->ants->x)*(x-sim->ants->x) + 
		   (y-sim->ants->y)*(y-sim->ants->y));
	ant1 = sim->ants;
	
	for(ant = sim->ants;ant != NULL;ant = ant->next) 
	  if((r=sqrt((x-ant->x)*(x-ant->x) + 
		     (y-ant->y)*(y-ant->y))) < rad) {
	    rad = r;
	    ant1 = ant;
	  }
	/*
	 * Exit if either of the next two positions don't correspond to a 
	 * symmetric permutation of the first one.
	 */
	if(n > 0 && rad > sim->obs.antsize/20)
	  return 0;
	/*
	 * And mark the visibilities.
	 */
	cpgslct(sim->uvplt_id);
	cpgvstd();
	cpgswin(sim->upa, sim->upb, sim->vpa, sim->vpb);
	cpgwnad(sim->upa,sim->upb,sim->vpa,sim->vpb);
	/*
	 * Get the nearest number of integrations.
	 */
	if(sim->obs.hastart==sim->obs.hastop)
	  nint = 1;
	else
	  nint = (sim->obs.hastop-sim->obs.hastart)/(sim->obs.tint/RTOS);
	/*
	 * Get the dh from the tint in seconds.
	 */
	dh = (sim->obs.hastop-sim->obs.hastart)/(nint);     
	/*
	 * Now mark the visibilities corresponding to baselines with this antenna.
	 */
	for(ant2 = sim->ants;ant2 != NULL;ant2 = ant2->next) {
	  if(ant1 != ant2) {
	    cpgsci((stat==ANT_TOG?((ant1->flag==ANT_US)?10:2):(stat==ANT_DEL?0:(ant2->flag==ANT_S?2:10))));
	    /*
	     * If hastart==hastop, just draw the snapshot.
	     */
	    if(sim->obs.hastart == sim->obs.hastop) {
	      sh = sin(sim->obs.hastart);
	      ch = cos(sim->obs.hastart);
	      for(j=0;j < sim->obs.nchan;j++) {
		wave = wave0 + j*dwave;
		u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		v = -sin(dec)*ch*(ant1->X-ant2->X) + sin(dec)*sh*(ant1->Y-ant2->Y) + cos(dec)*(ant1->Z-ant2->Z);
		u = u/wave;
		v = v/wave;
		cpgpt1(u,v,1);
		cpgpt1(-u,-v,1);
	      }
	    }
	    /*
	     * Else compute visibility tracks.
	     */
	    else {
	      for(i=0;i < nint;i++) {
		ha = sim->obs.hastart + dh*i + dh/2;
		sh = sin(ha);
		ch = cos(ha);
		for(j=0;j < sim->obs.nchan;j++) {
		  wave = wave0 + j*dwave;
		  
		  u = sh*(ant1->X-ant2->X) + ch*(ant1->Y-ant2->Y);
		  v = -sin(dec)*ch*(ant1->X-ant2->X) + sin(dec)*sh*(ant1->Y-ant2->Y) + cos(dec)*(ant1->Z-ant2->Z);
		  u = u/wave;
		  v = v/wave;
		  
		  cpgpt1(u,v,-1);
		  cpgpt1(-u,-v,-1);
		}
	      }
	    }
	  }
	}
      }
      cpgsci(oldci);
    }
  }
  return 0;
}
/*.......................................................................
 * Add a new antenna node to the list of antennas.
 */
static int add_Ant(Simint *sim, double x, double y, double obslat, 
		   double obslong, double altitude, double antsize, double 
		   apeff)
{
  Ant *ant=NULL,*tant=NULL;
  AntType *type=NULL;
  /*
   * Get a pointer to the antenna of this type
   */
  if((type=add_AntType(sim, antsize, apeff))==NULL)
    return 1;
  if((ant = new_Ant(x,y,obslat,obslong,altitude))==NULL)
    return 1;

  ant->type = type;

  if(sim->ants==NULL) {
    ant->iant = 0;
    sim->ants = ant;
  }
  else {
    for(tant = sim->ants;tant->next != NULL;tant = tant->next);
    tant->next = ant;
    /*
     * And install the sequence number of this antenna
     */
    ant->iant = tant->iant+1;
  }
  ++sim->nant;

  return 0;
}
/*.......................................................................
 * Return a pointer to the requested antenna type.  If the type doesn't yet
 * exist, create it and add it to the sim's list of types.
 */
static AntType *add_AntType(Simint *sim, double size, double apeff) 
{
  AntType *type=NULL;
  /*
   * See if the requested antenna type is already present in the array.
   */
  for(type=sim->anttypes;type != NULL;type=type->next) {
    if(type->size==size && type->apeff==apeff)
      return type;
  }
  if((type=(AntType *)malloc(sizeof(AntType)))==NULL)
    return NULL;

  type->size = size;
  type->apeff = apeff;
  /*
   * Now add it to the head of the sim's list.
   */
  type->next = sim->anttypes;
  sim->anttypes = type;
  
  return type;
}
/*.......................................................................
 * Remove an antenna node from the list of antennas.
 */
Ant *rem_Ant(Simint *sim, Ant *node)
{
  Ant *antp=NULL,*ant=NULL;
/*
 * Find the antenna descriptor in the list of antennae and the
 * descriptor that precedes it.
 */
  antp = NULL;
  for(ant=sim->ants; ant!=NULL && ant!=node; ant=antp->next)
    antp = ant;
/*
 * If not found then the antenna has already been deleted.
 */
  if(ant==NULL)
    return ant;
/*
 * Relink around the descriptor in the antenna list.
 */
  if(antp==NULL)
    sim->ants = ant->next;
  else
    antp->next = ant->next;

  --sim->nant;

  return del_Ant(node);
}
/*.......................................................................
 * Allocate a new antenna node.
 */
Ant *new_Ant(double x, double y, double obslat, double obslong, double altitude)
{
  Ant *ant=NULL;

  if((ant=(Ant *)malloc(sizeof(Ant)))==NULL)
    fprintf(stderr,"Unable to allocate a new Antenna container.\n");
  else {
    ant->x = x;
    ant->y = y;
    /*
     * Convert from x and y in a tangent plane to latitude and longitude,
     * And convert to XYZ from latitude and long.
     */
    getXYZ(ant, obslat, obslong, altitude);
    ant->next = NULL;
    /* 
     * Set the flag to unselected by default.
     */
    ant->flag = ANT_US;
  }
  return ant;
}
/*.......................................................................
 * Allocate a new antenna node.
 */
Ant *del_Ant(Ant *node)
{
  if(node==NULL)
    return NULL;
  else {
    node->type = NULL;
    free(node);
  }
  return NULL;
}
/*.......................................................................
 * Clear the list of antenna types.
 */
static int zap_AntTypes(Simint *sim)
{    
  AntType *type=NULL,*next=NULL;

  for(type=sim->anttypes;type != NULL;type = next) {
    next = type->next;
    free(type);
  }
  
  sim->anttypes = NULL;

  return 0;
}
/*.......................................................................
 * Clear the list of antenna positions.
 */
static int zap_ant(Simint *sim)
{    
  Ant *ant=NULL,*next=NULL;
  int waserr = 0;

  for(ant = sim->ants;ant != NULL;ant = next) {
    next = ant->next;
    waserr |= (rem_Ant(sim,ant) != NULL);
  }

#ifdef TEST
  fprintf(stdout,"Before zap_AntTypes()\n");
#endif
  
  waserr |= zap_AntTypes(sim);

  return waserr;
}
/*.......................................................................
 * Return the radian equivalent of a dec string
 */
double decfloat(char *dec)
{
  double deg,min,sec;
/*
 * We assume that this is a string of the form hh:mm:ss.ss
 */
  sscanf(dec, "%lf:%lf:%lf",&deg,&min,&sec);

  sec = (dec[0]=='-' ? -1 : 1)*((fabs(deg)*60 + min)*60 + sec);

  return sec/RTOAS;
}
/*
 * Return the radian equivalent of an ra string
 */
double rafloat(char *ra)
{
  double hr,min,sec;
/*
 * We assume that this is a string of the form hh:mm:ss.ss
 */
  sscanf(ra, "%lf:%lf:%lf",&hr,&min,&sec);

  sec = (ra[0]=='-' ? -1 : 1)*((fabs(hr)*60 + min)*60 + sec);
  return sec/RTOS;
}
/*.......................................................................
 * Convert from x and y (cm) in a tangent plane to the Earth to XYZ
 */
void getXYZ(Ant *ant, double obslat, double obslong, double altitude)
{
  double cl,sl,lat,dlong,r;
  /* 
   * Get the radial distance (in cm) from the center of the Earth.
   */
  r = RE + altitude*100;
    /*
     * Convert from x and y in a tangent plane to latitude and longitude.
     */
  cl = cos(obslat+1e-6);
  sl = sin(obslat+1e-6);

  dlong = atan2(ant->x/r,(cl - ant->y/r*sl));
  lat = atan2(cos(dlong)*(ant->y/r*cl + sl),(cl - ant->y/r*sl));
    /*
     * Now convert from Latitude and Longitude to XYZ
     */
  cl = cos(lat);

  ant->X = r*cl*cos(dlong);
  ant->Y = r*cl*sin(dlong);
  ant->Z = r*sin(lat);

  return;
}
/*.......................................................................
 * Compute a 2D power spectrum via fft
 *
 * Input:
 *  xstring   char *   The x-member
 *  ystring   char *   The y-member
 *  fstring   char *   The input procedure-member designation string
 *                     of the form PROC.member[chan] or PROC.member .
 *  bstring   char *   Name to give the power spectrum member.
 */
static int newfft(float *u, float *v, float *vis, int nvis, double cellsize)
{
  float *datarr=NULL;
  float *power=NULL;
  float *power2=NULL;
  float *datarr_cmplx=NULL;
  int nu,nv;
  unsigned long nn[3];
  int first = 1;
  float du,dv;
  int ind;
  int i,j,k;
  int n;
  double norm,fac;
  int waserr = 0;
/*
 * Get the gridding parameters from the Grid descriptor (setgrid_fn)
 */
  nu      = Grid[0];
  nv      = Grid[1];
/*
 * Ensure that the dimensions are powers of 2.
 */
  nu = pow(2.0f,floor((log((double)nu)/log(2.0)+0.5)));
  nv = pow(2.0f,floor((log((double)nv)/log(2.0)+0.5)));
  n = nu*nv/4;
/*
 * Grid the data.  Cellsize is in arcseconds -- we need to convert to
 * u and v.
 */
  du = RTOAS/(cellsize*nu);
  dv = RTOAS/(cellsize*nv);

  fprintf(stdout,"Gridding data into %d x %d bins.\n",nu,nv);
  fprintf(stdout,"Cellsize %.2f arcsec will result in a uvmax of: %.2f.\n",cellsize,nu*du/2);

  datarr = grid2d(nvis, u, v, vis, -nu*du/2, -nv*dv/2, nu, nv, du, dv, 0);
  power = (float *)malloc(n*sizeof(float));
  power2 = (float *)malloc(n*sizeof(float));
  datarr_cmplx = (float *)malloc(nu*nv*2*sizeof(float));
      
  waserr |= (power==NULL);

  if(!waserr) {
    nn[1] = nu;
    nn[2] = nv;
/*
 * Get the shifted version of datarr.
 */
    fprintf(stdout,"Shifting the data...");
    fflush(stdout);

    for(i=1;i < nu*nv; i++) {
      datarr_cmplx[2*i] = datarr[i];
      datarr_cmplx[2*i+1] = 0.0;
    }
/*
 * Transform the data.
 */ 
    fprintf(stdout,"\rTransforming data...                 ");
    fflush(stdout);
    fft_shift(datarr_cmplx,nu,nv);
    /*
     * Do the inverse transform
     */
    fourn(datarr_cmplx-1,nn,2,-1);
    fft_shift(datarr_cmplx,nu,nv); 
/*
 * Compute and plot the real transform -- we will extract only the central
 * quarter.
 */
    first = 1;
    for(j=0;j < nv/2;j++) 
      for(i=0;i < nu/2;i++) {
	k = i + j*nu/2;
	ind = (nv/4 + j)*nu + i + nu/4;
	power[k] = datarr_cmplx[2*ind];
	power2[k] = datarr_cmplx[2*ind+1];
	if(first) {
	  norm = power[k];
	  first = 0;
	}
	norm = power[k] > norm ? power[k] : norm;
	k++;
      }
    for(i=0;i < n;i++)
      power[i] /= norm;
    /*
     * u ranges from -1/(2*dx) to 1/(2*dx)
     */
    fac = RTOAS/(60*2*M_PI)*2*M_PI;
    v_grey2(n,power,nu/2,nv/2,-fac*1.0/(2*du),fac*1.0/(2*du),-fac*1.0/(2*dv),fac*1.0/(2*dv),NULL,0,0,"arcminutes","arcminutes","Synthesized Beam","");
    /*    v_grey2(n,power2,nu/2,nv/2,-fac*1.0/(2*du),fac*1.0/(2*du),-fac*1.0/(2*dv),fac*1.0/(2*dv),NULL,0,0);*/
  }
  /* 
   * Free any allocated memory.
   */
  if(datarr)
    free(datarr);
  if(power)
    free(power);
  if(power2)
    free(power2);
  if(datarr_cmplx)
    free(datarr_cmplx);

  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Implement the "compwin" command. This takes a list of user-specified antenna
 * locations and computes the appropriate window functions corresponding to the
 * various baselines.
 *
 * Input:
 *  sim       Simint *   The sim being serviced.
 *  interp Tcl_Interp *   The TCL interpreter of the sim.
 *  argc          int     The number of TCL arguments in argv[].
 *  argv         char **  An array of 'argc' TCL arguments.
 *
 * Output:
 *  return        int     TCL_OK    - Normal completion.
 *                        TCL_ERROR - The interpreter result will contain
 *                                    the error message.
 */
static int simint_compwin_command(Simint *sim, Tcl_Interp *interp, int argc,
				   char *argv[])
{
  double lat,lng,hastart,hastop,dec,wave,antsize;

  if(argc != 7) {
    Tcl_AppendResult(interp,
		     "Wrong number of arguments to the compwin command.\n",
		     "Should be: array antsize wavelength lat long hastart hastop dec", NULL);
    
    return TCL_ERROR;
  }
  /*
   * Convert the strings into radians.
   */
  if(Tcl_GetDouble(interp, argv[0], &wave) == TCL_ERROR ||
     Tcl_GetDouble(interp, argv[1], &antsize) == TCL_ERROR)
    return TCL_ERROR;

  lat = decfloat(argv[2]);
  lng = decfloat(argv[3]);
  hastart = rafloat(argv[4]);
  hastop = rafloat(argv[5]);
  if(strcmp(argv[6],"Zenith")!=0)
    dec = decfloat(argv[6]);
  else {
    hastart = 0;
    hastop = 0;
  }

  if(plotwin(sim,antsize,wave,lat,lng,hastart,hastop,dec))
    return TCL_ERROR;

  return TCL_OK;
}
  
/*.......................................................................
 * Get the values of various Tcl variables.
 */
static int getpar(Tcl_Interp *interp)
{
  int i;
  char* str=NULL;

  for(i=0;i < nwt;i++) {
    if((str = (char* )Tcl_GetVar(interp, Wt[i].name, TCL_GLOBAL_ONLY))==NULL) {
      fprintf(stderr,"No such Tcl variable: %s.\n",Wt[i].name);
      return 1;
    }
    Wt[i].val = (double)atof(str);
  }
  return 0;
}
/*.......................................................................
 * Extract the double value of a Tcl variable.
 */
int tcl_fltval(Tcl_Interp *interp, char *name, double *val)
{
  char *str=NULL;

  if((str = (char* )Tcl_GetVar(interp, name, TCL_GLOBAL_ONLY))==NULL) {
    fprintf(stderr,"No such Tcl variable: %s.\n", name);
    return 1;
  }
  *val = (double)atof(str);

  return 0;
}
/*.......................................................................
 * Extract the integer value of a Tcl variable.
 */
int tcl_intval(Tcl_Interp *interp, char *name, int *val)
{
  char *str=NULL;

  if((str = (char* )Tcl_GetVar(interp, name, TCL_GLOBAL_ONLY))==NULL) {
    fprintf(stderr,"No such Tcl variable: %s.\n", name);
    return 1;
  }
  *val = atoi(str);

  return 0;
}
/*.......................................................................
 * Extract the radian value of a Tcl RA string variable.
 */
int tcl_raval(Tcl_Interp *interp, char *name, double *val)
{
  char *str=NULL;

  if((str = (char* )Tcl_GetVar(interp, name, TCL_GLOBAL_ONLY))==NULL) {
    fprintf(stderr,"No such Tcl variable: %s.\n", name);
    return 1;
  }
  *val = rafloat(str);

  return 0;
}
/*.......................................................................
 * Extract the radian value of a Tcl DEC string variable.
 */
int tcl_decval(Tcl_Interp *interp, char *name, double *val)
{
  char *str=NULL;

  if((str = (char* )Tcl_GetVar(interp, name, TCL_GLOBAL_ONLY))==NULL) {
    fprintf(stderr,"No such Tcl variable: %s.\n", name);
    return 1;
  }
  *val = decfloat(str);

  return 0;
}

/*.......................................................................
 * Return a visibility at given k.
 */
double cluster_model(double k) 
{
  double vis = exp(-2*M_PI*k)/k;
  return vis;
}

/*.......................................................................
 * Write out an antenna configuration file.
 */
int writeant(Simint *sim, FILE *fp)
{
  Ant *ant=NULL;

  fprintf(fp,"uplevel #0 {set antarr %s}\n",sim->array->name);
  fprintf(fp,"uplevel #0 {simint zapant}\n");
  fprintf(fp,"uplevel #0 {simint drawframes $wave}\n");
  for(ant=sim->ants;ant != NULL;ant=ant->next) {
    fprintf(fp,"uplevel #0 {simint addant %.2f %.2f %.2f $wave $obslat $obslong 1 1}\n",ant->x,ant->y,ant->type->size);
  }
  return 0;
}
/*.......................................................................
 * Get the observation parameters from the Tcl interface.
 */
int getobs(Simint *sim)
{
  int waserr=0;
  /*
   * Get the Tcl variables we need to compute the visibilities.
   */
  waserr |= tcl_fltval(sim->interp,"tint",&sim->obs.tint);
  waserr |= tcl_fltval(sim->interp,"tscale",&sim->obs.tscale);
  waserr |= tcl_fltval(sim->interp,"tsys",&sim->obs.tsys0);
  waserr |= tcl_fltval(sim->interp,"correff",&sim->obs.correff);
  waserr |= tcl_fltval(sim->interp,"bw",&sim->obs.bw);
  waserr |= tcl_intval(sim->interp,"nchan",&sim->obs.nchan);
  waserr |= tcl_intval(sim->interp,"symm",&sim->obs.symm);
  waserr |= tcl_fltval(sim->interp,"antsize",&sim->obs.antsize);
  waserr |= tcl_fltval(sim->interp,"tau",&sim->obs.tau);
  waserr |= tcl_fltval(sim->interp,"wave",&sim->obs.wave);
  waserr |= tcl_raval(sim->interp,"hastart",&sim->obs.hastart);
  waserr |= tcl_raval(sim->interp,"hastop",&sim->obs.hastop);
  waserr |= tcl_decval(sim->interp,"obslat",&sim->obs.lat);
  waserr |= tcl_raval(sim->interp,"obslong",&sim->obs.lng);
  waserr |= tcl_decval(sim->interp,"srcdec",&sim->obs.srcdec);
  waserr |= tcl_intval(sim->interp,"ngrid",&sim->obs.ngrid);
  waserr |= tcl_fltval(sim->interp,"cellsize",&sim->obs.cellsize);
  waserr |= tcl_intval(sim->interp,"isouv",&sim->obs.isouv); 
  waserr |= tcl_fltval(sim->interp,"xmax",&sim->obs.xmax); 
  /*
   * Ensure that the xrange is positive.
   */
  sim->obs.xmax = fabs(sim->obs.xmax);

  return waserr;
}
/*.......................................................................
 * Construct the Julian date from the system date.
 */
double getsysdate(void)
{
  FILE *fp=NULL;
  char defdate[] = "Fri Jan 1 00:00:00 GMT 1999"; /* Default date */
  char sysdate[100],day[5],month[5],code[5];
  int i,iyear,imon,iday,hr,min,sec,status;
  double mjd;
  static int nmon=12;
  static char *months[]={
    "jan",
    "feb",
    "mar",
    "apr",
    "may",
    "jun",
    "jul",
    "aug",
    "sep",
    "oct",
    "nov",
    "dec"
  };

  if((fp = popen("date -u","r"))==NULL) 
    fprintf(stderr,"Unable to get the system date.\n");
  else {
    if(fgets(sysdate,100,fp)==NULL) {
      fprintf(stderr,"Unable to get the system date.\n");
      strcpy(sysdate,defdate);
    }
  }
  /*
   * Close the pipe if it was opened.
   */
  if(fp)
    pclose(fp);
  /*
   * Read the relevant information out of the system date string.
   */
  sscanf(sysdate,"%s %s %d %d:%d:%d %s %d",day,month,&iday,&hr,&min,&sec,code,&iyear);
  /*
   * Figure out which month this is.
   */
  for(i=0;i < nmon;i++)
    if(strcasecmp(months[i],month)==0) {
      imon = i+1;
      break;
    }
  /*
   * Set a default if the month is nonsensical.
   */
  if(i==nmon) {
    fprintf(stderr,"No such month.\n");
    imon = 1;
  }
  slaCldj(iyear,imon,iday,&mjd,&status);
  /*
   * Add the fraction of a day since 0 UT and convert to JD
   */
  mjd += (double)((hr*60 + min)*60 + sec)/DAYSEC + 2400000.5;
  return mjd;
}  
/*.......................................................................
 * Plot an image
 */
int plotim(Simint *sim, Image *image, Optype op)
{  
  char *xlab=NULL,*ylab=NULL,*unit=NULL;
  float *data=NULL;
  int i,waserr=0;

  if(sim->interactive) {
    /*
     * Redraw the UV plot.
     */
    cpgslct(sim->uvplt_id);
    cpgsci(1);
    cpgask(0);
    cpgpage();
    cpgvstd();
    /*
     * Compose the labels.
     */
    for(i=0;i < nfitsaxes;i++) {
      if(fitsaxes[i].axis==image->header->ctypes[0])
	xlab=fitsaxes[i].label;
      if(fitsaxes[i].axis==image->header->ctypes[1])
	ylab=fitsaxes[i].label;
    }
    for(i=0;i < nfitsunits;i++) {
      if(fitsunits[i].bunit==image->header->bunit)
	unit=fitsunits[i].label;
    }
    /*
     * What operation was requested?
     */
    waserr |= (data = checkop(image, op))==NULL;
    
    if(!waserr)
      im_grey(image, data, xlab,ylab, 
	      image->header->telescope ? image->header->telescope : "Unknown",
	      unit);
    /*
     * Free any allocated memory allocated by checkop()
     */
    if(data)
      free(data);
  }  
  return waserr;
}
/*.......................................................................
 * Private function for plotim to check inputs and return the appropriate
 * data array.
 */
float *checkop(Image *image, Optype op)
{
  float *data=NULL;
  int i;
  /*
   * If any complex operators were requested, check that the image has
   * the right type.
   */
  if(op==OP_IM || op==OP_ABSIM) {
    if(image->header->type == T_CMPLX) {
      /*
       * If the imaginary array exists, proceed.
       */
      if(image->im) {
	if((data = (float *)malloc(image->header->n*sizeof(float)))==NULL) {
	  fprintf(stderr,"Unable to allocate array,\n");
	  return data;
	}
      }
      else {
	fprintf(stderr,"NULL imaginary array encountered.\n");
	return data;
      }
      /*
       * Now extract the appropriate array.
       */
      if(op==OP_IM)
	for(i=0;i < image->header->n;i++)
	  data[i] = image->im[i];
      else if(op==OP_ABSIM)
	for(i=0;i < image->header->n;i++)
	  data[i] = fabs(image->im[i]);
      return data;
    }
    else {
      fprintf(stderr,"Invalid operator for image of type: real.\n");
      return data;
    }
  }
  /*
   * Else check real operators.
   */
  if(op==OP_RE || op==OP_ABSRE) {
      /*
       * If the real array exists, proceed.
       */
    if(image->re) {
      if((data = (float *)malloc(image->header->n*sizeof(float)))==NULL) {
	fprintf(stderr,"Unable to allocate array,\n");
	return data;
      }
    }
    else {
      fprintf(stderr,"NULL real array encountered.\n");
      return data;
    }
    /*
     * Now extract the appropriate array.
     */
    if(op==OP_RE)
      for(i=0;i < image->header->n;i++)
	data[i] = image->re[i];
    else if(op==OP_ABSRE)
      for(i=0;i < image->header->n;i++)
	data[i] = fabs(image->re[i]);
    return data;
  }
  /*
   * Finally, check for any mixed operators.
   */
  else if(op==OP_POW) {
    if(image->header->type == T_CMPLX) {
      /*
       * If the real and imaginary array exists, proceed.
       */
      if(image->re && image->im) {
	if((data = (float *)malloc(image->header->n*sizeof(float)))==NULL) {
	  fprintf(stderr,"Unable to allocate array,\n");
	  return data;
	}
      }
      else {
	fprintf(stderr,"NULL array encountered.\n");
	return data;
      }
      /*
       * Now extract the appropriate array.
       */
      if(op==OP_POW)
	for(i=0;i < image->header->n;i++)
	  data[i] = sqrt(image->re[i]*image->re[i]+image->im[i]*image->im[i]);
    }
    else {
      fprintf(stderr,"Invalid operator for image of type: real.\n");
      return data;
    }
  }
  else {
    fprintf(stderr,"Unrecognized operator type.\n");
    return data;
  }
  return data;
}
/*.......................................................................
 * Redraw the antenna array and UV array, possibly with new X-limits.
 */
static int simint_redraw_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[])
{
  int waserr=0;
  /*
   * There should be 0 arguments.
   */
  waserr = getobs(sim);
  if(!waserr) {
    sim->xpa = sim->xpasave = -sim->obs.xmax;
    sim->xpb = sim->xpbsave = sim->obs.xmax;
    sim->ypa = sim->ypasave = -sim->obs.xmax;
    sim->ypb = sim->ypbsave = sim->obs.xmax;

    sim->upa = sim->upasave = 0.0;
    sim->upb = sim->upbsave = 0.0;
    sim->vpa = sim->vpasave = 0.0;
    sim->vpb = sim->vpbsave = 0.0;
  }
  /*
   * And redraw the XY frame.
   */
  if(draw_xyframe(sim))
    return TCL_ERROR;
  /*
   * The antennas.
   */
  if(drawants(sim))
    return TCL_ERROR;
  /*
   * And the visibilities.
   */
  if(compvis(sim,sim->obs.wave,sim->obs.lat,sim->obs.lng,sim->obs.hastart,
	     sim->obs.hastop,sim->obs.srcdec))
    return TCL_ERROR;

  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Observe an image
 */
static int obsim(Simint *sim, Image *image)
{  
  float *data=NULL,sig,x,y;
  int waserr=0,i,j,ind;
  int n[2];
  rfftwnd_plan plan=NULL;
  /*
   * First get various parameters.
   */
  waserr = getobs(sim);
  if((data = (float *)malloc(image->header->n*sizeof(float)))==NULL) {
    fprintf(stderr,"Unable to allocate work array.\n");
    return 1;
  }
  /*
   * Multiply the image by the primary beam and redraw the UV plot.
   *
   * We will have to modify this for the heterogeneous array case:
   * if we have n different size antennas, for each pair of different
   * antennas, we simply have to multiply the image by the geometric
   * mean of each pair of primary beams and transform to get the
   * visibilities for those baselines involving that pair.  This means
   * we will have to do n*(n-1)/2 transforms instead of 1.
   *
   * Take the PB to be a gaussian of FWHM = 1.22*lambda/D
   */      
  sig = 1.22*sim->obs.wave/sim->obs.antsize/(sqrt(8*log(2.0))); /* Radians */
  /*
   * Convert to whatever units the axes are in 
   */
  if(image->header->ctypes[0]==AX_DEG)
    sig *= RTOD;
  else if(image->header->ctypes[0]==AX_RAD);
  else {
    fprintf(stderr,"Unrecognized image axis units: assuming radians.\n");
    waserr = 1;
  }
  /*
   * Multiply by the primary beam
   */
  if(!waserr) {
    for(j=0;j < image->header->naxes_data[1];j++)
      for(i=0;i < image->header->naxes_data[0];i++) {
	ind = image->header->naxes_actual[0]*j + i;
	x = image->header->crvals[0] + (i-image->header->crpixs[0])*
	  image->header->cdelts[0];
	y = image->header->crvals[1] + (j-image->header->crpixs[1])*
	  image->header->cdelts[1];
	/*
	 * Store the original image data so we can recover it later.
	 */
	data[ind] = image->re[ind];
	image->re[ind] *= gauss2d(x,0,y,0,sig,sig,0.0);
      }
    /*
     * Plot this image.
     */
    if(!waserr) 
      waserr |= plotim(sim, image, OP_RE);
    /*
     * Now create a plan for the transform.
     */
    if((plan=rfftwnd_create_plan(2, n, FFTW_REAL_TO_COMPLEX, 
				 FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL)
      waserr = 1;
    /*
     * Restore the image and exit.
     */
  }
  /*
   * Free any allocated data. 
   */
  if(data)
    free(data);

  return waserr;
}
/*.......................................................................
 * Test the FFTW transform.  We will create an image, and compute the real 
 * transform.
 */
static int simint_ptsrc_command(Simint *sim, Tcl_Interp *interp,
					int argc, char *argv[])
{
  Image *image=NULL;
  int waserr=0,i,j;
  int n[2];
  rfftwnd_plan plan=NULL,plan_inv=NULL;
  Fitshead *header=NULL;
  int N;
  float theta;
  float dtheta;
  float fluxlim;
  int ntotal;
  double k,gamma; /* Parameters for the source distribution */
  float fwhm,sig;
  static int nseed=1000;
  static int seeds[1000],iseed;
  static int first=1;

  getobs(sim);

  if(first) {
    for(i=0;i < 1000;i++)
      seeds[i] = rand();
    first = 0;
  }

  k = atof(argv[0]);
  gamma = atof(argv[1]);
  theta = atof(argv[2])/180*M_PI; /* Central quadrant of the image will
				     be theta degrees on a side. */
  /*
   * The effective pixel scale will be the size of the central quadrant, divided 
   * by the size of the image.
   */
  N = atoi(argv[3]);
  dtheta = theta/(N/2);
  fwhm = atof(argv[4])/60/180*M_PI;
  sig = fwhm/sqrt(8.0*log(2.0));
  /*
   * The limiting flux (lowest) we will generate.
   */
  fluxlim = 1.0/pow((double)(N/2*N/2*1.0/(k*theta*theta)),1.0/gamma);
  /*
   * The total mean number of sources we will generate.
   */
  ntotal = k*theta*theta*pow(fluxlim, 1-gamma)/(gamma-1);

#ifdef DEBUG
  printf("Limiting flux = %f\tNsource = %d\n",fluxlim, ntotal);
#endif

  n[0] = N;
  n[1] = N;
  /*
   * Initialize a header.
   */
  waserr |= (header = new_Fitshead("Jy"))==NULL;
  if(!waserr) 
    header->naxis = 2;

  rd_str(&header->telescope,"SIMINT");
  /*
   * Allocate memory for the FITS axes of this header.
   */
  waserr |= new_Axes(header, 2);
  /*
   * the last dimension must contain 2 extra elements so we have
   * enough room to store the returned complex array.
   */
  header->naxis = 2;
  header->naxes_actual[0] = N+2;
  header->naxes_actual[1] = N;
  header->n = N*(N+2);
  header->naxes_data[0] = N+2;
  header->naxes_data[1] = N;
  header->type = T_CMPLX;
  
  
  header->cdelts[0] = dtheta;
  header->cdelts[1] = dtheta;
  /*
   * Center pixel should be the N/2 one.
   */
  header->crpixs[0] = N/2;
  header->crpixs[1] = N/2;

  header->crvals[0] = 0.0;
  header->crvals[1] = 0.0;
  
  header->bunit = BU_JY;  
  /*
   * Set the boundaries of the extreme edges of the array.
   */
  header->xmin = -N/2*dtheta;
  header->xmax =  N/2*dtheta;
  header->ymin = -N/2*dtheta;
  header->ymax =  N/2*dtheta;
  
  header->ctypes[0] = AX_RAD;
  header->ctypes[1] = AX_RAD;
  /*
   * These are fortran indices for pgplot, so they should be offset
   * from the indices we really want to plot by +1
   */
#if FALSE
  header->imin = 0;
  header->imax = N+2-1;
  header->jmin = 0;
  header->jmax = N-1;
#endif

  header->naxes_data[0] = N/2;
  header->naxes_data[1] = N/2;
  /*
   * This is the index of the start of the data block.
   */
  header->data_start[0] = N/4;
  header->data_start[1] = N/4;

  header->imin = N/4;
  header->imax = 3*N/4-1;
  header->jmin = N/4;
  header->jmax = 3*N/4-1;
  /*
   * Allocate a new image.
   */
#ifdef DMALLOC
  dmalloc_verify(0);
#endif
  image = imfind("ptsrc");
  if(image!=NULL) {
    image = rem_image(imindex(image));
    image = del_Image(image);
  }
  waserr = (image=new_Image("ptsrc", header))==NULL;
  /*
   * Set the number of indices back to the size of the array we will
   * actually use.
   */
/*  header->naxes_actual[1] = N; */
#ifdef DMALLOC
  dmalloc_verify(0);
#endif
  /*
   * Initialize the image to 0
   */
  {
    double arg;
    double x,y,xmid,ymid;
    xmid = header->xmin + N/2*header->cdelts[0];
    ymid = header->ymin + N/2*header->cdelts[1];

    for(i=N/4;i < 3*N/4;i++)
      for(j=N/4;j < 3*N/4;j++) {
	/*
	 * Get the coordinates of the center of this pixel.  The
	 * center is offset by 0.5*dx from the position obtained by,
	 * say xmin + dx*i, since xmin is the coordinate of the edge
	 * of the leftmost pixel.
	 */
	x = header->xmin + i*header->cdelts[0];
	y = header->ymin + j*header->cdelts[1];

	image->re[i*N+j] = 0.0;
	arg = -((x-xmid)*(x-xmid)+(y-ymid)*(y-ymid))/(2*sig*sig);
	image->im[i+j*(N+2)] = exp(arg);
	image->re[i+j*(N+2)] = 0.0;
      }
  }
#ifdef DEBUG
    waserr |= plotim(sim, image, OP_IM);
#endif
  /*
   * Now generate random sources in the central quadrant.
   */
  srand(seeds[(iseed++)%nseed]);
  for(i=0;i < ntotal;i++) {
    int irand,jrand;
    float frand,flux;
    /*
     * Generate a pixel value for this source.  These should be
     * integers between N/4-1 and 3N/4
     */
    irand = (int)((float)(rand())/RAND_MAX*N/2 + N/4)-1;
    jrand = (int)((float)(rand())/RAND_MAX*N/2 + N/4)-1;
    /*
     * Generate the flux of this source.
     */
    frand = (float)(rand())/RAND_MAX;
    flux = pow(1.0-frand,1.0/(1.0-gamma))*fluxlim;

    image->re[irand + jrand*(N+2)] += flux;
  }

#ifdef DMALLOC
  dmalloc_verify(0);
#endif

  /*
   * Plot the image, just to make sure.
   */
#if FALSE
  if(!waserr) {
    waserr |= plotim(sim, image, OP_RE);
    waserr |= plotim(sim, image, OP_IM);
  }
#endif
  /*
   * Now create a plan for the transform.
   */
   if((plan=rfftwnd_create_plan(2, n, FFTW_REAL_TO_COMPLEX, 
				FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL)
     waserr = 1;

   if((plan_inv=rfftwnd_create_plan(2, n, FFTW_COMPLEX_TO_REAL,
				FFTW_ESTIMATE | FFTW_IN_PLACE))==NULL)
     waserr = 1;
  /*
   * Compute the transform of the two images, multiply in the fourier domain,
   * and transform back to recover the convolution.
   */  
  if(!waserr) {
    (void) rfftwnd_one_real_to_complex(plan, (fftw_real *)image->re, NULL);
    (void) rfftwnd_one_real_to_complex(plan, (fftw_real *)image->im, NULL);
    {
      int ind;
      float re1,im1,re2,im2;
      float re,im,scale = 1.0/(N*N);
      fftw_complex *complx1 = (fftw_complex *)image->re;
      fftw_complex *complx2 = (fftw_complex *)image->im;

      for(i=0;i < N;i++)
	for(j=0;j < N/2+1;j++) {
	  ind = i*(N/2+1) + j;

	  re1 = complx1[ind].re;
	  im1 = complx1[ind].im;
	  re2 = complx2[ind].re;
	  im2 = complx2[ind].im;

	  re = re1*re2 - im1*im2;
	  im = im1*re2 + im2*re1;
	  /*
	   * We multiply every other element by -1 so that the resulting 
	   * transform is back in the center of the image.
	   */
	  complx1[ind].re = re*scale*(i%2==0 ? -1 : 1)*(j%2==0 ? -1 : 1);
	  complx1[ind].im = im*scale*(i%2==0 ? -1 : 1)*(j%2==0 ? -1 : 1);
	}
    }
    (void)rfftwnd_one_complex_to_real(plan_inv, (fftw_complex *)image->re, NULL);
  }
#ifdef DEBUG
  for(i=0;i < image->header->naxes_actual[0];i++)
    for(j=0;j < image->header->naxes_actual[1];j++)
      image->re[i+image->header->naxes_actual[0]*j] = 0.0;

    image->re[image->header->naxes_actual[0]/2 + image->header->naxes_actual[1]/2*image->header->naxes_actual[0]] = 1.0;
#endif
  /*
   * Plot the convolution.
   */
  if(!waserr) 
    waserr |= plotim(sim, image, OP_RE);
  /*
   * And delete the new image.
   */
/* del_Image(image); */
  add_image(image);
  /*
   * And install the image as the default.
   */
  if(!waserr)
    sim->image = image;
  /*
   * Destroy the fftw plans we created in this routine.
   */
  (void) rfftwnd_destroy_plan(plan);
  (void) rfftwnd_destroy_plan(plan_inv);

  return waserr;
}
/*.......................................................................
 * Read a fits file.
 *
 * Input:
 *  iname    char  *  The internal name of the image.
 *  filename char  *  The name of the fits file.
 */
static int simint_rfits_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[])
{
  int waserr = 0;  
  char *filename=NULL;
  char *iname=NULL;
  char* units=NULL;
  FILE *fp=NULL;
  Image *image=NULL;
  Fitshead *header=NULL;
  int realcoord = 0;
  
  // There should be 3 arguments.

  if(argc != 3) {
    Tcl_AppendResult(interp, "Usage: filename name\n", NULL);
    return TCL_ERROR;
  };
  
  // See what sort of coordinate axes were requested, if any.

  realcoord = 1;

  // Get the units

  units = argv[2];


  // Get the image name

  iname = argv[1];
  
  // Get the fits file name

  filename = argv[0];
  
  // Check to see that we can open the file.

  if((fp = fopen(filename,"r"))==NULL) {
    fprintf(stderr,"Unable to open file: %s\n",filename);
    waserr = 1;
  }
  else {
    
    // Initialize a header.

    waserr |= (header = new_Fitshead(units))==NULL;
    
    // Get the relevant header items.

    if(!waserr) 
      waserr = getheader(fp, header, sim);
    
    // Locate the image in the list. If it already exists, delete that
    // descriptor and create a new image.

    if(!waserr) {
      image = imfind(iname);
      if(image != NULL) {
	image = rem_image(imindex(image));
	image = del_Image(image);
      }
      waserr = (image=new_Image(iname, header))==NULL;
    }
    
    // If the necessary items were present, read the data.

    if(!waserr) 
      waserr = readfits(fp, image, realcoord);
    
    if(!waserr) {
      fprintf(stdout,"Image size is: %d x %d\n",image->header->naxes_actual[0],
	      image->header->naxes_actual[1]);
      
      // If successful, add the image to the list of known images.

      add_image(image);
    }
    else
      del_Image(image);
  }
  
  // And install the image as the default.

  if(!waserr)
    sim->image = image;
  
  // And plot the image.

  if(!waserr)
    waserr |= plotim(sim, image, OP_RE);
  
  // Close the file.

  if(fp)
    fclose(fp);
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Install an image from the list of known images as the default image.
 *
 * Input:
 *  iname    char  *  The internal name of the image.
 */
static int simint_imslct_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[])
{
  char *iname=NULL;
  Image *image=NULL;
  int waserr=0;
  /*
   * There should be 1 argument.
   */
  if(argc != 1) {
    Tcl_AppendResult(interp, "Usage: imname\n", NULL);
    return TCL_ERROR;
  };
/*
 * Get the image name
 */
  iname = argv[0];
  /*
   * If this is the null image, reset the current image selection to none.
   */
  if(strcmp(iname,"none")==0) {
    sim->image = NULL;
    return TCL_OK;
  }
/*
 * Locate the image in the list. If it doesn't yet exist, complain.
 */
  image = imfind(iname);
  if(image == NULL) {
    fprintf(stderr,"No such image: %s.\n",iname);
    return TCL_ERROR;
  }
  sim->image = image;
  /*
   * If the necessary items were present, read the data.
   */
  /*
   * And plot the image.
   */
  if(!waserr)
    waserr |= plotim(sim, image, OP_RE);

  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Observe and image from the list of known images.
 *
 * Input:
 *  iname    char  *  The internal name of the image.
 */
static int simint_obsim_command(Simint *sim, Tcl_Interp *interp,
				int argc, char *argv[])
{
  char *iname=NULL;
  Image *image=NULL;
  int waserr=0;
  /*
   * There should be 1 argument.
   */
  if(argc != 1) {
    Tcl_AppendResult(interp, "Usage: imname\n", NULL);
    return TCL_ERROR;
  };
  /*
   * Get the image name
   */
  iname = argv[0];
  /*
 * Locate the image in the list. If it doesn't yet exist, complain.
 */
  image = imfind(iname);
  if(image == NULL) {
    fprintf(stderr,"No such image: %s.\n",iname);
    return TCL_ERROR;
  }
  sim->image = image;
  /*
   * And plot the image.
   */
  if(!waserr)
    waserr |= obsim(sim, image);
  
  return waserr ? TCL_ERROR : TCL_OK;
}
/*.......................................................................
 * Function to copy a FITS header 
 */
int copy_header(Fitshead *dest, Fitshead *src)
{
  int i;

  if(src->naxes_data==NULL || src->naxes_actual==NULL || src->crotas==NULL || src->cdelts==NULL 
     || src->crpixs==NULL || src->crvals==NULL || src->ctypes==NULL)
    return 1;
  if(dest->naxes_data==NULL || dest->naxes_actual==NULL || dest->crotas==NULL || dest->cdelts==NULL 
     || dest->crpixs==NULL || dest->crvals==NULL || dest->ctypes==NULL)
    return 1;
  if(dest->naxis != src->naxis) {
    fprintf(stderr, "Image containers have different sizes.\n");
    return 1;
  }

  for(i=0;i < src->naxis;i++) {
    dest->naxes_actual[i] = src->naxes_actual[i];
    dest->crpixs[i] = src->crpixs[i];
    dest->crvals[i] = src->crvals[i];
    dest->cdelts[i] = src->cdelts[i];
    dest->crotas[i] = src->crotas[i];
    dest->ctypes[i] = src->ctypes[i];
  }
  dest->xmin = src->xmin;
  dest->xmax = src->xmax;
  dest->ymin = src->ymin;
  dest->ymax = src->ymax;

  dest->bunit = src->bunit;
  /*
   * Copy strings.
   */
  if(src->telescope)
    rd_str(&dest->telescope,src->telescope);
  if(src->instrument)
    rd_str(&dest->instrument,src->instrument);

  dest->n = 1;
  for(i=0;i < dest->naxis;i++)
    dest->n *= dest->naxes_actual[i];

  return 0;
}
/*.......................................................................
 * Function to convert from an image header to its transformed UV header.
 */
int fill_uvheader(Fitshead *header)
{
  float dx,dy;

  if(header->naxes_actual==NULL || header->naxes_data==NULL || header->crotas==NULL || header->cdelts==NULL 
     || header->crpixs==NULL || header->crvals==NULL || header->ctypes==NULL)
    return 1;
  
  /*
   * Compute du and dv from the specified cellsize.
   */
  dx = (header->xmax-header->xmin)/(header->naxes_data[0]-1);
  dy = (header->ymax-header->ymin)/(header->naxes_data[1]-1);
  /*
   * If the axis type is not radians, convert to radians.
   */
  if(header->ctypes[0]==AX_DEG)
    dx /= RTOD;
  else if(header->ctypes[0]==AX_RAD);
  else
    fprintf(stderr,"Unknown axis type: assuming radians.\n");

  if(header->ctypes[1]==AX_DEG)
    dy /= RTOD;
  else if(header->ctypes[1]==AX_RAD);
  else
    fprintf(stderr,"Unknown axis type: assuming radians.\n");

  /*
   * And set the header ctypes to UV.
   */
  header->ctypes[0] = AX_U;
  header->ctypes[1] = AX_V;
  /*
   * Convert to du and dv
   */
  header->cdelts[0] = 1.0/(2*(header->naxes_data[0]-1)*dx);
  header->cdelts[1] = 1.0/(2*(header->naxes_data[1]-1)*dy);
  /*
   * Fill the axis information.
   */
  header->crvals[0] = 0.0;
  header->crvals[1] = 0.0;

  header->crpixs[0] = (header->naxes_data[0]/2)+0.5;
  header->crpixs[1] = (header->naxes_data[1]/2)+0.5;
  
  header->xmin = -1.0/(2*dx);
  header->xmax = 1.0/(2*dx);
  header->ymin = -1.0/(2*dy);
  header->ymax = 1.0/(2*dy);
  
  header->n = header->naxes_actual[0]*header->naxes_actual[1];

  return 0;
}

