# This is the top level makefile for stand alone dasi source tree
# The following sub dir need to be cleaned up and made like the
#  libx ones: scrfil_src logio_src, pager_src, recio_src, sphere_src,
#  dasi_src
#-----------------------------------------------------------------------

# Need to be defined to locate PGPLOT libraries

PGINCDIR = /usr/local/pgplot
PGLIBDIR = /usr/local/pgplot

SIMROOT = $(shell pwd)

INC = $(SIMROOT)/include
LIB = $(SIMROOT)/lib
BIN = $(SIMROOT)/bin
SLALIB = $(SIMROOT)/slalib_src
HELP = "`pwd`/help"
RANLIB = @:

CC = gcc

OS := $(shell uname)

# SunOS requires different libraries that linux doesn't need

ifeq ($(OS), SunOS)
  CFLAGS = -Wall -Wmissing-prototypes -Wmissing-declarations -D_BSD_SOURCE \
	-D_POSIX_C_SOURCE=199506L -D__EXTENSIONS__  -I$(INC) -I$(SHARE)
else

# If Comiling under linux, uncomment the following line:

  CFLAGS = -O -Wall -Wmissing-prototypes -Wmissing-declarations \
	-Dlinux_i486_gcc -D_BSD_SOURCE -D_POSIX_C_SOURCE=199506L \
	-D__EXTENSIONS__  -I$(INC)
endif

#-----------------------------------------------------------------------
# List the default targets.
#-----------------------------------------------------------------------

all: bin lib include make_scrfil make_logio make_recio make_slalib make_fftwc \
     make_fftwr make_simint tags

clean: clean_scrfil clean_logio clean_recio clean_slalib clean_fftwc \
	clean_fftwr clean_simint clean_dirs

#-----------------------------------------------------------------------
# Directory creation.
#-----------------------------------------------------------------------

bin:
	mkdir bin

lib:
	mkdir lib

include:
	mkdir include

tags:
	etags simint_src/*.c logio_src/*.c recio_src/*.c slalib_src/*.c \
	fftwc_src/*.c fftwr_src/*.c

#-----------------------------------------------------------------------
# Clean up
#-----------------------------------------------------------------------

clean_dirs:
	rm -rf bin
	rm -rf lib
	rm -rf include

#-----------------------------------------------------------------------
# Recursive make targets.
#-----------------------------------------------------------------------

make_scrfil:
	@echo ' ';echo 'Building lib/libscrfil.a'; echo ' '
	@cd scrfil_src; $(MAKE) 

clean_scrfil:
	@cd scrfil_src; $(MAKE) clean

make_logio:
	@echo ' ';echo 'Building lib/liblogio.a'; echo ' '
	@cd logio_src; $(MAKE) 

clean_logio:
	@cd logio_src; $(MAKE) clean

make_recio:
	@echo ' ';echo 'Building lib/librecio.a'; echo ' '
	@cd recio_src; $(MAKE) 

clean_recio:
	@cd recio_src; $(MAKE) clean

make_fftwr:
	@echo ' ';echo 'Building lib/libfftwr.a'; echo ' '
	@cd fftwr_src; $(MAKE) 

clean_fftwr:
	@cd fftwr_src; $(MAKE) clean

make_fftwc:
	@echo ' ';echo 'Building lib/libfftwc.a'; echo ' '
	@cd fftwc_src; $(MAKE) 

clean_fftwc:
	@cd fftwc_src; $(MAKE) clean

make_slalib:
	@echo ' ';echo 'Building lib/libsla.a'; echo ' '
	@cd slalib_src; $(MAKE) "INC=$(INC)" "LIB=$(LIB)" \
	"CC=$(CC)" "CFLAGS=$(CFLAGS)" "RANLIB=$(RANLIB)" "SLALIB=$(SLALIB)"

clean_slalib:
	@cd slalib_src; $(MAKE) clean "INC=$(INC)" "LIB=$(LIB)" \
	"CC=$(CC)" "CFLAGS=$(CFLAGS)" "RANLIB=$(RANLIB)" "SLALIB=$(SLALIB)"

make_simint:
	@echo ' ';echo 'Compiling simint itself'; echo ' '
	@cd simint_src; $(MAKE) "LIB=$(LIB)" "INC=$(INC)" \
	"CFLAGS=$(CFLAGS)" "PGINCDIR=$(PGINCDIR)" "PGLIBDIR=$(PGLIBDIR)" \
	"BIN=$(BIN)"

clean_simint:
	@cd simint_src; $(MAKE) clean "LIB=$(LIB)" "INC=$(INC)" \
	"CFLAGS=$(CFLAGS)" "PGINCDIR=$(PGINCDIR)" "PGLIBDIR=$(PGLIBDIR)" \
	"BIN=$(BIN)"

