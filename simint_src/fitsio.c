#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "fitsio.h"
#include "tksimint.h"
#include "image.h"
#include "vis.h"

/* #define DEBUG  */
/* #define PTSRC */
/*
 * Functions of the following type are called by put_phdu() to convert a passed
 * string value to the appropriate type.
 */
typedef int (*WRITEFN)(char *, char *);
/*
 * Functions of the following type are called by read_phdu() to convert a passed
 * string value to the appropriate type.
 */
typedef int (*READFN)(void *, char *);
/* 
 * Declare a container for a single header card.
 */
typedef struct {
  char name[NKEY+1];
  READFN  readfn;
  WRITEFN writefn;
  int required;
} Phdu;
/*....................................................................... 
 * Enumerate valid Header cards.
 */
Phdu phdus[] = {
  {"ALTRVAL", rd_flt, flt_str, 0},
  {"ALTRPIX", rd_flt, flt_str, 0},
  {"AUTHOR",  rd_str, str_str, 0},
  {"BITPIX",  rd_int, int_str, 1},
  {"BLOCKED", rd_log, log_str, 0},
  {"BSCALE",  rd_flt, flt_str, 0},
  {"BZERO",   rd_flt, flt_str, 0},
  {"BUNIT",  rd_bunit,str_str, 0},
  {"CDELT",   rd_flt, flt_str, 1},
  {"COMMENT", rd_str, str_str, 0},
  {"CROTA",   rd_flt, flt_str, 1},
  {"CRPIX",   rd_flt, flt_str, 1},
  {"CRVAL",   rd_flt, flt_str, 1},
  {"CTYPE",   rd_axis,str_str, 0},
  {"DATAMAX", rd_flt, flt_str, 0},
  {"DATAMIN", rd_flt, flt_str, 0},
  {"EXTEND",  rd_log, log_str, 0},
  {"END",     rd_null,null_str,1},
  {"EXTNAME", rd_str, str_str, 0},
  {"EXTVER",  rd_int, int_str, 0},
  {"EXTLEVEL",rd_int, int_str, 0},
  {"INSTRUME",rd_str, str_str, 0},
  {"GCOUNT",  rd_int, int_str, 0},
  {"GROUPS",  rd_log, log_str, 0},
  {"DATE",    rd_str, str_str, 0},
  {"DATE-MAP",rd_str, str_str, 0},
  {"DATE-OBS",rd_str, str_str, 0},
  {"EQUINOX", rd_flt, flt_str, 1},
  {"EXTNAME", rd_str, str_str, 0},
  {"EXTVER",  rd_int, int_str, 0},
  {"NAXIS",   rd_int, int_str, 1},
  {"OBJECT",  rd_str, str_str, 0},
  {"OBSRA",   rd_flt, flt_str, 0},
  {"OBSDEC",  rd_flt, flt_str, 0},
  {"ORIGIN",  rd_str, str_str, 0},
  {"PCOUNT",  rd_int, int_str, 0},
  {"PSCAL",   rd_flt, flt_str, 0},
  {"PTYPE",   rd_str, str_str, 0},
  {"PZERO",   rd_flt, flt_str, 0},
  {"RESTFREQ",rd_flt, flt_str, 0},
  {"SIMPLE",  rd_log, log_str, 0},
  {"TBCOL",   rd_int, int_str, 0},
  {"TELESCOP",rd_str, str_str, 0},
  {"TFIELDS", rd_int, int_str, 0},
  {"TFORM",   rd_str, str_str, 0},
  {"TSCAL",   rd_flt, flt_str, 0},
  {"TTYPE",   rd_str, str_str, 0},
  {"TUNIT",   rd_str, str_str, 0},
  {"TZERO",   rd_flt, flt_str, 0},
  {"VELREF",  rd_flt, flt_str, 0},
  {"XTENSION",rd_str, str_str, 0},
  {"NO_IF"   ,rd_int, int_str, 0},
  {" "       ,rd_null,null_str,0},
};
/*
 * Define the number of recognized header keywords.
 */
int nphdu = sizeof(phdus)/sizeof(Phdu);

extern Axiscard fitsaxes[];
extern Bunitcard fitsunits[];
extern int nfitsaxes;
extern int nfitsunits;

static int write_fqentry(FILE *fp, Simint *sim);
static int write_tabentry(FILE *fp, Ant *ant);

/*.......................................................................
 * Test writing out a random groups FITS file of the current antenna
 * positions and visibilities (these will be randomly generated amplitudes
 * and phases.
 */
int writevis(FILE *fp, Header *hdr, Simint *sim)
{
  int waserr=0;

#ifdef DEBUG
  if(fp)
    fclose(fp);
  fp = fopen("test_hdr.data","w"); 
#endif

  waserr = write_phdu(fp, sim, hdr);

#ifdef DEBUG
  if(fp)
    fclose(fp);
  fp = fopen("test_data.data","w"); 
#endif

  if(sim->obs.isouv)
    waserr = write_vis(fp, sim, T_UVF, NULL); 
  else
    waserr = write_sepvis(fp, sim, T_UVF, NULL); 

#ifdef DEBUG
  if(fp)
    fclose(fp);
  fp = fopen("test_table.data","w"); 
#endif
  waserr = write_antable(fp, sim); 
  
  return waserr;
}
/*.......................................................................
 * Test writing out a random groups FITS file of the current antenna
 * positions and visibilities (these will be randomly generated amplitudes
 * and phases.
 */
int new_writevis(FILE *fp, Header *hdr, Simint *sim)
{
  int waserr=0;

#ifdef DEBUG
  if(fp)
    fclose(fp);
  fp = fopen("test_hdr.data","w"); 
#endif

  waserr = write_phdu(fp, sim, hdr);

#ifdef DEBUG
  if(fp)
    fclose(fp);
  fp = fopen("test_data.data","w"); 
#endif

  waserr |= new_write_vis(fp, sim, T_UVF); 

#ifdef DEBUG
  if(fp)
    fclose(fp);
  fp = fopen("test_table.data","w"); 
#endif
  waserr |= write_antable(fp, sim); 
  waserr |= write_fqtable(fp, sim); 
  
  return waserr;
}
/*.......................................................................
 * Test writing out a random groups FITS file of the current antenna
 * positions and visiblities (these will be randomly generated amplitudes
 * and phases.
 */
int writedat(FILE *fp, Header *hdr, Simint *sim, Dtype type)
{
  int waserr=0;
  waserr = write_vis(fp, sim, type, NULL);

  return waserr;
};
/*.......................................................................
 * Put all the necessary header cards in the UVF files.
 */
int write_phdu(FILE *fp, Simint *sim, Header *hdr)
{
  int waserr=0,nhdu=0,nres,i;
  char kval[100];

  if(!(waserr =  put_phdu(fp,"SIMPLE","T","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"BITPIX","-32","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"NAXIS","7","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"NAXIS1","0","No standard image, just groups")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"NAXIS2","3","Complex visibilities: real, imag, wt")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"NAXIS3","1","Stokes")))
    ++nhdu;
  /*
   * This next is the number of correlator channels; this is the number of channels
   * per IF.
   */
  if(!(waserr |= put_phdu(fp,"NAXIS4","1","Correlator channels")))
    ++nhdu;
  sprintf(kval,"%d",hdr->nchan);
  if(!(waserr |= put_phdu(fp,"NAXIS5",kval,"IFs")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"NAXIS6","1","RA")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"NAXIS7","1","DEC")))
    ++nhdu;
  /*
   * The EXTEND card must appear immediately after the last NAXISn keyword,
   * or after the NAXIS keyword if NAXIS=0.
   */  
  if(!(waserr |= put_phdu(fp,"EXTEND","T","This is the antenna table.")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"BLOCKED","T","Tape may be blocked.")))
    ++nhdu;
    if(!(waserr |= put_phdu(fp,"OBJECT","SIMINT","Source name")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"TELESCOP",sim->array->label,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"INSTRUME",sim->array->label,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"DATE-OBS",hdr->date,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"DATE-MAP",hdr->date,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"BSCALE","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"BZERO","0.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"BUNIT","UNCALIB","")))
    ++nhdu;
  /*
   * Write the source ra and dec.
   */
  if(!(waserr |= put_phdu(fp,"EQUINOX","2000","")))
    ++nhdu;

  if(!(waserr |= put_phdu(fp,"VELREF","257","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"ALTRVAL","0.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"ALTRPIX","1.0","")))
    ++nhdu;

  sprintf(kval,"%f",hdr->obsra*RTOD);
  if(!(waserr |= put_phdu(fp,"OBSRA",kval,"")))
    ++nhdu;
  sprintf(kval,"%f",hdr->obsdec*RTOD);
  if(!(waserr |= put_phdu(fp,"OBSDEC",kval,"")))
    ++nhdu;

  sprintf(kval,"%2.5e",hdr->freq*1e9);
  if(!(waserr |= put_phdu(fp,"RESTFREQ",kval,"")))
    ++nhdu;

  if(!(waserr |= put_phdu(fp,"CTYPE2","COMPLEX","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRVAL2","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CDELT2","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRPIX2","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CROTA2","0.0","")))
    ++nhdu;
  /*
   * Write the Bogus Stokes data.
   */
  if(!(waserr |= put_phdu(fp,"CTYPE3","STOKES","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRVAL3","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CDELT3","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRPIX3","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CROTA3","0.0","")))
    ++nhdu;
  /*
   * Write the Frequency data.
   */
  if(!(waserr |= put_phdu(fp,"CTYPE4","FREQ","")))
    ++nhdu;
  sprintf(kval,"%2.5e",hdr->freq*1e9);
  if(!(waserr |= put_phdu(fp,"CRVAL4",kval,"")))
    ++nhdu;
  sprintf(kval,"%2.5e",hdr->dfreq*1e9);
  if(!(waserr |= put_phdu(fp,"CDELT4",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRPIX4","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CROTA4","0.0","")))
    ++nhdu;
  /*
   * Write the IF data.
   */
  if(!(waserr |= put_phdu(fp,"CTYPE5","IF","")))
    ++nhdu;
  sprintf(kval,"%2.5e",hdr->freq*1e9);
  if(!(waserr |= put_phdu(fp,"CRVAL5",kval,"")))
    ++nhdu;
  sprintf(kval,"%2.5e",hdr->dfreq*1e9);
  if(!(waserr |= put_phdu(fp,"CDELT5",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRPIX5","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CROTA5","0.0","")))
    ++nhdu;


  if(!(waserr |= put_phdu(fp,"CTYPE6","RA","")))
    ++nhdu;
  sprintf(kval,"%f",hdr->obsra*RTOD);
  if(!(waserr |= put_phdu(fp,"CRVAL6",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CDELT6","0.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRPIX6","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CROTA6","0.0","")))
    ++nhdu;

  if(!(waserr |= put_phdu(fp,"CTYPE7","DEC","")))
    ++nhdu;
  sprintf(kval,"%f",hdr->obsdec*RTOD);
  if(!(waserr |= put_phdu(fp,"CRVAL7",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CDELT7","0.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CRPIX7","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"CROTA7","0.0","")))
    ++nhdu;
  /*
   * Random groups information
   */
  if(!(waserr |= put_phdu(fp,"GROUPS","T","")))
    ++nhdu;
  /*
   * This next is the number of *groups*, not visibilities!
   */
  sprintf(kval,"%d",hdr->nvis/hdr->nchan);
  if(!(waserr |= put_phdu(fp,"GCOUNT",kval,"Total no. of groups")))
    ++nhdu;

  if(!(waserr |= put_phdu(fp,"PCOUNT","6","Random parameters for each group")))
    ++nhdu;
  /* 
   * Characterize the first parameter for the random group data format -- u
   */
  if(!(waserr |= put_phdu(fp,"PTYPE1","UU---SIN","Baseline u projection, (seconds)")))
    ++nhdu;

  sprintf(kval,"%e",1.0/Uvscale);
  if(!(waserr |= put_phdu(fp,"PSCAL1",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PZERO1","0.00000000000E+00","")))
    ++nhdu;
  /* 
   * Characterize the second parameter for the random group data format -- v
   */
  if(!(waserr |= put_phdu(fp,"PTYPE2","VV---SIN","Baseline v projection, (seconds)")))
    ++nhdu;
  sprintf(kval,"%e",1.0/Uvscale);
  if(!(waserr |= put_phdu(fp,"PSCAL2",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PZERO2","0.00000000000E+00","")))
    ++nhdu;
  /* 
   * Characterize the third parameter for the random group data format -- w
   */
  if(!(waserr |= put_phdu(fp,"PTYPE3","WW---SIN","Baseline w projection, (seconds)")))
    ++nhdu;
  sprintf(kval,"%e",1.0/Uvscale);
  if(!(waserr |= put_phdu(fp,"PSCAL3",kval,"")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PZERO3","0.00000000000E+00","")))
    ++nhdu;
  /*
   * Write the baseline crap here.
   */
  if(!(waserr |= put_phdu(fp,"PTYPE4","BASELINE","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PSCAL4","1.0","")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PZERO4","0.00000000000E+00","")))
    ++nhdu;
  /*
   * Write a bogus Julian date here.
   */
  if(!(waserr |= put_phdu(fp,"PTYPE5","DATE","Julian Date 1")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PSCAL5","1.0","days")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PZERO5","0.00000000000E+00","")))
    ++nhdu;
  /*
   * Continue the bogus date here.
   */
  if(!(waserr |= put_phdu(fp,"PTYPE6","DATE","Julian Date 2")))
    ++nhdu;
  sprintf(kval,"%e",1.0/DAYSEC);  
  if(!(waserr |= put_phdu(fp,"PSCAL6",kval,"seconds")))
    ++nhdu;
  if(!(waserr |= put_phdu(fp,"PZERO6","0.00000000000E+00","")))
    ++nhdu;
  /*
   * Finish the ASCI header with the END keyword.
   */ 
  if(!(waserr |= put_phdu(fp,"END","","")))
    ++nhdu;
  /*
   * Buffer the header up to the next integral number of logical units.
   */ 
  if(nhdu%36 > 0)
    nres = 36 - nhdu%36;
  else
    nres = 0;
  
  for(i=0;i < nres;i++) 
    waserr |= put_phdu(fp," ","","");

 return waserr;
}
/*.......................................................................
 * Write a header key to a file.
 */
int put_phdu(FILE *fp, char *name, char *val, char *comment)
{
 int waserr=0,i,j;
 char buf[NHEAD+1]; /* Standard 80 byte FITS buffer. */
 int nbuf=0;

 for(i=0;i <= NHEAD;i++)
   buf[i] = '\0';
 
 for(i=0;i < nphdu;i++) 
   if(strstr(name,phdus[i].name)!=NULL) {
     /*
      * Put the header name in first.
      */
     sprintf(buf,"%-8s",name);
     /* 
      * Then format the value argument (if any)
      */
     waserr = phdus[i].writefn(buf,val);
     /*
      * lastly, tack on an optional comment
      */
     strcat(buf,comment); 
     /*
      * And buffer with blanks to the header length.
      */
     /*
      * And buffer with blanks to the header length.
      */
     nbuf = strlen(buf);
     for(j=nbuf;j < NHEAD;j++)
       buf[j] = ' ';

     waserr |= (fputs(buf,fp)==0);

     return waserr;
   }

 fprintf(stderr,"Unrecognized keyword: %s\n",name);
 return 1;
}
/*
 * A do-nothing function for null value header keys.
 */
int null_str(char *buf, char *val)
{
  return 0;
}
/*.......................................................................
 * Convert a float to a string.
 */
int flt_str(char *buf, char *val)
{
  char valbuf[NHEAD-NKEY];

  sprintf(valbuf, "= %20.10E / ",atof(val));
  strcat(buf,valbuf);

  return 0;
}
/*.......................................................................
 * Convert an integer to a string.
 */
int int_str(char *buf, char *val)
{
  char valbuf[NHEAD-NKEY];

  sprintf(valbuf, "= %20d / ",atoi(val));
  strcat(buf,valbuf);

  return 0;
}
/*.......................................................................
 * Print a logical argument in column 30.
 */
int log_str(char *buf, char *val)
{
  char valbuf[NHEAD-NKEY];

  sprintf(valbuf, "= %20s / ",val);
  strcat(buf,valbuf);

  return 0;
}
/*.......................................................................
 * Print a string.
 */
int str_str(char *buf, char *val)
{
  char valbuf[NHEAD-NKEY];
  int i,nval;

  sprintf(valbuf, "= '%-8s'",val);
  /*
   * Pad with blanks out to the comment space.
   */
  nval = strlen(valbuf);
  for(i=0;i < 23-nval;i++) 
    valbuf[nval+i] = ' ';
  
  valbuf[nval+i] = '/';
  valbuf[nval+i+1] = ' ';
  valbuf[nval+i+2] = '\0';

  strcat(buf,valbuf);
  return 0;
}
/*.......................................................................
 * A do-nothing read function for null value header keys.
 */
int rd_null(void *ptr, char *string)
{
  return 0;
}
/*.......................................................................
 * Convert a string to a float argument.
 */
int rd_flt(void *ptr, char *string)
{
  sscanf(string, "%f", (float *)ptr);

  return 0;
}
/*.......................................................................
 * Convert a string to an integer argument.
 */
int rd_int(void *ptr, char *string)
{
  sscanf(string,"%d",(int *)ptr);

  return 0;
}
/*.......................................................................
 * Read a logical argument.
 */
int rd_log(void *ptr, char *string)
{
  if(strcmp(string,"T")==0)
    *((int *)ptr) = 1;
  else
    *((int *)ptr) = 0;

  return 0;
}
/*.......................................................................
 * Read a string.
 */
int rd_str(void *ptr, char *string)
{
  if(string != NULL) {
    if((*((char **)ptr) = (char *)malloc(strlen(string+1)*sizeof(char)))==NULL) {
      fprintf(stderr,"Unable to allocate string.\n");
      return 1;
    }
    strcpy(*((char **)ptr),string);
  }
  return 0;
}
/*.......................................................................
 * Read a Bunit derived type.
 */
int rd_bunit(void *ptr, char *string)
{
  int i;
  for(i=0;i < nfitsunits;i++) 
    if(strstr(string,fitsunits[i].string)!=NULL) {
      *((Bunit *)ptr) = fitsunits[i].bunit;
      break;
    }
  /*
   * If no match was found, set the bunit to the last bunit card (unknown).
   */
  if(i==nfitsunits)
    *((Bunit *)ptr) = BU_UNKNOWN;

  return 0;
}
/*.......................................................................
 * Read an Axis derived type.
 */
int rd_axis(void *ptr, char *string)
{
  int i;
  for(i=0;i < nfitsaxes;i++) 
    if(strstr(string,fitsaxes[i].string)!=NULL) {
      *((Axis *)ptr) = fitsaxes[i].axis;
      break;
    }
  /*
   * If no match was found, set the axes to the last axis card (unknown)
   */
  if(i==nfitsaxes)
    *((Axis *)ptr) = AX_UNKNOWN;

  return 0;
}
/*.......................................................................
 * Create a new header object.
 */
Header *new_Header(Simint *sim)
{
  Header *hdr=NULL;
  int waserr=0,nint;
  FILE *cp=NULL;
#ifdef PTSRC
  static int counter=0;
#endif

  if((hdr=(Header *)malloc(sizeof(Header)))==NULL)
    return hdr;
  if((hdr->date=(char *)malloc(sizeof(char)*9))==NULL) {
    free(hdr);
    return NULL;
  }
  if((cp = popen("date '+%d/%m/%y%n'","r"))==NULL) {
    fprintf(stderr,"Unable to get a date from the system.\n");
    free(hdr);
    return NULL;
  }

  waserr = getobs(sim);

  if(!waserr) {
    fgets(hdr->date,9,cp);
    hdr->date[8] = '\0';
  /*
   * Fake the source ra
   */
#ifndef PTSRC
    hdr->obsra = 0.0;
    hdr->obsdec = sim->obs.srcdec;
#else
    hdr->obsdec = -60.0*M_PI/180;
    hdr->obsra = 1.5*(counter%8)*M_PI/12;
    counter++;
#endif


    /*
     * Use the lowest frequency in the bandpass.
     */
    hdr->nchan = sim->obs.nchan;
    hdr->dfreq = sim->obs.bw/sim->obs.nchan;
    /*
     * Note that the center of the lowest channel is offset from the lowest
     * detected frequency by 0.5*dnu
     */
    hdr->freq = C/1e9/sim->obs.wave - sim->obs.nchan*hdr->dfreq/2 + 
      hdr->dfreq/2;
  /*
   * Figure out how many visibilities we will be writing out.
   */
    if(sim->obs.hastart==sim->obs.hastop)
      nint = 1;
    else
      nint = (sim->obs.hastop-sim->obs.hastart)/(sim->obs.tint/RTOS);
  /*
   * Nvis will be the nbaselines x nchan x nint
   */
    hdr->nvis = (sim->nant*(sim->nant-1))/2 * sim->obs.nchan * nint;
    
    if(cp)
      fclose(cp);
  }
  return hdr;
}
/*.......................................................................
 * Delete a header object.
 */
Header *del_Header(Header *hdr)
{
  if(hdr==NULL)
    return hdr;
  if(hdr->date)
    free(hdr->date);
  free(hdr);
  hdr = NULL;

  return hdr;
}
/*.......................................................................
 * Return the string equivalent of a declination in radians.
 */
char *decstring(double dec, char *string)
{
  /* Assumes dec is in radians */

  int deg,min;
  int sec;
  int sdec;
  int remain;
  int isneg = 0;

  sdec = dec*RTOAS;          /* dec in arcseconds */

  if(sdec < 0) {
    sdec *= -1;
    dec *= -1;
    isneg = 1;
  }

  remain = (dec*RTOAS-sdec)*100;
  
  deg = sdec/3600;
  min = (sdec%3600)/60;
  sec = sdec-60*(deg*60 + min);

  sprintf(string, "%c%02d:%02d:%02d.%02d",(isneg ? '-' : ' '),deg,min,sec,remain);

  return string;
}
/*.......................................................................
 * Return the string equivalent of a right ascension in radians.
 */
char *rastring(double fra, char *string)
{
  /* Assumes ra is in radians */

  int ra;
  int hr,min;
  int sec;
  int remain;

/* Convert to seconds */

  while(fra > 2*M_PI)
    fra -= 2*M_PI;

  ra = fra*RTOS;
  remain = (fra*RTOS - ra)*100;

  hr = ra/3600;
  min = (ra%3600)/60;
  sec = ra-60*(hr*60 + min);

  sprintf(string, "%02d:%02d:%02d.%02d",hr,min,sec,remain);

  return string;
}
/*.......................................................................
 * Write a FITS antenna table for the current antenna configuration.
 */
int write_antable(FILE *fp, Simint *sim)
{  
  int waserr=0,nhdu=0,nres,i;
  char kval[100];
  Ant *ant=NULL;
  char blank = '\0';

  if(!(waserr =  put_phdu(fp,"XTENSION","TABLE","EXTENSION TYPE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"BITPIX","8","PRINTABLE ASCII CODES")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"NAXIS","2","TABLE IS A MATRIX")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"NAXIS1","80","WIDTH OF TABLE IN CHARACTERS")))
    ++nhdu;
  /*
   * Write the number of antennas.
   */
  sprintf(kval,"%d",sim->nant);
  if(!(waserr =  put_phdu(fp,"NAXIS2",kval,"NUMBER OF ENTRIES IN TABLE")))
    ++nhdu;
   if(!(waserr =  put_phdu(fp,"PCOUNT","0","NO RANDOM PARAMETERS")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"GCOUNT","1","GROUP COUNT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFIELDS","5","NUMBER OF FIELDS PER ROW")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"EXTNAME","AIPS AN","AIPS ANTENNA TABLE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"EXTVER","1","VERSION NUMBER OF TABLE")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TBCOL1","1","STARTING COLUMN")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFORM1","I3","FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE1","ANT NO.","ANTENNA NUMBER")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TBCOL2","7","STARTING COLUMN")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFORM2","A8","FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE2","STATION","ANTENNA NAME")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TBCOL3","15","STARTING COLUMN")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFORM3","D20.10","FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE3","LX","ANTENNA X COORDINATE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT3","METERS","PHYSICAL UNITS")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TSCAL3","1.0","")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TZERO3","0.0","")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TBCOL4","35","STARTING COLUMN")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFORM4","D20.10","FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE4","LY","ANTENNA Y COORDINATE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT4","METERS","PHYSICAL UNITS")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TSCAL4","1.0","")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TZERO4","0.0","")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TBCOL5","55","STARTING COLUMN")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFORM5","D20.10","FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE5","LZ","ANTENNA Z COORDINATE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT5","METERS","PHYSICAL UNITS")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TSCAL5","1.0","")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TZERO5","0.0","")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"END","","")))
    ++nhdu;
  /*
   * Buffer the header up to the next integral number of logical units.
   */  
  nres = 36 - nhdu%36;
  
  for(i=0;i < nres;i++) 
    waserr |= put_phdu(fp," ","","");
  /*
   * Now write the actual table.  We'll arrange for the table entries to be
   * the same length as the phdu record, so increment the counter by the number
   * of antennas on success.
   */
  nhdu = 0;
  for(i=0,ant=sim->ants;ant != NULL;ant = ant->next,i++) {
    waserr |= write_tabentry(fp,ant);
    nhdu++;
  }
  /*
   * Buffer the header up to the next integral number of logical units.
   * These are supposed to be ASCII blanks.
   */  
  nres = 2880 - (nhdu*80)%2880;
  nres = nres/sizeof(char);
  /* 
   * Fill fitsbuf with blanks
   */
  for(i=0;i < nres;i++)
    fwrite((const void *)(&blank),sizeof(char),1,fp);

  return waserr;
}
/*.......................................................................
 * Write a single table entry antenna table for the current antenna configuration.
 */
static int write_tabentry(FILE *fp, Ant *ant)
{  
  char anname[6];

  sprintf(anname,"AN%d",ant->iant+1);
  /*
   * Write the antenna designation and position (in meters)
   */
  return (fprintf(fp,"%3d  %8s %20.10E%20.10E%20.10E      ",ant->iant+1,anname,ant->X/100,ant->Y/100,ant->Z/100) < 0);
}
/*.......................................................................
 * Read relevant items out of a FITS file header
 *
 * Output:
 *  The number of bytes read.
 *
 */
int getheader(FILE *fp, Fitshead *header, Simint *sim)
{
  unsigned char phdu[NHEAD];
  char key[NKEY+1],*cptr=NULL;
  int waserr=0,nhdu=0,go=1,iaxis,i;
  /*
   * Read header units until the END keyword is encountered.
   */
  for(nhdu=0;go && !waserr && fread(phdu,sizeof(unsigned char),NHEAD,fp) > 0;
      nhdu++) {
    /*
     * Extract the keyword from this header unit.
     */
    cptr = phdu;
    for(i=0;i < NKEY && isalnum((int) *cptr);i++,cptr++)
      key[i] = *cptr;
    key[i] = '\0';

    if(strcmp(key,"END")==0)
      go = 0;
    else
      strtok(phdu,"=");

    if(strcmp(key,"BITPIX")==0)
      waserr |= rd_int(&header->bitpix,strtok(NULL,"/"));
    else if(strcmp(key,"NAXIS")==0) {
      waserr |= rd_int(&header->naxis,strtok(NULL,"/"));
      if(header->naxis==0) {
	fprintf(stderr,"Not an image file (NAXIS = 0).\n");
	waserr = 1;
      }
      /*
       * Allocate memory for the new axes.
       */
      if(!waserr)
	waserr |= new_Axes(header,header->naxis);
    }
    else if(strstr(key,"NAXIS")!=NULL) {
      iaxis = atoi(&key[5])-1;
      waserr |= rd_int(&header->naxes_actual[iaxis],strtok(NULL,"/"));
      header->naxes_data[iaxis] = header->naxes_actual[iaxis];
    }
    else if(strstr(key,"CRPIX")!=NULL) {
      iaxis = atoi(&key[5])-1;
      waserr |= rd_flt(&header->crpixs[iaxis],strtok(NULL,"/")); 
    }
    else if(strstr(key,"CDELT")!=NULL) {
      iaxis = atoi(&key[5])-1;
      waserr |= rd_flt(&header->cdelts[iaxis],strtok(NULL,"/"));
      header->cdelts[iaxis] = fabs(header->cdelts[iaxis]);
    }
    else if(strstr(key,"CROTA")!=NULL) {
      iaxis = atoi(&key[5])-1;
      waserr |= rd_flt(&header->crotas[iaxis],strtok(NULL,"/"));
    }
    else if(strstr(key,"CRVAL")!=NULL) {
      iaxis = atoi(&key[5])-1;
      waserr |= rd_flt(&header->crvals[iaxis],strtok(NULL,"/")); 
      header->crvals[iaxis] = 0.0;
    }
    /*
     * String values will be enclosed by single quotes "'"
     */
    else if(strstr(key,"CTYPE")!=NULL) {
      iaxis = atoi(&key[5])-1;
      strtok(NULL,"'");
      waserr |= rd_axis(&header->ctypes[iaxis],strtok(NULL,"'"));
    }
    else if(strcmp(key,"BSCALE")==0) 
      waserr |= rd_flt(&header->bscale,strtok(NULL,"/"));
    else if(strcmp(key,"BUNIT")==0) {
      strtok(NULL,"'");
      waserr |= rd_bunit(&header->bunit,strtok(NULL,"'"));
    }
    else if(strcmp(key,"BZERO")==0) 
      waserr |= rd_flt(&header->bzero,strtok(NULL,"/"));
    else if(strcmp(key,"DATAMIN")==0) 
      waserr |= rd_flt(&header->bscale,strtok(NULL,"/"));
    else if(strcmp(key,"DATAMAX")==0) 
      waserr |= rd_flt(&header->bscale,strtok(NULL,"/"));
    else if(strcmp(key,"TELESCOP")==0) {
      strtok(NULL,"'");
      waserr |= rd_str(&header->telescope,strtok(NULL,"'"));
    }
    else if(strcmp(key,"INSTRUME")==0) {
      strtok(NULL,"'");
      waserr |= rd_str(&header->instrument,strtok(NULL,"'"));
    }
  }
  /*
   * Read to the beginning of the data record.
   */
  {
    int nres;
    unsigned char buf[NDATREC];

    nres = ((nhdu*NHEAD)%2880==0) ? 0 : 2880 - (nhdu*NHEAD)%2880;
    fread(buf,sizeof(unsigned char),nres,fp);
  }
  /*
   * Set the min/max pixel display to the data range.
   */  
  header->imin = 0;
  header->imax = header->naxes_data[0]-1;

  header->jmin = 0;
  header->jmax = header->naxes_data[1]-1;

  header->crpixs[0] = (header->naxes_actual[0]-1)/2;
  header->crpixs[1] = (header->naxes_actual[1]-1)/2;

  header->crvals[0] = 0.0;
  header->crvals[1] = 0.0;
  /*
   * If no axis type was encountered, install a fake one and update
   * the cellsize from the user-settable variable
   */
  for(i=0;i < header->naxis;i++) {
    if(header->ctypes[i]==AX_UNKNOWN) {
      waserr = getobs(sim);
      if(!waserr) {
	header->ctypes[i]=AX_RAD;
	header->cdelts[i]=sim->obs.cellsize/206265;
	/*
	 * Print a warning
	 */
	fprintf(stderr,"Warning: No axis type was encountered.  Defaulting to radians and %f (rad) cellsize\n",header->cdelts[i]);
      }
    }
  }
  /*
   * Check the unit
   */
  if(header->bunit==BU_UNKNOWN) {
    fprintf(stdout, "No unit encountered: defaulting to: %s\n", 
	    printUnits(header->bunit));
  }

  return waserr;
}
/*.......................................................................
 * Read the data out of a fits file and transfer it to an image structure.
 * Assumes the FILE pointer is pointing to the top of the data (as at the end
 * of getheader() ).
 */
int readfits(FILE *fp, Image *image, int realcoord)
{
  int n,j,nread;
  short *sarray=NULL;
  long *larray=NULL;
  unsigned char *carray=NULL;
  float rev,*tmp=NULL;
  int waserr = 0;
  /*
   * At this point, the FILE pointer points to the first data element.
   *
   * Now allocate the arrays we'll need to read the data into.  
   */
  n = image->header->naxes_actual[0]*image->header->naxes_actual[1];

  if(image->header->bitpix==16) {
    waserr = (sarray = (short *)malloc(n*sizeof(short)))==NULL;
  }
  else if(image->header->bitpix==8) {
    waserr = (carray = (unsigned char *)malloc(n*sizeof(unsigned char)))==NULL;
  }
  else if(image->header->bitpix==32) {
    waserr = (larray = (long *)malloc(n*sizeof(long)))==NULL;
  }
  if(!waserr) {
    /*
     * If BITPIX was -32, these are normal 4-byte floats.
     */
    if(image->header->bitpix == -32)
      nread = fread(image->re,sizeof(float),n,fp);
    /*
     * Else these are longs.
     */
    else if(image->header->bitpix==32) {
      fread(larray,sizeof(long),n,fp);
      for(j=0;j < image->header->naxes_actual[0]*image->header->naxes_actual[1];j++) 
	image->re[j] = (float)larray[j];
    }
    /*
     * If BITPIX was 16, these are shorts.
     */
    else if(abs(image->header->bitpix) == 16) {
      fread(sarray,sizeof(short),n,fp);
      for(j=0;j < image->header->naxes_actual[0]*image->header->naxes_actual[1];j++) 
	image->re[j] = (float)sarray[j];
    }
    /*
     * If BITPIX was 8, these are unsigned chars.
     */
    else if(abs(image->header->bitpix) == 8) {
      fread(carray,sizeof(unsigned char),n,fp);
      for(j=0;j < image->header->naxes_actual[0]*image->header->naxes_actual[1];j++) 
	image->re[j] = (float)carray[j];
    }
    else {
      fprintf(stderr,"Unrecognized BITPIX: %d\n",image->header->bitpix);
      waserr = 1;
    }
    /*
     * Always scale by bscale and bzero.
     */
    for(j=0;j < image->header->naxes_actual[0]*image->header->naxes_actual[1];j++) {

#ifdef linux_i486_gcc
      if(image->header->bitpix == 32 || image->header->bitpix == -32) {
	cp_4r4((unsigned char *)&rev, (unsigned char *)&image->re[j], 1);
	tmp = &rev;
      }
#else
      tmp = &image->re[j];
#endif
      image->re[j] = (*tmp)*image->header->bscale + image->header->bzero;
 
      /*
       * Test for NaNs.
       */
      if(isnanf(image->re[j]))
	image->re[j] = 0.0;
      /* 
       * Store the data min/max.
       */
      if(j==0)
	image->header->datamin = image->header->datamax = image->re[0];
      image->header->datamin = image->re[j] < image->header->datamin ?
	image->re[j] : image->header->datamin;
      image->header->datamax = image->re[j] > image->header->datamax ?
	image->re[j] : image->header->datamax;
    }
  }
  /*
   * Set derived members in the header structure.
   */
  image->header->n = n;
  image->header->xmin = (-image->header->crpixs[0])*
	    image->header->cdelts[0]+image->header->crvals[0];
  image->header->xmax = (image->header->naxes_actual[0]-image->header->crpixs[0])*
	    image->header->cdelts[0]+image->header->crvals[0];
  image->header->ymin = (-image->header->crpixs[1])*
	    image->header->cdelts[1]+image->header->crvals[1];
  image->header->ymax = (image->header->naxes_actual[1]-image->header->crpixs[1])*
	    image->header->cdelts[1]+image->header->crvals[1];
  /*
   * Flip around any silly axes.
   */
  {
    float temp;
    if(image->header->xmin > image->header->xmax) {
      temp = image->header->xmax;
      image->header->xmax = image->header->xmin;
      image->header->xmin = temp;
    }
    if(image->header->ymin > image->header->ymax) {
      temp = image->header->ymax;
      image->header->ymax = image->header->ymin;
      image->header->ymin = temp;
    }

  }
  /*
   * Free any allocated memory.
   */  
  if(larray)
    free(larray);
  if(sarray)
    free(sarray);
  if(carray)
    free(carray);

  return waserr;
}
/*.......................................................................
 * Allocate memory for the axes of a fits header.
 */
int new_Axes(Fitshead *header, int naxis) 
{
  int i,waserr = 0;

  waserr|=(header->naxes_data = (int *)malloc(naxis*sizeof(int)))==NULL;
  waserr|=(header->data_start = (int *)malloc(naxis*sizeof(int)))==NULL;
  waserr|=(header->naxes_actual = (int *)malloc(naxis*sizeof(int)))==NULL;
  waserr|=(header->crvals = (float *)malloc(naxis*sizeof(float)))==NULL;
  waserr|=(header->crpixs = (float *)malloc(naxis*sizeof(float)))==NULL;
  waserr|=(header->cdelts = (float *)malloc(naxis*sizeof(float)))==NULL;
  waserr|=(header->crotas = (float *)malloc(naxis*sizeof(float)))==NULL;
  waserr|=(header->ctypes = (Axis *)malloc(naxis*sizeof(Axis)))==NULL;

  if(!waserr)
    for(i=0;i < naxis;i++) {
      header->naxes_actual[i] = 0;
      header->naxes_data[i] = 0;
      header->data_start[i] = 0;
      header->crvals[i] = 0;
      header->crpixs[i] = 0;
      header->cdelts[i] = 1;
      header->crotas[i] = 0;
      header->ctypes[i] = AX_UNKNOWN;
    }
  return waserr;
}
/*.......................................................................
 * Write a FITS FQ table for the IFs
 */
int write_fqtable(FILE *fp, Simint *sim)
{  
  int waserr=0,nhdu=0,nres,i;
  char kval[100];
  char blank = '\0';

  sprintf(kval,"%d",sim->obs.nchan);

  if(!(waserr =  put_phdu(fp,"XTENSION","A3DTABLE","EXTENSION TYPE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"BITPIX","8","PRINTABLE ASCII CODES")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"NAXIS","2","TABLE IS A MATRIX")))
    ++nhdu;
  /*
   * The entry for each correlator will be:
   * 4 bytes for the designation
   * 8 bytes for the frequency
   * 8 bytes for the single-channel bandwidth
   * 8 bytes for the total bandwidth
   * 4 bytes for the sideband.
   * = 32 bytes per record
   */
  sprintf(kval,"%d",sim->obs.nchan*32);

  if(!(waserr =  put_phdu(fp,"NAXIS1",kval,"Width of table row in bytes")))
    ++nhdu;
  /*
   * Write the correlator information
   */
  sprintf(kval,"%d",sim->obs.nchan);

  if(!(waserr =  put_phdu(fp,"NAXIS2","1","NUMBER OF ROWS")))
    ++nhdu;
   if(!(waserr =  put_phdu(fp,"PCOUNT","0","NO RANDOM PARAMETERS")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"GCOUNT","1","GROUP COUNT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TFIELDS","5","NUMBER OF FIELDS PER ROW")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"EXTNAME","AIPS FQ","AIPS FQ TABLE")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"EXTVER","1","VERSION NUMBER OF TABLE")))
    ++nhdu;
  /*
   * Frequency ids are written as 32-bit ints (J)
   */
  sprintf(kval,"%dJ",sim->obs.nchan);
  if(!(waserr =  put_phdu(fp,"TFORM1",kval,"FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE1","FRQSEL","IF NUMBER")))
    ++nhdu;
  /*
   * Frequency data are written as 64-bit doubles (D)
   */
  sprintf(kval,"%dD",sim->obs.nchan);
  if(!(waserr =  put_phdu(fp,"TFORM2",kval,"FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE2","IF FREQ","IF Frequency")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT2","HZ","PHYSICAL UNITS")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TFORM3",kval,"FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE3","CH WIDTH","BANDWIDTH")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT3","HZ","PHYSICAL UNITS")))
    ++nhdu;

  if(!(waserr =  put_phdu(fp,"TFORM4",kval,"FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE4","TOTAL BANDWIDTH","BANDWIDTH")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT4","HZ","PHYSICAL UNITS")))
    ++nhdu;
  /*
   * Sidebands are written as 32-bit ints (J)
   */
  sprintf(kval,"%dJ",sim->obs.nchan);
  if(!(waserr =  put_phdu(fp,"TFORM5",kval,"FORTRAN FORMAT")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TTYPE5","SIDEBAND","")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"TUNIT5","","PHYSICAL UNITS")))
    ++nhdu;

  sprintf(kval,"%d",sim->obs.nchan);
  if(!(waserr =  put_phdu(fp,"NO_IF",kval,"")))
    ++nhdu;
  if(!(waserr =  put_phdu(fp,"END","","")))
    ++nhdu;
  /*
   * Buffer the header up to the next integral number of logical units.
   */  
  nres = 36 - nhdu%36;
  
  for(i=0;i < nres;i++) 
    waserr |= put_phdu(fp," ","","");
  /*
   * Now write the actual table.  We'll arrange for the table entries to be
   * the same length as the phdu record, so increment the counter by the number
   * of antennas on success.
   */
  waserr |= write_fqentry(fp, sim);
  /*
   * Buffer the table up to the next integral number of logical units.
   * These are supposed to be ASCII blanks.
   *
   * on success, write_fqentry() will have written 32 x sim->obs.nchan bytes
   */  
  nres = NDATREC - (sim->obs.nchan*32)%NDATREC;
  nres = nres/sizeof(char);
  /* 
   * Fill fitsbuf with blanks
   */
  for(i=0;i < nres;i++)
    fwrite((const void *)(&blank),sizeof(char),1,fp);

  return waserr;
}
/*.......................................................................
 * Write a single table entry antenna table for the current antenna 
 * configuration.
 */
static int write_fqentry(FILE *fp, Simint *sim)
{  
  int i;
  int waserr=0;
  double dnu;
  int icorr,irev,*tmp=NULL;
  /*
   * Write the correlator ids
   */
  dnu = sim->obs.bw/sim->obs.nchan;

  for(icorr=0;icorr < sim->obs.nchan;icorr++) {
#ifdef linux_i486_gcc
	    cp_4r4((unsigned char *)&irev, (unsigned char *)&icorr, 1);
	    tmp = &irev;
#else
	    tmp = &icorr;
#endif
    waserr |= (fwrite(tmp,sizeof(int),1,fp) != 1);
  }
  /*
   * Write the correlator frequencies, wrt to the first frequency in the header.
   */
  for(i=0;i < sim->obs.nchan;i++) {
    double freq,freqrev,*tmp;
    freq = i*dnu*1e9;
#ifdef linux_i486_gcc
	    cp_8r8((unsigned char *)&freqrev, (unsigned char *)&freq, 1);
	    tmp = &freqrev;
#else
	    tmp = &freq;
#endif
    waserr |= (fwrite(tmp,sizeof(double),1,fp) != 1);
  }
  /*
   * Write the correlator channel widths
   */
  for(i=0;i < sim->obs.nchan;i++) {
    double bw,bwrev,*tmp;
    bw = dnu*1e9;
#ifdef linux_i486_gcc
	    cp_8r8((unsigned char *)&bwrev, (unsigned char *)&bw, 1);
	    tmp = &bwrev;
#else
	    tmp = &bw;
#endif
    waserr |= (fwrite(tmp,sizeof(double),1,fp) != 1);
  }
  /*
   * Write the correlator frequencies.
   */
  for(i=0;i < sim->obs.nchan;i++) {
    double bw,bwrev,*tmp;
    bw = dnu*1e9;
#ifdef linux_i486_gcc
	    cp_8r8((unsigned char *)&bwrev, (unsigned char *)&bw, 1);
	    tmp = &bwrev;
#else
	    tmp = &bw;
#endif
    waserr |= (fwrite(tmp,sizeof(double),1,fp) != 1);
  }
  /*
   * Write the correlator sidebands
   */
  for(i=0;i < sim->obs.nchan;i++) {
    int side=1,siderev,*tmp;
#ifdef linux_i486_gcc
	    cp_4r4((unsigned char *)&siderev, (unsigned char *)&side, 1);
	    tmp = &siderev;
#else
	    tmp = &side;
#endif
    waserr |= (fwrite(tmp,sizeof(int),1,fp) != 1);
  }
  return waserr;
}
/*.......................................................................
 * Orig: 4-byte datatype.
 * Dest: 4-byte datatype with byte order reversed.
 */
void cp_4r4(unsigned char *dest, unsigned char *orig, size_t nitem)
{
  size_t i;
  for(i=0; i<nitem; i++, orig+=4, dest+=4) {
    dest[0] = orig[3];
    dest[1] = orig[2];
    dest[2] = orig[1];
    dest[3] = orig[0];
  };
}
/*.......................................................................
 * Orig: 8-byte datatype.
 * Dest: 8-byte datatype with byte order reversed.
 */
void cp_8r8(unsigned char *dest, unsigned char *orig, size_t nitem)
{
  size_t i;
  for(i=0; i<nitem; i++, orig+=8, dest+=8) {
    dest[0] = orig[7];
    dest[1] = orig[6];
    dest[2] = orig[5];
    dest[3] = orig[4];
    dest[4] = orig[3];
    dest[5] = orig[2];
    dest[6] = orig[1];
    dest[7] = orig[0];
  };
}
