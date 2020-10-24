// HEATMAP prefab
//
//
#setifnotgiven xcats = ""
#setifnotgiven ycats = ""
#setifnotgiven xbinsize = ""
#setifnotgiven ybinsize = ""
#setifnotgiven cutofflist = ""
#setifnotgiven colorlist = "yellow,dullyellow,orange,red,lightpurple,purple"
#setifnotgiven zerocolor = black
#setifnotgiven contentfield = ""
#setifnotgiven symbol = ""
#setifnotgiven outline = ""



#include $chunk_setstd

#if @xcats != ""
  #set xbinsize = 1
#endif
#if @ycats != ""
  #set ybinsize = 1
#endif

#if @xbinsize = ""
  #set figure_x = 1
#endif
#if @ybinsize = ""
  #set figure_y = 1
#endif

#if @cutofflist = ""
  #set autocuts = 1
  // set cutofflist just for first stab..
  #set cutofflist = 50,25,15,10,5,1
#endif



// read data 1st time to get bin sizes..
#if @figure_x = 1 || @figure_y = 1 || @contentfield != ""
  #include $chunk_read

  // get range of content field..
  #if @contentfield != ""
    #include $chunk_area
    xautorange: datafield=@contentfield
    yrange: 0 10
    #endproc
    #set RANGEMAX = @XMAX
  #endif

  #include $chunk_area
  #if @figure_x = 1
    xautorange: datafield=@x
  #else
    xrange: 0 10
  #endif
  #if @figure_y = 1
    yautorange: datafield=@y
  #else
    yrange: 0 10
  #endif
  #endproc
  #if @figure_x = 1
    #set XINC = $defaultinc( @XMIN, @XMAX )
    #set xbinsize = $arith( @XINC/2 )
  #endif
  #if @figure_y = 1
    #set YINC = $defaultinc( @YMIN, @YMAX )
    #set ybinsize = $arith( @YINC/2 )
  #endif

#endif

#write stderr
xbinsize is @xbinsize .. ybinsize is @ybinsize
#endwrite


// now read again, grouping by bin..
#if @contentfield = ""
  #set context = heatmap
  #include $chunk_read
#endif

#if @xcats = yes
  #proc categories
  axis: x
  datafield: @x
#endif

#if @ycats = yes
  #proc categories
  axis: y
  datafield: @y
#endif
  

#include $chunk_area
areacolor: @zerocolor
#if @xcats = yes
  xscaletype: categories
  // xcategories: datafield=@x
  // catcompmethod: exact
  #if @xaxis != "none"
    xaxis.stubs: usecategories
    #ifspec xlbl xaxis.label
    #ifspec xlbldet xaxis.labeldetails
    #ifspec stubfmt xaxis.stubformat
    #ifspec xstubfmt xaxis.stubformat
    #ifspec stubvert xaxis.stubvert
    #ifspec xgrid xaxis.grid
  #endif
#elseif @xrange = ""
  xautorange: datafield=@x incmult=2.0
//#elseif @xrange = 0
#elseif $ntoken( 2, @xrange ) = ""
  xautorange: datafield=@x mininit=@xrange incmult=2.0
#else
  xrange: @xrange
#endif

#if @ycats = yes
  yscaletype: categories
  // ycategories: datafield=@y
  // catcompmethod: exact
  #if @yaxis != none
    yaxis.stubs: usecategories
    #ifspec ylbl yaxis.label
    #ifspec ylbldet yaxis.labeldetails
    #ifspec ystubfmt yaxis.stubformat
    #ifspec ygrid yaxis.grid
  #endif
#elseif @yrange = ""
  yautorange: datafield=@y incmult=2.0
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  yautorange: datafield=@y mininit=@yrange incmult=2.0
#else
  yrange: @yrange
#endif
#endproc

#if @xcats = ""
  #include $chunk_xaxis
  #endproc
#endif

#if @ycats = ""
  #include $chunk_yaxis
  #endproc
#endif

#if @contentfield = ""

  // from now on use the grouped values..
  #set x = @xgrp
  #set y = @ygrp

  #if @autocuts = 1
    #call $squelch_display( 1 )
  #endif

  // run scatterplot (if autocuts = 1 this is a throwaway to find range of duplication..)
  #include $chunk_doheatmap

  #if @autocuts = 1
    #call $squelch_display( 0 )
  #endif

  #write stderr
  #+ max count at any cell is @MAXDUPS
  #endwrite

#endif

  
#if @autocuts = 1
  // automatically set cutofflist

  #if @contentfield = ""
    #set RANGEMAX = @MAXDUPS
  #endif

  #if @RANGEMAX <= 1
    #set cutofflist = "1.0,0.8,0.6,0.4,0.2,0"
  #elseif @RANGEMAX <= 8
    #set cutofflist = "6,5,4,3,2,1"
  #elseif @RANGEMAX <= 15
    #set cutofflist = "12,10,8,6,4,1"
  #elseif @RANGEMAX <= 30
    #set cutofflist = "25,20,15,10,5,1"
  #elseif @RANGEMAX <= 60
    #set cutofflist = "50,40,30,20,10,1"
  #elseif @RANGEMAX <= 120
    #set cutofflist = "100,80,60,40,20,1"
  #else
    #set cutofflist = "250,200,150,100,50,1"
  #endif

  #proc legend
    reset: yes

  // moved this down.. needs to also happen w/ contentfield   scg 4/11/04
  // #include $chunk_doheatmap  

#endif

#if @contentfield != "" || @autocuts = 1
  #include $chunk_doheatmap
#endif

#if @legend != no
 // legend != no added scg 2/10/10
 #setifnotgiven legend = "max+0.7 min+0.8"
 #proc legend
  location: @legend
  reverseorder: @reverseleg
  #ifspec legendfmt format
  #ifspec legendsep sep
  #ifspec legwrap wraplen
  #ifspec legbreak extent
  #ifspec legtitle title
  #ifspec legbox backcolor
  #ifspec legframe frame
  #ifspec legtextdet textdetails

 #endproc
#endif


//// title - added scg 8/8/05
#include $chunk_title


//// user post-plot include..
#if @include2 != ""
  #include @include2
#endif

