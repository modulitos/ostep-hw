// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net

// modifications to support multiple overlays (look for @overlay)
// by  Marshall Rose <mrose@dbc.mtview.ca.us>
//


//// DIST - frequency distribution

//// load dist-specific parameters..
#setifnotgiven binsize = ""
#setifnotgiven color = "pink"
#setifnotgiven barwidth = ""
#setifnotgiven savetbl = ""
#setifnotgiven cats = ""
#setifnotgiven order = "natural"
#setifnotgiven overlay = "no"
#setifnotgiven rotate = "no"
#setifnotgiven curveorder = 3
#setifnotgiven legend = "max max"

#if @overlay != yes
  #musthave fld
#else
  #musthave xrange
  #musthave yrange
#endif

//// load standard parameters..
#include $chunk_setstd


//// read data...
#include $chunk_read


//// do title..
#include $chunk_title


//// user pre-plot include
#if @include1 != ""
  #include @include1
#endif


//// set up plotting area..
#include $chunk_area
#if @cats = yes
  xrange: 0 100
#elseif @xrange = ""
  xautorange: datafield=@fld nearest=@xnearest 
#else
  xrange: @xrange
#endif
#if @yrange = ""
yrange: 0 100   // to be revised after we run the distribution..
#else
yrange: @yrange
#endif
#endproc

// for categories, stubs must be done AFTER the distribution is run below..
#if @cats != yes || @overlay = yes
  #include $chunk_xaxis
  stubs: inc @xinc
  stubcull: yes
  stubdetails: size=8
#endif

#if @binsize = ""
  #endproc 
  #if @cats != yes || @overlay = yes
    #set binsize = $arith( @XINC/2 ) 
  #endif
#endif


#if @overlay != yes
  #set NSAMPLES = @fld
#else
  #if @rotate = yes
      #proc processdata
      action: rotate
      // stack: no
      // showresults: yes
      #endproc
  #endif

  #set NSAMPLES = @NFIELDS
  #set fld = 1
#endif

#while @fld <= @NSAMPLES

  #if @overlay = yes
    #set ID = $dataitem(1, @fld)
    #set COLOR = $icolor(@fld)
  #endif
#write stderr
ID = @ID
#endwrite

//// tabulate the distribution of values, e.g. how many strains fell into each bin
  #proc tabulate
  // showresults: yes
  datafield1: @fld
  #if @cats != yes || @overlay = yes
    doranges1: yes
    rangespec1: @XMIN @binsize
    showrange: avg
  #else
    order1: @order
  #endif
  #ifspec savetbl savetable
  #endproc

//// do curve..
  #if @overlay = yes
    #proc curvefit
    curvetype: bspline
    order: @curveorder
    xfield: 1
    yfield: 2
    linedetails: color=@COLOR width=0.5
    legendlabel: @ID

    #proc usedata
      pop: 1
    #endproc
  #endif

  #set fld = $arith(@fld+1)
#endloop


#if @overlay != yes
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
  // // following added 9/2/02 scg
  // catcompmethod: exact

#else
  xrange: @XMIN @XMAX
#endif
#endif


#include $chunk_yaxis

#if @cats = yes
  #proc xaxis
  stubs: usecategories
  stubdetails: size=8
  #if @stubvert = yes
    stubvert: yes
  #endif
#endif


#if @overlay != yes
  //// do the background curve..
  #if @NRECORDS > 6
    #proc curvefit
    curvetype: bspline
    xfield: 1
    yfield: 2
    order: @curveorder
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
#else
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

//// user post-plot include
#if @include2 != ""
  #include @include2
#endif
