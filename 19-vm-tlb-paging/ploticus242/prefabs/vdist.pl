// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net


//// VDIST - vertical numeric frequency distribution & rangebar 

//// initialize key dist-specific parameters..
#setifnotgiven y2 = ""
#setifnotgiven title2 = ""
#setifnotgiven id = ""
#setifnotgiven id2 = ""
#setifnotgiven data2 = ""
#setifnotgiven dist = centered

#set rangebar = no
// #setifnotgiven rangebar = yes  // As of 2.34, the boxplot portion of this prefab is nonfunctional... 
				  // Not sure how much (if any) user interest is out there.
				  // If there is, this prefab would need to be modified to use proc boxplot.
                                  // Community involvement in accomplishing this is solicited.

#setifnotgiven ygrid = "color=gray(0.8)"

#if @dist like no*
  #setifnotgiven rangebarcenter = 3.0(s)
  #setifnotgiven rectangle = "1 1 2 5"
#else
  #setifnotgiven distcenter = 4.0(s)
  #setifnotgiven rangebarcenter = 8.0(s)
  #setifnotgiven rectangle = "1 1 4 5"
#endif

// rangebar parameters
#setifnotgiven barcolor = dullyellow
#setifnotgiven outliernearsym = ""
#setifnotgiven outlierfarsym = ""
#if @outliernearsym = "" && @outlierfarsym = "" 
  #setifnotgiven outlierlinelen = 0.1
#endif

// distribution parameters
#setifnotgiven bindiv = 5.0
#setifnotgiven distdotspread = 2.0
#setifnotgiven distcolor = blue
#setifnotgiven distdotshape = pixdiamond
#setifnotgiven distdotsize = 0.04

#setifnotgiven ptselect = ""
#setifnotgiven ptselect2 = ""


//// load standard parameters..
#include $chunk_setstd

#musthave y

#if @ptselect != ""
  #set select = @ptselect
#endif
//// read data just to get range...
#include $chunk_read

//// set up plotting area
#include $chunk_area
#if @data2 = ""
  #set df = @y "," @y2
#else
  #set df = @y
#endif
#if @yrange = ""
  yautorange: datafield=@df nearest=@ynearest 
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  yautorange: datafield=@df nearest=@ynearest mininit=@yrange
#else
  yrange: @yrange
#endif
xrange: 0 10

//// do title..
#include $chunk_title

//// show Y axis
#include $chunk_yaxis
#endproc

//// user pre-plot include
#if @include1 != ""
  #include @include1
#endif

//// determine bin size
#setifnotgiven binsize = $arith(@YINC/@bindiv)
#write stderr
bin size = @binsize
#endwrite

//// read data again.. this time w/filter to do grouping..
#set context = vdist
#include $chunk_read

//// render points distribution
#if @dist !like no*
  #proc scatterplot
  #if @dist like hist*
    clustermethod: rightward
    xlocation: 0.2(s)
  #else 
    clustermethod: horizontal
    xlocation: @distcenter
  #endif
  clusterfact: @distdotspread
  yfield: 1
  symbol: shape=@distdotshape style=fill color=@distcolor radius=@distdotsize
  #saveas D
#endif

////
#if @rangebar !like no*
  #proc rangebar
  barloc: @rangebarcenter
  datafield: 2
  nlocation: min-0.2
  color: @barcolor
  #ifspec showoutliers
  #ifspec id outlierlabelfield
  outlierlabeldetails: size=6
  #ifspec outlierlinelen
  #ifspec barwidth
  #ifspec mediansym
  #ifspec tailmode
  #ifspec 95tics
  #ifspec taildetails
  #ifspec outline
  #ifspec outliernearsym
  #ifspec outlierfarsym
  #ifspec outliernearfarcutoff
  #saveas R
#endif

/////////// do 2nd set if needed........ 

#if @y2 != ""

  // compute right hand rectangle based on the first one..
  // X1 is different from x1 !!
  #set X1 = $ntoken( 1, @rectangle )
  #set Y1 = $ntoken( 2, @rectangle )
  #set X2 = $ntoken( 3, @rectangle )
  #set Y2 = $ntoken( 4, @rectangle )
  #set width = $arith( @X2-@X1 )
  #set X1 = $arith( @X2+0.5 )
  #set X2 = $arith( @X1+@width )
  #set rectangle = @X1 " " @Y1 " " @X2 " " @Y2 

  #set y = @y2
  #if @id2 != ""
    #set id = @id2
  #endif

  //// set up plotting area
  #include $chunk_area
  yrange: @YMIN @YMAX
  xrange: 0 10

  //// do title..
  #set title = @title2
  #include $chunk_title

  //// show Y axis
  #set ylbl = ""
  #include $chunk_yaxis
  #if @nostubs2 = yes
    stubs: none
  #endif
  #endproc

  //// read data again.. this time w/filter to do grouping..
  #if @ptselect2 != ""
    #set select = @ptselect2
  #endif
  #set context = vdist
  #if @data2 != ""
    #set data = @data2
  #endif
  #include $chunk_read
  
  //// render points distribution
  #if @dist !like no*
    #proc scatterplot
    #clone D
  #endif
  
  ////
  #if @rangebar !like no*
    #proc rangebar
    #clone R
  #endif
  
#endif

//// user post-plot include
#if @include2 != ""
  #include @include2
#endif

