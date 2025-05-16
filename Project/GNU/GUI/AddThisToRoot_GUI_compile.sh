#! /bin/bash

#############################################################################
# Configure
Home=`pwd`

OS=$(uname -s)
if [ "$OS" = "Darwin" ]; then
    OS="mac"
elif [ "$(expr substr $OS 1 5)" = "Linux" ]; then
    OS="linux"
else
    OS="unix" # generic
fi

if [ "$OS" = "mac" ]; then
    export CFLAGS="-mmacosx-version-min=10.10 $CFLAGS"
    export CXXFLAGS="-mmacosx-version-min=10.10 $CXXFLAGS"
    export LDFLAGS="-mmacosx-version-min=10.10 $LDFLAGS"
fi


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
# Setup for qmake
Q_Make()
{
    if qmake --version >/dev/null 2>&1 ; then
        qmake $*
    elif qmake-qt5 --version >/dev/null 2>&1 ; then
        qmake-qt5 $*
    elif qmake5 --version >/dev/null 2>&1 ; then
        qmake5 $*
    else
        echo qmake not found, please install Qt development package
        exit 1
    fi
}

#############################################################################
# ZenLib
if [ "$OS" = "mac" ]; then
 if test -e ZenLib/Project/GNU/Library/configure; then
  cd ZenLib/Project/GNU/Library/
  test -e Makefile && rm Makefile
  chmod u+x configure
  ./configure --enable-shared --disable-static $*
  if test -e Makefile; then
   make clean
   Zen_Make
   if test -e .libs/libzen.dylib; then
    echo ZenLib compiled
   else
    echo Problem while compiling ZenLib
    exit 1
   fi
  else
   echo Problem while configuring ZenLib
   exit 1
  fi
 else
  echo ZenLib directory is not found
  exit 1
 fi
 cd $Home
fi

#############################################################################
# MediaInfoLib
if [ "$OS" = "mac" ]; then
 if test -e MediaInfoLib/Project/GNU/Library/configure; then
  cd MediaInfoLib/Project/GNU/Library/
  test -e Makefile && rm Makefile
  chmod u+x configure
  ./configure --enable-shared --disable-static $*
  if test -e Makefile; then
   make clean
   Zen_Make
   if test -e .libs/libmediainfo.dylib; then
    echo MediaInfoLib compiled
   else
    echo Problem while compiling MediaInfoLib
    exit 1
   fi
  else
   echo Problem while configuring MediaInfoLib
   exit 1
  fi
 else
  echo MediaInfoLib directory is not found
  exit 1
 fi
 cd $Home
fi

#############################################################################
# MediaInfo
if [ "$OS" = "mac" ]; then
 if test -e MediaInfo/Project/GNU/CLI/configure; then
  cd MediaInfo/Project/GNU/CLI/
  test -e Makefile && rm Makefile
  chmod u+x configure
  ./configure $*
  if test -e Makefile; then
   make clean
   Zen_Make
   if test -e mediainfo; then
    echo MediaInfo compiled
   else
    echo Problem while compiling MediaInfo
    exit 1
   fi
  else
   echo Problem while configuring MediaInfo
   exit 1
  fi
 else
  echo MediaInfo directory is not found
  exit 1
 fi
 cd $Home
fi

#############################################################################
# DVRescue
if test -e dvrescue/Project/GNU/CLI/configure; then
 cd dvrescue/Project/GNU/CLI/
 test -e Makefile && rm Makefile
 chmod u+x configure
 ./configure $*
 if test -e Makefile; then
  make clean
  Zen_Make
  if test -e dvrescue; then
   echo DVRescue compiled
  else
   echo Problem while compiling DVRescue
   exit 1
  fi
 else
  echo Problem while configuring DVRescue
  exit 1
 fi
else
 echo DVRescue directory is not found
 exit 1
fi
cd $Home

#############################################################################
# yasm
if test -e yasm/configure; then
 cd yasm
 ./configure --prefix="$PWD"
 if test -e Makefile; then
  Zen_Make
  make install
  if test -e bin/yasm; then
   echo yasm compiled
  else
   echo Problem while compiling yasm
   exit 1
  fi
 else
  echo Problem while configuring yasm
  exit 1
 fi
else
 echo yasm directory is not found
 exit 1
fi
cd $Home

#############################################################################
# freetype
if test -e freetype/meson.build; then
 mkdir freetype/build
 cd freetype/build
 if [ "$OS" = "mac" ]; then
  CFLAGS=-mmacosx-version-min=10.12 LDFLAGS=-mmacosx-version-min=10.12 meson setup --prefix $PWD/../output --default-library=static -Dbrotli=disabled -Dbzip2=disabled -Dharfbuzz=disabled -Dpng=disabled -Dzlib=internal ..
 else
  meson setup --prefix $PWD/../output --default-library=static -Dbrotli=disabled -Dbzip2=disabled -Dharfbuzz=disabled -Dpng=disabled -Dzlib=internal ..
 fi
 if test -e build.ninja; then
  ninja
  ninja install
  if test -e ../output/lib/libfreetype.a; then
   echo freetype compiled
  else
   echo Problem while compiling freetype
   exit 1
  fi
 else
  echo Problem while configuring freetype
  exit 1
 fi
else
 echo freetype directory is not found
 exit 1
fi
cd $Home

#############################################################################
# harfbuzz
if test -e harfbuzz/meson.build; then
 mkdir harfbuzz/build
 cd harfbuzz/build
 if [ "$OS" = "mac" ]; then
  CFLAGS=-mmacosx-version-min=10.12 LDFLAGS=-mmacosx-version-min=10.12 PKG_CONFIG_PATH=$PWD/../../freetype/output/lib/pkgconfig meson setup --prefix $PWD/../output --default-library=static -Dglib=disabled -Dgobject=disabled -Dcairo=disabled -Dchafa=disabled -Dicu=disabled -Dgraphite=disabled -Dgraphite2=disabled -Dgdi=disabled -Ddirectwrite=disabled -Dcoretext=disabled -Dwasm=disabled -Dtests=disabled -Dintrospection=disabled -Ddocs=disabled -Ddoc_tests=false -Dutilities=disabled ..
 else
  PKG_CONFIG_PATH=$PWD/../../freetype/output/lib/pkgconfig meson setup --prefix $PWD/../output --default-library=static -Dglib=disabled -Dgobject=disabled -Dcairo=disabled -Dchafa=disabled -Dicu=disabled -Dgraphite=disabled -Dgraphite2=disabled -Dgdi=disabled -Ddirectwrite=disabled -Dcoretext=disabled -Dwasm=disabled -Dtests=disabled -Dintrospection=disabled -Ddocs=disabled -Ddoc_tests=false -Dutilities=disabled ..
 fi
 if test -e build.ninja; then
  ninja
  ninja install
  if test -e ../output/lib/libharfbuzz.a; then
   echo harfbuzz compiled
  else
   echo Problem while compiling harfbuzz
   exit 1
  fi
 else
  echo Problem while configuring harfbuzz
  exit 1
 fi
else
 echo harfbuzz directory is not found
 exit 1
fi
cd $Home

#############################################################################
# ffmpeg
if test -e ffmpeg/configure; then
 cd ffmpeg
 if [ "$OS" = "mac" ]; then
  # fix ffmpeg configure for static freetype2
  sed -i '' 's/^enabled libfreetype.*//g' configure
  # fix ffmpeg configure for static harfbuzz
  sed -i '' 's/^enabled libharfbuzz.*//g' configure
  ./configure --x86asmexe=$Home/yasm/bin/yasm --enable-gpl --disable-securetransport --disable-videotoolbox --disable-autodetect --disable-doc --disable-debug --enable-pic --enable-shared --disable-static --prefix="$PWD" --enable-libfreetype --enable-libharfbuzz --extra-ldflags="-mmacosx-version-min=10.12" --extra-cflags="-mmacosx-version-min=10.12 -I../freetype/output/include/freetype2 -I../harfbuzz/output/include/harfbuzz" --extra-libs="../freetype/output/lib/libfreetype.a ../harfbuzz/output/lib/libharfbuzz.a"
 else
  # fix ffmpeg configure for static freetype2
  sed -i 's/^enabled libfreetype.*//g' configure
  # fix ffmpeg configure for static harfbuzz
  sed -i 's/^enabled libharfbuzz.*//g' configure
  ./configure --x86asmexe=$Home/yasm/bin/yasm --enable-gpl --disable-autodetect --disable-doc --disable-programs --disable-debug --enable-pic --enable-static --disable-shared --prefix="$PWD" --enable-libfreetype --enable-libharfbuzz --extra-cflags="-I../freetype/output/include/freetype2 -I../harfbuzz/output/include/harfbuzz" --extra-libs="../freetype/output/lib/libfreetype.a ../harfbuzz/output/lib/libharfbuzz.a"
 fi
 if test -e Makefile; then
  Zen_Make
  make install
  if test -e lib/libavcodec.a || test -e lib/libavcodec.dylib; then
   echo ffmpeg compiled
  else
   echo Problem while compiling ffmpeg
   exit 1
  fi
 else
  echo Problem while configuring ffmpeg
  exit 1
 fi
else
 echo ffmpeg directory is not found
 exit 1
fi
cd $Home

#############################################################################
# xmlstarlet
if [ "$OS" = "mac" ]; then
 if test -e xmlstarlet/configure; then
  cd xmlstarlet
  ./configure
  if test -e Makefile; then
   Zen_Make
   if test -e xml; then
    echo xmlstarlet compiled
   else
    echo Problem while compiling xmlstarlet
    exit 1
   fi
  else
   echo Problem while configuring xmlstarlet
   exit 1
  fi
 else
  echo xmlstarlet directory is not found
  exit 1
 fi
 cd $Home
fi

#############################################################################
# qwt
if test -e qwt/qwt.pro; then
 cd qwt
 if test "$OS" != "mac"; then
     export QWT_STATIC=1
 fi
 export QWT_NO_SVG=1
 export QWT_NO_OPENGL=1
 export QWT_NO_DESIGNER=1
 Q_Make
 if test -e Makefile; then
  Zen_Make
  if test -e lib/libqwt.a || test -e lib/qwt.framework; then
   echo qwt compiled
  else
   echo Problem while compiling qwt
   exit 1
  fi
 else
  echo Problem while configuring qwt
  exit 1
 fi
else
 echo qwt directory is not found
 exit 1
fi
cd $Home

#############################################################################
# dvrescue
if test -e dvrescue/Source/GUI/dvrescue/dvrescue.pro; then
 mkdir dvrescue/Source/GUI/dvrescue/build
 cd dvrescue/Source/GUI/dvrescue/build
 Q_Make ..
 if test -e Makefile; then
  Zen_Make
  if test -e dvrescue/dvrescue || test -e dvrescue/dvrescue.app/Contents/MacOS/dvrescue ; then
    if [ "$OS" = "mac" ] && ! macdeployqt dvrescue/dvrescue.app -qmldir=../dvrescue ; then
      echo Problem while bundling qt frameworks
      exit 1
     fi
   echo dvrescue compiled
  else
   echo Problem while compiling dvrescue
   exit 1
  fi
 else
  echo Problem while configuring dvrescue
  exit 1
 fi
else
 echo dvrescue directory is not found
 exit 1
fi
cd $Home

if [ "$OS" = "mac" ]; then
 mkdir -p dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a ZenLib/Project/GNU/Library/.libs/libzen.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Frameworks
 install_name_tool -id @executable_path/../Frameworks/libzen.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Frameworks/libzen.0.dylib
 cp -a MediaInfoLib/Project/GNU/Library/.libs/libmediainfo.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Frameworks
 install_name_tool -id @executable_path/../Frameworks/libmediainfo.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Frameworks/libmediainfo.0.dylib
 install_name_tool -change /usr/local/lib/libzen.0.dylib @executable_path/../Frameworks/libzen.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Frameworks/libmediainfo.0.dylib
 cp -a MediaInfo/Project/GNU/CLI/.libs/mediainfo dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 install_name_tool -change /usr/local/lib/libzen.0.dylib @executable_path/../Frameworks/libzen.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/mediainfo
 install_name_tool -change /usr/local/lib/libmediainfo.0.dylib @executable_path/../Frameworks/libmediainfo.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/mediainfo
 cp -a dvrescue/Project/GNU/CLI/.libs/dvrescue dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 install_name_tool -change /usr/local/lib/libzen.0.dylib @executable_path/../Frameworks/libzen.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/dvrescue
 install_name_tool -change /usr/local/lib/libmediainfo.0.dylib @executable_path/../Frameworks/libmediainfo.0.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/dvrescue
 cp -a ffmpeg/ffmpeg dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 avdevicever=$(basename $(ls "${PWD}"/ffmpeg/lib/libavdevice.*.dylib) | cut -d. -f2)
 avfilterver=$(basename $(ls "${PWD}"/ffmpeg/lib/libavfilter.*.dylib) | cut -d. -f2)
 avformatver=$(basename $(ls "${PWD}"/ffmpeg/lib/libavformat.*.dylib) | cut -d. -f2)
 avcodecver=$(basename $(ls "${PWD}"/ffmpeg/lib/libavcodec.*.dylib) | cut -d. -f2)
 postprocver=$(basename $(ls "${PWD}"/ffmpeg/lib/libpostproc.*.dylib) | cut -d. -f2)
 swresamplever=$(basename $(ls "${PWD}"/ffmpeg/lib/libswresample.*.dylib) | cut -d. -f2)
 swscalever=$(basename $(ls "${PWD}"/ffmpeg/lib/libswscale.*.dylib) | cut -d. -f2)
 avutilver=$(basename $(ls "${PWD}"/ffmpeg/lib/libavutil.*.dylib) | cut -d. -f2)
 install_name_tool -change "${PWD}"/ffmpeg/lib/libavdevice.${avdevicever}.dylib @executable_path/../Frameworks/libavdevice.${avdevicever}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libavfilter.${avfilterver}.dylib @executable_path/../Frameworks/libavfilter.${avfilterver}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libavformat.${avformatver}.dylib @executable_path/../Frameworks/libavformat.${avformatver}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libavcodec.${avcodecver}.dylib @executable_path/../Frameworks/libavcodec.${avcodecver}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libpostproc.${postprocver}.dylib @executable_path/../Frameworks/libpostproc.${postprocver}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libswresample.${swresamplever}.dylib @executable_path/../Frameworks/libswresample.${swresamplever}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libswscale.${swscalever}.dylib @executable_path/../Frameworks/libswscale.${swscalever}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 install_name_tool -change "${PWD}"/ffmpeg/lib/libavutil.${avutilver}.dylib @executable_path/../Frameworks/libavutil.${avutilver}.dylib dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/ffmpeg
 cp -a xmlstarlet/xml dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers/xmlstarlet
 cp -a dvrescue/tools/avfctl/avfctl dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvgraph dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvloupe dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvmap dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvpackager dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvplay dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvrescue2csv dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/dvsampler dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/test dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
 cp -a dvrescue/tools/testtc dvrescue/Source/GUI/dvrescue/build/dvrescue/dvrescue.app/Contents/Helpers
fi

#############################################################################
# Going home
cd $Home
echo "dvrescue (GUI) executable is in Source/GUI/dvrescue"
