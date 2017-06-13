#-----------------------------------------------------------------------
# Sample script to run inside simint; load in the 
# 
#    File --> Load antenna file 
#
# dialog box
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# These commands set the value of global variables used by simint
#-----------------------------------------------------------------------

# The correlator efficiency -- used in calculating noise & weights

uplevel #0 {set correff 1}

# The aperture efficiency -- used in calculating noise & weights

uplevel #0 {set apeff 2}

# The antenna diameter (m)

uplevel #0 {set antsize 350}

# The observing wavelength

uplevel #0 {set wave 1}

# The total bandwidth

uplevel #0 {set bw 8}

# The number of separate channels in which the total bandwidth is correlated

uplevel #0 {set nchan 8}

# The observing latitude

uplevel #0 {set obslat 37:13:57.5}

# The observing longitude

uplevel #0 {set obslong 07:37:03.819}

# The observing altitude (m)

uplevel #0 {set obsalt 3000}

# The source declination

uplevel #0 {set srcdec 34:04:43.497}

# The HA start of a track

uplevel #0 {set hastart -12:00:00}

# The HA stop of a track

uplevel #0 {set hastop 12:00:00}

# The integration time for each visibility

uplevel #0 {set tint 60}

# The scale factor by which to multiply the integration time for each
# visibility

uplevel #0 {set tscale 1}

# Atmospheric opacity (applies over the whole bandwidth)

uplevel #0 {set tau 0.045}

# Fixed Tsys component

uplevel #0 {set tsys 20.0}

# Set up the maximum x-value for the antenna (and visibility) display.
# Display is assumed to be symmetric in x-y (ie, will extend from x =
# -max --> max, y = -max --> max.  Values will not take effect until
# the "redraw" command below is issued.

uplevel #0 {set xmax 10000}

#-----------------------------------------------------------------------
# These commands define the array
#-----------------------------------------------------------------------

# These commands just clear the current display in preparation for
# starting a new one, and redraws the frames.  "simint" is a prefix
# which prefaces every command sent to the C layer of the program.

uplevel #0 {simint zapant}
uplevel #0 {simint drawframes $wave}

# Add antennas at the specified locations.
# Arguments to this command can be:
#
#   x location (cm)
#   y location (cm)
#   antenna diameter (cm)
#   wavelength (cm)
#   latitude (e.g., 34:04:43.497)
#   longitude (e.g., 107:37:03.819)
#   HA start (hh:mm:ss)
#   HA stop (hh:mm:ss)
#   symmetry (value of n will add antennas at n-fold symmetric locations, 
#            if possible.  Usually 1.)
#   add (1 to add the antenna, 0 to delete it)
#
# To use the value of one of the variables defined above, pass the variable
# name, preceded by a $.

uplevel #0 {simint addant   496  -20 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant   161  533 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant     0    0 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant  -567  206 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant    50 -598 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant  -581 -247 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant -2121 5595 350 $wave $obslat $obslong 1 1}
uplevel #0 {simint addant  3879 3095 350 $wave $obslat $obslong 1 1}

# This will redraw the antenna and visibility windows

uplevel #0 {simint redraw}

#-----------------------------------------------------------------------
# These commands read in a FITS image file
#-----------------------------------------------------------------------

# Cellsize (arcseconds) is used to set the pixel scale when generating
# an image, or the default pixel size when reading in an image.  If a
# fits image file contains the CRDELT keywords, the pixel scale will
# be taken from the image file instead.

uplevel #0 {set cellsize 1.17}

# Uncomment the following to read a FITS image in.  In the example
# below, myimage.fits is the name of the file to read, and compz is
# the internal handle by which simint will refer to the image.  

uplevel #0 {simint rfits compz.fits compz Y}
#uplevel #0 {simint rfits ptsrc.fits ptsrc Jy}

#-----------------------------------------------------------------------
# And this command writes out a FITS UVF file
#-----------------------------------------------------------------------

# Un-comment the following to write out a FITS UVF file that is the
# observation of this image with the array specified above.  Note that
# executing the rfits commands (above) automatically installs the
# image as the current image (equivalent to selecting it in the Image
# drop-down menu).

uplevel #0 {simint newwritevis compz_24h.uvf}
#uplevel #0 {simint newwritevis ptsrc.uvf}


