// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

// ability to display dumpfile .... scg 8/5/04
#setifnotgiven dumpfile = ""
#if @dumpfile != ""
  #proc drawcommands
    dumpfile: @dumpfile

  // blank line and endproc added - scg 9/16/05
  #endproc
  #exit
#endif

#setifnotgiven xrange = "0 8"
#setifnotgiven yrange = "0 8"
#setifnotgiven rectangle = "0 0 8 8"
#musthave cmdfile

#proc areadef
rectangle: @rectangle
xrange: @xrange
yrange: @yrange

#proc drawcommands
file: @cmdfile

