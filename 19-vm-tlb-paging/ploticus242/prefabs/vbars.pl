// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// VBARS - do a bar graph  (vertical bars)

//// load vbars specific parms..
#setifnotgiven barwidth = ""
#setifnotgiven xnumeric = ""
#setifnotgiven color = orange
#setifnotgiven errcolor = black
#if @CM_UNITS = 1
  #setifnotgiven errwidth = 0.2
  #setifnotgiven legend = "min+1.25 max+1.25"
#else
  #setifnotgiven errwidth = 0.08
  #setifnotgiven legend = "min+0.5 max+0.5"
#endif
#setifnotgiven errthick = 0.5
#setifnotgiven errunder = no
#setifnotgiven erroneway = @errunder
#setifnotgiven erronly = no
#setifnotgiven vals = ""

#setifnotgiven y2 = ""
#setifnotgiven err2 = ""
#setifnotgiven color2 = "powderblue"
#setifnotgiven errcolor2 = black

#setifnotgiven name = "#usefname"
#setifnotgiven name2 = "#usefname"
#setifnotgiven legend = "min+0.5 max+0.5"
#setifnotgiven sep = 0.15
#setifnotgiven outline = no
#setifnotgiven curve = ""
#setifnotgiven order = 5
#setifnotgiven crossover = ""


//// load standard parms..
#include $chunk_setstd


//// read data..
#include $chunk_read   

//// required vars..
#musthave x y

#if @xnumeric != yes
  #proc categories
  axis: x
  datafield: @x
#endif
  

//// plotting area..
#include $chunk_area
#if @xnumeric = yes
  xautorange: datafield=@x incmult=2.0 nearest=@xnearest
#else
  xscaletype: categories
  // xcategories: datafield=@x
  // // the following was added 9/2/02 scg
  // catcompmethod: exact
#endif
#if @yrange = ""
  #if @y2 = ""
    yautorange: datafields=@y,@err combomode=hilo incmult=2.0  nearest=@ynearest
  #else
    yautorange: datafields=@y,@y2 incmult=2.0  nearest=@ynearest
  #endif
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  #if @y2 = ""
    yautorange: datafields=@y,@err combomode=hilo incmult=2.0 mininit=@yrange nearest=@ynearest
  #else
    yautorange: datafields=@y,@y2 incmult=2.0 mininit=@yrange nearest=@ynearest
  #endif
#else
  yrange: @yrange
#endif


//// X axis..
#include $chunk_xaxis
stubcull: yes
#if @xnumeric = yes
  stubs: inc @xinc
#else
  stubs: usecategories
#endif


//// Y axis..
#include $chunk_yaxis
stubcull: yes


//// title..
#include $chunk_title


//// user pre-plot include..
#if @include1 != ""
  #include @include1
#endif


// optional curve behind first bar set 
#if @curve != ""
  #proc curvefit
  xfield: @x
  yfield: @y
  order: @order
  #if @curve = yes
   linedetails: color=pink width=0.5
  #else
   linedetails: @curve
  #endif
#endif


//// begin 1st bar set ////

#if @y2 != ""
  #proc catslide
  axis: x
  amount: -@sep
#endif



//// do bar graph
#if @erronly != "yes"
  #proc bars
  locfield: @x
  lenfield: @y
  color: @color
  outline: @outline
  #if @barwidth = line
    thinbarline: color=@color
  #else
    #ifspec barwidth
  #endif
  #ifspec vals showvalues
  legendlabel: @name
  #ifspec crossover
  #ifspec clickmapurl
  #ifspec clickmaplabel
  #ifspec ptselect select
  #ifspec truncate
#endif

//// do error bars 
#if @err != "" 
  #proc bars
  locfield: @x
  lenfield: @y
  #if @erroneway = yes
    errbarfield: 0 @err
  #else
    errbarfield: @err
  #endif
  thinbarline: color=@errcolor width=@errthick
  tails: @errwidth
  truncate: yes
  #if @erronly = yes
    legendlabel: @name
  #endif
  #ifspec ptselect select
#endif


//// data points..
#if @erronly = yes
  #proc scatterplot
  xfield: @x
  yfield: @y
  #set TICLEN = $arith(@errwidth*0.5)
  linelen: 0.02
  linedetails: color=@errcolor width=2
  #ifspec ptselect select
#endif
  


//// begin 2nd bar set ////

#if @y2 != ""
  #proc catslide
  axis: x
  amount: @sep


  //// bar graph..
  #if @erronly != "yes"
    #proc bars
    locfield: @x
    lenfield: @y2
    color: @color2
    outline: @outline
    #ifspec barwidth
    #ifspec vals showvalues
    legendlabel: @name2
    #ifspec crossover
    #ifspec clickmapurl
    #ifspec clickmaplabel
    #ifspec ptselect2 select
    #ifspec truncate
  #endif
  
  //// err bars over..
  #if @err != "" 
    #proc bars
    locfield: @x
    lenfield: @y2
    #if @erroneway = yes
      errbarfield: 0 @err2
    #else
      errbarfield: @err2
    #endif
    thinbarline: color=@errcolor2 width=@errthick
    tails: @errwidth
    truncate: yes
    #if @erronly = yes
      legendlabel: @name2
    #endif
    #ifspec ptselect2 select
  #endif

  //// data points
  #if @erronly = yes
    #proc scatterplot
    xfield: @x
    yfield: @y2
    #set TICLEN = $arith(@errwidth*0.5)
    linelen: 0.02
    linedetails: color=@errcolor2 width=2
    #ifspec ptselect2 select
  #endif
 
#endif


//// crossover line..
#if @crossover != ""
  #proc line
  linedetails: width=0.5
  points: min @crossover(s) max @crossover(s)
#endif

//// do legend..
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
