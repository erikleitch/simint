# Copyright (c) 1993 by the California Institute of Technology.

# ANSI-C compiler.

CC = gcc

# ANSI-C run-time library

CCLIB = `gcc -print-libgcc-file-name`

# FORTRAN compiler.

FC = f77

# C and FORTRAN compilation flags.

CFLAGS = -g -O -I.
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

# List rfftw object files.

NOTW_CODELETS= frc_1.o frc_2.o frc_3.o frc_4.o frc_5.o frc_6.o frc_7.o frc_8.o frc_9.o frc_10.o frc_11.o frc_12.o frc_13.o frc_14.o frc_15.o frc_16.o frc_32.o frc_64.o frc_128.o
TWID_CODELETS= fhf_2.o fhf_3.o fhf_4.o fhf_5.o fhf_6.o fhf_7.o fhf_8.o fhf_9.o fhf_10.o fhf_16.o fhf_32.o
NOTWI_CODELETS= fcr_1.o fcr_2.o fcr_3.o fcr_4.o fcr_5.o fcr_6.o fcr_7.o fcr_8.o fcr_9.o fcr_10.o fcr_11.o fcr_12.o fcr_13.o fcr_14.o fcr_15.o fcr_16.o fcr_32.o fcr_64.o fcr_128.o
TWIDI_CODELETS= fhb_2.o fhb_3.o fhb_4.o fhb_5.o fhb_6.o fhb_7.o fhb_8.o fhb_9.o fhb_10.o fhb_16.o fhb_32.o

CODELETS=$(NOTW_CODELETS) $(TWID_CODELETS) $(NOTWI_CODELETS) $(TWIDI_CODELETS)
OTHERSRC = rconfig.o rplanner.o rexec.o rexec2.o rfftwnd.o rgeneric.o \
           rfftwf77.o

FFTWR_OBS = $(CODELETS) $(OTHERSRC)

# The library name.

LIBFFTWR = $(LIBDIR)/libfftwr.a

#-----------------------------------------------------------------------

default: $(LIBFFTWR) install-header

install-header:	$(INCDIR)/rfftw.h

$(INCDIR)/rfftw.h: rfftw.h
	cp rfftw.h $(INCDIR)/rfftw.h
	chmod ugo+r $(INCDIR)/rfftw.h

$(LIBFFTWR): $(FFTWR_OBS)
	ar ru $(LIBFFTWR) $(FFTWR_OBS)
	$(RANLIB) $(LIBFFTWR)

clean:
	/bin/rm -f *.o
	/bin/rm -f $(INCDIR)/rfftw.h
	/bin/rm -f $(LIBFFTWR)

