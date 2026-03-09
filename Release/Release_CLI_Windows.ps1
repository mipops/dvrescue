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
$arch="x64"

#-----------------------------------------------------------------------
# Cleanup
$artifact = "${release_directory}\dvrescue_CLI_${version}_Windows_${arch}"
if (Test-Path "${artifact}") {
    Remove-Item -Force -Recurse "${artifact}"
}

$artifact = "${release_directory}\dvrescue_CLI_${version}_Windows_${arch}.zip"
if (Test-Path "${artifact}") {
    Remove-Item -Force "${artifact}"
}

#-----------------------------------------------------------------------
# Package CLI
Push-Location "${release_directory}"
    New-Item -Force -ItemType Directory -Path "dvrescue_CLI_${version}_Windows_${arch}"
    Push-Location "dvrescue_CLI_${version}_Windows_${arch}"
        ### Copying: Exe ###
        Copy-Item -Force "..\..\Project\MSVC2022\${arch}\Release\DVRescue.exe" .
        ### Copying: Information files ###
        Copy-Item -Force "..\..\LICENSE.txt" .
        Copy-Item -Force "..\..\History.txt" .
        ### Archive
        7za.exe a -r -tzip -mx9 "..\dvrescue_CLI_${version}_Windows_${arch}.zip" *
        7za.exe a -r -tzip -mx9 "..\dvrescue_CLI_${version}_Windows_${arch}_DebugInfo.zip" "..\..\Project\MSVC2022\${arch}\Release\DVRescue.pdb"
    Pop-Location
Pop-Location