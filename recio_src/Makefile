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
# This is the (hopefully) site independant part of the librecio.a makefile.
# The site-specific part should be appended to this makefile and named
# Makefile by the configure script. You should not have to modify this
# file - changes should only be made to the configure script.
# The configure script must define CC, CFLAGS, INCDIR, LIBDIR, HELPDIR,
# RANLIB.
#-----------------------------------------------------------------------
# Description: Makefile for the record I/O interface library.
#
# Martin Shepherd  (mcs@phobos.caltech.edu)
#-----------------------------------------------------------------------

# Compile time C compiler flags.

CFLAGC = -c $(CFLAGS) -I$(INCDIR)

# This tells make how to compile a C file - don't touch.

.c.o:
	$(CC) $(CFLAGC) $<

# List recio object files.

RECIO_OBS = recio.o

# The library name.

LIBRECIO = $(LIBDIR)/librecio.a

#-----------------------------------------------------------------------

default: $(LIBRECIO) install-header

$(LIBRECIO): $(RECIO_OBS)
	ar ru $(LIBRECIO) $(RECIO_OBS)
	$(RANLIB) $(LIBRECIO)

install-header:	$(INCDIR)/recio.h

$(INCDIR)/recio.h: recio.h
	cp recio.h $(INCDIR)/recio.h
	chmod ugo+r $(INCDIR)/recio.h

clean:
	/bin/rm -f $(INCDIR)/recio.h
	/bin/rm -f *.o
	/bin/rm -f $(LIBRECIO)

#-----------------------------------------------------------------------
# list object file dependancies

recio.o: recio.h $(INCDIR)/logio.h $(INCDIR)/scrfil.h
