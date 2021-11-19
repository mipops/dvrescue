!macro Install_VC_Runtime
  SetOutPath "$INSTDIR"
  File "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.28.29910\X64\Microsoft.VC142.CRT\concrt140.dll"
  File "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.28.29910\X64\Microsoft.VC142.CRT\msvcp140.dll"
  File "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.28.29910\X64\Microsoft.VC142.CRT\vccorlib140.dll"
  File "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Redist\MSVC\14.28.29910\X64\Microsoft.VC142.CRT\vcruntime140.dll"
!macroend

!macro Uninstall_VC_Runtime
  Delete "$INSTDIR\concrt140.dll"
  Delete "$INSTDIR\msvcp140.dll"
  Delete "$INSTDIR\vccorlib140.dll"
  Delete "$INSTDIR\vcruntime140.dll"
!macroend
