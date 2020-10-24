######## PLOTICUS version 2.41    Copyright 1998-2009 Stephen C. Grubb  http://ploticus.sourceforge.net
########
######## Use this Makefile to build  pl executable  or  libploticus.  Root permission is not required.
######## Configure the 3 settings below then do:  make clean; make
######## Default settings should be ready to go for most linux platforms.
######## Problem reports and questions may be sent to  ploticus@yahoogroups.com


# defaults..
CC = gcc
PLATFORM = UNIX
RMCOM = rm -f
NOX11 = -DNOX11
NOSWF = -DNOSWF
EXE = pl



####################################################################
########## Setting 1:
########## What do you wish to build?  pl executable?  libploticus?
########## Select ONE option below by activating the appropriate lines...
########## All other lines should be commented out.
####################################################################


############ Option 1: build the pl executable (unix), no X11 ....
NOX11 = -DNOX11



############ Option 2: build the pl executable (unix), with X11 enabled .... 
# NOX11 = 
# XOBJ = x11.o interact.o

############ ...AND... activate the lines below for your platform... (X11 libs & header files)
#### linux
# XLIBS = -L/usr/X11R6/lib -lX11
# XINCLUDEDIR = -I/usr/X11R6/include
#### alt linux
# XLIBS = -lX11
# XINCLUDEDIR = -I/usr/include/X11
#### 64bit linux
# XLIBS = -L/usr/X11R6/lib64 -lX11
#### solaris
# XLIBS = -L/usr/openwin/lib -lX11
# XINCLUDEDIR = -I/usr/openwin/include
#### mac osx
# CC = cc -arch ppc -arch i386
# XLIBS = -L/usr/X11R6/lib -lX11
# XINCLUDEDIR = -I/usr/X11R6/include
#### cygwin 
# XLIBS = -L/usr/X11R6/lib -lX11
# XINCLUDEDIR = -I/usr/X11R6/include
# PLATFORM = WIN32



############ Option 3: build pl executable (native win32 pl.exe)
# PLATFORM = WIN32
# RMCOM = del



############ Option 4: build static libploticus (unix) with PNG support (remember to also select PNG in Setting 2)
# LIBEXT = a
# ARCOM = ar r
# target: libploticus-static
#### and if you want X11 enabled uncomment the following line..
# XOBJ = x11.o interact.o



############ Option 5: build shared libploticus (unix)
############ Note: untested by me ... suggestions welcome -sg
# LIBEXT = a
# FPIC = -fPIC
# SONAME = libploticus.so.0
# SHAREFLAG = -shared -Wl,-soname,$(SONAME)
# target: libploticus-so



############################################################################
######### Setting 2:
######### Ploticus uses the GD library to create PNG, GIF, JPEG images.
######### This distribution includes GD13 (GIF), and GD16 (PNG) code.
######### You can choose to use one of these or you can choose to use your 
######### own GD version, or you can choose to not use GD at all.
############################################################################


########### Option 1: use bundled GD16 (PNG only).  Requires libpng and zlib.
exetarget: plpng 
GD16LIBS = -lpng -lz
GD16H =  -I/usr/local/include
ZFLAG = -DWZ
### These should remain commented unless your libs are not in usual places, then alter to suit...
### GD16LIBS = /home/scg/lib/libpng.a /home/scg/lib/libz.a
### GD16H = -I/home/scg/lib

########### Option 2: use bundled GD13 (GIF only). No additional lib dependencies.
# exetarget: pl 

########### Option 3: use your own GD resource.  Requires GD 1.84+, libpng, zlib, libjpeg
# exetarget: plgd18 
# GD18LIBS = -lgd -lpng -lz -ljpeg 
# GD18H = 
# GDFREETYPE = 
# ZFLAG = -DWZ

########### Option 4: use your own GD resource with FreeType2 (ttf) fonts enabled.  
###########           Requires GD 1.84+, libpng, zlib, libjpeg and libfreetype
# exetarget: plgd18 
# GD18LIBS = -lgd -lpng -lz -ljpeg -lfreetype
# GD18H = 
# GDFREETYPE = -DGDFREETYPE
# ZFLAG = -DWZ

########### Option 5: don't use GD at all.
# exetarget: plnogd 
# NOGD = -DNOGD



################################################################
######## Settings 3:
######## Other various settings you may want to make.
################################################################


#### For SWF (flash) output capability activate the following.  If necessary change -lming to location of ming.
#### Note, successful with ming 0.2a but not with later ming versions..
# MING = -lming
#optional for non-standard location:  MINGH = -I/home/scg/lib


#### To set a hard-coded directory for prefabs files, edit the PREFABS_DIR define in pl.h before doing the build.
#### If the installer does this, users won't be required to have PLOTICUS_PREFABS environment variable defined.


#### If you do a "make install", where do you want the executable(s) to be moved to?
INSTALLBIN = /usr/local/bin


#### For LOCALE support (non-roman alphabets & collation), uncomment the following..
# LOCALEOBJ = localef.o
# LOCALE_FLAG = -DLOCALE

#### SVG output is always available by default.
#### If you want compressed SVG (.svgz), and you selected GD13 or no GD above, 
#### uncomment the following.  You need to have zlib.  If necessary change -lz to location of zlib.
# ZLIB = -lz
# ZFLAG = -DWZ

#### To completely remove PostScript / EPS output capability, uncomment the following..
# NOPS = -DNOPS 

#### To completely remove SVG output capability, uncomment the following..
# NOSVG = -DNOSVG 

#### To see all possible compiler warnings, uncomment the following line..
# WALL = -Wall



###############################################################################
######## Done.  Now save this file and type:  make clean; make      ###########
########                                                            ###########
###############################################################################

######## FYI... additional Makefile statement examples:
######## eg: GD16LIBS = /home/scg/lib/libpng.a /home/scg/lib/libz.a
######## eg: GD16H = -I/home/scg/lib
######## GD18LIBS = -L /home/scg/lib -lgd -lpng -lz -ljpeg -lfreetype
######## GD18H = -I/home/scg/lib/freetype-2.0.4/include -I/home/scg/lib/gd-1.8.4
######## eg: GD18H = -I/home/scg/lib/gd-1.8.4 -I/home/scg/lib/jpeg-6b -I/home/scg/lib -I/home/scg/lib/freetype-2.0.4/include
######## eg: MINGH = -I/home/scg/lib
######## eg: MING = /home/scg/lib/libming.a

###############################################################################
###############################################################################
###############################################################################
###############################################################################


GD13 = grgd13.o devstuff13.o gd13.o 
GD16 = grgd16.o devstuff16.o gd16.o gd_io.o gd_io_dp.o gd_io_file.o gd_png.o 
GD = gdfontg.o gdfontl.o gdfontmb.o gdfonts.o gdfontt.o

LIBS = $(XLIBS) -lm $(ADDLIBS) 

CFLAGS = $(FPIC) $(WALL) -DPLOTICUS -D$(PLATFORM) -DTDH_NOREC $(ZFLAG) $(LOCALE_FLAG) \
  $(NOGD) $(NOX11) $(NOPS) $(NOSVG) $(NOSWF) $(XINCLUDEDIR) $(MINGH) 

OBJ = proc_annotate.o proc_areadef.o proc_axis.o proc_bars.o proc_rect.o proc_breakaxis.o \
  proc_categories.o proc_curvefit.o proc_settings.o proc_drawcommands.o \
  proc_getdata.o proc_image.o proc_legend.o proc_legendentry.o proc_line.o proc_lineplot.o \
  proc_page.o proc_pie.o proc_print.o proc_boxplot.o proc_rangesweep.o proc_usedata.o proc_pvalue.o \
  proc_scatterplot.o proc_vector.o proc_symbol.o proc_processdata.o proc_tabulate.o proc_venndisk.o clickmap.o proc_tree.o proc_catlines.o \
  plhead.o execscriptfile.o execline.o lib.o details.o parse.o units.o preliminaries.o \
  process_arg.o plvalue_subst.o cats.o fieldnames.o autorange.o nearest.o select.o datasets.o \
  color.o plg.o init.o arrow.o mark.o block.o pcode.o ps.o svg.o swf.o stub.o winscale.o lineclip.o smoothfit.o ttest.o \
  condex.o dates.o dbinterface.o err.o functions.o glroutines.o reslimits.o secondaryops.o  \
  shell.o sinterp.o tdhkit.o times.o value.o value_subst.o variable.o custom.o \
  $(LOCALEOBJ)


# pl with gif support
pl: pl.o $(OBJ) $(XOBJ) gd00files gd13files
	$(CC) pl.o $(OBJ) $(GD13) $(GD) $(XOBJ) $(LIBS) $(ZLIB) $(MING) -o $(EXE)

# pl with png support
plpng: pl.o $(OBJ) $(XOBJ) gd00files gd16files
	$(CC) pl.o $(OBJ) $(GD16) $(GD) $(XOBJ) $(GD16LIBS) $(LIBS) $(MING) -o $(EXE)

# pl with jpeg support (user must supply gd 1.8+)
plgd18: pl.o $(OBJ) $(XOBJ) gd18files
	$(CC) pl.o grgd.o devstuff.o $(OBJ) $(XOBJ) $(GD18LIBS) $(LIBS) $(MING) -o $(EXE)

# pl with no GD support
plnogd: pl.o $(OBJ) $(XOBJ) nogdfiles
	$(CC) pl.o devstuff.o $(OBJ) $(XOBJ) $(LIBS) $(ZLIB) $(MING) -o $(EXE)

# libploticus.a with png
libploticus-static: $(OBJ) $(XOBJ) api.o gd00files gd16files
	$(RMCOM) libploticus.$(LIBEXT)
	$(ARCOM) libploticus.$(LIBEXT) $(OBJ) $(XOBJ) api.o $(GD16) $(GD)

# this one needs work... suggestions welcome
libploticus-so: $(OBJ) $(GDOBJ)
	$(RMCOM) libploticus.$(LIBEXT)
	$(CC) $(SHAREFLAG) -o libploticus.$(LIBEXT) $(OBJ) $(GDOBJ)


gd00files: 
	$(CC) $(FPIC) -I./gd13 -I./ -c gdfontg.c gdfontl.c gdfontmb.c gdfonts.c gdfontt.c
	echo "done with gd00files" > gd00files

gd13files: 
	$(CC) -c $(WALL) grgd.c -DGD13 -DPLOTICUS -I./gd13 -o grgd13.o
	$(CC) -c $(CFLAGS) devstuff.c -DGD13 -o devstuff13.o
	$(CC) -c gd13.c -I./gd13 
	echo "done with gd13files" > gd13files

gd16files: 
	$(CC) -c $(WALL) $(FPIC) grgd.c -DGD16 -DPLOTICUS -I./ -I./gd16 -o grgd16.o
	$(CC) -c $(CFLAGS) $(FPIC) devstuff.c -DGD16 -o devstuff16.o
	$(CC) -c $(FPIC) -I./gd16 -I./ $(GD16H) gd16.c gd_io.c gd_io_dp.c gd_io_file.c gd_png.c
	echo "done with gd16files" > gd16files

gd18files:
	$(CC) -c grgd.c devstuff.c -DGD18 $(GDFREETYPE) $(GD18H) $(CFLAGS)

nogdfiles:
	$(CC) -c devstuff.c $(CFLAGS)

devgrgd:
	$(CC) -c $(WALL) grgd.c -DGD13 -DPLOTICUS -I./gd13 -o grgd13.o
	touch pcode.c
	make



install:
	cp $(EXE) $(INSTALLBIN)

clean:
	$(RMCOM) *.o gd??files 

distclean:  clean
	$(RMCOM) $(EXE)


########
######## PLOTICUS     Copyright 1998-2009 Stephen C. Grubb  http://ploticus.sourceforge.net
########
