// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// LINES - do line graph 


//// load lines-specific parameters..
#setifnotgiven cats = ""

#setifnotgiven errcolor = black
#setifnotgiven errwidth = 0.08
#setifnotgiven errthick = 0.5

#setifnotgiven y2 = ""
#setifnotgiven y3 = ""
#setifnotgiven y4 = ""

#setifnotgiven err2 = ""
#setifnotgiven err3 = ""
#setifnotgiven err4 = ""

#setifnotgiven name = "#usefname"
#setifnotgiven name2 = "#usefname"
#setifnotgiven name3 = "#usefname"
#setifnotgiven name4 = "#usefname"

#setifnotgiven linedet = "color=red"
#setifnotgiven linedet2 = "color=blue"
#setifnotgiven linedet3 = "color=green"
#setifnotgiven linedet4 = "color=orange"

#setifnotgiven pointsym = "shape=square style=outline fillcolor=white"
#setifnotgiven pointsym2 = "shape=triangle style=outline fillcolor=white"
#setifnotgiven pointsym3 = "shape=diamond style=outline fillcolor=white"
#setifnotgiven pointsym4 = "shape=downtriangle style=outline fillcolor=white"

#setifnotgiven fill = ""
#setifnotgiven fill2 = ""
#setifnotgiven fill3 = ""
#setifnotgiven fill4 = ""

#setifnotgiven fill = ""
#setifnotgiven step = ""
#if @CM_UNITS = 1
  #setifnotgiven legend = "min+1.2 min-1.2"
#else
  #setifnotgiven legend = "min+0.5 min-0.5"
#endif



//// load standard parameters..
#include $chunk_setstd


//// read data..
#include $chunk_read

#musthave x y

#if @cats = yes
  #proc categories
  axis: x
  datafield: @x
#endif


//// plotting area..
#include $chunk_area
#if @cats != yes
  #if @xrange = ""
    xautorange: datafield=@x nearest=@xnearest 
  #else
    xrange: @xrange
  #endif
#else
  xscaletype: categories
  #ifspec stubvert xaxis.stubvert
#endif
#if @yrange = ""
  #if @y2 = ""
    yautorange: datafields=@y,@err combomode=hilo incmult=2.0 nearest=@ynearest 
  #else
    yautorange: datafields=@y,@y2,@y3,@y4 incmult=2.0 nearest=@ynearest 
  #endif
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  #if @y2 = ""
    yautorange: datafields=@y,@err combomode=hilo incmult=2.0 mininit=@yrange nearest=@ynearest 
  #else
    yautorange: datafields=@y,@y2,@y3,@y4 incmult=2.0 mininit=@yrange nearest=@ynearest 
  #endif
#else
  yrange: @yrange
#endif
#endproc

//// X axis..
#include $chunk_xaxis
#if @cats != yes
  stubs: inc @xinc
#else
  stubs: usecategories
#endif
stubcull: yes

//// Y axis...
#include $chunk_yaxis
stubcull: yes


//// title..
#include $chunk_title


//// user pre-plot include...
#if @include1 != ""
  #include @include1
#endif


// turn off point symbols if doing stairsteps..
#if @step = yes
  #set pointsym = ""
  #set pointsym2 = ""
  #set pointsym3 = ""
  #set pointsym4 = ""
#endif

//// do error bar for group 1
#if @err != ""
 #proc bars
  locfield: @x
  lenfield: @y
  errbarfield: @err
  thinbarline: color=@errcolor width=@errthick
  tails: @errwidth
  truncate: yes
  #ifspec ptselect select
#endif

//// do line plot for group 1
#proc lineplot
xfield: @x
yfield: @y
#if @step = yes
  stairstep: @step
  lastseglen: 0.2
#endif
#if @fill != ""
  fill: @fill
#else
  linedetails: @linedet
#endif
legendlabel: @name
pointsymbol: @pointsym
#if @pointsym = none
  legendsampletype: line
#else
  legendsampletype: line+symbol
#endif
#ifspec gapmissing
#ifspec ptselect select
#ifspec numbers
#ifspec accum
#ifspec clip

  
//// do error bars and line for group 2
#if @y2 != ""

  #if @err2 != ""
    #proc bars
    locfield: @x
    lenfield: @y2
    errbarfield: @err2
    thinbarline: color=@errcolor width=@errthick
    tails: @errwidth
    truncate: yes
    #ifspec ptselect2 select
  #endif

  #proc lineplot
  xfield: @x
  yfield: @y2
  legendlabel: @name2
  #if @fill2 != ""
    fill: @fill2
  #else
    linedetails: @linedet2
  #endif
  #if @step = yes
    stairstep: @step
    lastseglen: 0.2
  #endif
  pointsymbol: @pointsym2
  #if @pointsym2 = none
    legendsampletype: line
  #else
    legendsampletype: line+symbol
  #endif
  #ifspec gapmissing
  #ifspec ptselect2 select
  #ifspec numbers
  #ifspec accum
  #ifspec clip
#endif


//// do error bars and line for group 3
#if @y3 != ""

  #if @err3 != ""
    #proc bars
    locfield: @x
    lenfield: @y3
    errbarfield: @err3
    thinbarline: color=@errcolor width=@errthick
    tails: @errwidth
    truncate: yes
    #ifspec ptselect3 select
  #endif

  #proc lineplot
  xfield: @x
  yfield: @y3
  legendlabel: @name3
  #if @fill3 != ""
    fill: @fill3
  #else
    linedetails: @linedet3
  #endif
  #if @step = yes
    stairstep: @step
    lastseglen: 0.2
  #endif
  pointsymbol: @pointsym3
  #if @pointsym3 = none
    legendsampletype: line
  #else
    legendsampletype: line+symbol
  #endif
  #ifspec gapmissing
  #ifspec ptselect3 select
  #ifspec numbers
  #ifspec accum
  #ifspec clip
#endif


//// do error bars and line for group 4
#if @y4 != ""

  #if @err4 != ""
    #proc bars
    locfield: @x
    lenfield: @y4
    errbarfield: @err4
    thinbarline: color=@errcolor width=@errthick
    tails: @errwidth
    truncate: yes
    #ifspec ptselect4 select
  #endif

  #proc lineplot
  xfield: @x
  yfield: @y4
  legendlabel: @name4
  #if @fill4 != ""
    fill: @fill4
  #else
    linedetails: @linedet4
  #endif
  #if @step = yes
    stairstep: @step
    lastseglen: 0.2
  #endif
  pointsymbol: @pointsym4
  #if @pointsym4 = none
    legendsampletype: line
  #else
    legendsampletype: line+symbol
  #endif
  #ifspec gapmissing
  #ifspec ptselect4 select
  #ifspec numbers
  #ifspec accum
  #ifspec clip
  
#endif
  

// do legend
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


//// user post-plot include..
#if @include2 != ""
  #include @include2
#endif

