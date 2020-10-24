echo off
rem Batch file to test ploticus prefabs

set PL=pl
set PLOTICUS_PREFABS=c:..\prefabs

echo Testing %PL% .. output will be gif and svg..
echo
echo Be sure you have pl.exe in your PATH, or else copied into this dir..
echo
echo This batch file is setting PLOTICUS_PREFABS to: %PLOTICUS_PREFABS%
echo (you may need to change this)
pause

%PL% -gif -prefab dist fld=1  data=data6  curve=yes  binsize=0.05 barwidth=0.08  ygrid=yes -o dist1.gif

%PL% -gif -prefab dist  fld=1  data=data8  cats=yes  yrange=0  stubvert=yes  barwidth=0.05  ylbl="# Hits"   order=rev  -o dist2.gif

%PL%  -gif -prefab pie  values=3  labels=1  data=data9  delim=tab  title="Student enrollment" -o pie1.gif 

%PL%  -gif -prefab chron  data=data14  x=1  y=2  datefmt=yy/mm/dd  xinc="1 month" stubfmt=M  xyears=yyyy  yrange="0 25"  barwidth=line  color=red  title="# hits per day"  omitweekends=yes -o chron1.gif

%PL% -svg -prefab dist fld=1  data=data6  curve=yes  binsize=0.05 barwidth=0.08  ygrid=yes -o dist1.svg

%PL% -svg -prefab dist  fld=1  data=data8  cats=yes  yrange=0  stubvert=yes  barwidth=0.05  ylbl="# Hits"   order=rev  -o dist2.svg

%PL%  -svg -prefab pie  values=3  labels=1  data=data9  delim=tab  title="Student enrollment" -o pie1.svg

%PL%  -svg -prefab chron  data=data14  x=1  y=2  datefmt=yy/mm/dd  xinc="1 month" stubfmt=M  xyears=yyyy  yrange="0 25"  barwidth=line  color=red  title="# hits per day"  omitweekends=yes -o chron1.svg

%PL% -gif -prefab stack data=data10 x=1 y=2 y2=4 y3=6 y4=8 xlbl=Months name="Group A" name2="Group B" name3="Group C" name4="Group D" -o stack1.gif

%PL% -gif -prefab multidist overlay=yes data=data21 rotate=yes xrange="0 2000" yrange="0 20" -o multidist1.gif

%PL% -gif -prefab vdist data=data6 data2=data6b y=1 y2=1 dist=hist title="Set 1" title2="Set 2" -o vdist1.gif

%PL% -gif -prefab compare data=dexa.dat header=yes catfields=strain,sex y=RFbmc y2=LFbmc leftsub=f rightsub=m stubvert=yes title="Comparison of two variables, with f & m subcats" titledet="adjust=0,+0.4"  -o compare1.gif

