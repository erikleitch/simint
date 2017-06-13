# Copyright (c) 1993 by the California Institute of Technology.

# ANSI-C compiler.

CC = gcc

# ANSI-C run-time library

CCLIB = `gcc -print-libgcc-file-name`

# FORTRAN compiler.

FC = f77

# C and FORTRAN compilation flags.

CFLAGS = -O -I.
FFLAGS = -c -O

# Directory for includes.

INCDIR = ../include

# Directory for libraries.

LIBDIR = ../lib

# Loader flags.

LDFLAGS = 

# Symbolic link creation.

LN = ln -s

# The way to invoke ranlib.

RANLIB = echo ranlib

# The default pager.

PAGER = more

#-----------------------------------------------------------------------
# This is the (hopefully) site independant part of the libscrfil.a makefile.
# The site-specific part should be appended to this makefile and named
# Makefile by the configure script also present in this directory. You
# should not have to modify this file - changes should only be made to
# the configure script.
# The configure script must define CC, CFLAGS, INCDIR, LIBDIR, HELPDIR,
# RANLIB.
#-----------------------------------------------------------------------

# Compile time C compiler flags.

CFLAGC = -c $(CFLAGS) -DFFTW_ENABLE_FLOAT

# This tells make how to compile a C file - don't touch.

.c.o:
	$(CC) $(CFLAGC) $<

# List fftwc object files.

NOTW_CODELETS= fn_1.o fn_2.o fn_3.o fn_4.o fn_5.o fn_6.o fn_7.o fn_8.o fn_9.o fn_10.o fn_11.o fn_12.o fn_13.o fn_14.o fn_15.o fn_16.o fn_32.o fn_64.o
TWID_CODELETS= ftw_2.o ftw_3.o ftw_4.o ftw_5.o ftw_6.o ftw_7.o ftw_8.o ftw_9.o ftw_10.o ftw_16.o ftw_32.o ftw_64.o
NOTWI_CODELETS= fni_1.o fni_2.o fni_3.o fni_4.o fni_5.o fni_6.o fni_7.o fni_8.o fni_9.o fni_10.o fni_11.o fni_12.o fni_13.o fni_14.o fni_15.o fni_16.o fni_32.o fni_64.o
TWIDI_CODELETS= ftwi_2.o ftwi_3.o ftwi_4.o ftwi_5.o ftwi_6.o ftwi_7.o ftwi_8.o ftwi_9.o ftwi_10.o ftwi_16.o ftwi_32.o ftwi_64.o

CODELETS=$(NOTW_CODELETS) $(TWID_CODELETS) $(NOTWI_CODELETS) $(TWIDI_CODELETS)
OTHERSRC = timer.o config.o planner.o twiddle.o executor.o \
	   generic.o fftwnd.o malloc.o wisdom.o wisdomio.o putils.o rader.o \
           fftwf77.o f77_func.h

FFTWC_OBS = $(CODELETS) $(OTHERSRC)

# The library name.

LIBFFTWC = $(LIBDIR)/libfftwc.a

#-----------------------------------------------------------------------

default: $(LIBFFTWC) install-header

install-header:	$(INCDIR)/fftw.h $(INCDIR)/fftw-int.h $(INCDIR)/config.h

$(LIBFFTWC): $(FFTWC_OBS)
	ar ru $(LIBFFTWC) $(FFTWC_OBS)
	$(RANLIB) $(LIBFFTWC)

$(INCDIR)/fftw.h: fftw.h
	cp fftw.h $(INCDIR)/fftw.h
	chmod ugo+r $(INCDIR)/fftw.h

$(INCDIR)/fftw-int.h: fftw-int.h
	cp fftw-int.h $(INCDIR)/fftw-int.h
	chmod ugo+r $(INCDIR)/fftw-int.h

$(INCDIR)/config.h: config.h
	cp config.h $(INCDIR)/config.h
	chmod ugo+r $(INCDIR)/config.h

clean:
	/bin/rm -f *.o
	/bin/rm -f $(INCDIR)/fftw.h
	/bin/rm -f $(INCDIR)/fftw-int.h
	/bin/rm -f $(INCDIR)/config.h
	/bin/rm -f $(LIBFFTWC)
