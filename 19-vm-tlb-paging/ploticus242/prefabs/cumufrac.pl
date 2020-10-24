// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net

// CUMUFRAC - cumulative fraction plot

// logmode 		pass as 'log' for log plot 
// x x2
// linedet, linedet2
// name, name2


#setifnotgiven logmode = linear
#setifnotgiven x2 = ""
#setifnotgiven rectangle = "2 2 7 5"
    // (what about cm units??)
#setifnotgiven linedet = "color=green width=0.5"
#setifnotgiven linedet2 = "color=red width=0.5"

#setifnotgiven name = "#usefname"
#setifnotgiven name2 = "#usefname"
#if @CM_UNITS = 1
  #setifnotgiven legend = "min+1.2 min-1.2"
#else
  #setifnotgiven legend = "min+0.5 min-0.5"
#endif


/// get standard parameters..
#include $chunk_setstd

/// read the data for the 1st curve..
/// the select screens out missing data codes and other non-numerics..
#set select = "$isnumber(@" @x ") = 1" 
#include $chunk_read


#musthave x

// chunk_area uses unittyp...
#set unittype = @logmode

/// set up plotting area...
#include $chunk_area
yrange: 0 @NRECORDS
#if @logmode = linear
  // xautorange: datafield=@x,@x2 nearest=@xnearest  // changed as below, scg 2/20/07...
  xautorange: datafield=@x,@x2 mininit=0 nearest=@xnearest 
#else
  xautorange: datafield=@x,@x2 mininit=0.05 nearest=@xnearest 
#endif
#endproc



///// do x axis (y axis done below)..
#proc xaxis
#ifspec xlbl label
#ifspec xlbldet labeldetails
#if @logmode != "linear"
  selflocatingstubs: text
  #include $chunk_logstubs
#else
  stubs: inc @xinc
#endif
#ifspec @stubfmt
stubrange: @XMIN
#ifspec @stubvert
#ifspec xgrid grid
location: min-0.2
stubcull: yes


#if @logmode != "linear"
  // do log tic marks..
  #proc xaxis
  location: min-0.2
  selflocatingstubs: text
  #include $chunk_logtics
#endif

/// title..
#include $chunk_title

/// user pre-plot include...
#if @include1 != ""
  #include @include1
#endif

///// plot the curve using accum and instancemode..
#proc lineplot
xfield: @x
linedetails: @linedet 
sort: yes
instancemode: yes
accum: yes
stairstep: yes
#if @logmode = "linear"
  firstpoint: 0 0
#endif
lastseglen: 0.2
legendlabel: @name
#saveas L

#if @x2 != ""
  /// read the data for the 2nd curve..
  /// the select screens out missing data codes and other non-numerics..
  #set select = "$isnumber(@" @x2 ") = 1" 
  #include $chunk_read

  #include $chunk_area
  yrange: 0 @NRECORDS
  xrange: @XMIN @XMAX
      // set in first areadef above..

  // plot 2nd curve
  #proc lineplot
  #clone: L
  xfield: @x2
  linedetails: @linedet2
  legendlabel: @name2
#endif


///// draw a plotting area frame allowing a small margin below X=0 and below Y=0..
#proc line
linedetails: width=0.5 color=black
points: min-0.2 min-0.2 max+0.2 min-0.2
	max+0.2 max+0.2
	min-0.2 max+0.2
	min-0.2 min-0.2


///// redraw the plotting area so we can express Y axis as a percentage 0.0 to 1.0 ..
#proc areadef
rectangle: @rectangle
xrange: 0 1
yrange: 0 1
yaxis.stubs: inc 0.2
yaxis.location: min-0.2



// do legend
#if @legend != "no"
  #if @name != "#usefname" || @header = yes
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
#endif


//// user post-plot include..
#if @include2 != ""
  #include @include2
#endif
