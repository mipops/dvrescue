##  Copyright (c) MediaArea.net SARL. All Rights Reserved.
##
##  Use of this source code is governed by a BSD-style license that can
##  be found in the License.html file in the root of the source tree.
##

$ErrorActionPreference = "Stop"

#-----------------------------------------------------------------------
# Setup
$release_directory = $PSScriptRoot
$version = (Get-Content "${release_directory}\..\Project\version.txt" -Raw).Trim()
$arch = "x64"

#-----------------------------------------------------------------------
# Cleanup
$artifact = "${release_directory}\dvrescue_GUI_${version}_Windows_${arch}"
if (Test-Path "${artifact}") {
    Remove-Item -Force -Recurse "${artifact}"
}

$artifact = "${release_directory}\dvrescue_GUI_${version}_Windows_${arch}.zip"
if (Test-Path "${artifact}") {
    Remove-Item -Force "${artifact}"
}

$artifact = "${release_directory}\dvrescue_GUI_${version}_Windows.exe"
if (Test-Path "${artifact}") {
    Remove-Item -Force "${artifact}"
}

#-----------------------------------------------------------------------
# Prepare
Push-Location "${release_directory}\.."
    attrib +S dvrescue\Source\GUI\dvrescue\dvrescue\windows-tools\cygwin\\dev\*
    attrib +S dvrescue\Source\GUI\dvrescue\dvrescue\windows-tools\cygwin\\bin\*
Pop-Location

Push-Location "${release_directory}"
    if (-not (Test-Path "xmlstarlet-1.6.1")) {
        curl.exe -LO "http://downloads.sourceforge.net/project/xmlstar/xmlstarlet/1.6.1/xmlstarlet-1.6.1-win32.zip"
        7za.exe x "xmlstarlet-1.6.1-win32.zip"
    }
Pop-Location

#-----------------------------------------------------------------------
# Package GUI
Push-Location "${release_directory}"
    New-Item -ItemType Directory -Path "dvrescue_GUI_${version}_Windows_${arch}"
    Push-Location -Path "dvrescue_GUI_${version}_Windows_${arch}"
        Copy-Item -Force -Path "${release_directory}\xmlstarlet-1.6.1\xml.exe" .
        Copy-Item -Force -Path "${release_directory}\..\..\ffmpeg\ffmpeg.exe" .
        Copy-Item -Force -Path "${release_directory}\..\..\MediaInfo\Project\MSVC2022\x64\Release\MediaInfo.exe" .
        Copy-Item -Force -Path "${release_directory}\..\History.txt" .
        Copy-Item -Force -Path "${release_directory}\..\LICENSE.txt" .
        Copy-Item -Force -Path "${release_directory}\..\Project\MSVC2022\x64\Release\DVRescue.exe" .
        Copy-Item -Force -Path "${release_directory}\..\Source\GUI\dvrescue\build\dvrescue\release\dvrescue.exe" dvrescue-gui.exe
        Copy-Item -Force -Path "${release_directory}\..\Source\GUI\dvrescue\build\dvrescue\release\qwt.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\concrt140.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\msvcp140.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\msvcp140_1.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\msvcp140_2.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\msvcp140_atomic_wait.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\msvcp140_codecvt_ids.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\vccorlib140.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\vcruntime140.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\vcruntime140_1.dll" .
        Copy-Item -Path "${Env:VCToolsRedistDir}\${arch}\Microsoft.VC143.CRT\vcruntime140_threads.dll" .
        New-Item -ItemType Directory -Path "scripts"
        Push-Location -Path "scripts"
            Copy-Item -Force -Path "${release_directory}\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\dvrescue.sh" .
            Copy-Item -Force -Path "${release_directory}\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\ffmpeg.sh" .
            Copy-Item -Force -Path "${release_directory}\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\mediainfo.sh" .
            Copy-Item -Force -Path "${release_directory}\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\xml.sh" .
        Pop-Location
        New-Item -ItemType Directory -Path "tools"
        Push-Location -Path "tools"
            Copy-Item -Force -Path "${release_directory}\..\tools\dvgraph" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvloupe" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvmap" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvpackager" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvplay" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvrescue.xsd" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvrescue2csv" .
            Copy-Item -Force -Path "${release_directory}\..\tools\dvsampler" .
        Pop-Location
        New-Item -ItemType Directory -Path "cygwin"
        Push-Location -Path "cygwin"
            xcopy /y /e /h "${release_directory}\..\Source\GUI\dvrescue\dvrescue\windows-tools\cygwin\*" # Use xcopy to preserve files attributes
        Pop-Location
        windeployqt --release --no-translations --no-compiler-runtime --qmldir "${release_directory}\..\Source\GUI\dvrescue\dvrescue" dvrescue-gui.exe

        7za.exe a -r -tzip -mx9 "..\dvrescue_GUI_${version}_Windows_${arch}.zip" *
    Pop-Location
Pop-Location

#-----------------------------------------------------------------------
# Package installer
Push-Location "${release_directory}"
    makensis.exe "..\Source\Install\dvrescue.nsi"
Pop-Location
