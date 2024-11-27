!macro Install_VC_Runtime
  SetOutPath "$INSTDIR"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\concrt140.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\msvcp140.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\msvcp140_1.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\msvcp140_2.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\msvcp140_atomic_wait.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\msvcp140_codecvt_ids.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\vccorlib140.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\vcruntime140.dll"
  File "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Redist\MSVC\14.42.34433\X64\Microsoft.VC143.CRT\vcruntime140_1.dll"
!macroend

!macro Uninstall_VC_Runtime
  Delete "$INSTDIR\concrt140.dll"
  Delete "$INSTDIR\msvcp140.dll"
  Delete "$INSTDIR\msvcp140_1.dll"
  Delete "$INSTDIR\msvcp140_2.dll"
  Delete "$INSTDIR\msvcp140_atomic_wait.dll"
  Delete "$INSTDIR\msvcp140_codecvt_ids.dll"
  Delete "$INSTDIR\vccorlib140.dll"
  Delete "$INSTDIR\vcruntime140.dll"
  Delete "$INSTDIR\vcruntime140_1.dll"
!macroend
