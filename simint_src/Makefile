#-----------------------------------------------------------------------
# The calling makefile must define BIN, INC, LIB, SHARE,
# HELP, PGINCDIR, PGLIBDIR, SLALIB
#-----------------------------------------------------------------------

# ANSI-C compiler.

CC = gcc

# ANSI-C run-time library

CCLIB = `gcc -print-libgcc-file-name`

# FORTRAN compiler.

FC = f77

# C and FORTRAN compilation flags.

FFLAGS = -c -O

OPT = 

# Loader flags.

LDFLAGS = 

# Symbolic link creation.

LN = ln -s

# The way to invoke ranlib.

RANLIB = echo ranlib

# The default pager.

PAGER = more

# Compile time flags.

CFLAGC = -c $(CFLAGS) -I$(INC) -I$(PGINCDIR)

FFLAGC = -c $(FFLAGS)

# Specify how the FFTWR library is to be specified on the loader line.

FFTWR = -L$(LIB) -lfftwr

# Specify how the FFTWC library is to be specified on the loader line.

FFTWC = -L$(LIB) -lfftwc 

# Specify how the SCRFIl library is to be specified on the loader line.

SCRFIL = -L$(LIB) -lscrfil

# Specify how the SCRFIl library is to be specified on the loader line.

SCRFIL = -L$(LIB) -lscrfil

# Specify how the record I/O library is to be specified on the loader line.

RECIO = -L$(LIB) -lrecio

# Specify how the slalib library is to be specified on the loader line.

SLALIB = -L$(LIB) -lsla

# Optional Malloc debugging library -- uncomment this line if compiling with dmalloc

# MALLOC_LIB = -L$(LIB) -ldmalloc

ifdef MALLOC_LIB
CFLAGC = -c $(CFLAGS) -I$(INC) -I$(PGINCDIR) -DDMALLOC
endif

# This tells make how to compile a C file - don't touch.

.c.o:
	$(CC) $(CFLAGC) $<

.f.o:
	$(FC) $(FFLAGC) $<

#-----------------------------------------------------------------------
# Default to making just tksimint.

default: tksimint simint

#-----------------------------------------------------------------------
# SIMINT
#-----------------------------------------------------------------------

PROG_OBS = tksimint.o vplot.o fourier.o fourn.o image.o fitsio.o vis.o rand.o \
	imgen.o

tksimint: $(PROG_OBS) 
	$(FC) -o tksimint $(LDFLAGS) $(PROG_OBS) $(RECIO) \
	$(SCRFIL) $(SLALIB) $(FFTWR) $(FFTWC) $(MALLOC_LIB) \
	-ltk -ltcl -lm -L/usr/X11R6/lib -lX11 -lnsl -ldl \
	-L$(PGLIBDIR) -lcpgplot -ltkpgplot -lpgplot
	mv tksimint $(BIN)/tksimint
	chmod ugo+rx $(BIN)/tksimint

simint: simint.tcl
	\rm -f $@
	echo '#!'$(BIN)/tksimint > $(BIN)/$@
	cat simint.tcl >> $(BIN)/$@
	chmod +x $(BIN)/$@

clean:
	/bin/rm -f *.o
	/bin/rm -f $(BIN)/simint
	/bin/rm -f $(BIN)/tksimint

#-----------------------------------------------------------------------
# list object  & header file dependencies
#-----------------------------------------------------------------------

image.o: image.h fitsio.h

tksimint.o: tksimint.c tksimint.h color_tab.h vplot.h fourier.h \
	image.h fitsio.h vis.h $(INC)/fftw.h $(INC)/fftw-int.h \
	$(INC)/rfftw.h $(PGINCDIR)/cpgplot.h $(PGINCDIR)/tkpgplot.h
	gcc tksimint.c $(CFLAGC) -DFFTW_ENABLE_FLOAT 

vplot.o:  tksimint.h color_tab.h vplot.h fourier.h $(PGINCDIR)/cpgplot.h 

fourier.o: fourier.c tksimint.h fourier.h fitsio.h vplot.h $(INC)/fftw.h \
	$(INC)/fftw-int.h $(INC)/rfftw.h
	gcc fourier.c $(CFLAGC) -DFFTW_ENABLE_FLOAT 

fitsio.o: fitsio.h tksimint.h image.h

ifdef MALLOC_LIB
vis.o: vis.c tksimint.h fitsio.h vis.h rand.h $(INC)/fftw.h $(INC)/fftw-int.h \
	$(INC)/rfftw.h fourier.h $(INC)/dmalloc.h
	gcc vis.c $(CFLAGC) -DFFTW_ENABLE_FLOAT 
else
vis.o: vis.c tksimint.h fitsio.h vis.h $(INC)/fftw.h $(INC)/fftw-int.h \
	$(INC)/rfftw.h fourier.h
	gcc vis.c $(CFLAGC) -DFFTW_ENABLE_FLOAT 
endif

imgen.o: imgen.c tksimint.h fitsio.h imgen.h rand.h $(INC)/fftw.h \
	$(INC)/fftw-int.h $(INC)/rfftw.h
	gcc imgen.c $(CFLAGC) -DFFTW_ENABLE_FLOAT 

rand.o: rand.h

ifdef MALLOC_LIB
tksimint.h: image.h $(INC)/dmalloc.h
else
tksimint.h: image.h
endif

fourier.h: tksimint.h

vplot.h: color_tab.h tksimint.h

fitsio.h: tksimint.h image.h





