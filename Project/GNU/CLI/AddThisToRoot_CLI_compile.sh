#! /bin/sh

#############################################################################
# Configure
Home=`pwd`
DVRescue_Options="--enable-staticlibs"
MediaInfoLib_Options="--enable-static --disable-shared"
ZenLib_Options="--enable-static --disable-shared"

#############################################################################
# Setup for parallel builds
Zen_Make()
{
 if test -e /proc/stat; then
  numprocs=`egrep -c ^cpu[0-9]+ /proc/stat || :`
  if [ "$numprocs" = "0" ]; then
   numprocs=1
  fi
  make -s -j$numprocs
 else
  make
 fi
}

#############################################################################
# ZenLib
if test -e ZenLib/Project/GNU/Library/configure; then
 cd ZenLib/Project/GNU/Library/
 test -e Makefile && rm Makefile
 chmod u+x configure
 ./configure $ZenLib_Options $*
 if test -e Makefile; then
  make clean
  Zen_Make
  if test -e libzen.la; then
   echo ZenLib compiled
  else
   echo Problem while compiling ZenLib
   exit
  fi
 else
  echo Problem while configuring ZenLib
  exit
 fi
else
 echo ZenLib directory is not found
 exit
fi
cd $Home

#############################################################################
# MediaInfoLib
if test -e MediaInfoLib/Project/GNU/Library/configure; then
 cd MediaInfoLib/Project/GNU/Library/
 test -e Makefile && rm Makefile
 chmod u+x configure
 ./configure $MediaInfoLib_Options $*
 if test -e Makefile; then
  make clean
  Zen_Make
  if test -e libmediainfo.la; then
   echo MediaInfoLib compiled
  else
   echo Problem while compiling MediaInfoLib
   exit
  fi
 else
  echo Problem while configuring MediaInfoLib
  exit
 fi
else
 echo MediaInfoLib directory is not found
 exit
fi
cd $Home

#############################################################################
# DVRescue
if test -e dvrescue/Project/GNU/CLI/configure; then
 cd dvrescue/Project/GNU/CLI/
 test -e Makefile && rm Makefile
 chmod u+x configure
 ./configure $DVRescue_Options $*
 if test -e Makefile; then
  make clean
  Zen_Make
  if test -e dvrescue; then
   echo DVRescue compiled
  else
   echo Problem while compiling DVRescue
   exit
  fi
 else
  echo Problem while configuring DVRescue
  exit
 fi
else
 echo DVRescue directory is not found
 exit
fi
cd $Home

#############################################################################
# Going home
cd $Home
echo "DVRescue executable is in Project/GNU/CLI"
echo "For installing, cd Project/GNU/CLI && make install"
