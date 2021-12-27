#NSIS: encoding=UTF-8
; Request application privileges for Windows Vista
RequestExecutionLevel admin

; Some defines
!define PRODUCT_NAME "dvrescue"
!define PRODUCT_PUBLISHER "MediaArea.net"
!define PRODUCT_VERSION "0.21.11"
!define PRODUCT_VERSION4 "${PRODUCT_VERSION}.0"
!define PRODUCT_WEB_SITE "https://www.mipops.org/dvrescue"
!define COMPANY_REGISTRY "Software\MediaArea.net"
!define PRODUCT_REGISTRY "Software\MediaArea.net\dvrescue"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\dvrescue-gui.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; Compression
SetCompressor /FINAL /SOLID lzma

; Conditional
!include LogicLib.nsh

; x64 stuff
!include "x64.nsh"

; VC runtime
!include "vcruntime.nsh"

; Qt dependencies
!include "qt.nsh"

; Cygwin
!include "cygwin.nsh"

; File size
!include FileFunc.nsh
!include WinVer.nsh

; Modern UI
!include "MUI2.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "..\..\Source\GUI\dvrescue\dvrescue\icons\icon.ico"

; Language Selection Dialog Settings
!define MUI_LANGDLL_REGISTRY_ROOT "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_LANGDLL_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MUI_LANGDLL_REGISTRY_VALUENAME "NSIS:Language"

; Installer pages
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
; Uninstaller pages
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Language files
!insertmacro MUI_LANGUAGE "English"

; Info
VIProductVersion "${PRODUCT_VERSION4}"
VIAddVersionKey "CompanyName"      "${PRODUCT_PUBLISHER}"
VIAddVersionKey "ProductName"      "${PRODUCT_NAME}"
VIAddVersionKey "ProductVersion"   "${PRODUCT_VERSION4}"
VIAddVersionKey "FileDescription"  "dvrescue"
VIAddVersionKey "FileVersion"      "${PRODUCT_VERSION4}"
VIAddVersionKey "LegalCopyright"   "${PRODUCT_PUBLISHER}"
VIAddVersionKey "OriginalFilename" "${PRODUCT_NAME}_GUI_${PRODUCT_VERSION}_Windows.exe"
BrandingText " "

; Modern UI end

Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "..\..\${PRODUCT_NAME}_GUI_${PRODUCT_VERSION}_Windows.exe"
InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails nevershow
ShowUnInstDetails nevershow

Function .onInit
  ${If} ${RunningX64}
    SetRegView 64
  ${EndIf}
FunctionEnd

Section "SectionPrincipale" SEC01
  SetOverwrite on
  SetOutPath "$SMPROGRAMS"
  CreateShortCut "$SMPROGRAMS\dvrescue.lnk" "$INSTDIR\dvrescue-gui.exe" "" "$INSTDIR\dvrescue-gui.exe" 0 "" "" "dvrescue"
  SetOutPath "$INSTDIR"
  File "..\..\..\..\xmlstarlet-1.6.1\xml.exe"
  File "..\..\..\ffmpeg\ffmpeg.exe"
  File "..\..\..\MediaInfo\Project\MSVC2019\CLI\x64\Release\mediainfo.exe"
  File "/oname=dvrescue-gui.exe" "..\..\Source\GUI\dvrescue\build\dvrescue\release\dvrescue.exe"
  File "..\..\Project\MSVC2017\CLI\x64\Release\dvrescue.exe"
  File "..\..\Source\GUI\dvrescue\build\dvrescue\release\QtAVPlayer.dll"
  File "..\..\Source\GUI\dvrescue\build\dvrescue\release\qwt.dll"
  File "..\..\History.txt"
  File "..\..\LICENSE.txt"

  SetOutPath "$INSTDIR\tools"
  File "..\..\tools\dvgraph"
  File "..\..\tools\dvloupe"
  File "..\..\tools\dvmap"
  File "..\..\tools\dvpackager"
  File "..\..\tools\dvplay"
  File "..\..\tools\dvrescue.xsd"
  File "..\..\tools\dvrescue2csv"
  File "..\..\tools\dvsampler"

  SetOutPath "$INSTDIR\scripts"
  File "..\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\dvrescue.sh"
  File "..\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\ffmpeg.sh"
  File "..\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\mediainfo.sh"
  File "..\..\Source\GUI\dvrescue\dvrescue\windows-tools\scripts\xml.sh"

  !insertmacro Install_VC_Runtime
  !insertmacro Install_Cygwin_Files
  !insertmacro Install_Qt_Files

  # Create files
  WriteIniStr "$INSTDIR\${PRODUCT_NAME}.url" "InternetShortcut" "URL" "${PRODUCT_WEB_SITE}"
SectionEnd

Section -Post
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\dvrescue-gui.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName"     "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher"       "${PRODUCT_PUBLISHER}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon"     "$INSTDIR\dvrescue-gui.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion"  "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout"    "${PRODUCT_WEB_SITE}"

  ${If} ${AtLeastWin7}
    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0 ; Convert the decimal KB value in $0 to DWORD, put it right back into $0
    WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "EstimatedSize" "$0" ; Create/Write the reg key with the dword value
  ${EndIf}
SectionEnd


Section Uninstall
  SetRegView 64

  Delete "$INSTDIR\${PRODUCT_NAME}.url"
  Delete "$INSTDIR\uninst.exe"
  Delete "$INSTDIR\dvrescue-gui.exe"
  Delete "$INSTDIR\dvrescue.exe"
  Delete "$INSTDIR\mediainfo.exe"
  Delete "$INSTDIR\ffmpeg.exe"
  Delete "$INSTDIR\xml.exe"
  Delete "$INSTDIR\QtAVPlayer.dll"
  Delete "$INSTDIR\qwt.dll"
  Delete "$INSTDIR\History.txt"
  Delete "$INSTDIR\LICENSE.txt"
  Delete "$INSTDIR\tools\dvgraph"
  Delete "$INSTDIR\tools\dvloupe"
  Delete "$INSTDIR\tools\dvmap"
  Delete "$INSTDIR\tools\dvpackager"
  Delete "$INSTDIR\tools\dvplay"
  Delete "$INSTDIR\tools\dvrescue.xsd"
  Delete "$INSTDIR\tools\dvrescue2csv"
  Delete "$INSTDIR\tools\dvsampler"
  RMDir  "$INSTDIR\tools"
  Delete "$INSTDIR\scripts\dvrescue.sh"
  Delete "$INSTDIR\scripts\ffmpeg.sh"
  Delete "$INSTDIR\scripts\mediainfo.sh"
  Delete "$INSTDIR\scripts\xml.sh"
  RMDir  "$INSTDIR\scripts"

  ; Old QtAV files
  Delete "$INSTDIR\QtAV1.dll"
  Delete "$INSTDIR\QtAV\plugins.qmltypes"
  Delete "$INSTDIR\QtAV\QmlAV.dll"
  Delete "$INSTDIR\QtAV\qmldir"
  Delete "$INSTDIR\QtAV\Video.qml"
  RMDir  "$INSTDIR\QtAV"

  !insertmacro Uninstall_VC_Runtime
  !insertmacro Uninstall_Qt_Files
  !insertmacro Uninstall_Cygwin_Files
  RMDir  "$INSTDIR"
  Delete "$SMPROGRAMS\dvrescue.lnk"

  SetRegView 64
  DeleteRegKey HKLM "${PRODUCT_REGISTRY}"
  DeleteRegKey /ifempty HKLM "${COMPANY_REGISTRY}"
  DeleteRegKey HKCU "${PRODUCT_REGISTRY}"
  DeleteRegKey /ifempty HKCU "${COMPANY_REGISTRY}"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
SectionEnd
