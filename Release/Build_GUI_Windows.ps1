##  Copyright (c) MediaArea.net SARL. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license that can
##  be found in the License.html file in the root of the source tree.
##

Param([parameter(Mandatory=$true)][String]$arch)

$ErrorActionPreference = "Stop"

#-----------------------------------------------------------------------
# Setup
$release_directory = $PSScriptRoot
$version = (Get-Content "${release_directory}\..\Project\version.txt" -Raw).Trim()

#-----------------------------------------------------------------------
# Prepare
Push-Location -Path "${release_directory}\..\..\zlib\contrib\vstudio\vc17"
    ((Get-Content -Path zlibstat.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path zlibstat.vcxproj
Pop-Location

Push-Location -Path "${release_directory}\..\..\ZenLib\Project\MSVC2022\Library"
    ((Get-Content -Path ZenLib.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path ZenLib.vcxproj
Pop-Location

Push-Location -Path "${release_directory}\..\..\MediaInfoLib\Project\MSVC2022"
    ((Get-Content -Path Library\MediaInfoLib.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path Library\MediaInfoLib.vcxproj
    ((Get-Content -Path Dll\MediaInfoDll.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path Dll\MediaInfoDll.vcxproj
    ((Get-Content -Path Example\HowToUse_Dll.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path Example\HowToUse_Dll.vcxproj
    ((Get-Content -Path ShellExtension\MediaInfoShellExt.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path ShellExtension\MediaInfoShellExt.vcxproj
    ((Get-Content -Path FieldsDescription\FieldsDescription.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path FieldsDescription\FieldsDescription.vcxproj
    ((Get-Content -Path RegressionTest\RegressionTest.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path RegressionTest\RegressionTest.vcxproj
    ((Get-Content -Path PreRelease\PreRelease.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path PreRelease\PreRelease.vcxproj
Pop-Location

Push-Location -Path "${release_directory}\..\..\MediaInfo\Project\MSVC2022"
    ((Get-Content -Path CLI\MediaInfo-CLI.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path CLI\MediaInfo-CLI.vcxproj
Pop-Location

Push-Location -Path "${release_directory}\..\Project\MSVC2022"
    ((Get-Content -Path CLI\DVRescue.vcxproj) -Replace 'MultiThreadedDLL','MultiThreaded') | Set-Content -Path CLI\DVRescue.vcxproj
Pop-Location

#-----------------------------------------------------------------------
# Build
Push-Location -Path "${release_directory}\..\..\freetype"
    New-Item -ItemType directory -Name "build"
    Push-Location -Path "build"
        $Env:PKG_CONFIG_PATH="${release_directory}\..\..\output\lib\pkgconfig"
        meson setup --prefix "${release_directory}\..\..\output" --buildtype=release --default-library=static -Db_vscrt=md -Dbrotli=disabled -Dbzip2=disabled -Dharfbuzz=disabled -Dpng=disabled -Dzlib=internal ..
        ninja install
    Pop-Location
Pop-Location

Push-Location -Path "${release_directory}\..\..\harfbuzz"
    New-Item -ItemType directory -Name "build"
    Push-Location -Path "build"
        $Env:PKG_CONFIG_PATH="${release_directory}\..\..\output\lib\pkgconfig"
        meson setup --prefix "${release_directory}\..\..\output" --buildtype=release --default-library=static -Db_vscrt=md -Dglib=disabled -Dgobject=disabled -Dcairo=disabled -Dchafa=disabled -Dicu=disabled -Dgraphite=disabled -Dgraphite2=disabled -Dgdi=disabled -Ddirectwrite=disabled -Dcoretext=disabled -Dwasm=disabled -Dtests=disabled -Dintrospection=disabled -Ddocs=disabled -Ddoc_tests=false -Dutilities=disabled ..
        ninja install
    Pop-Location
Pop-Location

Push-Location -Path "${release_directory}\..\..\output\lib\pkgconfig"
    Get-ChildItem -Path *.pc | ForEach-Object {
        (Get-Content -Raw -Path $_.Name) -Replace "`r`n", "`n" | Set-Content -Path $_.Name
    }
Pop-Location

Push-Location -Path "${release_directory}\..\..\output\lib"
    Get-ChildItem lib*.a | Rename-Item -NewName { $_.Name -Replace 'lib','' -Replace '\.a$','.lib' }
Pop-Location

Push-Location -Path "${release_directory}\..\..\ffmpeg"
    (Get-Content -Raw -Path configure) -Replace 'EXESUF=\$\(exesuf \$target_os\)', 'EXESUF=.exe' | Set-Content -Path configure
    (Get-Content -Raw -Path configure) -Replace 'HOSTEXESUF=\$\(exesuf \$host_os\)', 'HOSTEXESUF=.exe' | Set-Content -Path configure
    wsl --shell-type standard --% PKG_CONFIG_PATH=$PWD/../output/lib/pkgconfig ./configure --toolchain=msvc --prefix=$PWD --enable-gpl --enable-version3 --disable-autodetect --disable-doc --disable-debug --enable-static --disable-shared --enable-libfreetype --extra-libs=msvcrt.lib
    wsl --shell-type standard --% make install
    Get-ChildItem -Recurse -Filter lib*.a | Rename-Item -NewName { $_.Name -Replace 'lib','' -Replace '\.a','.lib' }
    Copy-Item -Path "${release_directory}\..\..\output\lib\*.lib" -Destination "lib"
Pop-Location

Push-Location -Path "${release_directory}\..\..\MediaInfo\Project\MSVC2022"
    MSBuild "/p:Configuration=Release;Platform=${arch}" "/t:MediaInfo-CLI" MediaInfo.sln
Pop-Location

Push-Location -Path "${release_directory}\..\Project\MSVC2022"
    MSBuild -p:"Configuration=Release;Platform=${arch}"
Pop-Location

Push-Location -Path "${release_directory}\..\..\qwt"
    $Env:QWT_NO_SVG=1
    $Env:QWT_NO_OPENGL=1
    $Env:QWT_NO_DESIGNER=1
    qmake -recursive
    nmake Release
Pop-Location

Push-Location -Path "${release_directory}\..\Source\GUI\dvrescue"
    New-Item -ItemType directory -Name "build"
    Push-Location -Path "build"
        qmake DEFINES+=QT_AVPLAYER_MULTIMEDIA LIBS+="-lStrmiids -lGdi32 -lUser32 -lShlwapi -lVfw32 -lOleAut32 -lOle32 -lBcrypt -lWs2_32 -lSecur32 -lMfplat -lMfuuid -lfreetype" ..
        nmake
        windeployqt --release --no-translations --no-compiler-runtime --qmldir ..\dvrescue dvrescue\release\dvrescue.exe
    Pop-Location
Pop-Location
