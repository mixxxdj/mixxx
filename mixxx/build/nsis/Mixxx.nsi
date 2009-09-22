; Mixxx.nsi
;  Original Mixxx.nsi done by Tue Haste Andersen <haste@diku.dk>, Jun 2004
;  Refactored by Garth Dahlstrom in Mar 2009 
;  Pass version into NSIS passing BINDIR like this: /DBINDIR="bin"  (SCons -> "dist", Qmake/QtC -> "bin", Qmake/Crosscompile -> "bin-win32")
!ifndef BINDIR
  !echo ""
  !echo "Usage error: "
  !echo "  /DBINDIR=X -- must be provided.  Where X is one of SCons -> 'dist', Qmake/QtC -> 'bin', Qmake/Crosscompile -> 'bin-win32')"
  !echo "  [/DRELEASE=1] -- optional, if set to 1 then the Installer is labeled without date and BZR Build number"
  !echo ""
  !error "Error: Incorrect Usage"
!endif

!define /date NOW "%Y%m%d_%H%M"
!searchparse /noerrors /file src\defs.h `#define VERSION "` VER_MAJOR `.` VER_MINOR `.` VER_BUGFIX `"`
!ifdef RELEASE
	!define VERSION "${VER_MAJOR}.${VER_MINOR}.${VER_BUGFIX}"
!else
	!ifndef BUILD_REV
		!tempfile BZR_VERION_TMPFILE
		!system 'bzr revno > "${BZR_VERION_TMPFILE}"'
		!searchparse /noerrors /file ${BZR_VERION_TMPFILE} `` BUILD_REV `:`
		!delfile "${BZR_VERION_TMPFILE}"
	!endif
	!define VERSION "BZR-${BUILD_REV}-${NOW}"
!endif

!echo '${VER_MAJOR}.${VER_MINOR}.${VER_BUGFIX} BZR: ${BUILD_REV}'

;Include Modern UI
!include "MUI.nsh"

; The name of the installer
Name "Mixxx ${VERSION}"

; The file to write
OutFile "mixxx-${VERSION}-win.exe"

; Disable the Nullsoft Installer branding text at the bottom.
BrandingText " "

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
  File "${BINDIR}\mixxx.exe"
  !ifdef INCLUDE_GDB
    File "${BINDIR}\gdb.exe"
  !endif
  File "${BINDIR}\*.dll"
  
  ; And documentation, licence etc.
  File "Mixxx-Manual.pdf"
  File "LICENSE"
  File "README"
  File "COPYING"

  SetOutPath $INSTDIR\midi
  File /r /x .bzr res\midi\*.*

  SetOutPath $INSTDIR\keyboard
  File /r /x .bzr res\keyboard\*.*

  SetOutPath $INSTDIR\skins
  File /r /x .bzr res\skins\*.*

  SetOutPath $INSTDIR\ladspa_presets
  File /r /x .bzr res\ladspa_presets\*.*

  ;Disabled for initial 1.6.0 release
  ;SetOutPath $INSTDIR\promo
  ;File  /x .bzr "dist\promo\*"

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
  !ifdef INCLUDE_GDB
    CreateShortCut "$SMPROGRAMS\Mixxx\Mixxx (inside GNU Debugger).lnk" "$INSTDIR\gdb.exe" "-silent --eval-command=run --args mixxx.exe" "$INSTDIR\mixxx.exe" 0
  !endif

  CreateShortCut "$SMPROGRAMS\Mixxx\Manual.lnk" "$INSTDIR\Mixxx-Manual.pdf" "" "$INSTDIR\Mixxx-Manual.pdf" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Desktop Shortcut" SecDesktop

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  !ifdef INCLUDE_GDB
    CreateShortCut "$DESKTOP\Mixxx (inside GNU Debugger).lnk" "$INSTDIR\gdb.exe" "-silent --eval-command=run --args mixxx.exe" "$INSTDIR\mixxx.exe" 0
  !endif
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
  RMDir /r /REBOOTOK $INSTDIR

  ; Remove shortcuts, if any
  RMDir /r /REBOOTOK "$SMPROGRAMS\Mixxx"

  Delete "$DESKTOP\Mixxx.lnk"
  !ifdef INCLUDE_GDB
    Delete "$DESKTOP\Mixxx (inside GNU Debugger).lnk"
  !endif
SectionEnd
