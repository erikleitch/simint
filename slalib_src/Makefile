# Makefile for libsla.
#
# The makefile that calls it is expected to define the following
# macros:
#
#  INC       - include directory.
#  LIB       - lib directory.
#  SLALIB    - The directory that contains the slalib source code.
#  CC        - The C compiler to be used.
#  CFLAGS    - Flags for the C compiler.
#  RANLIB    - The archiver randomizer command (use : if not needed).
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# List the default targets.
#-----------------------------------------------------------------------

all: $(LIB)/libsla.a $(INC)/slalib.h

clean:
	/bin/rm -f $(LIB)/libsla.a
	/bin/rm -f $(INC)/slalib.h
	/bin/rm -f *.o

#-----------------------------------------------------------------------
# List the slalib modules that we are interested in.
#-----------------------------------------------------------------------

SLA_OBJ = cldj.o 

#-----------------------------------------------------------------------
# Get the compilation rules.
#-----------------------------------------------------------------------

include $(SLALIB)/Makefile.rules

#-----------------------------------------------------------------------
# Library generation.
#-----------------------------------------------------------------------

$(LIB)/libsla.a: $(SLA_OBJ)
	ar ru $@ $(SLA_OBJ)
	$(RANLIB) $@

#-----------------------------------------------------------------------
# Install the slalib include file in the shared include directory.
#-----------------------------------------------------------------------

$(INC)/slalib.h: $(SLALIB)/slalib.h
	cp -p $(SLALIB)/slalib.h $@
