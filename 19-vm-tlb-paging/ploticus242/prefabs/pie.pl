// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// PIE - pie graph

//// set defaults..
#setifnotgiven colors = "dullyellow"
#setifnotgiven title = ""
#setifnotgiven data = ""
#setifnotgiven inlinedata = ""

// stick with the prefab convention where 'legend' contains legend location..
#setifnotgiven legend = no
#set dolegend = 0
#if @legend != no
  #set dolegend = 1
  #if @legend = yes
    #set legend = ""
  #endif
#endif
#if @CM_UNITS = 1 
    #setifnotgiven  center = "6.25 6.25"
    #setifnotgiven  radius = 2.5
    #setifnotgiven  legend = "12 18"
#else
    #setifnotgiven  center = "2.5 2.5"
    #setifnotgiven  radius = 1
    #setifnotgiven  legend = "4 3"
#endif

// following added scg 8/4/04...
#proc settings
  encodenames: yes
  #ifspec encodenames
  enable_suscripts: yes
  #ifspec enable_suscripts


#include $chunk_read


//// do title..
#if @title != ""
  #proc annotate
  #if @CM_UNITS = 1
    location: 6.25 10.5
  #else
    location: 2.5 4.2
  #endif
  #ifspec titledet textdetails
  text: @title
  #endproc
#endif

//// do pie graph..
#proc pie
center: @center
radius: @radius
datafield: @values
#ifspec labels labelfield
#ifspec wraplen
#if @labelfarout like -*
  labelmode: label
#elseif @dolegend = 1
  labelmode: legend
#else
  labelmode: line+label
#endif
#ifspec colorfld exactcolorfield
#ifspec colors
#ifspec explode
#ifspec firstslice
#ifspec lbldet textdetails
#ifspec outlinedetails
#ifspec total
#ifspec labelback
#ifspec labelfarout
#ifspec clickmapurl
#ifspec clickmaplabel
#ifspec labelfmtstring
  
#if @dolegend = 1
  #proc legend
  location: @legend
  #ifspec legendfmt format
  #ifspec legendsep sep
  #ifspec legwrap wraplen 
  #ifspec legbreak extent
  #ifspec legtitle title
  #ifspec legbox backcolor
  #ifspec legframe frame
  #ifspec legtextdet textdetails
#endif
