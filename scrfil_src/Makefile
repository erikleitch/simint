# Copyright (c) 1993 by the California Institute of Technology.

# ANSI-C compiler.

CC = gcc

# ANSI-C run-time library

CCLIB = `gcc -print-libgcc-file-name`

# FORTRAN compiler.

FC = f77

# C and FORTRAN compilation flags.

CFLAGS = -O
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
# Description: Makefile for the scratch file utilities library.
#
# Martin Shepherd  (mcs@phobos.caltech.edu)
#-----------------------------------------------------------------------

# Compile time C compiler flags.

CFLAGC = -c $(CFLAGS) -I$(INCDIR)

# This tells make how to compile a C file - don't touch.

.c.o:
	$(CC) $(CFLAGC) $<

# List scrfil object files.

SCRFIL_OBS = scrnam.o exist.o edfile.o

# The library name.

LIBSCRFIL = $(LIBDIR)/libscrfil.a

#-----------------------------------------------------------------------

default: $(LIBSCRFIL) install-header

$(LIBSCRFIL): $(SCRFIL_OBS)
	ar ru $(LIBSCRFIL) $(SCRFIL_OBS)
	$(RANLIB) $(LIBSCRFIL)

install-header:	$(INCDIR)/scrfil.h

$(INCDIR)/scrfil.h: scrfil.h
	cp scrfil.h $(INCDIR)/scrfil.h
	chmod ugo+r $(INCDIR)/scrfil.h

clean:
	/bin/rm -f *.o

#-----------------------------------------------------------------------
# list object file dependancies

scrnam.o: scrfil.h

exist.o: scrfil.h

edfile.o: scrfil.h
