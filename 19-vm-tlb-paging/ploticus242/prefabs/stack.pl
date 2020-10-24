// ploticus data display engine.  Software, documentation, and examples.  
// Copyright 1998-2002 Stephen C. Grubb  (scg@jax.org).
// Covered by GPL; see the file 'Copyright' for details. 
// http://ploticus.sourceforge.net

//// STACK - do a stacked bar graph 


//// load stack-specific parms..
#setifnotgiven xnumeric = ""
#setifnotgiven y2 = ""
#setifnotgiven y3 = ""
#setifnotgiven y4 = ""
#setifnotgiven color = orange
#setifnotgiven color2 = "powderblue"
#setifnotgiven color3 = "dullyellow"
#setifnotgiven color4 = "drabgreen"
#setifnotgiven name = "#usefname"
#setifnotgiven name2 = "#usefname"
#setifnotgiven name3 = "#usefname"
#setifnotgiven name4 = "#usefname"
#setifnotgiven outline = no
#setifnotgiven reverseleg = no
#setifnotgiven stackarea = no
#if @CM_UNITS = 1
  #setifnotgiven legend = "min+1.25 max+1.25"
#else
  #setifnotgiven legend = "min+0.5 max+0.5"
#endif


//// load standard parms..
#include $chunk_setstd


//// read data..
#set context = stack
#include $chunk_read

#if @xnumeric != yes
  #proc categories
  axis: x
  datafield: @x
#endif

//// plot area..
#include $chunk_area
#if @xnumeric = yes
  #set incmult = 2.0
  #if @stackarea = yes
    #set incmult = 1.0
  #endif
  #write stderr
    incmult = @incmult
  #endwrite
  xautorange: datafield=@x incmult=@incmult nearest=@xnearest
#else
  xscaletype: categories
  // xcategories: datafield=@x
  // following added 9/2/02 scg
  // catcompmethod: exact
#endif

#if @stackarea = yes
  #set combomode = normal
#else
  #set combomode = stack
#endif
#if @yrange = ""
  yautorange: datafields=@y,@y2,@y3,@y4 combomode=@combomode incmult=2.0  nearest=@ynearest
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  yautorange: datafields=@y,@y2,@y3,@y4 combomode=@combomode incmult=2.0 mininit=@yrange nearest=@ynearest
#else
  yrange: @yrange
#endif


//// x axis..
#include $chunk_xaxis
stubcull: yes
#if @xnumeric = yes
  stubs: inc @xinc
#else
  stubs: usecategories
#endif


//// y axis..
#include $chunk_yaxis
stubcull: yes


//// title..
#include $chunk_title


//// user pre-plot include..
#if @include1 != ""
  #include @include1
#endif

#if @stackarea = yes
  #if @name = "#usefname"
    #set name = "Tier 1"
    #set name2 = "Tier 2"
    #set name3 = "Tier 3"
    #set name4 = "Tier 4"
    #write stderr
       Warning: With stackarea, legend labels must be specified (name= .. name2=..)
    #endwrite
  #endif
  #if @y4 != ""
    #proc lineplot 
     xfield: @x
     yfield: @y4
     fill: @color4
     legendlabel: @name4
  #endif
  #if @y3 != ""
    #proc lineplot 
     xfield: @x
     yfield: @y3
     fill: @color3
     legendlabel: @name3
  #endif
  #if @y2 != ""
    #proc lineplot 
     xfield: @x
     yfield: @y2
     fill: @color2
     legendlabel: @name2
  #endif
  #proc lineplot
  xfield: @x
  yfield: @y
  fill: @color
  legendlabel: @name

#else
  
  //// do 1st level bars..
  #proc bars
  locfield: @x
  lenfield: @y
  color: @color
  outline: @outline
  #ifspec barwidth
  legendlabel: @name
  #ifspec ptselect select
  
  //// 2nd level bars..
  #if @y2 != ""
    #proc bars
    locfield: @x
    lenfield: @y2
    stackfields: *
    color: @color2
    outline: @outline
    #ifspec barwidth
    legendlabel: @name2
    #ifspec ptselect2 select
  #endif
  
  //// 3rd level bars..
  #if @y3 != ""
    #proc bars
    locfield: @x
    lenfield: @y3
    stackfields: *
    color: @color3
    outline: @outline
    #ifspec barwidth
    legendlabel: @name3
    #ifspec ptselect3 select
  #endif
  
  //// 4th level bars..
  #if @y4 != ""
    #proc bars
    locfield: @x
    lenfield: @y4
    stackfields: *
    color: @color4
    outline: @outline
    #ifspec barwidth
    legendlabel: @name4
    #ifspec ptselect4 select
  #endif

#endif
  
//// legend..
#if @name != "#usefname" || @header = yes
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

#endif

//// user post-plot include..
#if @include2 != ""
  #include @include2
#endif
