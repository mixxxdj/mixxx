; Mixxx.nsi
;
; Mixxx NSI install script. 
; has uninstall support and (optionally) installs start menu shortcuts.
;
; By Tue Haste Andersen <haste@diku.dk>, June 2004.
; Heavily modified since by Albert Santoni, Garth Dahlstrom and Sean Pappalardo.
; 
; Lots of bits lifted from http://www.improve.dk/downloads/InstallScript.txt
;
;Include Modern UI
!include "MUI.nsh"

; Definitions
!define PRODUCT_NAME "Mixxx"
;!define PRODUCT_VERSION ""  ; Specified by the SConscript
!define PRODUCT_PUBLISHER "The Mixxx Team"
!define PRODUCT_WEB_SITE "http://www.mixxx.org"
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Mixxx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define PRODUCT_UNINST_ROOT_KEY "HKLM"

; The name of the installer
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"

; Disable the Nullsoft Installer branding text at the bottom.
BrandingText " "

; The file to write and default installation directory
!ifdef x64
    OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-x64.exe"
    InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
!else
    OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-x86.exe"
    InstallDir "$PROGRAMFILES\${PRODUCT_NAME}"
!endif

; Use best compression
SetCompressor /SOLID lzma

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
;InstallDirRegKey HKLM "Software\NSIS_Mixxx" "Install_Dir"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""

;Interface Settings
!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP "res\images\mixxx_install_logo.bmp"
!define MUI_ICON "res\images\icon.ico"

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
; Install functions

Function .onInit    ; Prevent multiple installer instances
    System::Call 'kernel32::CreateMutexA(i 0, i 0, t "runningMixxxInstallerMutex") i .r1 ?e'
    Pop $R0
 
    StrCmp $R0 0 +3
        MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
        Abort
FunctionEnd

;--------------------------------
; The stuff to install
Section "Mixxx (required)" SecMixxx

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put binary files there
  File "dist\mixxx.exe"
  File "dist\*.dll"
  
  ; NOTE: you need to check the mixxx.exe.manifest file in the win??_build directory
  ; and place the appropriate versions of the listed DLL files and their manifest files
  ; into the mixxx-win[64]lib directory for packaging before making the installer
  ; (Visual C++ 2005 is msvc?80.dll and Microsoft.VC80.CRT.manifest, Visual C++ 2008 is msvc?90.dll and Microsoft.VC90.CRT.manifest)
  ;
  ; See http://mixxx.org/wiki/doku.php/build_windows_installer for full details.
  
  !ifdef x64    ; x64 versions
    File ..\mixxx-win64lib\msvcr*.dll
    File ..\mixxx-win64lib\msvcp*.dll
    File /nonfatal ..\mixxx-win64lib\msvcm*.dll
    File ..\mixxx-win64lib\Microsoft.VC*.CRT.manifest
  !else         ; x86 versions
    File ..\mixxx-winlib\msvcr*.dll
    File ..\mixxx-winlib\msvcp*.dll
    File /nonfatal ..\mixxx-winlib\msvcm*.dll
    File ..\mixxx-winlib\Microsoft.VC*.CRT.manifest
  !endif

  ; And documentation, licence etc.
  File "Mixxx-Manual.pdf"
  File "LICENSE"
  File "README"
  File "COPYING"

  SetOutPath $INSTDIR\midi
  File /r /x ".svn" /x ".bzr" dist\midi\*.*

  ;Disabled for initial 1.6.0 release
  ;SetOutPath $INSTDIR\promo
  ;File "dist\promo\*"

  SetOutPath $INSTDIR\keyboard
  File "dist\keyboard\Standard.kbd.cfg"
  File "dist\keyboard\Old.kbd.cfg"

  SetOutPath "$INSTDIR\skins"
  File /r /x ".svn" /x ".bzr" dist\skins\*.*

  ; Write the installation path into the registry
  ;WriteRegStr HKLM SOFTWARE\NSIS_Mixxx "Install_Dir" "$INSTDIR"
  WriteRegStr HKLM "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Mixxx.exe"
  
  ; Write the uninstall keys for Windows
  WriteUninstaller "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\uninst.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Mixxx.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts" SecStartMenu

  CreateDirectory "$SMPROGRAMS\Mixxx"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Mixxx\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Manual.lnk" "$INSTDIR\Mixxx-Manual.pdf" "" "$INSTDIR\Mixxx-Manual.pdf" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0
  
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

Function un.onUninstSuccess
  HideWindow
  MessageBox MB_ICONINFORMATION|MB_OK "$(^Name) was successfully removed from your computer."
FunctionEnd

Function un.onInit
!insertmacro MUI_UNGETLANGUAGE
  MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure you want to completely remove $(^Name) and all of its components?" IDYES +2
  Abort
FunctionEnd

Section "Uninstall"

  ; Remove files and uninstaller
  Delete $INSTDIR\mixxx.exe
  Delete $INSTDIR\mixxx.log
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\uninst.exe
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

  ; Remove registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  ;DeleteRegKey HKLM SOFTWARE\NSIS_Mixxx
  DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
  
SectionEnd
