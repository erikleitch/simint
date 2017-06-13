#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "tksimint.h"
#include "cpgplot.h"
#include "color_tab.h"
#include "vplot.h"
#include "fourier.h"
#include "ctype.h"

/*.......................................................................
 * Plot Image data
 *
 * Input: 
 *  ferret    Ferret *  The ferret to be plotted.
 *  xmem        Dmem *  The xmember.
 *  ymem        Dmem *  The ymember.
 *  zmem        Dmem *  The zmember.
 *  nx           int    The number of points to use in x.
 *  ny           int    The number of points to use in y.
 *  z1         float    The foreground greyscale.
 *  z2         float    The background greyscale.
 * Output:
 *  return    int       0 - OK.
 */
int im_grey(Image *image, float *zdata, char *xlab, char *ylab, char *title, 
	    char *unit)
{
  int nx=image->header->naxes_actual[0], ny=image->header->naxes_actual[1];
  float xmins=image->header->xmin,xmaxs=image->header->xmax;
  float ymins=image->header->ymin,ymaxs=image->header->ymax;
  float z1=0,z2=0;
  float *flag=NULL;
  float xmin,xmax,dx;
  float ymin,ymax,dy;
  float xzmin,xzmax,yzmin,yzmax;
  float zmins, zmaxs;
  int i,j;
  char answer[100];
  int docurs=0;
  int first = 1;
  float tr[6];
  int slen=100;
  int i1,i2,j1,j2,nbin;
  float xtemp,ytemp,xpos[2],ypos[2];
  static char *mess1="\n %c - Select start and end vertices using this key\n";
  static char *mess2=" %c - Select the whole plot\n";
  static char *mess3=" %c - Abort selection\n";
  int cancel,dofull,accepted;
  int iter;
  char key = 'L';
  int oldcol;
  float bright=0.5,contra=-1.0;
  int read = 1;
  Cmap *grey, *rain, *cmap, *heat;
  int n,exc;
  float mean,sd,min,max;
  float x,y,rad,r1,r2,r,h;
  float xmid,ymid;
  float sig;

  enum {                      /* Function keys in the gaphical interface */
    G_CUR   = 'A',
    G_FLG   = 'B',            /* Toggle display flagged data. */
    G_CUT   = 'C',
    G_CAN   = 'D',
    G_DEF   = 'D',

    G_FID   = 'F',
    G_GREY  = 'G',
    G_HELP  = 'H',
    G_INS   = 'I',
    G_FIT   = 'J',
    G_RAD   = 'K',
    G_DIS   = 'L',

    G_OVRPLT= 'O',
    G_COPY  = 'P',
    G_HEAT  = 'Q',
    G_RAIN  = 'R',
    G_STAT  = 'S',

    G_HORI  = 'U',
    G_VERT  = 'V',

    G_QUIT  = 'X',
    G_YSC   = 'Y',
    G_ZOOM  = 'Z',
  };	    

  for(i=0;i < n_std_cmap;i++) {
    if(strcmp(std_cmaps[i].name,"grey")==0) grey = &std_cmaps[i];
    if(strcmp(std_cmaps[i].name,"rainbow")==0) rain = &std_cmaps[i];
    if(strcmp(std_cmaps[i].name,"heat")==0) heat = &std_cmaps[i];
  };
  /*
   * Set the indices to be displayed
   */
  i1 = image->header->imin+1;
  j1 = image->header->jmin+1;
  i2 = image->header->imax+1;
  j2 = image->header->jmax+1;
  /*
   * Set the displayed plot limits.
   */
  dx = image->header->cdelts[0];
  dy = image->header->cdelts[1];
  /*
   * Get the min/max of the data to be displayed only.
   */
  xmin = xmins+(i1-1)*dx;
  xmax = xmins+i2*dx;
  ymin = ymins+(j1-1)*dy;
  ymax = ymins+j2*dy;
  /*
   * Only compute greyscale limits for displayed data.
   */
  if(z1==z2){
    float x,y;
    int first=1,ind;

    for(i=image->header->imin;i <= image->header->imax;i++) {
      for(j=image->header->jmin;j <= image->header->jmax;j++) {
	x = xmins+i*dx;
	y = ymins+j*dy;
	ind = i+j*image->header->naxes_actual[0];
	if(x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
	  if(first) {
	    zmins = zmaxs = zdata[ind];
	    first = 0;
	  }
	  zmins = MIN(zmins, zdata[ind]);
	  zmaxs = MAX(zmaxs, zdata[ind]);
	}
      }
    }
  }
  /*
   * Else default to the user-requested range.
   */
  else {
    zmins = z1;
    zmaxs = z2;
  }
  /*
   * Set a non-zero default range if the data have no dynamic range.
   */
  if(zmins==zmaxs) {
    /*
     * Ensure non-zero limits, so pgplot won't complain.
     */
    if(zmins == 0.0) {
      zmins = -1;
      zmaxs = 1;
    }
    else {
      zmins -= 0.1*zmins;
      zmaxs += 0.1*zmaxs;
    }
  }
  
  xmid = (xmaxs+xmins)/2;
  ymid = (ymaxs+ymins)/2;
  /*
   * Set the transformation matrix for the data array.
   *
   * The center in x of each cell in the array is at:
   *
   *  tr[0] + i*tr[1]
   * 
   * Since we will pass xmins and xmaxs as the extreme boundaries of
   * the array (i.e., these mark the values at the *edges* of the
   * extreme pixels, we want to increment xmins by dx/2 to find the
   * _centers_ of the pixels.
   *
   * Since the indices are fortran indices, we also need to subtract
   * one from each axis to get the right center.  
   */
  tr[0]=xmins+dx/2-dx;
  tr[1]=dx;
  tr[2]=0.0;
  tr[3]=ymins+dy/2-dy;
  tr[4]=0.0;
  tr[5]=dy;
  /*
   * Do we have a cursor?  
   */
  cpgqinf("CURSOR", answer, &slen);
  /*  docurs = strncmp(answer,"YES",3) == 0; */
  docurs = 1;

  cpgswin(xmin-dx,xmax+dx,ymin-dy,ymin+dy);

  cmap = heat;
  /*
   * Zoom limits.
   */
  xzmin = xmin;
  xzmax = xmax;
  yzmin = ymin;
  yzmax = ymax;

  if(docurs) {
    printf("For HELP, hit the \'%c\' key on your keyboard\n", G_HELP);
    do {
      cancel = 0;
      switch(key) {
      case G_CUR:
	cpgqci(&oldcol);
	cpgsci(1);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_LINE, 0, xmid, ymid, &xtemp,&ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
/*
 * Distance from the origin.
 */
	rad = sqrt((xmid-xpos[0])*(xmid-xpos[0])+(ymid-ypos[0])*(ymid-ypos[0]));
/*
 * Width of the filter.
 */
	sig = 0.5*sqrt((xpos[0]-xpos[1])*(xpos[0]-xpos[1])+(ypos[0]-ypos[1])*(ypos[0]-ypos[1]));
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      r = sqrt((xmid-x)*(xmid-x)+(ymid-y)*(ymid-y));
	      h = 1.0/(1+(r*sig/(r*r-rad*rad))*(r*sig/(r*r-rad*rad)));
	      flag[i+nx*j] = exc ? h : -(h-1);
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
      case G_CUT:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband((iter==0 ? B_NORM : B_LINE), 0, xtemp, ytemp, &xtemp, &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	sig = 2*sqrt((xpos[0]-xpos[1])*(xpos[0]-xpos[1])+(ypos[0]-ypos[1])*(ypos[0]-ypos[1]));
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      r1 = sqrt((x-xpos[0])*(x-xpos[0])+(y-ypos[0])*(y-ypos[0]));
	      r2 = sqrt((2*xmid-x-xpos[0])*(2*xmid-x-xpos[0])+(2*ymid-y-ypos[0])*(2*ymid-y-ypos[0]));
	      h = 1.0/(1+(sig/r1))*1.0/(1+(sig/r2));
	      flag[i+nx*j] = h;
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
	/*
	 * Make a radial plot of the image.
	 */
      case G_RAD:
	cpgqci(&oldcol);
	cpgsci(1);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_LINE, 0, xmid, ymid, &xtemp,&ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	/*
	 * Distance from the origin.
	 */
	rad = sqrt((xmid-xpos[0])*(xmid-xpos[0])+
		   (ymid-ypos[0])*(ymid-ypos[0]));
	/*
	 * Use a number of radial bins proportional to the distance (max will
	 * be the native image resolution, ie, ngrid/2
	 */
	nbin = (int)(nx/2*rad/sqrt((xmax-xmid)*(xmax-xmid) + (ymax-ymid)*(ymax-ymid)));
	v_radplot(zdata,nbin,xmin,xmax,nx,ymin,ymax,ny);
	break;
      case G_YSC:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<1 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_YRNG, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	sig = fabs(ypos[0]-ymid);
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      y = ymins+dy*j;
	      if(fabs(y-ymid) <= sig)
		flag[i+nx*j] = 0;
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
      case G_HORI:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<1 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_XRNG, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	sig = fabs(xmid-xpos[0]);
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      if(fabs(x-xmid) <= sig)
		flag[i+nx*j] = 0;
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
      case G_FIT:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<1 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_LINE, 0, xmid, ymid, &xtemp, &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	rad = sqrt((xpos[0]-xmid)*(xpos[0]-xmid)+(ypos[0]-ymid)*(ypos[0]-ymid));
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      r = sqrt((x-xmid)*(x-xmid)+(y-ymid)*(y-ymid));
	      h = 1.0/(1+(rad/r)*(rad/r));
	      flag[i+nx*j] = exc ? h : -(h-1);
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
	/*
	 * Zoom the plot
	 */
      case G_ZOOM:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband((iter==0) ? B_NORM : B_RECT, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_ZOOM:
	      accepted = dofull = 1;
	      break;
	    case G_CAN:      /* Abort box selection */
	      accepted = cancel = 1;
	      break;
	    case G_QUIT:     /* Quit now */
	      cpgend();
	      free(zdata);
	      return 0;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	if(dofull) {
	  xzmin = xmin;
	  yzmin = ymin;
	  xzmax = xmax;
	  yzmax = ymax;
	}
	else {
	  /*
	   * Only reverse the boundaries if the "min" and "max" are contrary
	   * to the sense of dx and dy.
	   */
	  if((xpos[0] < xpos[1] && dx > 0.0) || 
	     (xpos[0] > xpos[1] && dx < 0.0)) {
	    xzmin = xpos[0];
	    xzmax = xpos[1];
	  }
	  else {
	    xzmin = xpos[1];
	    xzmax = xpos[0];
	  }
	  if((ypos[0] < ypos[1] && dy > 0.0) || 
	     (ypos[0] > ypos[1] && dy < 0.0)){
	    yzmin = ypos[0];
	    yzmax = ypos[1];
	  }
	  else {
	    yzmin = ypos[1];
	    yzmax = ypos[0];
	  }
	}
	/*
	 * Recompute the midpoint of the displayed image.
	 */
	xmid = (xzmin+xzmax)/2;
	ymid = (yzmin+yzmax)/2;
	/*
	 * Compute the new greyscale boundaries
	 */
	if(z1==z2){
	  float x,y;
	  int first=1,ind;

	  for(i=i1-1;i < i2;i++)
	    for(j=j1-1;j < j2;j++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      ind = i+j*image->header->naxes_actual[0];

	      if(x >= xzmin && x <= xzmax && y >= yzmin && y <= yzmax) {
		if(first) {
		  zmins = zmaxs = zdata[ind];
		  first = 0;
		}
		zmins = MIN(zmins, zdata[ind]);
		zmaxs = MAX(zmaxs, zdata[ind]);
	      }
	    }
	}
	cpgswin(xzmin-dx,xzmax+dx,yzmin-dy,yzmax+dy);
	cpgsci(oldcol);
      case G_DIS:
	if(!cancel) {
	  cpgpage();
	  cpgvstd();
	  cpgswin(0,1,0,1);
	  cpgwnad(0,1,0,1); 
	  cpgswin(xzmin-dx,xzmax+dx,yzmin-dy,yzmax+dy);
	  cpgwnad(xzmin-dx,xzmax+dx,yzmin-dy,yzmax+dy);
	  cpgbbuf();

	  cpgctab(cmap->l,cmap->r,cmap->g,cmap->b,cmap->n,contra,bright);
#if FALSE
	  {
	    int c1,c2;
	    cpgqcir(&c1,&c2);
	    fprintf(stdout,"C1: %d C2: %d.\n",c1,c2);
	  }
#endif
	  if(zmins==zmaxs) {
	    zmins -= 0.1*zmins;
	    zmaxs += 0.1*zmaxs;

	    if(zmins==0.0) {
	      zmins = -1;
	      zmaxs = 1;
	    }
	  }

	  cpgimag(zdata,nx,ny,i1,i2,j1,j2,zmaxs,zmins,tr);
	  cpgsci(1);
	  cpgbox("BCNST",0.0,0,"BCNST",0.0,0);
	  cpglab(xlab,ylab,title);
	  /*
	   * Draw a ramp on the side.
	   */
	  cpgwedg("RI",0,4,zmaxs,zmins,unit); 
	  cpgebuf();
	};
	break;
	/*
	 * Compute statistics on a selected region of the plot.
	 */
      case G_STAT:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband((iter==0) ? B_NORM : B_RECT, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_STAT:
	      accepted = dofull = 1;
	      break;
	    case G_CAN:      /* Abort box selection */
	      accepted = cancel = 1;
	      break;
	    case G_QUIT:     /* Quit now */
	      cpgend();
	      free(zdata);
	      return 0;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	if(dofull) {
	  xpos[0] = xmins;
	  ypos[0] = ymins;
	  xpos[1] = xmaxs;
	  ypos[1] = ymaxs;
	}
	/*
	 * Here we want xpos[0] and xpos[1], etc. to be the absolute 
	 * minimum and maximum, since we test if a data point falls between 
	 * these values.
	 */
	if(xpos[0] > xpos[1]) {
	  xtemp = xpos[1];
	  xpos[1] = xpos[0];
	  xpos[0] = xtemp;
	}
	if(ypos[0] > ypos[1]) {
	  ytemp = ypos[1];
	  ypos[1] = ypos[0];
	  ypos[0] = ytemp;
	}
	mean = 0.0;
	n = 0;
	/*
	 * Only compute the statistics over the displayed indices, and
	 * the requested range.
	 */
	{
	  int ind;
	  for(i=i1-1;i < i2;i++) 
	    for(j=j1-1;j < j2;j++) {
	      xtemp = xmins + dx/2 + i*dx;
	      ytemp = ymins + dy/2 + j*dy;
	      ind = i + j*nx;
	      if(xtemp >= xpos[0] && xtemp <= xpos[1] && ytemp >= ypos[0] && 
		 ytemp <= ypos[1]) {
		if(first) {
		  min = max = zdata[ind];
		  first = 0;
		}
		min = MIN(zdata[ind],min);
		max = MAX(zdata[ind],max);
		mean += (zdata[ind] - mean)/(n+1);
		++n;
	      }
	    }
	  sd = 0.0;
	  n = 0;
	  for(i=i1-1;i < i2;i++) 
	    for(j=j1-1;j < j2;j++) {
	      xtemp = xmins + dx/2 + i*dx;
	      ytemp = ymins + dy/2 + j*dy;
	      ind = i + j*nx;
	      if(xtemp >= xpos[0] && xtemp <= xpos[1] && ytemp >= ypos[0] && 
		 ytemp <= ypos[1]) {
		sd += ((zdata[ind] - mean)*(zdata[ind]-mean) - sd)/(n+1);
		++n;
	      }
	    }
	}
	/*
	 * Compute the sd from the variance.
	 */
	if(n > 1)
	  sd = sqrt(sd*n/(n-1));
	else sd = 0.0f;

	fprintf(stdout, "\n\n\t\tmean\t=\t%g\n\t\tsd\t=\t%g\n\t\tmin\t=\t%g\n\t\tmax\t=\t%g\n\t\tnpts\t=\t%d\n", mean, sd, min, max, n);
	first = 1;
	cpgsci(oldcol);
	break;
      case G_HELP:     /* Print usage info */
	printf("\nYou requested help by pressing \'%c\'.\n", G_HELP);
	printf("All cursor positions are entered with \'%c\' key (Left mouse button)\n", G_CUR);
	printf("\n %c - Select a sub-image to be displayed.\n", G_ZOOM);
	printf(" %c - Redisplay current plot.\n", G_DIS);
	printf(" %c - Fiddle contrast & brightness.\n", G_FID);
	printf(" %c - Use greyscale.\n", G_GREY);
	printf(" %c - Use rainbow colormap.\n", G_RAIN);
	printf(" %c - Use heat colormap.\n", G_HEAT);
	printf("\nTo end this session hit the \'%c\' key (Right mouse button)\n", G_QUIT);
	printf("\n");
	break;
      default :
	break;
      case G_GREY:
	cmap = grey;
	break;
      case G_RAIN:
	cmap = rain;
	break;
      case G_HEAT:
	cmap = heat; 
	break;
      case G_FID:
	do {
	  contra = 5.0 * (ypos[0]-ymid)/(ypos[0] < ymid ? (yzmin-ymid) : -(yzmax-ymid));
	  /*	  contra = 5.0 * ypos[0]/(ypos[0] < 0.0f ? yzmin : -yzmax); */
	  bright = 0.5 + 1.0 * (fabs(contra)+1.0) *
	    	    ((xpos[0] - xzmax)/(xzmin - xzmax) - 0.5);
	  cpgctab(cmap->l,cmap->r,cmap->g,cmap->b,cmap->n,contra,bright);
	  
	  cpgband(B_NORM, 0, xpos[0], ypos[0], &xpos[0], &ypos[0], &key);
	  if(islower((int) key))
	    key = (char) toupper((int) key);
	} while(key == G_FID);
	read = 0;
      }
      if(read) 
	cpgband(B_NORM, 0, xpos[0], ypos[0], &xpos[0], &ypos[0], &key);
      read = 1;
      if(islower((int) key))
	key = (char) toupper((int) key);
    } while(key != G_QUIT);
  }
  else {
    cpgpage();
    cpgvstd();
    cpgbbuf();
    if(z1==z2){
      z1 = zmins;
      z2 = zmaxs;
    }
    fprintf(stdout,"zmin = %g\tzmax = %g\n",zmins,zmaxs);
    cpgimag(zdata,nx,ny,i1,i2,j1,j2,z2,z1,tr);
    cpgbox("BCNST",0.0,0,"BCNST",0.0,0);
    cpgwedg("RI",0,4,z2,z1,unit);
    cpgebuf();
  };

  return 0;
}
/*.......................................................................
 * Grid data & make a grayscale map 
 *
 * Input: 
 *  ferret    Ferret *  The ferret to be plotted.
 *  xmem        Dmem *  The xmember.
 *  ymem        Dmem *  The ymember.
 *  zmem        Dmem *  The zmember.
 *  nx           int    The number of points to use in x.
 *  ny           int    The number of points to use in y.
 *  z1         float    The foreground greyscale.
 *  z2         float    The background greyscale.
 * Output:
 *  return    int       0 - OK.
 */
int v_grey2(int ndata, float *zdata, int nx,int ny, float xmina, float xmaxa, float ymina, float ymaxa, float *flag,float z1, float z2, char *xlab, char *ylab, char *title, char *unit)
{
  float xmins,xmaxs,xmin,xmax,dx;
  float ymins, ymaxs,ymin,ymax,dy;
  float zmins, zmaxs;
  int i,j;
  char answer[10];
  int docurs=0;
  int first = 1;
  float tr[6];
  int slen;
  int i1,i2,j1,j2,nbin;
  float xtemp,ytemp,xpos[2],ypos[2];
  static char *mess1="\n %c - Select start and end vertices using this key\n";
  static char *mess2=" %c - Select the whole plot\n";
  static char *mess3=" %c - Abort selection\n";
  int cancel,dofull,accepted;
  int iter;
  char key = 'L';
  int oldcol;
  float bright=0.5,contra=-1.0;
  int read = 1;
  Cmap *grey, *rain, *cmap, *heat;
  int n,xind,yind,exc;
  float mean,sd,min,max;
  float x,y,rad,r1,r2,r,h;
  float xmid,ymid;
  float sig;
  enum {                      /* Function keys in the gaphical interface */
    G_CUR   = 'A',
    G_FLG   = 'B',            /* Toggle display flagged data. */
    G_CUT   = 'C',
    G_CAN   = 'D',
    G_DEF   = 'D',

    G_FID   = 'F',
    G_GREY  = 'G',
    G_HELP  = 'H',
    G_INS   = 'I',
    G_FIT   = 'J',
    G_RAD   = 'K',
    G_DIS   = 'L',

    G_OVRPLT= 'O',
    G_COPY  = 'P',
    G_HEAT  = 'Q',
    G_RAIN  = 'R',
    G_STAT  = 'S',

    G_HORI  = 'U',
    G_VERT  = 'V',

    G_QUIT  = 'X',
    G_YSC   = 'Y',
    G_ZOOM  = 'Z',
  };	    

  for(i=0;i < n_std_cmap;i++) {
    if(strcmp(std_cmaps[i].name,"grey")==0) grey = &std_cmaps[i];
    if(strcmp(std_cmaps[i].name,"rainbow")==0) rain = &std_cmaps[i];
    if(strcmp(std_cmaps[i].name,"heat")==0) heat = &std_cmaps[i];
  };

  zmins = zmaxs = zdata[0];
  for(i=0;i < ndata;i++) {
      zmins = MIN(zmins, zdata[i]);
      zmaxs = MAX(zmaxs, zdata[i]);
    }
  
  if(zmins==zmaxs) {
    zmins -= 0.1*zmins;
    zmaxs += 0.1*zmaxs;
  }
  if(z1==z2){
    z1 = zmins;
    z2 = zmaxs;
  }

  xmins = xmina;
  xmaxs = xmaxa;
  ymins = ymina;
  ymaxs = ymaxa;

  xmid = (xmaxs+xmins)/2;
  ymid = (ymaxs+ymins)/2;

  dx = (xmaxs-xmins)/(nx-1);
  dy = (ymaxs-ymins)/(ny-1);
/*
 * Set the transformation matrix for the data array.
 */
  tr[0]=xmins-dx;
  tr[1]=dx;
  tr[2]=0.0;
  tr[3]=ymins-dy;
  tr[4]=0.0;
  tr[5]=dy;
  

  i1 = j1 = 1;
  i2 = nx;
  j2 = ny;
/*
 * Do we have a cursor?
 */
//  cpgqinf("CURSOR", answer, &slen);
  /*  docurs = strncmp(answer,"YES",3) == 0; */
  docurs = 1;

  cpgswin(xmins,xmaxs,ymins,ymaxs);

  cmap = heat;
  /*
   * Expand the plot limits 
   */
  xmins -= dx/2;
  xmaxs += dx/2;
  ymins -= dy/2;
  ymaxs += dy/2;

  xmin = xmins;
  xmax = xmaxs;
  ymin = ymins;
  ymax = ymaxs;
  if(docurs) {
    printf("For HELP, hit the \'%c\' key on your keyboard\n", G_HELP);
    do {
      cancel = 0;
      switch(key) {
      case G_CUR:
	cpgqci(&oldcol);
	cpgsci(1);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_LINE, 0, xmid, ymid, &xtemp,&ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
/*
 * Distance from the origin.
 */
	rad = sqrt((xmid-xpos[0])*(xmid-xpos[0])+(ymid-ypos[0])*(ymid-ypos[0]));
/*
 * Width of the filter.
 */
	sig = 0.5*sqrt((xpos[0]-xpos[1])*(xpos[0]-xpos[1])+(ypos[0]-ypos[1])*(ypos[0]-ypos[1]));
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      r = sqrt((xmid-x)*(xmid-x)+(ymid-y)*(ymid-y));
	      h = 1.0/(1+(r*sig/(r*r-rad*rad))*(r*sig/(r*r-rad*rad)));
	      flag[i+nx*j] = exc ? h : -(h-1);
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
      case G_CUT:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband((iter==0 ? B_NORM : B_LINE), 0, xtemp, ytemp, &xtemp, &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	sig = 2*sqrt((xpos[0]-xpos[1])*(xpos[0]-xpos[1])+(ypos[0]-ypos[1])*(ypos[0]-ypos[1]));
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      r1 = sqrt((x-xpos[0])*(x-xpos[0])+(y-ypos[0])*(y-ypos[0]));
	      r2 = sqrt((2*xmid-x-xpos[0])*(2*xmid-x-xpos[0])+(2*ymid-y-ypos[0])*(2*ymid-y-ypos[0]));
	      h = 1.0/(1+(sig/r1))*1.0/(1+(sig/r2));
	      flag[i+nx*j] = h;
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
	/*
	 * Make a radial plot of the image.
	 */
      case G_RAD:
	cpgqci(&oldcol);
	cpgsci(1);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_LINE, 0, xmid, ymid, &xtemp,&ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	/*
	 * Distance from the origin.
	 */
	rad = sqrt((xmid-xpos[0])*(xmid-xpos[0])+
		   (ymid-ypos[0])*(ymid-ypos[0]));
	/*
	 * Use a number of radial bins proportional to the distance (max will
	 * be the native image resolution, ie, ngrid/2
	 */
	nbin = (int)(nx/2*rad/sqrt((xmax-xmid)*(xmax-xmid) + (ymax-ymid)*(ymax-ymid)));
	v_radplot(zdata,nbin,xmina,xmaxa,nx,ymina,ymaxa,ny);
	break;
      case G_YSC:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<1 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_YRNG, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	sig = fabs(ypos[0]-ymid);
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      y = ymins+dy*j;
	      if(fabs(y-ymid) <= sig)
		flag[i+nx*j] = 0;
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
      case G_HORI:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<1 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_XRNG, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	sig = fabs(xmid-xpos[0]);
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      if(fabs(x-xmid) <= sig)
		flag[i+nx*j] = 0;
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
      case G_FIT:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<1 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband(B_LINE, 0, xmid, ymid, &xtemp, &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_CAN:      /* Abort box selection */
	      accepted = 1;
	      exc = 0;
	      break;
	    case G_QUIT:     /* Quit now */
	      accepted = cancel = 1;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      exc = 1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	rad = sqrt((xpos[0]-xmid)*(xpos[0]-xmid)+(ypos[0]-ymid)*(ypos[0]-ymid));
/*
 * Zap the requested points.
 */
	if(flag!=NULL) {
	  first = 1;
	  for(j=0;j < ny;j++) 
	    for(i=0;i < nx;i++) {
	      x = xmins+dx*i;
	      y = ymins+dy*j;
	      r = sqrt((x-xmid)*(x-xmid)+(y-ymid)*(y-ymid));
	      h = 1.0/(1+(rad/r)*(rad/r));
	      flag[i+nx*j] = exc ? h : -(h-1);
	      zdata[i+nx*j] *= flag[i+nx*j];
	      if(first) {
		z1 = z2 = zdata[i+nx*j];
		first = 0;
	      }
	      z1 = MIN(zdata[i+nx*j],z1);
	      z2 = MAX(zdata[i+nx*j],z2);
	    }  
	}
	break;
	/*
	 * Zoom the plot
	 */
      case G_ZOOM:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband((iter==0) ? B_NORM : B_RECT, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_ZOOM:
	      accepted = dofull = 1;
	      break;
	    case G_CAN:      /* Abort box selection */
	      accepted = cancel = 1;
	      break;
	    case G_QUIT:     /* Quit now */
	      cpgend();
	      free(zdata);
	      return 0;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	if(dofull) {
	  xmin = xmins;
	  ymin = ymins;
	  xmax = xmaxs;
	  ymax = ymaxs;
	}
	else {
	  /*
	   * Only reverse the boundaries if the "min" and "max" are contrary
	   * to the sense of dx and dy.
	   */
	  if((xpos[0] < xpos[1] && dx > 0.0) || 
	     (xpos[0] > xpos[1] && dx < 0.0)) {
	    xmin = xpos[0];
	    xmax = xpos[1];
	  }
	  else {
	    xmin = xpos[1];
	    xmax = xpos[0];
	  }
	  if((ypos[0] < ypos[1] && dy > 0.0) || 
	     (ypos[0] > ypos[1] && dy < 0.0)){
	    ymin = ypos[0];
	    ymax = ypos[1];
	  }
	  else {
	    ymin = ypos[1];
	    ymax = ypos[0];
	  }
	}
	cpgswin(xmin+dx/2,xmax+dx/2,ymin+dy/2,ymax+dy/2);
	cpgsci(oldcol);
      case G_DIS:
	if(!cancel) {
	  cpgpage();
	  cpgvstd();
	  cpgswin(0,1,0,1);
	  cpgwnad(0,1,0,1); 
	  cpgswin(xmin-dx/2,xmax+dx/2,ymin-dx/2,ymax+dx/2);
	  cpgwnad(xmin-dx/2,xmax+dx/2,ymin-dx/2,ymax+dx/2);
	  cpgbbuf();
	  if(z1==z2){
	    z1 = zmins;
	    z2 = zmaxs;
	  }
	  fprintf(stdout,"zmin = %g\tzmax = %g\n",zmins,zmaxs);
	  /*	  cpgwnad(xmin-dx/2,xmax+dx/2,ymin-dx/2,ymax+dx/2); */

	  cpgctab(cmap->l,cmap->r,cmap->g,cmap->b,cmap->n,contra,bright);
	  {
	    int c1,c2;
	    cpgqcir(&c1,&c2);
	    fprintf(stdout,"C1: %d C2: %d.\n",c1,c2);
	  }
	  cpgimag(zdata,nx,ny,i1,i2,j1,j2,z2,z1,tr);
	  cpgsci(1);
	  cpgbox("BCNST",0.0,0,"BCNST",0.0,0);
	  cpglab(xlab,ylab,title);
	  /*
	   * Draw a ramp on the side.
	   */
	  cpgwedg("RI",0,4,z2,z1,unit); 
	  cpgebuf();
	};
	break;
	/*
	 * Compute statistics on a selected region of the plot.
	 */
      case G_STAT:
	cpgqci(&oldcol);
	cpgsci(5);
	dofull = 0;
	cancel = 0;
	for(iter = 0;iter<2 && !dofull && !cancel;iter++) {
	  do {
	    accepted = 0;
	    cpgband((iter==0) ? B_NORM : B_RECT, 0, xtemp, ytemp, &xtemp,
		    &ytemp, &key);
	    if(islower((int) key))
	      key = (char) toupper((int) key);
	    xpos[iter] = xtemp;
	    ypos[iter] = ytemp;
	    switch(key) {
	    case G_STAT:
	      accepted = dofull = 1;
	      break;
	    case G_CAN:      /* Abort box selection */
	      accepted = cancel = 1;
	      break;
	    case G_QUIT:     /* Quit now */
	      cpgend();
	      free(zdata);
	      return 0;
	      break;
	    case G_CUR:             /* Accept the selected start vertex */
	      accepted=1;
	      break;
	    default:            /* Unexpected cursor input key - show usage */
	      printf(mess1, G_CUR);
	      printf(mess2, G_ZOOM);
	      printf(mess3, G_CAN);
	      break;
	    };
	  } while(!accepted);
	};
	if(dofull) {
	  xpos[0] = xmins;
	  ypos[0] = ymins;
	  xpos[1] = xmaxs;
	  ypos[1] = ymaxs;
	}
	/*
	 * Here we want xpos[0] and xpos[1], etc. to be the absolute 
	 * minimum and maximum, since we test if a data point falls between 
	 * these values.
	 */
	if(xpos[0] > xpos[1]) {
	  xtemp = xpos[1];
	  xpos[1] = xpos[0];
	  xpos[0] = xtemp;
	}
	if(ypos[0] > ypos[1]) {
	  ytemp = ypos[1];
	  ypos[1] = ypos[0];
	  ypos[0] = ytemp;
	}
	mean = 0.0;
	n = 0;
	for(i=0;i < ndata;i++) {
	  yind = i/nx;
	  xind = i - yind*nx;
	  xtemp = xmins + dx/2 + xind*dx;
	  ytemp = ymins + dy/2 + yind*dy;
	  if(xtemp >= xpos[0] && xtemp <= xpos[1] && ytemp >= ypos[0] && 
	     ytemp <= ypos[1]) {
	    if(first) {
	      min = max = zdata[i];
	      first = 0;
	    }
	    min = MIN(zdata[i],min);
	    max = MAX(zdata[i],max);
	    mean += (zdata[i] - mean)/(n+1);
	    ++n;
	  }
	}
	sd = 0.0;
	n = 0;
	for(i=0;i < ndata;i++) {
	  yind = i/nx;
	  xind = i - yind*nx;
	  xtemp = xmins + dx/2 + xind*dx;
	  ytemp = ymins + dy/2 + yind*dy;
	  if(xtemp >= xpos[0] && xtemp <= xpos[1] && ytemp >= ypos[0] && 
	     ytemp <= ypos[1]) {
	    sd += ((zdata[i] - mean)*(zdata[i]-mean) - sd)/(n+1);
	    ++n;
	  }
	}
	if(n > 1)
	  sd = sqrt(sd*n/(n-1));
	else sd = 0.0f;
	fprintf(stdout, "\n\n\t\tmean\t=\t%g\n\t\tsd\t=\t%g\n\t\tmin\t=\t%g\n\t\tmax\t=\t%g\n\t\tnpts\t=\t%d\n", mean, sd, min, max, n);
	first = 1;
	cpgsci(oldcol);
	break;
      case G_HELP:     /* Print usage info */
	printf("\nYou requested help by pressing \'%c\'.\n", G_HELP);
	printf("All cursor positions are entered with \'%c\' key (Left mouse button)\n", G_CUR);
	printf("\n %c - Select a sub-image to be displayed.\n", G_ZOOM);
	printf(" %c - Redisplay current plot.\n", G_DIS);
	printf(" %c - Fiddle contrast & brightness.\n", G_FID);
	printf(" %c - Use greyscale.\n", G_GREY);
	printf(" %c - Use rainbow colormap.\n", G_RAIN);
	printf(" %c - Use heat colormap.\n", G_HEAT);
	printf("\nTo end this session hit the \'%c\' key (Right mouse button)\n", G_QUIT);
	printf("\n");
	break;
      default :
	break;
      case G_GREY:
	cmap = grey;
	break;
      case G_RAIN:
	cmap = rain;
	break;
      case G_HEAT:
	/* Test drawing a line over the greyscale plot, and erasing it. */
	/*	cmap = heat; */
	{
	  int oldci;
	  cpgqci(&oldci);
	 
	  cpgsci(1);
	  cpgmove(xmin,ymin);
	  cpgdraw(xmax,ymax);
	  cpgsci(0);
	  cpgmove(xmin,ymin);
	  cpgdraw(xmax,ymax);
	  cpgsci(oldci);
	}
	break;
      case G_FID:
	do {
	  contra = 5.0 * (ypos[0]-ymid)/(ypos[0] < ymid ? (ymin-ymid) : -(ymax-ymid));
	  /*	  contra = 5.0 * ypos[0]/(ypos[0] < 0.0f ? ymin : -ymax); */
	  bright = 0.5 + 1.0 * (fabs(contra)+1.0) *
	    	    ((xpos[0] - xmax)/(xmin - xmax) - 0.5);
	  cpgctab(cmap->l,cmap->r,cmap->g,cmap->b,cmap->n,contra,bright);
	  
	  cpgband(B_NORM, 0, xpos[0], ypos[0], &xpos[0], &ypos[0], &key);
	  if(islower((int) key))
	    key = (char) toupper((int) key);
	}while(key == G_FID);
	read = 0;
      }
      if(read) 
	cpgband(B_NORM, 0, xpos[0], ypos[0], &xpos[0], &ypos[0], &key);
      read = 1;
      if(islower((int) key))
	key = (char) toupper((int) key);
    } while(key != G_QUIT);
  }
  else {
    cpgpage();
    cpgvstd();
    cpgbbuf();
    if(z1==z2){
      z1 = zmins;
      z2 = zmaxs;
    }
    fprintf(stdout,"zmin = %g\tzmax = %g\n",zmins,zmaxs);
    cpgimag(zdata,nx,ny,i1,i2,j1,j2,z2,z1,tr);
    cpgbox("BCNST",0.0,0,"BCNST",0.0,0);
    cpgwedg("RI",0,4,z2,z1,unit);
    cpgebuf();
  };

  return 0;
}
/*.......................................................................
 * Draw a histogram envelope.
 */
int v_hdraw(float hist[], int nbin,float xmin,float xmax,float ymin, 
		   float ymax,float dx, char *xlab, char *ylab, char *title)
{
  int i;
  float x1,x2;
  float ypts[5],xpts[5];
  float xrange,yrange;

  xrange = xmax-xmin;
  yrange = ymax-ymin;

  cpgask(0);
  cpgpage();
  cpgvstd();
  cpgbbuf();
  cpgsci(1);
  cpgswin(xmin-0.1*xrange,xmax+0.1*xrange,ymin,ymax+0.1*yrange);
  cpgbox("BCNST",0.0,0,"BCNST",0.0,0);

  cpgsci(10);
/*
 * Draw the histogram envelope.
 */
  for(i=0;i < nbin;i++) {
    
    x1 = xmin+(i-0.5)*dx;
    x2 = xmin+(i+0.5)*dx;
    
    xpts[0] = x1;
    xpts[1] = x1;
    xpts[2] = x2;
    
    if(i==0) {
      ypts[0] = ymin;
      ypts[1] = hist[i];
      ypts[2] = hist[i];
      
      cpgline(3,xpts,ypts);
    }
    else if(i==nbin-1) {
      xpts[3] = x2;
      
      ypts[0] = hist[i-1];
      ypts[1] = hist[i];
      ypts[2] = hist[i];
      ypts[3] = ymin;
      
      cpgline(4,xpts,ypts);
    }
    else {
      ypts[0] = hist[i-1];
      ypts[1] = hist[i];
      ypts[2] = hist[i];
      
      cpgline(3,xpts,ypts);    
    }
  }
  cpgsci(1);
/*
 * Redraw the bottom axis.
 */
  xpts[0] = xmin;
  xpts[1] = xmax;
  ypts[0] = ypts[1] = ymin;

  cpgsls(1);
  cpgline(2, xpts, ypts);
  cpglab(xlab,ylab,title);
  cpgebuf(); 

  return 1;
}
/*.......................................................................
 * Draw a histogram envelope.
 */
int v_radplot(float data[],int nbin,float xmin,float xmax,
int nx, float ymin,float ymax,int ny)
{
  int i,xind,yind,ind,waserr=0;
  float lmax,xmid,ymid,dl,dx,dy,l,xtemp,ytemp,min,max,xhalf,yhalf;
  float *lxs=NULL,*lys=NULL;
  int *lns=NULL;

  fprintf(stderr, "Entering v_radplot with nbin = %d xmin = %f xmax = %f\n",
	  nbin, xmin, xmax);

  xmid = (xmax+xmin)/2;
  ymid = (ymax+ymin)/2;


  xhalf = (xmax-xmin)/2;
  yhalf = (ymax-ymin)/2;

  lmax = sqrt(xhalf*xhalf+yhalf*yhalf);

  waserr = (lns = (int *)malloc(nbin*sizeof(int)))==NULL;
  waserr |= (lxs = (float *)malloc(nbin*sizeof(float)))==NULL;
  waserr |= (lys = (float *)malloc(nbin*sizeof(float)))==NULL;

  if(!waserr) {
    /*
     * Use nbin bins between 0 and lmax
     */
    dl = lmax/(nbin-1);
    dx = (xmax-xmin)/(nx-1);
    dy = (ymax-ymin)/(ny-1);

    for(i=0;i < nbin;i++) {
      lns[i] = 0;
      lxs[i] = dl*i;
      lys[i] = 0.0;
    }

    for(i=0;i < nx*ny;i++) {
      yind = i/nx;
      xind = i - yind*nx;
      xtemp = xmin + dx/2 + xind*dx;
      ytemp = ymin + dy/2 + yind*dy;
      l = sqrt((xtemp-xmid)*(xtemp-xmid) + (ytemp-ymid)*(ytemp-ymid));
      ind = floor((l)/dl+0.5);
      /*
       * Keep a running mean for each bin.
       */
      lys[ind] += (data[i] - lys[ind])/(lns[ind]+1);
      ++lns[ind];
    }
    min = max = lys[0];
    for(i=0;i < nbin;i++) {
      max = MAX(max,lys[i]);
      min = MIN(min,lys[i]);
    }

    cpgask(0);
    cpgpage();
    cpgvstd();
    cpgbbuf();
    cpgsci(1);
    cpgswin(0-lmax*0.1,lmax*1.1,min - (max-min)*0.1,max+(max-min)*0.1);
    cpgbox("BCNST",0.0,0,"BCNST",0.0,0);
    cpgsci(10);
    cpgsls(1);
    cpgline(nbin, lxs, lys);
    cpgebuf(); 
  }
  /*
   * Free any allocated memory,
   */
  if(lns)
    free(lns);
  if(lxs)
    free(lxs);
  if(lys)
    free(lys);

  return waserr;
}
