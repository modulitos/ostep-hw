// ploticus data display engine.  Software, documentation, and examples.
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details.
// http://ploticus.sourceforge.net

//// CHRON - do a graph over time 

//// load chron-specific parameters
#setifnotgiven y = ""
#setifnotgiven y2 = ""
#setifnotgiven y3 = ""
#setifnotgiven y4 = ""
#setifnotgiven color = orange
#setifnotgiven color2 = blue
#setifnotgiven color3 = red
#setifnotgiven color4 = dullyellow
#setifnotgiven name = "#usefname"
#setifnotgiven name2 = "#usefname"
#setifnotgiven name3 = "#usefname"
#setifnotgiven name4 = "#usefname"
#setifnotgiven linedet = "color=green"
#setifnotgiven linedet2 = "color=red"
#setifnotgiven linedet3 = "color=blue"
#setifnotgiven linedet4 = "color=orange"
#setifnotgiven err2 = ""
#setifnotgiven err3 = ""
#setifnotgiven err4 = ""
#setifnotgiven pointsym = ""
#setifnotgiven pointsym2 = ""
#setifnotgiven pointsym3 = ""
#setifnotgiven pointsym4 = ""

#setifnotgiven errcolor = black
#setifnotgiven errwidth = 0.08
#setifnotgiven errthick = 0.5

#setifnotgiven datefmt = "mmddyy"
#setifnotgiven mode = bars
#setifnotgiven barwidth = ""
#setifnotgiven outline = "no"
#setifnotgiven crossover = ""
#setifnotgiven stack = no
#setifnotgiven step = "no"
#setifnotgiven tab = ""
#setifnotgiven tabmode = "mid"
#setifnotgiven xyears = ""
// #setifnotgiven xmargin = ""
// xmargin set below.. scg 10/21/04
#setifnotgiven curve = ""
#setifnotgiven order = "5"
#setifnotgiven omitweekends = "no"
#setifnotgiven unittype = date
#setifnotgiven timefld = ""
#if @CM_UNITS = 1
  #setifnotgiven legend = "min+1.2 min-1.2"
#else
  #setifnotgiven legend = "min+0.5 min-0.5"
#endif

#if @timefld != ""
  #set unittype = datetime
#endif

/////// change to use the new 'datematic' feature..
// #if @unittype = time
//  #setifnotgiven nearest = hour
//  #setifnotgiven stubfmt = HHA
// #else
//  #setifnotgiven nearest = day
//  #setifnotgiven stubfmt = "MMMdd"
// #endif
// #setifnotgiven xinc = "7"
#setifnotgiven nearest = datematic
#setifnotgiven xinc = datematic
#if @xinc != datematic
  #if @unittyp = time
    #setifnotgiven stubfmt = HHA
  #else
    #setifnotgiven stubfmt = "MMMdd"
  #endif
#endif

#include $chunk_setstd

#proc settings
dateformat: @datefmt
omitweekends: @omitweekends

#musthave data

//// read data file..  
#set context = chron
#include $chunk_read


//// restrictions..
#if @tab = "" && @y = ""
  #write stderr
    Error: 'y' is required unless 'tab' is being used.
  #endwrite 
  #exit
#endif

#if @tab != "" && @y2 != ""
  #write stderr
    only one curve is supported with tab=yes .. y2 (etc.) cancelled
  #endwrite
  #set y2 = ""
#endif



#if @tab != ""
  // count or summate..

  #proc processdata
  action: count
  #if @y = ""
    fields: @x
  #else 
    fields: @x @y
  #endif
  // showresults: yes
  #endproc

  #set x = 1
  #set y = 2

#endif

// map xnearest -> nearest (nearest is historical)
#if @xnearest = auto
  #set xnearest = @nearest
#endif

// changed these to setifnotgiven - scg 10/21/04
#if @mode = bars
  #setifnotgiven xmargin = 1
#else
  #setifnotgiven xmargin = 0
#endif

//// plot area
#include $chunk_area
// X range & Y range..
#if @xrange = ""
  xautorange: datafield=@x  nearest=@xnearest  margin=@xmargin
#else
  xrange: @xrange
#endif
#if @stack = yes
  #set combomode = stack
#else
  #set combomode = normal
#endif
#if @yrange = ""
  yautorange: datafields=@y,@y2,@y3,@y4 incmult=2.0  nearest=@ynearest  combomode=@combomode  
// #elseif @yrange = "0"
#elseif $ntoken( 2, @yrange ) = ""
  yautorange: datafields=@y,@y2,@y3,@y4 incmult=2.0 mininit=@yrange  nearest=@ynearest  combomode=@combomode
#else
  yrange: @yrange
#endif


//// Y axis..
#include $chunk_yaxis


//// X axis..
#include $chunk_xaxis
#setifnotgiven autoyears = @xyears
#ifspec autoyears
#ifspec automonths
#ifspec autodays


//// title..
#include $chunk_title


//// user pre-plot include..
#if @include1 != ""
  #include @include1
#endif


#if @pointsym = default
  #set pointsym = "shape=square style=outline fillcolor=white"
#endif
#if @pointsym2 = default
  #set pointsym2 = "shape=triangle style=outline fillcolor=white"
#endif
#if @pointsym3 = default
  #set pointsym3 = "shape=diamond style=outline fillcolor=white"
#endif
#if @pointsym4 = default
  #set pointsym4 = "shape=downtriangle style=outline fillcolor=white"
#endif
// turn off point symbols if doing stairsteps..
#if @step = yes
  #set pointsym = ""
  #set pointsym2 = ""
  #set pointsym3 = ""
  #set pointsym4 = ""
#endif


#set ncluster = 1
#if @y4 != ""
  #set ncluster = 4
#elseif @y3 != ""
  #set ncluster = 3
#elseif @y2 != ""
  #set ncluster = 2
#endif
#write stderr
  [@ncluster]
#endwrite


//// do curve fit..
#if @curve != ""
  #if @curve = "yes"
    #set curve = "color=pink width=0.5"
  #endif
  #proc curvefit
  xfield: @x
  yfield: @y
  linedetails: @curve
  order: @order
#endif
 


//// do bars or line..
#if @mode = bars
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
  #ifspec crossover
  #ifspec clickmapurl
  #ifspec clickmaplabel
  #ifspec ptselect select
  
  #if @ncluster > 1  && @stack != yes
    cluster: 1 / @ncluster
  #endif
  #saveas B

#elseif @mode = line
  // note... this is a procdef that is saved then cloned below...
  #procdef lineplot
  xfield: @x
  linedetails: @linedet
  stairstep: @step
  #ifspec gapmissing
  // the following was added - scg 11/19/04..
  #ifspec lineclip clip
  #if @pointsym != ""
    legendsampletype: line+symbol
  #else
    legendsampletype: line
  #endif

  #saveas L

  #proc lineplot
  #clone L
  yfield: @y
  pointsymbol: @pointsym
  #ifspec ptselect select
  #ifspec fill
#endif
legendlabel: @name

#if @err != ""
 #write stderr
   #+ [Please ignore any warning messages re: barwidth not useful... they are incorrect]
 #endwrite
 #proc bars
  locfield: @x
  lenfield: @y
  errbarfield: @err
  thinbarline: color=@errcolor width=@errthick
  tails: @errwidth
  truncate: yes
  #ifspec barwidth
  #ifspec ptselect select
  #if @ncluster > 1  && @stack != yes
    cluster: 1 / @ncluster
  #endif
#endif


// optional 2nd curve or set of bars..
#if @y2 != ""

  #if @mode = bars
    #proc bars
    #clone B
    lenfield: @y2
    color: @color2
    #if @barwidth = line
      thinbarline: color=@color2
    #else
      #ifspec barwidth
    #endif
    #if @stack = yes
      stackfields: *
    #else
      cluster: 2 / @ncluster
    #endif
    #ifspec ptselect2 select
  #elseif @mode = line
    #proc lineplot
    #clone L
    yfield: @y2
    linedetails: @linedet2
    pointsymbol: @pointsym2
    #ifspec fill2 fill
    #ifspec ptselect2 select
  #endif
  legendlabel: @name2 

  #if @err2 != ""
    #proc bars
    locfield: @x
    lenfield: @y2
    errbarfield: @err2
    thinbarline: color=@errcolor width=@errthick
    tails: @errwidth
    truncate: yes
    cluster: 2 / @ncluster
    #ifspec barwidth
    #ifspec ptselect select
  #endif
#endif
    

// optional 3d curve or set of bars..
#if @y3 != ""
  #if @mode = bars
    #proc bars
    #clone B
    lenfield: @y3
    color: @color3
    #if @barwidth = line
      thinbarline: color=@color3
    #else
      #ifspec barwidth
    #endif
    #if @stack = yes
      stackfields: *
    #else
      cluster: 3 / @ncluster
    #endif
    #ifspec ptselect3 select
  #elseif @mode = line
    #proc lineplot
    #clone L
    yfield: @y3
    linedetails: @linedet3
    pointsymbol: @pointsym3
    #ifspec fill3 fill
    #ifspec ptselect3 select
  #endif
  legendlabel: @name3 

  #if @err3 != ""
    #proc bars
    locfield: @x
    lenfield: @y3
    errbarfield: @err3
    thinbarline: color=@errcolor width=@errthick
    tails: @errwidth
    truncate: yes
    cluster: 3 / @ncluster
    #ifspec barwidth
    #ifspec ptselect select
  #endif
#endif


// optional 4th curve or set of bars..
#if @y4 != ""
  #if @mode = bars
    #proc bars
    #clone B
    lenfield: @y4
    color: @color4
    #if @barwidth = line
      thinbarline: color=@color4
    #else
      #ifspec barwidth
    #endif
    #if @stack = yes
      stackfields: *
    #else
      cluster: 4 / @ncluster
    #endif
    #ifspec ptselect4 select

  #elseif @mode = line
    #proc lineplot
    #clone L
    yfield: @y4
    linedetails: @linedet4
    pointsymbol: @pointsym4
    #ifspec fill4 fill
    #ifspec ptselect4 select

  #endif
  legendlabel: @name4

  #if @err4 != ""
    #proc bars
    locfield: @x
    lenfield: @y4
    errbarfield: @err4
    thinbarline: color=@errcolor width=@errthick
    tails: @errwidth
    truncate: yes
    cluster: 4 / @ncluster
    #ifspec barwidth
    #ifspec ptselect select
  #endif
#endif


//// crossover line..
#if @crossover != ""
  #proc line
  linedetails: width=0.5
  points: min @crossover(s) max @crossover(s)
#endif


// do legend
#if @tab = yes && @header = yes && @name = "#usefname"
  // field names not valid after tabulate filter..
  #set header = no
#endif
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


//// user post-plot include
#if @include2 != ""
  #include @include2
#endif

