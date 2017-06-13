#-----------------------------------------------------------------------
# Create an unmapped prompt dialog.
#
# This is split into a top section and a bottom section.
# The top section contains a title and an entry widget $w.top.entry.
# The bottom section contains three buttons, $w.bot.ok, $w.bot.cancel
# and $w.bot.help. Only the "close" button is assigned a command.
# The other buttons should be set by the caller. Note that the help
# button is displayed disabled
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w        The name to give the widget.
#  title    The title to give the dialog.
#  label    The message to place above the entry widget.
#-----------------------------------------------------------------------
proc create_prompt_dialog {w title msg} {
#
# Create the toplevel dialog window withdrawn.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create the top and bottom frames.
#
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both -expand 1
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both -expand 1
#
# Create a label and an entry widget in the top frame.
#
    message $w.top.msg -justify left -width 8c -anchor w -text $msg
    entry $w.top.entry -relief sunken -bd 2 -width 30
    pack $w.top.msg $w.top.entry -side top -anchor w
#
# Create three buttons in the bottom frame.
#
    button $w.bot.ok -text OK
    button $w.bot.cancel -text Cancel -command "wm withdraw $w"
    button $w.bot.help -text Help -state disabled
    pack $w.bot.ok $w.bot.cancel $w.bot.help -side left -expand 1 -pady 2m -padx 2m
#
# Arrange for carriage-return to invoke the OK key.
#
    bind $w <Return> "$w.bot.ok invoke"
}
#-----------------------------------------------------------------------
# Create an unmapped prompt dialog with a radiobutton menu.
#
# This is split into a top section and a bottom section.
# The top section contains a title and an entry widget $w.top.entry.
# The bottom section contains three buttons, $w.bot.ok, $w.bot.cancel
# and $w.bot.help. Only the "close" button is assigned a command.
# The other buttons should be set by the caller. Note that the help
# button is displayed disabled
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w        The name to give the widget.
#  title    The title to give the dialog.
#  label    The message to place above the entry widget.
#-----------------------------------------------------------------------
proc create_prompt2button_dialog {w title msg but1 but2} {
#
# Create the toplevel dialog window withdrawn.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create the top middle and bottom frames.
#
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both -expand 1
    frame $w.mid -relief raised -bd 1
    pack $w.mid -side top -fill both -expand 1
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both -expand 1
#
# Create a label and an entry widget in the top 2 frames.
#
    message $w.top.msg -justify left -width 8c -anchor w -text $msg
    entry $w.top.entry -relief sunken -bd 2 -width 30
    pack $w.top.msg $w.top.entry -side top -anchor w

# Create two un-mapped radio buttons.

    radiobutton $w.mid.rad1 -text $but1 
    radiobutton $w.mid.rad2 -text $but2 
    pack $w.mid.rad1 $w.mid.rad2 -side top -anchor w
#
# Create three buttons in the bottom frame.
#
    button $w.bot.ok -text OK
    button $w.bot.cancel -text Cancel -command "wm withdraw $w"
    button $w.bot.help -text Help -state disabled
    pack $w.bot.ok $w.bot.cancel $w.bot.help -side left -expand 1 -pady 2m -padx 2m
#
# Arrange for carriage-return to invoke the OK key.
#
    bind $w.top.entry <Return> "$w.bot.ok invoke"
}

#-----------------------------------------------------------------------
# Create an unmapped 2-prompt dialog.
#
# This is split into a top section and a bottom section.
# The top section contains a title and an entry widget $w.top.entry.
# The bottom section contains three buttons, $w.bot.ok, $w.bot.cancel
# and $w.bot.help. Only the "close" button is assigned a command.
# The other buttons should be set by the caller. Note that the help
# button is displayed disabled
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w        The name to give the widget.
#  title    The title to give the dialog.
#  label    The message to place above the entry widget.
#-----------------------------------------------------------------------
proc create_2prompt_dialog {w title msg1 msg2} {
#
# Create the toplevel dialog window withdrawn.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create the top and bottom frames.
#
    frame $w.top -relief raised -bd 1
    pack $w.top -side top -fill both -expand 1
    frame $w.mid -relief raised -bd 1
    pack $w.mid -side top -fill both -expand 1
    frame $w.bot -relief raised -bd 1
    pack $w.bot -side bottom -fill both -expand 1
#
# Create a label and an entry widget in the top 2 frames.
#
    message $w.top.msg -justify left -width 8c -anchor w -text $msg1
    entry $w.top.entry -relief sunken -bd 2 -width 30
    pack $w.top.msg $w.top.entry -side top -anchor w

    message $w.mid.msg -justify left -width 8c -anchor w -text $msg2
    entry $w.mid.entry -relief sunken -bd 2 -width 30
    pack $w.mid.msg $w.mid.entry -side top -anchor w
#
# Create three buttons in the bottom frame.
#
    button $w.bot.ok -text OK
    button $w.bot.cancel -text Cancel -command "wm withdraw $w"
    button $w.bot.help -text Help -state disabled
    pack $w.bot.ok $w.bot.cancel $w.bot.help -side left -expand 1 -pady 2m -padx 2m
#
# Arrange for carriage-return to invoke the OK key.
#
    bind $w.top.entry <Return> [list focus $w.mid.entry]
    bind $w.mid.entry <Return> "$w.bot.ok invoke"
}

#-----------------------------------------------------------------------
# Create an unmapped image dialog
#
# This is split into a top section and a bottom section.
# The top section contains a title and an entry widget $w.top.entry.
# The bottom section contains three buttons, $w.bot.ok, $w.bot.cancel
# and $w.bot.help. Only the "close" button is assigned a command.
# The other buttons should be set by the caller. Note that the help
# button is displayed disabled
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w        The name to give the widget.
#  title    The title to give the dialog.
#  label    The message to place above the entry widget.
#-----------------------------------------------------------------------
proc create_image_dialog {w title msg1 msg2} {

    global units
#
# Create the toplevel dialog window withdrawn.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create four frames.
#
    frame $w.filename -relief raised -bd 1
    pack $w.filename -side top -fill both -expand 1
    frame $w.imagename -relief raised -bd 1
    pack $w.imagename -side top -fill both -expand 1
    frame $w.units -relief raised -bd 1
    pack $w.units -side top -fill both -expand 1
    frame $w.buttons -relief raised -bd 1
    pack $w.buttons -side bottom -fill both -expand 1
#
# Create a label and an entry widget in the top 2 frames.
#
    message $w.filename.msg -justify left -width 10c -anchor w -text $msg1
    entry $w.filename.entry -relief sunken -bd 2 -width 30
    pack $w.filename.msg $w.filename.entry -side top -anchor w

    message $w.imagename.msg -justify left -width 10c -anchor w -text $msg2
    entry $w.imagename.entry -relief sunken -bd 2 -width 30
    pack $w.imagename.msg $w.imagename.entry -side top -anchor w

#
# Create a radio button array in the next frame.
#

    message $w.units.msg -justify left -width 12c -anchor w -text "Units (will not override units defined in the FITS header):"
    pack $w.units.msg -side top -anchor w

    set ib 0
    set unitnames {MJy/sr Jy Jy/beam muK K Y}

    foreach item $unitnames {
	radiobutton $w.units.b$ib -variable units -text $item -value [string toupper $item]
	pack $w.units.b$ib -side left
	incr ib
    }

    set units [string toupper "muK"]
#
# Create three buttons in the bottom frame.
#
    button $w.buttons.ok -text OK
    button $w.buttons.cancel -text Cancel -command "wm withdraw $w"
    button $w.buttons.help -text Help -state disabled
    pack $w.buttons.ok $w.buttons.cancel $w.buttons.help -side left -expand 1 -pady 2m -padx 2m
#
# Arrange for carriage-return to invoke the OK key.
#
    bind $w.filename.entry <Return> [list focus $w.imagename.entry]
    bind $w.imagename.entry <Return> "$w.buttons.ok invoke"
}
#-----------------------------------------------------------------------
# Create an unmapped 4-prompt dialog.
#
# This is split into a top section and a bottom section.
# The top section contains a title and an entry widget $w.top.entry.
# The bottom section contains three buttons, $w.bot.ok, $w.bot.cancel
# and $w.bot.help. Only the "close" button is assigned a command.
# The other buttons should be set by the caller. Note that the help
# button is displayed disabled
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w        The name to give the widget.
#  title    The title to give the dialog.
#  label    The message to place above the entry widget.
#-----------------------------------------------------------------------
proc create_4prompt_dialog {w title msg1 msg2 msg3 msg4} {
#
# Create the toplevel dialog window withdrawn.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create 3 frames altogether.
#
    frame $w.f1 -relief raised -bd 1
    pack $w.f1 -side top -fill both -expand 1
    frame $w.f2 -relief raised -bd 1
    pack $w.f2 -side top -fill both -expand 1
    frame $w.f3 -relief raised -bd 1
    pack $w.f3 -side top -fill both -expand 1

# Subdivide each of the first two frames into two side-by-side frames.

    frame $w.f1.right -relief raised -bd 1
    frame $w.f1.left -relief raised -bd 1
    pack $w.f1.left -side left -fill both -expand 1
    pack $w.f1.right -side left -fill both -expand 1

    frame $w.f2.right -relief raised -bd 1
    frame $w.f2.left -relief raised -bd 1
    pack $w.f2.left -side left -fill both -expand 1
    pack $w.f2.right -side left -fill both -expand 1

#
# Create label and entry widgets in the top 3 frames.
#

    message $w.f1.left.msg -justify left -width 8c -anchor w -text $msg1
    entry $w.f1.left.entry -relief sunken -bd 2 -width 30
    pack $w.f1.left.msg $w.f1.left.entry -side top -anchor w

    message $w.f1.right.msg -justify right -width 8c -anchor w -text $msg2
    entry $w.f1.right.entry -relief sunken -bd 2 -width 30
    pack $w.f1.right.msg $w.f1.right.entry -side top -anchor w

    message $w.f2.left.msg -justify left -width 8c -anchor w -text $msg3
    entry $w.f2.left.entry -relief sunken -bd 2 -width 30
    pack $w.f2.left.msg $w.f2.left.entry -side top -anchor w

    message $w.f2.right.msg -justify right -width 8c -anchor w -text $msg4
    entry $w.f2.right.entry -relief sunken -bd 2 -width 30
    pack $w.f2.right.msg $w.f2.right.entry -side top -anchor w

#
# Create three buttons in the bottom frame.
#
    button $w.f3.ok -text OK
    button $w.f3.cancel -text Cancel -command "wm withdraw $w"
    button $w.f3.help -text Help -state disabled
    pack $w.f3.ok $w.f3.cancel $w.f3.help -side left -expand 1 -pady 2m -padx 2m
#
# Arrange for carriage-return to step through the entrues.  The last will
# invoke the OK key.
#
    bind $w.f1.left.entry <Return> [list focus $w.f1.right.entry]
    bind $w.f1.right.entry <Return> [list focus $w.f2.left.entry]
    bind $w.f2.left.entry <Return> [list focus $w.f2.right.entry]
    bind $w.f2.right.entry <Return> "$w.f3.ok invoke"
# 
# Arrange for tab to step through the entries.  The last will return to the first field
#
#    bind $w.f1.left.entry <Right> [list focus $w.f1.right.entry]
#    bind $w.f1.left.entry <Left> [list focus $w.f2.right.entry]
    bind $w.f1.left.entry <Down> [list focus $w.f2.left.entry]


#    bind $w.f1.right.entry <Right> [list focus $w.f2.left.entry]
#    bind $w.f1.right.entry <Left> [list focus $w.f1.left.entry]
    bind $w.f1.right.entry <Down> [list focus $w.f2.right.entry]

#    bind $w.f2.left.entry <Right> [list focus $w.f2.right.entry]
#    bind $w.f2.left.entry <Left> [list focus $w.f1.right.entry]
    bind $w.f2.left.entry <Up> [list focus $w.f1.left.entry]


#    bind $w.f2.right.entry <Right> [list focus $w.f1.left.entry]
#    bind $w.f2.right.entry <Left> [list focus $w.f2.left.entry]
    bind $w.f2.right.entry <Up> [list focus $w.f1.right.entry]

}
#-----------------------------------------------------------------------
# Create an unmapped 5-prompt dialog.
#
# This is split into four sections.  The top three sections each contain 
# two side-by-side title and an entry widget $w.f1-f3.entrys.
# The bottom section contains three buttons, $w.bot.ok, $w.bot.cancel
# and $w.bot.help. Only the "close" button is assigned a command.
# The other buttons should be set by the caller. Note that the help
# button is displayed disabled
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w        The name to give the widget.
#  title    The title to give the dialog.
#  label    The message to place above the entry widget.
#-----------------------------------------------------------------------
proc create_5prompt_dialog {w title msg1 msg2 msg3 msg4 msg5} {
#
# Create the toplevel dialog window withdrawn.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create 4 frames altogether.
#
    frame $w.f1 -relief raised -bd 1
    pack $w.f1 -side top -fill both -expand 1

    frame $w.f2 -relief raised -bd 1
    pack $w.f2 -side top -fill both -expand 1

    frame $w.f3 -relief raised -bd 1
    pack $w.f3 -side top -fill both -expand 1

    frame $w.f4 -relief raised -bd 1
    pack $w.f4 -side top -fill both -expand 1

    frame $w.f5 -relief raised -bd 1
    pack $w.f5 -side bottom -fill both -expand 1

# Subdivide the first two frames into two side-by-side frames.

    frame $w.f1.right -relief raised -bd 1
    frame $w.f1.left -relief raised -bd 1
    pack $w.f1.left -side left -fill both -expand 1
    pack $w.f1.right -side left -fill both -expand 1

    frame $w.f2.right -relief raised -bd 1
    frame $w.f2.left -relief raised -bd 1
    pack $w.f2.left -side left -fill both -expand 1
    pack $w.f2.right -side left -fill both -expand 1
#
# Create 2 label and entry widgets in the top 2 frames.
#
    message $w.f1.left.msg -justify left -width 8c -anchor w -text $msg1
    entry $w.f1.left.entry -relief sunken -bd 2 -width 30
    pack $w.f1.left.msg $w.f1.left.entry -side top -anchor w

    message $w.f1.right.msg -justify left -width 8c -anchor w -text $msg2
    entry $w.f1.right.entry -relief sunken -bd 2 -width 30
    pack $w.f1.right.msg $w.f1.right.entry -side top -anchor w

    message $w.f2.left.msg -justify left -width 8c -anchor w -text $msg3
    entry $w.f2.left.entry -relief sunken -bd 2 -width 30
    pack $w.f2.left.msg $w.f2.left.entry -side top -anchor w

    message $w.f2.right.msg -justify left -width 8c -anchor w -text $msg4
    entry $w.f2.right.entry -relief sunken -bd 2 -width 30
    pack $w.f2.right.msg $w.f2.right.entry -side top -anchor w

# Create only one in the third frame

    message $w.f3.msg -justify left -width 8c -anchor w -text $msg5
    entry $w.f3.entry -relief sunken -bd 2 -width 30
    pack $w.f3.msg $w.f3.entry -side top -anchor w

#
# Create three buttons in the bottom frame.
#
    button $w.f4.ok -text OK
    button $w.f4.cancel -text Cancel -command "wm withdraw $w"
    button $w.f4.help -text Help -state disabled
    pack $w.f4.ok $w.f4.cancel $w.f4.help -side left -expand 1 -pady 2m -padx 2m
#
# Arrange for carriage-return to step through the entries.  The last will
# invoke the OK key.
#
    bind $w.f1.left.entry <Return> [list focus $w.f1.right.entry]
    bind $w.f1.right.entry <Return> [list focus $w.f2.left.entry]
    bind $w.f2.left.entry <Return> [list focus $w.f2.right.entry]
    bind $w.f2.right.entry <Return> [list focus $w.f3.entry]
    bind $w.f3.entry <Return> "$w.f4.ok invoke"
}

#-----------------------------------------------------------------------
# Create an unmapped help dialog.
#
# Note that the dialog is not initially mapped. To display it temporarily
# use the command {wm deiconify $w} and then when it is no longer required
# call {wm withdraw $w}.
#
# Input:
#  w           The name to give the widget.
#  title       The dialog title.
#  text        The text to display in the widget.
#-----------------------------------------------------------------------
proc create_help_dialog {w title text} {
#
# Create the dialog container and tell the window-manager what to call
# both it and its icon.
#
    toplevel $w -class dialog
    wm withdraw $w
    wm title $w $title
    wm iconname $w Dialog
#
# Create the top-half of the dialog and display display the usage message
# in it.
#
    frame $w.top -relief raised -bd 1
    message $w.top.msg -width 12c -text $text
    pack $w.top.msg -side left -expand 1 -fill both
#
# Create the bottom half of the dialog and place a single OK button in
# it. Arrange that pressing the OK button unmaps the dialog.
#
    frame $w.bot -relief raised -bd 1
    button $w.bot.ok -text OK -command "wm withdraw $w"
    pack $w.bot.ok -pady 2m
#
# Arrange for carriage-return to invoke the OK key.
#
    bind $w <Return> "$w.bot.ok invoke"
#
# Place the widgets in their assigned places top and bottom.
#
    pack $w.top $w.bot -side top -fill both -expand 1
}

#-----------------------------------------------------------------------
# Create a labelled option menu.
#
# The name of the menu widget will be $w.menu and the option-menu value
# will be maintained in a global variable of name global$w.menu.
#
# Input:
#  w          The name for the frame-widget that encloses the menu.
#  label      The label to place to the left of the option-menu button.
#  cmd        The command to be called whenever the option-menu value
#             is changed. This will be called as a "trace variable"
#             callback, whenever global$w.menu is written to.
#  name_list  The list of option names.
#-----------------------------------------------------------------------
proc create_option_menu {w label cmd name_list} {
#
# Create a frame to enclose the menu.
#
    frame $w
#
# Create the option-menu label.
#
    label $w.label -text $label
#
# Get the name of the variable this is to be used to trace menu-value
# changes.
#
    set var global$w.menu
    global $var
#
# Create the option menu.
#
    eval tk_optionMenu $w.menu $var $name_list
    trace variable $var w $cmd
#
# Set the width of the menu button to be the maxmimum of all
# menu options. This removes the need for dynamic resizing.
#
    set maxwidth 0
    foreach name $name_list {
	set length [string length $name]
	if [ expr $length > $maxwidth ] {
	    set maxwidth $length
	}
    }
    $w.menu configure -width $maxwidth
#
# Place the label to the left of the menu button.
#
    pack $w.label $w.menu -side left
}
#-----------------------------------------------------------------------
# Create an unmapped image dialog
#-----------------------------------------------------------------------
proc create_newim_dialog {w} {
    create_image_dialog $w "Read a FITS image:" "Filename:" "Image Name:"

    $w.buttons.ok configure -command "wm withdraw $w;update;newim_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped power spectrum dialog 
#-----------------------------------------------------------------------
proc create_powind_dialog {w} {
    create_4prompt_dialog $w "Power Spectrum" "Image Name:" "Power law index n (p(l) ~ l^n):" "Field size (degrees)" "N pixels:" 

    $w.f3.ok configure -command "wm withdraw $w;update;powind_image_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped point source generator dialog
#-----------------------------------------------------------------------
proc create_ptsrc_dialog {w} {
    create_5prompt_dialog $w "Ptsrc" "Power law normalization (#/sr):" "Power law spectral index:" "Field size (degrees)" "N pixels" "Beam fwhm (arcmin)"

    $w.f4.ok configure -command "wm withdraw $w;update;ptsrc_image_callback $w"
    $w.f1.left.entry insert 0 "90"
    $w.f1.right.entry insert 0 "2.5"
    $w.f2.left.entry insert 0 "5"
    $w.f2.right.entry insert 0 "256"
    $w.f3.entry insert 0 "18"
}
#-----------------------------------------------------------------------
# Create an unmapped power spectrum dialog (gaussian)
#-----------------------------------------------------------------------
proc create_gsig_dialog {w} {
    create_4prompt_dialog $w "Power Spectrum" "Image Name:" "Gaussian width (dl_FWHM):" "Field size (degrees):" "N pixels:"

    $w.f3.ok configure -command "wm withdraw $w;update;gsig_image_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped power spectrum dialog (file)
#-----------------------------------------------------------------------
proc create_pfile_dialog {w} {
    create_4prompt_dialog $w "Power Spectrum" "Image Name:" "File name:" "Field size (degrees):" "N pixels:"

    $w.f3.ok configure -command "wm withdraw $w;update;pfile_image_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped radplot dialog.
#-----------------------------------------------------------------------
proc create_rad_dialog {w} {
    create_prompt_dialog $w "Radplot" "Enter the number of bins:"
    $w.bot.ok configure -command "wm withdraw $w;update;rad_image_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped want-image dialog.
#-----------------------------------------------------------------------
proc create_want_dialog {w} {
    create_prompt_dialog $w "Antenna file" "Enter an output file name:"
    $w.bot.ok configure -command "wm withdraw $w;update;want_image_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped ant-image dialog.
#-----------------------------------------------------------------------
proc create_ant_dialog {w} {
    create_prompt_dialog $w "Antenna file" "Enter a tcl file name:"
    $w.bot.ok configure -command "wm withdraw $w;update;ant_image_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped write visiblities (.dat format) dialog.
#-----------------------------------------------------------------------
proc create_wdat_dialog {w} {
    create_prompt_dialog $w "Output visibilities (.dat)" "Enter an output file name:"
    $w.bot.ok configure -command "wm withdraw $w;update;wdat_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped write visiblities (.bin format) dialog.
#-----------------------------------------------------------------------
proc create_wbin_dialog {w} {
    create_prompt_dialog $w "Output visibilities (.bin)" "Enter an output file name:"
    $w.bot.ok configure -command "wm withdraw $w;update;wbin_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped write visiblities dialog.
#-----------------------------------------------------------------------
proc create_wvis_dialog {w} {
    global isouv

    create_prompt_dialog $w "Output visibilities" "Enter an output file name:"
    $w.bot.ok configure -command "wm withdraw $w;update;wvis_callback $w"
}
#-----------------------------------------------------------------------
# Create an unmapped read visiblities dialog.
#-----------------------------------------------------------------------
proc create_rvis_dialog {w} {
    create_prompt_dialog $w "Input visibilities" "Enter a FITS file name:"
    $w.bot.ok configure -command "wm withdraw $w;update;rvis_callback $w"
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# ant-image dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc want_image_callback {w} {
    simint writeant [$w.top.entry get]
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# ant-image dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc ant_image_callback {w} {
    source [$w.top.entry get]
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# ant-image dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc powind_image_callback {w} {
    global dispvis
    set dispvis 0
# 
# Note that the type-specific argument is passed to the powspec command last, even though it
# is the second prompt in the dialog box.
#
    simint powspec "pow" [$w.f1.left.entry get] [$w.f2.left.entry get] [$w.f2.right.entry get] [$w.f1.right.entry get] 

# Add the newly created image to the list of known images.

    addim $w.f1.left.entry

# And insert the newly requested image size and npix as the default in the dialog boxes 
# for the other types of power spectra

    .pfile.f2.left.entry delete 0 end
    .pfile.f2.left.entry insert 0 [$w.f2.left.entry get] 
    .pfile.f2.right.entry delete 0 end
    .pfile.f2.right.entry insert 0 [$w.f2.right.entry get] 

    .gsig.f2.left.entry delete 0 end
    .gsig.f2.left.entry insert 0 [$w.f2.left.entry get] 
    .gsig.f2.right.entry delete 0 end
    .gsig.f2.right.entry insert 0 [$w.f2.right.entry get] 

}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# ptsrc generator dialog.
#
# Input:
#  w         The name of the dialog
#-----------------------------------------------------------------------
proc ptsrc_image_callback {w} {
    global dispvis

    set dispvis 0
    simint ptsrc [$w.f1.left.entry get] [$w.f1.right.entry get] [$w.f2.left.entry get] [$w.f2.right.entry get] [$w.f3.entry get]

    set m .newim

    $m.imagename.entry delete 0 end
    $m.imagename.entry insert 0 "ptsrc"
    addim $m.imagename.entry
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# ant-image dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc gsig_image_callback {w} {
    global dispvis
    set dispvis 0
# 
# Note that the type-specific argument is passed to the powspec command last, even though it
# is the second prompt in the dialog box.
#
    simint powspec "gauss" [$w.f1.left.entry get] [$w.f2.left.entry get] [$w.f2.right.entry get] [$w.f1.right.entry get] 

# Add the newly created image to the list of known images.

    addim $w.f1.left.entry

# And insert the newly requested image size and npix as the default in the dialog boxes 
# for the other types of power spectra

    .powind.f2.left.entry delete 0 end
    .powind.f2.left.entry insert 0 [$w.f2.left.entry get] 
    .powind.f2.right.entry delete 0 end
    .powind.f2.right.entry insert 0 [$w.f2.right.entry get] 

    .pfile.f2.left.entry delete 0 end
    .pfile.f2.left.entry insert 0 [$w.f2.left.entry get] 
    .pfile.f2.right.entry delete 0 end
    .pfile.f2.right.entry insert 0 [$w.f2.right.entry get] 

}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# ant-image dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc pfile_image_callback {w} {
    global dispvis
    set dispvis 0
# 
# Note that the type-specific argument is passed to the powspec command last, even though it
# is the second prompt in the dialog box.
#
    simint powspec "file" [$w.f1.left.entry get] [$w.f2.left.entry get] [$w.f2.right.entry get] [$w.f1.right.entry get] 

# Add the newly created image to the list of known images.

    addim $w.f1.left.entry

# And insert the newly requested image size and npix as the default in the dialog boxes 
# for the other types of power spectra

    .powind.f2.left.entry delete 0 end
    .powind.f2.left.entry insert 0 [$w.f2.left.entry get] 
    .powind.f2.right.entry delete 0 end
    .powind.f2.right.entry insert 0 [$w.f2.right.entry get] 

    .gsig.f2.left.entry delete 0 end
    .gsig.f2.left.entry insert 0 [$w.f2.left.entry get] 
    .gsig.f2.right.entry delete 0 end
    .gsig.f2.right.entry insert 0 [$w.f2.right.entry get] 

}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# newim dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc newim_callback {w} {

    global dispvis
    global units

    set dispvis 0

    simint rfits [$w.filename.entry get] [$w.imagename.entry get] $units
    
    addim $w.imagename.entry
}

proc read_image {name handle} {
    global dispvis
    set dispvis 0

    simint rfits [$name] [$handle]
    
    addim $handle
}
#-----------------------------------------------------------------------
# This function initializes the image list by adding a bogus entry
#
# Input: 
#  w        The name of the image menu
#-----------------------------------------------------------------------
proc initim {w} {
    
    global images
    global nimage
    global bg
    global curimage

# Store the name of this image.

    set images($nimage) "none"

# And increment the global image counter
    
    set nimage [expr $nimage+1]

    set m .ftop.menubar
    
    $m.im.menu add separator
    $m.im.menu add command -label "none" -command {
	global images
	global nimage
	global curimage
	global bg
	
# Reset the background color of the currently selected image

	.ftop.menubar.im.menu entryconfigure $curimage -background $bg
	set curimage [.ftop.menubar.im.menu index active] 
	.ftop.menubar.im.menu entryconfigure $curimage -background red
	
	set i [expr [.ftop.menubar.im.menu index active]/2 - 1]
	imslct $images($i)
    }

# Now highlight the image just read in

    set curimage [.ftop.menubar.im.menu index last] 
    .ftop.menubar.im.menu entryconfigure $curimage -background red

}
#-----------------------------------------------------------------------
# This function adds an image entry to the Image menu button
#
# Input: 
#  entry        The name of the entry widget in which the image name is located
#-----------------------------------------------------------------------
proc addim {entry} {
    
    global images
    global nimage
    global bg
    global curimage

    set imname [$entry get]

# Only create a new entry if this image doesn't already exist.

    set exists {false}
    for {set i 0} {$i < $nimage} {incr i} {
	if {[string match $images($i) $imname]} {
	    set exists {true}
	    set imind [expr 2*($i+1)]
	}
    }

    if {[string match $exists false]} {

# Store the name of this image.

	set images($nimage) [$entry get]

# And increment the global image counter

	set nimage [expr $nimage+1]
	
	set m .ftop.menubar

	$m.im.menu add separator
	$m.im.menu add command -label [$entry get] -command {
	    global images
	    global nimage
	    global curimage
	    global bg

# Reset the background color of the currently selected image

	    .ftop.menubar.im.menu entryconfigure $curimage -background $bg
	    set curimage [.ftop.menubar.im.menu index active] 
	    .ftop.menubar.im.menu entryconfigure $curimage -background red
	    
	    set i [expr [.ftop.menubar.im.menu index active]/2 - 1]
	    imslct $images($i)
	}

# Unset the currently selected image and highlight the image just read in

	.ftop.menubar.im.menu entryconfigure $curimage -background $bg
	set curimage [.ftop.menubar.im.menu index last] 
	.ftop.menubar.im.menu entryconfigure $curimage -background red
    } else {
# Else the image exists: unset the currently selected image and highlight the 
# names image:
    
	.ftop.menubar.im.menu entryconfigure $curimage -background $bg
	set curimage $imind
	.ftop.menubar.im.menu entryconfigure $curimage -background red
    }
}

proc imslct {w} {
    global dispvis
    set dispvis 0

    simint imslct $w
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# radplot dialog.
#
# Input:
#  w         The name of the ant dialog.
#-----------------------------------------------------------------------
proc rad_image_callback {w} {
   radplot [$w.top.entry get]
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# rvis dialog.
#
# Input:
#  w         The name of the vis dialog.
#-----------------------------------------------------------------------
proc rvis_callback {w} {
    simint readvis [$w.top.entry get]
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# wvis dialog.
#
# Input:
#  w         The name of the vis dialog.
#-----------------------------------------------------------------------
proc wvis_callback {w} {
    simint newwritevis [$w.top.entry get]
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# wdat dialog.
#
# Input:
#  w         The name of the dat dialog.
#-----------------------------------------------------------------------
proc wdat_callback {w} {
    simint writedat [$w.top.entry get] "dat"
}
#-----------------------------------------------------------------------
# This function is called when the user presses the OK button of the
# wbin dialog.
#
# Input:
#  w         The name of the bin dialog.
#-----------------------------------------------------------------------
proc wbin_callback {w} {
    simint writedat [$w.top.entry get] "bin"
}
#-----------------------------------------------------------------------
# Draw the currently selected image function.
#-----------------------------------------------------------------------
proc draw_image {args} {
    upvar #0 global.function.menu mode_menu
#
# Display a busy-cursor.
#
    . configure -cursor {watch}
    .imagearea.pgplot configure -cursor {}
    update
#
# Display the new function.
#
    simint function $mode_menu
#
# Reset the cursor.
#
    . configure -cursor {}
    .imagearea.pgplot configure -cursor {crosshair black white}
    update
#
# Arm the cursor of the image window for the selection of a slice.
#
    prepare_for_slice
}
#-----------------------------------------------------------------------
# Draw an antenna array
#-----------------------------------------------------------------------
proc draw_antarray {} {
    upvar #0 global.function.menu mode_menu

    global antarr
    global obslat
    global obslong
    global hastart
    global hastop
    global srcdec
    global antsize
    global wave
    global symm
#
# Display a busy-cursor.
#
    . configure -cursor {watch}
    .ftop.xyarea.pgplot configure -cursor {}
    .ftop.uvarea.pgplot configure -cursor {}
    update
#
# Display a blank antenna array field.
#
    simint antarray $antarr $antsize $wave $obslat $obslong $hastart $hastop $srcdec
#
# Reset the cursor.
#
    . configure -cursor {}
    .ftop.xyarea.pgplot configure -cursor {crosshair black white}
    .ftop.uvarea.pgplot configure -cursor {crosshair black white}
    update
#
# Bind the cursor of the image window for the selection of an antenna position
#
    bind .ftop.xyarea.pgplot <ButtonPress-1> {addant %x %y $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm 1}
    bind .ftop.xyarea.pgplot <ButtonPress-2> {markant %x %y $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm}
    bind .ftop.xyarea.pgplot <ButtonPress-3> {addant %x %y $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm 0}
#
# And arm the uvzoom command
#
    prepare_for_uvzoom
}
#-----------------------------------------------------------------------
# Reset the xyarea key bindings
#-----------------------------------------------------------------------
proc reset_xybindings {} {

    global antarr
    global obslat
    global obslong
    global hastart
    global hastop
    global srcdec
    global antsize
    global wave
    global symm

    .ftop.xyarea.pgplot setcursor norm 0.0 0.0 1
#
# Bind the cursor of the image window for the selection of an antenna position
#
    bind .ftop.xyarea.pgplot <ButtonPress-1> {addant %x %y $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm 1}
}

#-----------------------------------------------------------------------
# This receives the x and y coordinates of a point from the callback arguments
# provided by the pgplot widget.
#
# Input:
#  x1 y1          The coordinate of the start of the slice in the image
#                 window. These values were supplied when the callback
#                 was registered by start_slice.
#  wx2 wy2        The X-window coordinate of the end of the slice.
#-----------------------------------------------------------------------
proc addant {wx wy antsize wave obslat obslong hastart hastop srcdec symm add} {
    global dispvis

    set pg .ftop.xyarea.pgplot
#
# Convert from X coordinates to world coordinates.
#
    set x1 [$pg world x $wx]
    set y1 [$pg world y $wy]

# If adding an antenna, draw the antenna, then add the visibilities

    if {$add == 1} {
# Only mark the visibilities if we successfully added the antenna
	if {![catch {simint addant $x1 $y1 $antsize $wave $obslat $obslong $symm $add}]} {
# Only update the visibilities if we are currently displaying the visibilities
	    if {$dispvis} {
		simint markvis $x1 $y1 $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm $add
	    }
	}
    } else {
# If deleting an antenna, delete the visibilities, then remove the antenna
# Only update the visiblities if we are not currently displaying the beam
	if {$dispvis} {
	    simint markvis $x1 $y1 $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm $add
	}
	simint addant $x1 $y1 $antsize $wave $obslat $obslong $symm $add
    }
}
#-----------------------------------------------------------------------
# This receives the x and y coordinates of a point from the callback arguments
# provided by the pgplot widget.  
#
# Input:
#  x1 y1          The coordinate of the start of the slice in the image
#                 window. These values were supplied when the callback
#                 was registered by start_slice.
#  wx2 wy2        The X-window coordinate of the end of the slice.
#-----------------------------------------------------------------------
proc markant {wx wy antsize wave obslat obslong hastart hastop srcdec symm} {
    set pg .ftop.xyarea.pgplot
    global dispvis
#
# Convert from X coordinates to world coordinates.
#
    set x1 [$pg world x $wx]
    set y1 [$pg world y $wy]

    simint markant $x1 $y1 $antsize $wave $obslat $obslong $hastart $hastop $srcdec $symm $dispvis
}
#-----------------------------------------------------------------------
# Arm the uv-widget cursor such that when the user next presses a
# mouse button or key within the image window the start of a zoom
# will be selected.
#-----------------------------------------------------------------------
proc prepare_for_uvzoom {args} {
    
  .ftop.uvarea.pgplot setcursor norm 0.0 0.0 1
  bind .ftop.uvarea.pgplot <ButtonPress-1> {start_uvzoom %x %y}
}
#-----------------------------------------------------------------------
# Arm the xy-widget cursor such that when the user next presses a
# mouse button or key within the antenna window the start of a zoom
# will be selected.
#-----------------------------------------------------------------------
proc prepare_for_xyzoom {args} {
    
    .ftop.xyarea.pgplot setcursor norm 0.0 0.0 1
    bind .ftop.xyarea.pgplot <ButtonPress-1> {start_xyzoom %x %y}

}

#-----------------------------------------------------------------------
# Arm the image-widget cursor such that when the user next presses a
# mouse button or key within the image window the start of a slice
# will be selected.
#-----------------------------------------------------------------------
proc prepare_for_slice {args} {
  .ftop.imagearea.pgplot setcursor norm 0.0 0.0 1
  bind .ftop.imagearea.pgplot <ButtonPress> {start_slice %x %y}
}
#-----------------------------------------------------------------------
# This is used as a pgplot image-widget cursor callback. It augments the
# cursor in the image window with a line rubber-band anchored at the
# selected cursor position and registers a new callback to receive both
# the current coordinates and coordinates of the end of the slice when
# selected.
#
# Input:
#  wx wy   The X-window coordinates of the position that the user selected
#          with the cursor.
#-----------------------------------------------------------------------
proc start_xyzoom {wx wy} {
  set pg .ftop.xyarea.pgplot
#
# Convert from X coordinates to world coordinates.
#
  set x [$pg world x $wx]
  set y [$pg world y $wy]
  
  $pg setcursor rect $x $y 5
  bind $pg <ButtonPress-1> "end_xyzoom $x $y %x %y"
}
#-----------------------------------------------------------------------
# This is used as a pgplot image-widget cursor callback. It augments the
# cursor in the image window with a line rubber-band anchored at the
# selected cursor position and registers a new callback to receive both
# the current coordinates and coordinates of the end of the slice when
# selected.
#
# Input:
#  wx wy   The X-window coordinates of the position that the user selected
#          with the cursor.
#-----------------------------------------------------------------------
proc start_uvzoom {wx wy} {
  set pg .ftop.uvarea.pgplot
#
# Convert from X coordinates to world coordinates.
#
  set x [$pg world x $wx]
  set y [$pg world y $wy]
  
  $pg setcursor rect $x $y 5
  bind $pg <ButtonPress-1> "end_uvzoom $x $y %x %y"
}

#-----------------------------------------------------------------------
# This is used as a pgplot image-widget cursor callback. It augments the
# cursor in the image window with a line rubber-band anchored at the
# selected cursor position and registers a new callback to receive both
# the current coordinates and coordinates of the end of the slice when
# selected.
#
# Input:
#  wx wy   The X-window coordinates of the position that the user selected
#          with the cursor.
#-----------------------------------------------------------------------
proc start_slice {wx wy} {
  set pg .imagearea.pgplot
#
# Convert from X coordinates to world coordinates.
#
  set x [$pg world x $wx]
  set y [$pg world y $wy]
  $pg setcursor line $x $y 3
  bind $pg <ButtonPress> "end_slice $x $y %x %y"
}
#-----------------------------------------------------------------------
# This image-window pgplot-cursor callback is registered by start_slice.
# It receives the start coordinates of a slice from start_slice and
# the coordinate of the end of the slice from the callback arguments
# provided by the pgplot widget.
#
# Input:
#  x1 y1          The coordinate of the start of the slice in the image
#                 window. These values were supplied when the callback
#                 was registered by start_slice.
#  wx2 wy2        The X-window coordinate of the end of the slice.
#-----------------------------------------------------------------------
proc end_uvzoom {x1 y1 wx2 wy2} {
  set pg .ftop.uvarea.pgplot
  prepare_for_uvzoom

  set wx2 [$pg world x $wx2]
  set wy2 [$pg world y $wy2]

  simint uvzoom $x1 $y1 $wx2 $wy2
}
#-----------------------------------------------------------------------
# This image-window pgplot-cursor callback is registered by start_slice.
# It receives the start coordinates of a slice from start_slice and
# the coordinate of the end of the slice from the callback arguments
# provided by the pgplot widget.
#
# Input:
#  x1 y1          The coordinate of the start of the slice in the image
#                 window. These values were supplied when the callback
#                 was registered by start_slice.
#  wx2 wy2        The X-window coordinate of the end of the slice.
#-----------------------------------------------------------------------
proc end_xyzoom {x1 y1 wx2 wy2} {
    set pg .ftop.xyarea.pgplot
    reset_xybindings
    
    set wx2 [$pg world x $wx2]
    set wy2 [$pg world y $wy2]
    
    simint xyzoom $x1 $y1 $wx2 $wy2
    
    # And re-bind the z-key to start a zoom
    
    bind .ftop.xyarea.pgplot <KeyPress-z> {prepare_for_xyzoom}
}

#-----------------------------------------------------------------------
# This image-window pgplot-cursor callback is registered by start_slice.
# It receives the start coordinates of a slice from start_slice and
# the coordinate of the end of the slice from the callback arguments
# provided by the pgplot widget.
#
# Input:
#  x1 y1          The coordinate of the start of the slice in the image
#                 window. These values were supplied when the callback
#                 was registered by start_slice.
#  wx2 wy2        The X-window coordinate of the end of the slice.
#-----------------------------------------------------------------------
proc end_slice {x1 y1 wx2 wy2} {
  set pg .imagearea.pgplot
  prepare_for_slice
  simint slice $x1 $y1 [$pg world x $wx2] [$pg world y $wy2]
}

#-----------------------------------------------------------------------
# This procedure creates the main menubar of the application.
#
# Input:
#  w            The name to give the widget.
#-----------------------------------------------------------------------
proc create_main_menubar {w} {

    global wave
    global antarr
    global obslat
    global obslong
    global hastart
    global hastop
    global srcdec
    global xymax
    global nit
#
# Create a raised frame for the menubar.
#
    frame $w -relief raised -bd 2 -width 11c
#
# Create the file menu.
#
    menubutton $w.file -text File -menu $w.file.menu

    menu $w.file.menu -tearoff 0
    $w.file.menu add command -label {Load tcl file} -command {
	wm deiconify .ant
        raise .ant
    }
    $w.file.menu add separator
    $w.file.menu add command -label {Write out antenna file} -command {
	wm deiconify .want
        raise .want
    }
    $w.file.menu add separator
    $w.file.menu add command -label {Read in visibilities} -command {
	wm deiconify .rvis
        raise .rvis
    }
    $w.file.menu add separator
    $w.file.menu add command -label {Write out .uvf file} -command {
	wm deiconify .wvis
        raise .wvis
    }
    $w.file.menu add separator
    $w.file.menu add command -label {Write out .bin file} -command {
	wm deiconify .wbin
        raise .wbin
    }
    $w.file.menu add separator
    $w.file.menu add command -label {Write out .dat file} -command {
	wm deiconify .wdat
        raise .wdat
    }
    $w.file.menu add separator
    $w.file.menu add command -label {Quit} -command {exit}

# Define a menubutton that will display the main menu

    menubutton $w.array -text Array -menu $w.array.menu

# Define the main menu

    menu $w.array.menu -tearoff 0

# Define a cascaded menu that will descend from the DASI and VLA entries

    menu $w.array.menu.vla -tearoff 0
    menu $w.array.menu.dasi -tearoff 0

    $w.array.menu add cascade -label "DASI" -menu $w.array.menu.dasi
    $w.array.menu add radio -label "User defined" -variable antarr -value user
    $w.array.menu add radio -label "SZA" -variable antarr -value sza
    $w.array.menu add radio -label "JCA" -variable antarr -value jca 
    $w.array.menu add radio -label "ATI compact" -variable antarr -value ati_compact
    $w.array.menu add radio -label "ATI hetero" -variable antarr -value ati_hetero
    $w.array.menu add cascade -label "VLA" -menu $w.array.menu.vla

    $w.array.menu.dasi add radio -label "26.5 GHz" -variable wave -value 1.13 -command {set antarr dasi} 
    $w.array.menu.dasi add radio -label "27.5 GHz" -variable wave -value 1.09 -command {set antarr dasi} 
    $w.array.menu.dasi add radio -label "28.5 GHz" -variable wave -value 1.05 -command {set antarr dasi}  
    $w.array.menu.dasi add radio -label "29.5 GHz" -variable wave -value 1.02 -command {set antarr dasi}
    $w.array.menu.dasi add radio -label "30.5 GHz" -variable wave -value 0.98 -command {set antarr dasi}  
    $w.array.menu.dasi add radio -label "31.5 GHz" -variable wave -value 0.95 -command {set antarr dasi}
    $w.array.menu.dasi add radio -label "32.5 GHz" -variable wave -value 0.92 -command {set antarr dasi}
    $w.array.menu.dasi add radio -label "33.5 GHz" -variable wave -value 0.90 -command {set antarr dasi}
    $w.array.menu.dasi add radio -label "34.5 GHz" -variable wave -value 0.87 -command {set antarr dasi}  
    $w.array.menu.dasi add radio -label "35.5 GHz" -variable wave -value 0.85 -command {set antarr dasi}
    $w.array.menu.dasi add radio -label "all" -variable wave -value 0.00 -command {set antarr dasi}


    $w.array.menu.vla add cascade -label "A Array" -menu $w.array.menu.vla.banda
    $w.array.menu.vla add cascade -label "B Array" -menu $w.array.menu.vla.bandb
    $w.array.menu.vla add cascade -label "C Array" -menu $w.array.menu.vla.bandc
    $w.array.menu.vla add cascade -label "D Array" -menu $w.array.menu.vla.bandd

# Define a cascaded menus that will descend from these

    menu $w.array.menu.vla.banda -tearoff 0
    menu $w.array.menu.vla.bandb -tearoff 0
    menu $w.array.menu.vla.bandc -tearoff 0
    menu $w.array.menu.vla.bandd -tearoff 0

    $w.array.menu.vla.banda add radio -label "P Band" -variable tsys -value 165 -variable wave -value 90 -command {set antarr a} 
    $w.array.menu.vla.banda add radio -label "L Band" -variable tsys -value 56 -variable wave -value 20 -command {set antarr a} 
    $w.array.menu.vla.banda add radio -label "C Band" -variable tsys -value 44 -variable wave -value 6 -command {set antarr a}  
    $w.array.menu.vla.banda add radio -label "X Band" -variable tsys -value 34 -variable wave -value 3.6 -command {set antarr a}
    $w.array.menu.vla.banda add radio -label "U Band" -variable tsys -value 110 -variable wave -value 2 -command {set antarr a}  
    $w.array.menu.vla.banda add radio -label "K Band" -variable tsys -value 175 -variable wave -value 1.3 -command {set antarr a}
    $w.array.menu.vla.banda add radio -label "Q Band" -variable tsys -value 115 -variable wave -value 0.7 -command {set antarr a}
						 
    $w.array.menu.vla.bandb add radio -label "P Band" -variable tsys -value 165 -variable wave -value 90 -command {set antarr b} 
    $w.array.menu.vla.bandb add radio -label "L Band" -variable tsys -value 56 -variable wave -value 20 -command {set antarr b} 
    $w.array.menu.vla.bandb add radio -label "C Band" -variable tsys -value 44 -variable wave -value 6 -command {set antarr b}  
    $w.array.menu.vla.bandb add radio -label "X Band" -variable tsys -value 34 -variable wave -value 3.6 -command {set antarr b}
    $w.array.menu.vla.bandb add radio -label "U Band" -variable tsys -value 110 -variable wave -value 2 -command {set antarr b}  
    $w.array.menu.vla.bandb add radio -label "K Band" -variable tsys -value 175 -variable wave -value 1.3 -command {set antarr b}
    $w.array.menu.vla.bandb add radio -label "Q Band" -variable tsys -value 115 -variable wave -value 0.7 -command {set antarr b}
						 	    
    $w.array.menu.vla.bandc add radio -label "P Band" -variable tsys -value 165 -variable wave -value 90 -command {set antarr c} 
    $w.array.menu.vla.bandc add radio -label "L Band" -variable tsys -value 56 -variable wave -value 20 -command {set antarr c} 
    $w.array.menu.vla.bandc add radio -label "C Band" -variable tsys -value 44 -variable wave -value 6 -command {set antarr c}  
    $w.array.menu.vla.bandc add radio -label "X Band" -variable tsys -value 34 -variable wave -value 3.6 -command {set antarr c}
    $w.array.menu.vla.bandc add radio -label "U Band" -variable tsys -value 110 -variable wave -value 2 -command {set antarr c}  
    $w.array.menu.vla.bandc add radio -label "K Band" -variable tsys -value 175 -variable wave -value 1.3 -command {set antarr c}
    $w.array.menu.vla.bandc add radio -label "Q Band" -variable tsys -value 115 -variable wave -value 0.7 -command {set antarr c}
						 	    
    $w.array.menu.vla.bandd add radio -label "P Band" -variable tsys -value 165 -variable wave -value 90 -command {set antarr d} 
    $w.array.menu.vla.bandd add radio -label "L Band" -variable tsys -value 56 -variable wave -value 20 -command {set antarr d} 
    $w.array.menu.vla.bandd add radio -label "C Band" -variable tsys -value 44 -variable wave -value 6 -command {set antarr d}  
    $w.array.menu.vla.bandd add radio -label "X Band" -variable tsys -value 34 -variable wave -value 3.6 -command {set antarr d}
    $w.array.menu.vla.bandd add radio -label "U Band" -variable tsys -value 110 -variable wave -value 2 -command {set antarr d}  
    $w.array.menu.vla.bandd add radio -label "K Band" -variable tsys -value 175 -variable wave -value 1.3 -command {set antarr d}
    $w.array.menu.vla.bandd add radio -label "Q Band" -variable tsys -value 115 -variable wave -value 0.7 -command {set antarr d}

#
# Define function buttons
#
    button $w.update -text "Display Visibilities" -command compvis

    button $w.radplot -text "Radplot" -command {
	wm deiconify .rad
        raise .rad
    }

    button $w.beam -text "Compute Beam" -command compbeam

    button $w.win -text "Compute Window" -command compwin

    button $w.obsim -text "Observe Image" -command obsim
#
# Define a button for ptsrc generation
#
    button $w.ptsrc -text "Ptsrc" -command {
	wm deiconify .ptsrc
        raise .ptsrc
    }

#
# Define a button for image manipulation
#
    menubutton $w.im -text "Images" -menu $w.im.menu 

# Define the associated menu

    menu $w.im.menu -tearoff 0
    $w.im.menu add command -label {New Image} -command {
	wm deiconify .newim
        raise .newim
    }
#    $w.im.menu entryconfigure 0 -background white
#
# Define a button for power spectrum computation
#
    menubutton $w.pow -text "Power Spectrum" -menu $w.pow.menu 

# Define the associated menu

    menu $w.pow.menu -tearoff 0
    $w.pow.menu add command -label {Power} -command {
	wm deiconify .powind
        raise .powind
    }
    $w.pow.menu add separator
    $w.pow.menu add command -label {Gaussian} -command {
	wm deiconify .gsig
        raise .gsig
    }
    $w.pow.menu add separator
    $w.pow.menu add command -label {File} -command {
	wm deiconify .pfile
        raise .pfile
    }
#
# Arrange that Alt-Q will abort the application.
#
    bind all <Alt-KeyPress-q> {exit}
    bind all <KeyPress-z> {prepare_for_xyzoom;prepare_for_uvzoom}
#
# Create the help menu.
#
    menubutton $w.help -text Help -menu $w.help.menu
    menu $w.help.menu -tearoff 0
    $w.help.menu add command -label {Usage} -command {
	wm deiconify .usage_help
        raise .usage_help
    }
#
# Pack all but the help menu at the left side of the menubar.
#
    pack $w.file -side left
    pack $w.array -side left
    pack $w.update -side left
    pack $w.radplot -side left
#    pack $w.beam -side left
#    pack $w.win -side left
#    pack $w.obsim -side left
    pack $w.ptsrc -side left
    pack $w.im -side left
    pack $w.pow -side left

#
# Pack the help menu against the right edge of the menubar, as specified
# by the Motif style guide.
#
    pack $w.help -side right
}

#-----------------------------------------------------------------------
# Create an area in which to display the world coordinates of the cursor
# when it is over the image window.
#
# Input:
#  w            The name to give the frame widget that encloses the area.
#-----------------------------------------------------------------------
proc create_world_labels {w} {
    global symm
#
# Enclose the area in a frame.
#
    frame $w -width 11c -height 1c
#
# Create a static title label.
#
    label $w.title -text "World coordinates: "
#
# Create the X and Y labels for displaying the respective coordinates.
#
    label $w.x -width 12 -anchor w
    label $w.y -width 12 -anchor w
    pack $w.title -side left -anchor w
    pack $w.x $w.y -side left -anchor w -padx 2m
# 
# Create a button to toggle 3-fold symmetry 
#
    menubutton $w.symm -text "Symmetry" -menu $w.symm.menu
    menu $w.symm.menu -tearoff 0

    $w.symm.menu add radio -label "None" -variable symm -value 1
    $w.symm.menu add radio -label "2-fold" -variable symm -value 2
    $w.symm.menu add radio -label "3-fold" -variable symm -value 3
    $w.symm.menu add radio -label "4-fold" -variable symm -value 4
    $w.symm.menu add radio -label "5-fold" -variable symm -value 5
    $w.symm.menu add radio -label "6-fold" -variable symm -value 6
    $w.symm.menu add radio -label "7-fold" -variable symm -value 7
    $w.symm.menu add radio -label "8-fold" -variable symm -value 8
    $w.symm.menu add radio -label "9-fold" -variable symm -value 9

    pack $w.symm -side left 
}

#-----------------------------------------------------------------------
# Create the area that contains the image PGPLOT window.
#
# Input:
#  w            The name to give the frame widget that encloses the area.
#-----------------------------------------------------------------------
proc create_image_area {w} {
#
# Frame the workarea.
#
  frame $w -width 11c -height 11c
#
# Create the PGPLOT image window.
#
  pgplot $w.pgplot -width 10c -height 10c -maxcolors 1 -bd 2 -bg black -fg white
#
# Create horizontal and vertical scroll-bars and have them
# call the pgplot xview and yview scroll commands to scroll the
# image within the window.
#
  scrollbar $w.xscroll -command "$w.pgplot xview" -orient horizontal
  scrollbar $w.yscroll -command "$w.pgplot yview" -orient vertical
#
# Tell the PGPLOT widget how to update the scrollbar sliders.
#
  $w.pgplot configure -xscrollcommand "$w.xscroll set"
  $w.pgplot configure -yscrollcommand "$w.yscroll set"
#
# Position the PGPLOT widget and the scrollbars.
#
  pack $w.xscroll -side bottom -fill x
  pack $w.yscroll -side right -fill y
  pack $w.pgplot -side left -fill both -expand true
#
# Bind motion events to the world coordinate x and y label widgets.
#
  bind $w.pgplot <Motion> {report_motion %W %x %y}
}
#-----------------------------------------------------------------------
# Create the area that contains the xy plot area.
#
# Input:
#  w            The name to give the frame widget that encloses the area.
#-----------------------------------------------------------------------
proc create_xy_area {w} {
#
# Frame the workarea.
#
  frame $w -width 16c -height 16c
#
# Create the PGPLOT image window.
#
  pgplot $w.pgplot -width 15c -height 15c -maxcolors 16 -bd 2 -bg black -fg white 

# Create horizontal and vertical scroll-bars and have them
# call the pgplot xview and yview scroll commands to scroll the
# image within the window.
#
  scrollbar $w.xscroll -command "$w.pgplot xview" -orient horizontal
  scrollbar $w.yscroll -command "$w.pgplot yview" -orient vertical
#
# Tell the PGPLOT widget how to update the scrollbar sliders.
#
  $w.pgplot configure -xscrollcommand "$w.xscroll set"
  $w.pgplot configure -yscrollcommand "$w.yscroll set"
#
# Position the PGPLOT widget and the scrollbars.
#
  pack $w.xscroll -side bottom -fill x
  pack $w.yscroll -side right -fill y
  pack $w.pgplot -side left -fill both -expand true
#
# Bind motion events to the world coordinate x and y label widgets.
#
  bind $w.pgplot <Motion> {xyreport_motion %W %x %y}
}
#-----------------------------------------------------------------------
# Create the area that contains the xy plot area.
#
# Input:
#  w            The name to give the frame widget that encloses the area.
#-----------------------------------------------------------------------
proc create_uv_area {w} {
#
# Frame the workarea.
#
  frame $w -width 16c -height 16c
#
# Create the PGPLOT image window.
#
  pgplot $w.pgplot -width 15c -height 15c -maxcolors 100 -bd 2 -bg black -fg white 
#
# Create horizontal and vertical scroll-bars and have them
# call the pgplot xview and yview scroll commands to scroll the
# image within the window.
#
  scrollbar $w.xscroll -command "$w.pgplot xview" -orient horizontal
  scrollbar $w.yscroll -command "$w.pgplot yview" -orient vertical
#
# Tell the PGPLOT widget how to update the scrollbar sliders.
#
  $w.pgplot configure -xscrollcommand "$w.xscroll set"
  $w.pgplot configure -yscrollcommand "$w.yscroll set"
#
# Position the PGPLOT widget and the scrollbars.
#
  pack $w.xscroll -side bottom -fill x
  pack $w.yscroll -side right -fill y
  pack $w.pgplot -side left -fill both -expand true
#
# Bind motion events to the world coordinate x and y label widgets.
#
  bind $w.pgplot <Motion> {uvreport_motion %W %x %y}
}
#-----------------------------------------------------------------------
# Create the area that contains the xy plot area.
#
# Input:
#  w            The name to give the frame widget that encloses the area.
#-----------------------------------------------------------------------
proc create_obs_widget {w} {

# Access global variables inside this procedure

upvar #0 hastart has

global hastart
global hastop
global obslat
global obslong
global srcdec
global wave
global antsize
global cellsize
global units
global ngrid
global uvrad
global apeff
global tsys
global correff
global bw
global tint
global tscale
global tau
global nchan

#
# Frame the workarea.
#
frame $w -width 32c -height 5c

# Create 3 frames for sets of entry widgets.  These will be packed left to right

frame $w.fl  -relief raised -bd 2
frame $w.fm  -relief raised -bd 2
frame $w.fr  -relief raised -bd 2
frame $w.frr -relief raised -bd 2

# Within each frame, create a top and bottom frame.

frame $w.fl.ft
frame $w.fl.fb
frame $w.fl.fbb

frame $w.fm.ft
frame $w.fm.fb
frame $w.fm.fbb

frame $w.fr.ft
frame $w.fr.fb
frame $w.fr.fbb
frame $w.fr.fb3
frame $w.fr.fb4
frame $w.fr.fb5
frame $w.fr.fb6
frame $w.fr.fb7
frame $w.fr.fb8
frame $w.fr.fb9

frame $w.frr.ft
frame $w.frr.fb
frame $w.frr.fbb
frame $w.frr.fb3

# Make entry widget/label combinations.

entry $w.fl.ft.obslat -width 20 -textvariable obslat
label $w.fl.ft.obslatlab -anchor w -width 11 -text "Latitude" -anchor w
bind  $w.fl.ft.obslat <Down> [list focus $w.fl.fb.obslong]
bind  $w.fl.ft.obslat <Return> update_antpos

entry $w.fl.fb.obslong -width 20 -textvariable obslong
label $w.fl.fb.obslonglab -width 11 -text "Longitude" -anchor w
bind  $w.fl.fb.obslong <Up>     [list focus $w.fl.ft.obslat]
bind  $w.fl.fb.obslong <Down>   [list focus $w.fl.fbb.obsalt]
bind  $w.fl.fb.obslong <Return> update_antpos

entry $w.fl.fbb.obsalt -width 20 -textvariable obsalt
label $w.fl.fbb.obsaltlab -width 11 -text "Altitude (m)" -anchor w
bind  $w.fl.fbb.obsalt <Up>     [list focus $w.fl.fb.obslong]
bind  $w.fl.fbb.obsalt <Return> update_antpos

# The middle panel

entry $w.fm.ft.hastart -width 20 -textvariable hastart
label $w.fm.ft.hastartlab -width 11 -text "HA start" -anchor w
bind  $w.fm.ft.hastart <Down> [list focus $w.fm.fb.hastop]

entry $w.fm.fb.hastop -width 20 -textvariable hastop
label $w.fm.fb.hastoplab -width 11 -text "HA stop" -anchor w
bind  $w.fm.fb.hastop <Up>   [list focus $w.fm.ft.hastart]
bind  $w.fm.fb.hastop <Down> [list focus $w.fm.fbb.srcdec]

entry $w.fm.fbb.srcdec -width 20 -textvariable srcdec
label $w.fm.fbb.srcdeclab -width 11 -text "SRC dec" -anchor w
bind  $w.fm.fbb.srcdec <Up>  [list focus $w.fm.fb.hastop]

entry $w.fr.ft.wave -width 20 -textvariable wave
label $w.fr.ft.wavelab -width 25 -text "Wavelength (cm)" -anchor w
bind  $w.fr.ft.wave <Down> [list focus $w.fr.fb.antsize]

entry $w.fr.fb.antsize -width 20 -textvariable antsize
label $w.fr.fb.antsizelab -width 25 -text "Antenna Diameter (cm)" -anchor w
entry $w.fr.fbb.apeff -width 20 -textvariable apeff
label $w.fr.fbb.apefflab -width 25 -text "Aperture efficiency" -anchor w
entry $w.fr.fb3.tsys -width 20 -textvariable tsys
button $w.fr.fb3.tsyslab -width 23 -text "Fixed Tsys (K)" -anchor w
entry $w.fr.fb4.correff -width 20 -textvariable correff
label $w.fr.fb4.correfflab -width 25 -text "Correlator Efficiency" -anchor w
entry $w.fr.fb5.bw -width 20 -textvariable bw
label $w.fr.fb5.bwlab -width 25 -text "Correlator Bandwidth (GHz)" -anchor w
entry $w.fr.fb6.nchan -width 20 -textvariable nchan
label $w.fr.fb6.nchanlab -width 25 -text "Correlator Channels (#)" -anchor w
entry $w.fr.fb7.tint -width 20 -textvariable tint
label $w.fr.fb7.tintlab -width 25 -text "Integration time (sec)" -anchor w
entry $w.fr.fb8.tscale -width 20 -textvariable tscale
label $w.fr.fb8.tscalelab -width 25 -text "Integration scale factor" -anchor w
entry $w.fr.fb9.tau -width 20 -textvariable tau
label $w.fr.fb9.taulab -width 25 -text "Opacity" -anchor w

entry $w.frr.ft.ngrid -width 20 -textvariable ngrid
label $w.frr.ft.ngridlab -width 20 -text "Ngrid" -anchor w
entry $w.frr.fb.cell -width 20 -textvariable cellsize
label $w.frr.fb.celllab -width 20 -text "Cell Size (arcsec)" -anchor w
label $w.frr.fbb.uvradlab -width 20 -text "UV taper (fwhm)" -anchor w
entry $w.frr.fbb.uvrad -width 20 -textvariable uvrad
label $w.frr.fb3.xmaxlab -width 20 -text "Xmax (cm)" -anchor w
entry $w.frr.fb3.xmax -width 20 -textvariable xmax

# Pack label+entry widgets in a sensible manner

pack $w.fl.ft.obslatlab -side left
pack $w.fl.ft.obslat -side left
pack $w.fl.fb.obslonglab -side left
pack $w.fl.fb.obslong -side left
pack $w.fl.fbb.obsaltlab -side left
pack $w.fl.fbb.obsalt -side left

pack $w.fm.ft.hastartlab -side left
pack $w.fm.ft.hastart -side left
pack $w.fm.fb.hastoplab -side left
pack $w.fm.fb.hastop -side left
pack $w.fm.fbb.srcdeclab -side left
pack $w.fm.fbb.srcdec -side left

pack $w.fr.ft.wavelab -side left
pack $w.fr.ft.wave -side left
pack $w.fr.fb.antsizelab -side left
pack $w.fr.fb.antsize -side left
pack $w.fr.fbb.apefflab -side left
pack $w.fr.fbb.apeff -side left
pack $w.fr.fb3.tsyslab -side left
pack $w.fr.fb3.tsys -side left

pack $w.fr.fb4.correfflab -side left
pack $w.fr.fb4.correff -side left
pack $w.fr.fb5.bwlab -side left
pack $w.fr.fb5.bw -side left
pack $w.fr.fb6.nchanlab -side left
pack $w.fr.fb6.nchan -side left
pack $w.fr.fb7.tintlab -side left
pack $w.fr.fb7.tint -side left
pack $w.fr.fb8.tscalelab -side left
pack $w.fr.fb8.tscale -side left
pack $w.fr.fb9.taulab -side left
pack $w.fr.fb9.tau -side left

pack $w.frr.ft.ngridlab -side left
pack $w.frr.ft.ngrid -side left
pack $w.frr.fb.celllab -side left
pack $w.frr.fb.cell -side left
pack $w.frr.fbb.uvradlab -side left
pack $w.frr.fbb.uvrad -side left
pack $w.frr.fb3.xmaxlab -side left
pack $w.frr.fb3.xmax -side left

bind $w.frr.fb3.xmax <Return> {simint redraw}

# Within each frame, pack the labels on top of each other

pack $w.fl.ft -side top
pack $w.fl.fb -side top
pack $w.fl.fbb -side top

pack $w.fm.ft -side top
pack $w.fm.fb -side top
pack $w.fm.fbb -side top

pack $w.fr.ft  -side top
pack $w.fr.fb  -side top
pack $w.fr.fbb -side top
pack $w.fr.fb3 -side top
pack $w.fr.fb4 -side top
pack $w.fr.fb5 -side top
pack $w.fr.fb6 -side top
pack $w.fr.fb7 -side top
pack $w.fr.fb8 -side top
pack $w.fr.fb9 -side top

pack $w.frr.ft -side top
pack $w.frr.fb -side top
pack $w.frr.fbb -side top
pack $w.frr.fb3 -side top

# Pack the four enclosing frames left to right

pack $w.fl -side left
pack $w.fm -side left
pack $w.fr -side left
pack $w.frr -side left
}

#-----------------------------------------------------------------------
# This procedure is called whenever cursor motion is detected in the
# the xyplt widget. It displays the world coordinates of the cursor
# in previously created label widgets.
#
# Input:
#  pg     The image pgplot widget.
#  x y    The X-window coordinates of the cursor.
#-----------------------------------------------------------------------
proc xyreport_motion {pg x y} {
  global tcl_precision
  set tcl_precision 3
  .ftop.world.x configure -text "X=[$pg world x $x]"
  .ftop.world.y configure -text "Y=[$pg world y $y]"
  set tcl_precision 6
}
#-----------------------------------------------------------------------
# This procedure is called whenever cursor motion is detected in the
# the uvplt widget. It displays the world coordinates of the cursor
# in previously created label widgets.
#
# Input:
#  pg     The image pgplot widget.
#  x y    The X-window coordinates of the cursor.
#-----------------------------------------------------------------------
proc uvreport_motion {pg x y} {
  global tcl_precision
  set tcl_precision 3
  .ftop.world.x configure -text "U=[$pg world x $x]"
  .ftop.world.y configure -text "V=[$pg world y $y]"
  set tcl_precision 6
}

#-----------------------------------------------------------------------
# Create the area that contains the slice PGPLOT window.
#
# Input:
#  w            The name to give the frame widget that encloses the area.
#-----------------------------------------------------------------------
proc create_slice_area {w} {
#
# Frame the workarea.
#
  frame $w -width 11c -height 6c
#
# Create the PGPLOT slice window.
#
  pgplot $w.pgplot -width 10c -height 5c -maxcolors 1 -bd 2 -bg black -fg white
#
# Position the PGPLOT widget.
#
  pack $w.pgplot -side left -fill both -expand true
#
# Arrange for the plot to be redrawn whenever the widget is resized.
#
  bind $w.pgplot <Configure> {simint redraw_slice}
}

proc update_xyuvplot {} {

}
proc compvis {} {
# Access global variables
    global antarr
    global wave
    global antsize
    global obslat
    global obslong
    global hastart
    global hastop
    global srcdec
    global dispvis

    set dispvis 1

# And update the antenna and visibility plot.
    
    simint uvframe
    simint compvis $wave $obslat $obslong $hastart $hastop $srcdec
}    
proc radplot {nbin} {

# Access global variables

    global dispvis

    set dispvis 0

# And update the visibility plot.
    
    simint radplot $nbin
}    
proc compbeam {} {
# Access global variables
    global antarr
    global wave
    global antsize
    global obslat
    global obslong
    global hastart
    global hastop
    global srcdec
    global dispvis
    global cellsize
    global ngrid
    global uvrad

    set dispvis 0

# And update the visibility plot.
    
    simint compbeam $wave $obslat $obslong $hastart $hastop $srcdec $cellsize $ngrid $uvrad
}    
proc compwin {} {
# Access global variables
    global antarr
    global wave
    global antsize
    global obslat
    global obslong
    global hastart
    global hastop
    global srcdec
    global dispvis

    set dispvis 0

# And update the visibility plot.
    
    simint compwin $wave $antsize $obslat $obslong $hastart $hastop $srcdec
}    
#.......................................................................
# Observe the currently selected image.
# 
proc obsim {} {
    global images
    global curimage

    set i [expr $curimage/2 - 1]
    simint obsim $images($i)

}    
proc update_array {name1 name2 op} {
    global antarr
    global wave
    global antsize
    global obslat
    global obslong
    global obsalt
    global hastart
    global hastop
    global srcdec
    global cellsize

    global arrinfo
    global correff
    global bw
    global nchan
    global tsys
    global tau

    global dispvis

# update the phase reference if an array has been selected

    set obslat $arrinfo($antarr,lat)
    set obslong $arrinfo($antarr,long)
    set obsalt $arrinfo($antarr,alt)

# update the antenna size and weighting information

    set antsize $arrinfo($antarr,antsize)
    set bw $arrinfo($antarr,bw)
    set nchan $arrinfo($antarr,nchan)
    set correff $arrinfo($antarr,correff)
    set tau $arrinfo($antarr,tau)

# update the antenna size.

    set cellsize $arrinfo($antarr,cellsize)

# and set the default to display a zenith snapshot

    set srcdec $arrinfo($antarr,lat)
    set hastart {00:00:00}
    set hastop {00:00:00}

# Update the wavelength and tsys if the array is not the VLA or DASI

    if {[string match $antarr user] || [string match $antarr jca] || [string match $antarr ati_compact] || [string match $antarr ati_hetero]} {
	set wave $arrinfo($antarr,wave)
	set tsys $arrinfo($antarr,tsys)
    }
    if {[string match $antarr dasi]} {
        if {$wave==0.0} {
          set nchan 10
          set wave 0.968
          set bw 10.0
	} else {
	  set nchan 1
          set bw 1.0
	}
    }
#
# Display a blank antenna array field.
#
    simint antarray $antarr $antsize $wave $obslat $obslong $hastart $hastop $srcdec

#
# And reset display of the visibilities.
#
    set dispvis 1
}

# Update the antenna positions in XYZ whenever the phase center shifts.

proc update_antpos {} {
    global obslat
    global obslong
    global obsalt

#
# Display a blank antenna array field.
#
    simint antpos $obslat $obslong $obsalt
}

#-----------------------------------------------------------------------
# This is the main procedure of this script.
#-----------------------------------------------------------------------

# Set the title of the application window and its icon.

wm title . "Simint"
wm iconname . "Simint"

# Prevent other applications from sending commands to this one!

# rename send {}

# Override selected widget defaults.

option add *font -Adobe-Times-Medium-R-Normal-*-140-* widgetDefault

# Set default widget colors.

set bg "#bfe5ff"
set alt_bg "#00ddff"
. configure -bg $bg
option add *background $bg widgetDefault
option add *activeBackground $bg widgetDefault
option add *activeForeground blue widgetDefault
option add *highlightBackground $bg widgetDefault
option add *troughColor $bg widgetDefault
option add *Scrollbar.width 3m widgetDefault
option add *Scrollbar.background $alt_bg widgetDefault
option add *Scrollbar*Foreground $alt_bg widgetDefault
option add *Button*Background $alt_bg widgetDefault
option add *Button*activeBackground $alt_bg widgetDefault
option add *Button*activeForeground black widgetDefault
option add *Menubutton*activeForeground black widgetDefault

# Set up global variables to store the antenna information for various arrays

set arrinfo(dasi,lat) {-90:00:00}
set arrinfo(dasi,long) {00:00:00}
set arrinfo(dasi,alt) {2841}
set arrinfo(dasi,wave) {1}
set arrinfo(dasi,antsize) {20}

# Analog correlator efficiency is 100%

set arrinfo(dasi,correff) {1.0} 
set arrinfo(dasi,apeff) {0.834} 
set arrinfo(dasi,bw) {1.0} 
set arrinfo(dasi,nchan) {10} 
set arrinfo(dasi,tsys) {15} 
set arrinfo(dasi,tau) {0.045} 

set arrinfo(sza,lat) {37:13:56}
set arrinfo(sza,long) {118:16:54}
set arrinfo(sza,alt) {3000}
set arrinfo(sza,wave) {0.97}
set arrinfo(sza,antsize) {350}
set arrinfo(sza,cellsize) {300}
set arrinfo(sza,correff) {0.88} 
set arrinfo(sza,bw) {8} 
set arrinfo(sza,nchan) {8} 
set arrinfo(sza,tsys) {21} 
set arrinfo(sza,tau) {0.03} 

set arrinfo(jca,lat) {37:13:56}
set arrinfo(jca,long) {118:16:54}
set arrinfo(jca,alt) {3000}
set arrinfo(jca,wave) {0.97}
set arrinfo(jca,antsize) {250}
set arrinfo(jca,cellsize) {300}
set arrinfo(jca,correff) {0.88} 
set arrinfo(jca,bw) {10} 
set arrinfo(jca,nchan) {1} 
set arrinfo(jca,tsys) {21} 
set arrinfo(jca,tau) {0.03} 

set arrinfo(ati_compact,lat) {37:13:56}
set arrinfo(ati_compact,long) {118:16:54}
set arrinfo(ati_compact,alt) {3000}
set arrinfo(ati_compact,wave) {0.97}
set arrinfo(ati_compact,antsize) {350}
set arrinfo(ati_compact,cellsize) {300}
set arrinfo(ati_compact,correff) {0.88} 
set arrinfo(ati_compact,bw) {8} 
set arrinfo(ati_compact,nchan) {1} 
set arrinfo(ati_compact,tsys) {21} 
set arrinfo(ati_compact,tau) {0.03} 

set arrinfo(ati_hetero,lat) {37:13:56}
set arrinfo(ati_hetero,long) {118:16:54}
set arrinfo(ati_hetero,alt) {3000}
set arrinfo(ati_hetero,wave) {0.97}
set arrinfo(ati_hetero,antsize) {350}
set arrinfo(ati_hetero,cellsize) {300}
set arrinfo(ati_hetero,correff) {0.88} 
set arrinfo(ati_hetero,bw) {3} 
set arrinfo(ati_hetero,nchan) {1} 
set arrinfo(ati_hetero,tsys) {21} 
set arrinfo(ati_hetero,tau) {0.03} 

set arrinfo(user,lat) {34:04:43.497}
set arrinfo(user,long) {107:37:03.819}
set arrinfo(user,alt) {3000}
set arrinfo(user,wave) {1}
set arrinfo(user,antsize) {20}
set arrinfo(user,cellsize) {100}
# Assume correlator efficiency is 100%
set arrinfo(user,correff) {1.0} 
set arrinfo(user,bw) {1.0}
set arrinfo(user,nchan) {1.0}
set arrinfo(user,tsys) {21}  
set arrinfo(user,tau) {0.045}  

# VLA arrays

foreach index [list a b c d] {
    set arrinfo($index,antsize) {2500}
    set arrinfo($index,lat) {34:04:43.497} 
    set arrinfo($index,long) {107:37:03.819}
    set arrinfo($index,cellsize) {0.5}
# Typical correlator efficiency is 88%
    set arrinfo($index,correff) {0.88}
# VLA bandwidth is 50 MHz
    set arrinfo($index,bw) {0.05} 
    set arrinfo($index,nchan) {1} 
    set arrinfo($index,alt) {1000} 
    set arrinfo($index,tau) {0.045} 
}

# For all  set up cellsize to produce an image 2 x the size of the primary beam
# PB ~ 1.22*lambda/Diam

foreach index [list user sza jca ati_compact ati_hetero dasi] {
    set arrinfo(dasi,cellsize) [expr 2*1.22*$arrinfo($index,wave)/$arrinfo($index,antsize)*206265/256]
}

# Set up global variables to track any user changes

set antarr {sza}
set wave $arrinfo($antarr,wave)
set antsize  $arrinfo($antarr,antsize)
set nant {0}
set obslat $arrinfo($antarr,lat)
set obslong $arrinfo($antarr,long)
set obsalt $arrinfo($antarr,alt)
set srcdec $arrinfo($antarr,lat)
set hastart {00:00:00}
set hastop {00:00:00}
set cellsize [expr 2*1.22*$arrinfo($antarr,wave)/$arrinfo($antarr,antsize)*206265/256]
set ngrid {256}
set uvrad {0}
set dispvis {1}
set symm {1}
set tau $arrinfo($antarr,tau)
set nchan $arrinfo($antarr,nchan)

set apeff {0.834}
set correff $arrinfo($antarr,correff)
set bw $arrinfo($antarr,bw)
set tsys $arrinfo($antarr,tsys)
set tint {50}
set tscale {1}

set curimage 0
set nimage 0
set images(0) {}

set isouv 0

# Test setting up an array

set testarr {0 1 2 3 4}
set iel 0
foreach item $testarr {
  incr iel
}

# Split the window into two frames, one with the plot areas and 

frame .ftop
frame .fbot

# Create the menu-bar.

create_main_menubar .ftop.menubar

# Create label widgets for use in displaying image world coordinates.

create_world_labels .ftop.world

# Create a PGPLOT window with scroll bars, and enclose it in a frame.
# This is the array window.

create_xy_area .ftop.xyarea

# Create a PGPLOT window with scroll bars, and enclose it in a frame.
# This is the uv window.

create_uv_area .ftop.uvarea

# Create an observations entry widget

create_obs_widget .fbot.obswidget

# Create dialogs for later display.

create_newim_dialog .newim
initim .newim

create_powind_dialog .powind

create_gsig_dialog .gsig

create_pfile_dialog .pfile

create_rad_dialog .rad

create_ptsrc_dialog .ptsrc

create_ant_dialog .ant

create_want_dialog .want

create_rvis_dialog .rvis

create_wvis_dialog .wvis

create_wdat_dialog .wdat

create_wbin_dialog .wbin

create_help_dialog .ftop.usage_help {Usage information} {
    To see a slice through the displayed image, move the mouse into
    the image display window and use any mouse button to select the
    two end points of a line.

    To display a different image select a new image function from the
    "Select a display function" option menu.
}

# Place the menubar at the top of the main window and the work-areas
# underneath it.

pack .ftop.menubar -side top -fill x
pack .ftop.world -side top -anchor w
pack .ftop.xyarea -side left -fill both -expand true
pack .ftop.uvarea -side right -fill both -expand true
pack .fbot.obswidget -side top -fill x -expand true

# Pack the frames on top of one another

pack .ftop -side top -fill both -expand true
pack .fbot -side bottom -fill both -expand true

# Create the simint command.  If we were passed an argument, interpret
# this as a script to be run in non-interactive mode

if [ expr $argc != 0 ] {

    # Create the simint command with NULL arguments

    create_simint simint none none

    # Initialize initial image.  This is required, even in
    # non-interactive mode, since it initializes internal C variables
    # which otherwise will be left NULL
    
     simint antarray $antarr $antsize $wave $obslat $obslong $hastart $hastop $srcdec
    
    # Source the script

    source $argv

    # And exit when done

    exit

} else {

    # Else create the application in interactive mode

    create_simint simint .ftop.xyarea.pgplot/xtk .ftop.uvarea.pgplot/xtk

    # Set up a trace for the array variable

    trace variable antarr w update_array

    # Windows in Tk do not take on their final sizes until the whole
    # application has been mapped. This makes it impossible for the
    # PGPLOT widget to correctly guess what size of pixmap to allocate
    # when it starts the first page. To avoid this problem, force Tk
    # to create all of the windows before drawing the first plot.
    
    update idletasks
    
    # Draw the initial image.
    
    draw_antarray
    
    update idletasks
    
    tk_focusFollowsMouse
}