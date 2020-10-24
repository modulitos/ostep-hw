REM MSDOS batch file to run pl examples..
REM
REM 
echo off
set PL=pl

REM which %PL

REM available modes are generally: gif, svg, eps
set MODE=gif

echo Testing %PL% .. output format will be %MODE%
echo Be sure you have pl.exe in your PATH, or else copied into this dir..
pause


set OPTS=

echo "--------- stock2..." 
%PL% -%MODE% %OPTS% stock2.htm 
echo "--------- kmslide..." 
%PL% -%MODE% %OPTS% kmslide.htm 
echo "--------- propbars1..." 
%PL% -%MODE% %OPTS% propbars1.htm 
echo "--------- td..." 
%PL% -%MODE% %OPTS% td.htm 
echo "5 plots done"
echo "--------- errbar5..." 
%PL% -%MODE% %OPTS% errbar5.htm 
echo "Note: some warnings are ok in errbar5..  " 
echo "--------- scatterplot10..." 
%PL% -%MODE% %OPTS% scatterplot10.htm 
echo "--------- devol..." 
%PL% -%MODE% %OPTS% devol.htm 
echo "10 plots done"
echo "--------- lineplot4..."
%PL% -%MODE% %OPTS% lineplot4.htm 
echo "--------- lineplot5..." 
%PL% -%MODE% %OPTS% lineplot5.htm 
echo "--------- pie1..." 
%PL% -%MODE% %OPTS% pie1.htm 
echo "--------- bars3..." 
%PL% -%MODE% %OPTS% bars3.htm 
echo "--------- quarters..." 
%PL% -%MODE% %OPTS% quarters.htm 
echo "15 plots done"
echo "--------- timeline2..." 
%PL% -%MODE% %OPTS% timeline2.htm 
echo "--------- scatterplot4..." 
%PL% -%MODE% %OPTS% scatterplot4.htm 
echo "--------- annot2..." 
%PL% -%MODE% %OPTS% annot2.htm 
echo "--------- drawcom..." 
%PL% -%MODE% %OPTS% drawcom.htm 
echo "--------- hitcount3..." 
%PL% -%MODE% %OPTS% hitcount3.htm 
echo "Note: there should be 8 'warning: time is outside of window range' msgs above..  " 
echo "20 plots done.."
echo "--------- lineplot20..." 
%PL% -%MODE% %OPTS%  lineplot20.htm 

echo "--------- colorgrid2 (should produce client-side image map to file csmap.out)..." 
%PL% -%MODE% %OPTS%  colorgrid2.htm  -csmap > csmap.out

echo "--------- heatmap3..." 
%PL% -%MODE% %OPTS%  heatmap3.htm 
echo "--------- vector1..." 
%PL% -%MODE% %OPTS%  vector1.htm 
echo "--------- windbarbs..." 
%PL% -%MODE% %OPTS%  windbarbs.htm 
echo "--------- venn..." 
%PL% -%MODE% %OPTS%  venn.htm 
echo "--------- catlines1..." 
%PL% -%MODE% %OPTS%  catlines1.htm 
echo "--------- tree1..." 
%PL% -%MODE% %OPTS%  tree1.htm 
echo "--------- dtfut..." 
%PL% -%MODE% %OPTS%  dtfut.htm 
echo "--------- rangesweep2_dostext..." 
%PL% -%MODE% %OPTS%  rangesweep2.htm 
echo "--------- sar-cpu..." 
%PL% -%MODE% %OPTS%  sar-cpu.htm 


echo "Finished."
