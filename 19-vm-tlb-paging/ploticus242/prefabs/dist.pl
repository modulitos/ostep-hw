// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net


//// DIST - frequency distribution

//// initialize key dist-specific parameters..
#setifnotgiven binsize = ""
#setifnotgiven color = "pink"
#setifnotgiven cats = ""
#setifnotgiven order = "natural"
#setifnotgiven fld = ""


//// load standard parameters..
#include $chunk_setstd


//// read data...
#include $chunk_read

#if @fld != ""
  #set x = @fld
#endif
#musthave x


//// user pre-plot include
#if @include1 != ""
  #include @include1
#endif


//// set up plotting area..
#include $chunk_area
#if @cats = yes
  xrange: 0 100
#elseif @xrange = ""
  xautorange: datafield=@x nearest=@xnearest 
#else
  xrange: @xrange
#endif
yrange: 0 100   // to be revised after we run the distribution..


//// do title..
#include $chunk_title


// for categories, stubs must be done AFTER the distribution is run below..
#if @cats != yes
  #include $chunk_xaxis
  stubs: inc @xinc
  stubcull: yes
  stubdetails: size=8
#endif

#if @binsize = "" && @cats != yes
  #endproc 
  #set binsize = $arith( @XINC/2 ) 
#endif

//// tabulate the distribution of values, e.g. how many strains fell into each bin
#proc tabulate
datafield1: @x
#if @cats != yes
  doranges1: yes
  rangespec1: @XMIN @binsize
  showrange: avg
#else
  order1: @order
#endif
#ifspec savetbl showresults

#if @cats = yes
  #proc categories
    axis: x
    datafield: 1

#endif

//// now that we have the distribution, recompute the plotting area with a auto Y range
#include $chunk_area
#if @yrange = ""
  yautorange: datafield=2 nearest=@ynearest 
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  yautorange: datafield=2 mininit=@yrange nearest=@ynearest 
#else
  yrange: @yrange
#endif
#if @cats = yes
  xscaletype: categories
  // xcategories: datafield=1
  // following added 9/2/02 scg
  // catcompmethod: exact
#else
  xrange: @XMIN @XMAX
#endif

#include $chunk_yaxis

#if @cats = yes
  #proc xaxis
  stubs: usecategories
  stubdetails: size=8
  #ifspec stubvert
#endif



//// do the background curve..
#if @curve = yes && @NRECORDS > 6
  #proc curvefit
  curvetype: bspline
  xfield: 1
  yfield: 2
  order: 5
  linedetails: color=gray(0.5) width=0.5
#endif


//// do the bars..
#proc bars
locfield: 1
lenfield: 2
color: @color
#ifspec barwidth
outline: no
hidezerobars: yes
#ifspec clickmapurl
#ifspec clickmaplabel

//// user post-plot include
#if @include2 != ""
  #include @include2
#endif
