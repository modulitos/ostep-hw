// COMPARE prefab
// (c) 2004 Stephen C. Grubb
//
// 2 July 2004 - added ptselect, ptselect2..4 and ptselecttags.. 
// this allows one field to be used for y, y2, y3, etc.. with different sets of rows selected
// in the code, requires tagging the fieldname with an additional tag symbolizing the select.
//
#musthave y catfields

#write stdout
  As of 2.34 the 'compare' prefab is nonfunctional.. support for it is suspended... 
  I'm not sure if there's any user interest out there... if there is, this prefab
  needs to be rewritten, probably to use proc processdata action: summary  (the 
  old proc rangebar functionality is no longer supported).  Community involvement 
  in accomplishing this is solicited.
#endwrite
#exit

#set catfields = $change( " ", ",", @catfields )
#set nbf = $count( "*", @catfields )
#set catf = $nmember( 1, @catfields )
#if @nbf = 2
  #set subcatf = $nmember( 2, @catfields )
#endif
  
#set catfields = $change( ",", " ", @catfields )
#set tmpstats = $tmpfilename( pf_compare )

#setifnotgiven color = orange
#setifnotgiven color2 = powderblue
#setifnotgiven color3 = green
#setifnotgiven color4 = dullyellow
#setifnotgiven y2 = ""
#setifnotgiven y3 = ""
#setifnotgiven y4 = ""
#setifnotgiven outline = no
#if @CM_UNITS = 1
  #setifnotgiven errwidth = 0.17
  #setifnotgiven barwidth = 0.17
  #setifnotgiven clustersep = 0.04
#else
  #setifnotgiven errwidth = 0.05
  #setifnotgiven barwidth = 0.05
  #setifnotgiven clustersep = 0.012
#endif
#setifnotgiven legend = "min+0.5 max+0.5"
#setifnotgiven erroneway = no
#setifnotgiven subgroupoffset = 0.4
#setifnotgiven crossover = ""
#setifnotgiven dataoutfile = ""

// ptselect .......
#setifnotgiven ptselecttags = ""
#if @ptselecttags != ""
  #set pttag1 = $nmember( 1, @ptselecttags )
  #set pttag2 = $nmember( 2, @ptselecttags )
  #set pttag3 = $nmember( 3, @ptselecttags )
  #set pttag4 = $nmember( 4, @ptselecttags )
#else
  #set pttag1 = ""
  #set pttag2 = ""
  #set pttag3 = ""
  #set pttag4 = ""
#endif
#if $def(ptselect) = 1 && @ptselecttags = ""
  #write stderr
  Error: if ptselect* is used, ptselecttags must also be specified.
  #endwrite
  #exit
#endif


#set subgroupoffset = $arith(@subgroupoffset/2.0)

#set nclust = 1
#setifnotgiven name = @y " " @pttag1
#if @y2 != ""
  #set nclust = 2
  #setifnotgiven name2 = @y2 " " @pttag2
#endif
#if @y3 != ""
  #set nclust = 3
  #setifnotgiven name3 = @y3 " " @pttag3
#endif
#if @y4 != ""
  #set nclust = 4
  #setifnotgiven name4 = @y4 " " @pttag4
#endif


//// load standard parms..
#include $chunk_setstd

//// read data..
#include $chunk_read   

 

///////////////////////////////////////////
// compute means and SD for each group... 
// and write this out to tmpstats file...
///////////////////////////////////////////

#loop
  #proc processdata
      action: breaks
      fields: @catfields
  #endproc
 
  #if @NRECORDS = 0
      #break
  #endif
 
  #setifnotgiven BREAKFIELD2 = ""

  #proc rangebar
  datafield: @y
  #ifspec ptselect select
  // compute means and sds only..
  showbriefstats: only
  briefstatstag: @BREAKFIELD1 @BREAKFIELD2 @y:@pttag1
  showstatsfile: @tmpstats
  skipmed: yes

  #if @y2 != ""
    #proc rangebar
    datafield: @y2
    #ifspec ptselect2 select
    // compute means and sds only..
    showbriefstats: only
    briefstatstag: @BREAKFIELD1 @BREAKFIELD2 @y2:@pttag2
    showstatsfile: @tmpstats
    skipmed: yes
  #endif
  #if @y3 != ""
    #proc rangebar
    datafield: @y3
    #ifspec ptselect3 select
    // compute means and sds only..
    showbriefstats: only
    briefstatstag: @BREAKFIELD1 @BREAKFIELD2 @y3:@pttag3
    showstatsfile: @tmpstats
  #endif
  #if @y4 != ""
    #proc rangebar
    datafield: @y4
    #ifspec ptselect4 select
    // compute means and sds only..
    showbriefstats: only
    briefstatstag: @BREAKFIELD1 @BREAKFIELD2 @y4:@pttag4
    showstatsfile: @tmpstats
  #endif


  #proc usedata
      original: yes
 
#endloop



/////////////////////////////////
// now read in the temp data file..
// and produce the plot..
/////////////////////////////////
#proc getdata
pathname: @tmpstats
#if @nbf = 1
  fieldnames: @catf varfield foo n mean sd median min max missing
#elseif @nbf = 2
  fieldnames: @catf subfield varfield foo n mean sd median min max missing
#endif
// showresults: yes
#endproc 


#proc categories
axis: x
datafield: @catf



//// plotting area..
#include $chunk_area
xscaletype: categories
// xcategories: datafield=@catf
// catcompmethod: exact
#if @yrange = ""
  yautorange: datafields=mean,sd  incmult=2.0  nearest=@ynearest
//#elseif @yrange = 0
#elseif $ntoken( 2, @yrange ) = ""
  yautorange: datafields=mean,sd incmult=2.0 mininit=@yrange nearest=@ynearest
#else
  yrange: @yrange
#endif


//// X axis..
#set xstubdet = @xstubdet " adjust=0,-0.1"
#include $chunk_xaxis
stubcull: yes
stubs: usecategories
tics: none


//// Y axis..
#include $chunk_yaxis
stubcull: yes


//// title..
#include $chunk_title


//// user pre-plot include..
#if @include1 != ""
  #include @include1
#endif

#procdef bars
  #saveas B
  locfield: @catf
  lenfield: mean
  barwidth: @barwidth
  color: @color
  outline: @outline
  clustersep: @clustersep
  #ifspec crossover

#procdef bars
  #saveas EB
  locfield: @catf
  lenfield: mean
  #if @erroneway = yes
    errbarfields: 0 sd
  #else
    errbarfields: sd sd
  #endif
  tails: @errwidth
  barwidth: @barwidth
  clustersep: @clustersep
  #ifspec errline thinbarline

#if @nbf = 1
  #proc bars
  #clone B
  cluster: 1 / @nclust
  select: @@varfield = @y:@pttag1
  legendlabel: @name
  #proc bars
  #clone EB
  cluster: 1 / @nclust
  select: @@varfield = @y:@pttag1

  #if @y2 != ""
    #proc bars
    #clone B
    cluster: 2 / @nclust
    select: @@varfield = @y2:@pttag2
    color: @color2
    legendlabel: @name2
    #proc bars
    #clone EB
    cluster: 2 / @nclust
    select: @@varfield = @y2:@pttag2
  #endif
  #if @y3 != ""
    #proc bars
    #clone B
    cluster: 3 / @nclust
    select: @@varfield = @y3:@pttag3
    color: @color3
    legendlabel: @name3
    #proc bars
    #clone EB
    cluster: 3 / @nclust
    select: @@varfield = @y3:@pttag3
  #endif
  #if @y4 != ""
    #proc bars
    #clone B
    cluster: 4 / @nclust
    select: @@varfield = @y4:@pttag4
    color: @color4
    legendlabel: @name4
    #proc bars
    #clone EB
    cluster: 4 / @nclust
    select: @@varfield = @y4:@pttag4
  #endif


#elseif @nbf = 2

   ////////// left side //////////
   #proc categories
     axis: x
     slideamount: -@subgroupoffset

   #proc scatterplot
     ylocation: min-0.1
     xfield: @catf
     text: @leftsub
     select: @@varfield = @y:@pttag1 && @subfield = @leftsub

   #proc bars
   #clone B
   cluster: 1 / @nclust
   select: @@varfield = @y:@pttag1 && @@subfield = @leftsub
   legendlabel: @name
   #proc bars
   #clone EB
   cluster: 1 / @nclust
   select: @@varfield = @y:@pttag1 && @@subfield = @leftsub
   
   #if @y2 != ""
     #proc bars
     #clone B
     cluster: 2 / @nclust
     select: @@varfield = @y2:@pttag2 && @@subfield = @leftsub
     color: @color2
     legendlabel: @name2
     #proc bars
     #clone EB
     cluster: 2 / @nclust
     select: @@varfield = @y2:@pttag2 && @@subfield = @leftsub
   #endif

   #if @y3 != ""
     #proc bars
     #clone B
     cluster: 3 / @nclust
     select: @@varfield = @y3:@pttag3 && @@subfield = @leftsub
     color: @color3
     legendlabel: @name3
     #proc bars
     #clone EB
     cluster: 3 / @nclust
     select: @@varfield = @y3:@pttag3 && @@subfield = @leftsub
   #endif

   #if @y4 != ""
     #proc bars
     #clone B
     cluster: 4 / @nclust
     select: @@varfield = @y4:pttag4 && @@subfield = @leftsub
     color: @color4
     legendlabel: @name4
     #proc bars
     #clone EB
     cluster: 4 / @nclust
     select: @@varfield = @y4:pttag4 && @@subfield = @leftsub
   #endif
   



   ////////// right side //////////
   #proc categories
     axis: x
     slideamount: @subgroupoffset

   #proc scatterplot
     ylocation: min-0.1
     xfield: @catf
     text: @rightsub
     select: @@varfield = @y:@pttag1 && @subfield = @rightsub

   #proc bars
   #clone B
   cluster: 1 / @nclust 
   select: @@varfield = @y:@pttag1 && @@subfield = @rightsub
   #proc bars
   #clone EB
   cluster: 1 / @nclust
   select: @@varfield = @y:@pttag1 && @@subfield = @rightsub

   #if @y2 != ""
     #proc bars
     #clone B
     cluster: 2 / @nclust
     select: @@varfield = @y2:@pttag2 && @@subfield = @rightsub
     color: @color2
     #proc bars
     #clone EB
     cluster: 2 / @nclust
     select: @@varfield = @y2:@pttag2 && @@subfield = @rightsub
   #endif

   #if @y3 != ""
     #proc bars
     #clone B
     cluster: 3 / @nclust
     select: @@varfield = @y3:@pttag3 && @@subfield = @rightsub
     color: @color3
     #proc bars
     #clone EB
     cluster: 3 / @nclust
     select: @@varfield = @y3:@pttag3 && @@subfield = @rightsub
   #endif

   #if @y4 != ""
     #proc bars
     #clone B
     cluster: 4 / @nclust
     select: @@varfield = @y4:@pttag4 && @@subfield = @rightsub
     color: @color4
     #proc bars
     #clone EB
     cluster: 4 / @nclust
     select: @@varfield = @y4:@pttag4 && @@subfield = @rightsub
   #endif


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


#shell
  #if @dataoutfile != ""
    mv @tmpstats @dataoutfile
  #else
    // remove the tmp file 
    rm -f @tmpstats
  #endif
#endshell
