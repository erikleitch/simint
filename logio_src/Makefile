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
# This is the (hopefully) site independant part of the liblogio.a makefile.
# The site-specific part should be appended to this makefile and named
# Makefile by the configure script also present in this directory. You
# should not have to modify this file - changes should only be made to
# the configure script.
# The configure script must define CC, CFLAGS, INCDIR, LIBDIR, HELPDIR,
# RANLIB.
#-----------------------------------------------------------------------
# Description: Makefile for the log I/O library.
#
# Martin Shepherd  (mcs@phobos.caltech.edu)
#-----------------------------------------------------------------------

# Compile time C compiler flags.

CFLAGC = -c $(CFLAGS) -I$(INCDIR)

# This tells make how to compile a C file - don't touch.

.c.o:
	$(CC) $(CFLAGC) $<

# List logio object files.

LOGIO_OBS = logio.o print.o

# The library name.

LIBLOGIO = $(LIBDIR)/liblogio.a

#-----------------------------------------------------------------------

default: $(LIBLOGIO) install-header

$(LIBLOGIO): $(LOGIO_OBS)
	ar ru $(LIBLOGIO) $(LOGIO_OBS)
	$(RANLIB) $(LIBLOGIO)

install-header:	$(INCDIR)/logio.h

$(INCDIR)/logio.h: logio.h
	cp logio.h $(INCDIR)/logio.h
	chmod ugo+r $(INCDIR)/logio.h
clean:
	/bin/rm -f *.o

#-----------------------------------------------------------------------
# list object file dependancies

logio.o: logio.h $(INCDIR)/scrfil.h

print.o: logio.h
