; Mixxx.nsi
;
; Mixxx NSI install script. 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; By Tue Haste Andersen <haste@diku.dk>, June 2004.
;
;Include Modern UI
!include "MUI.nsh"

; The name of the installer
Name "Mixxx"

; Disable the Nullsoft Installer branding text at the bottom.
BrandingText " "

; The file to write
OutFile "mixxx-1.7.0~beta1-win.exe"

; The default installation directory
InstallDir $PROGRAMFILES\Mixxx

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
InstallDirRegKey HKLM "Software\NSIS_Mixxx" "Install_Dir"

;Interface Settings
!define MUI_ABORTWARNING

; Pages
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
  
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

;Languages
!insertmacro MUI_LANGUAGE "English"

;--------------------------------

; The stuff to install
Section "Mixxx (required)" SecMixxx

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put binary files there
  File "dist\mixxx.exe"
  File "dist\*.dll"
;  File "..\mixxx-winlib\msvcm80.dll"
;  File "..\mixxx-winlib\msvcp80.dll"
;  File "..\mixxx-winlib\msvcr80.dll"

  ; And documentation, licence etc.
  File "Mixxx-Manual.pdf"
  File "LICENSE"
  File "README"
  File "COPYING"

  SetOutPath $INSTDIR\midi
  File /r /x ".svn" /x ".bzr" res\midi\*.*

  ;Disabled for initial 1.6.0 release
  ;SetOutPath $INSTDIR\promo
  ;File "dist\promo\*"

  SetOutPath $INSTDIR\keyboard
  File "res\keyboard\Standard.kbd.cfg"
  File "res\keyboard\Old.kbd.cfg"

  SetOutPath "$INSTDIR\skins"
  File /r /x ".svn" /x ".bzr" res\skins\*.*

  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\NSIS_Mixxx "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mixxx" "DisplayName" "Mixxx"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mixxx" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mixxx" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mixxx" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" SecStartMenu

  CreateDirectory "$SMPROGRAMS\Mixxx"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Mixxx\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Manual.lnk" "$INSTDIR\Mixxx-Manual.pdf" "" "$INSTDIR\Mixxx-Manual.pdf" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut" SecDesktop

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecMixxx ${LANG_ENGLISH} "Mixxx software."
  LangString DESC_SecStartMenu ${LANG_ENGLISH} "Start menu shortcuts."
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Desktop shortcut."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMixxx} $(DESC_SecMixxx)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END


;--------------------------------

; Uninstaller

Section "Uninstall"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Mixxx"
  DeleteRegKey HKLM SOFTWARE\NSIS_Mixxx

  ; Remove files and uninstaller
  Delete $INSTDIR\mixxx.exe
  Delete $INSTDIR\mixxx.log
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\uninstall.exe
  Delete $INSTDIR\Mixxx-Manual.pdf
  Delete $INSTDIR\LICENSE
  Delete $INSTDIR\README
  Delete $INSTDIR\COPYING

  ; Remove skins, keyboard, midi defs
  Delete $INSTDIR\skins\outline\*.*
  Delete $INSTDIR\skins\outlineClose\*.*
  Delete $INSTDIR\skins\outlineNetbook\*.*
  Delete $INSTDIR\skins\outlineSmall\*.*
  Delete $INSTDIR\skins\outlineMini\*.*
  Delete "$INSTDIR\skins\Collusion (1280)\*.*"
  Delete "$INSTDIR\skins\Collusion (1280-WS)\*.*"
  Delete $INSTDIR\skins\hercules\*.*
  Delete $INSTDIR\skins\nCut\*.*
  Delete $INSTDIR\skins\traditional\*.*
  Delete $INSTDIR\skins\*.*
  Delete $INSTDIR\keyboard\*.*
  Delete $INSTDIR\midi\*.*
  ;Delete $INSTDIR\promo\*.*
  RMDir "$INSTDIR\skins\outline"
  RMDir "$INSTDIR\skins\outlineNetbook"
  RMDir "$INSTDIR\skins\outlineClose"
  RMDir "$INSTDIR\skins\outlineSmall"
  RMDir "$INSTDIR\skins\outlineMini"
  RMDir "$INSTDIR\skins\Collusion (1280)"
  RMDir "$INSTDIR\skins\Collusion (1280-WS)"
  RMDir "$INSTDIR\skins\hercules"
  RMDir "$INSTDIR\skins\nCut"
  RMDir "$INSTDIR\skins\traditional"
  RMDir "$INSTDIR\skins"
  RMDir "$INSTDIR\midi"
  RMDir "$INSTDIR\keyboard"
  ;RMDir "$INSTDIR\promo"


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Mixxx\*.*"
  Delete "$DESKTOP\Mixxx.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Mixxx"
  RMDir "$INSTDIR"

SectionEnd
