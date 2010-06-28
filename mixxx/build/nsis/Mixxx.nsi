; Mixxx.nsi
;
; Mixxx NSI install script. 
; This has uninstall support, optional mutli-user install support, 
;	and optionally installs start menu shortcuts and additional skins.
;
; By Tue Haste Andersen <haste@diku.dk>, June 2004.
; Heavily modified since by Albert Santoni, Garth Dahlstrom and Sean Pappalardo.
; 
; Lots of bits lifted from http://www.improve.dk/downloads/InstallScript.txt
;
; Use best compression
SetCompressor /SOLID lzma

;--------------------------------
; Definitions

!define PRODUCT_NAME "Mixxx"
;!define PRODUCT_VERSION ""  ; Specified by the SConscript
!define PRODUCT_PUBLISHER "The Mixxx Team"
!define PRODUCT_WEB_SITE "http://www.mixxx.org"

!define DEFAULT_SKIN "outlineNetbook"

; Assumes this script is locaed in <base>\mixxx\build\nsis
!define BASE_BUILD_DIR "..\.."

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Mixxx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} (${PRODUCT_VERSION})"

!define MULTIUSER_INSTALLMODE_INSTDIR "${PRODUCT_NAME}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_DEFAULT_REGISTRY_VALUENAME "InstallDir"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_KEY "${PRODUCT_UNINST_KEY}"
!define MULTIUSER_INSTALLMODE_INSTDIR_REGISTRY_VALUENAME "InstallDir"

!define PRODUCT_UNINST_ROOT_KEY SHELL_CONTEXT

!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE   ; Allows command-line installs to specify the mode
                                            ;   with /AllUsers or /CurrentUser
!include MultiUser.nsh

;Include Modern UI
!include "MUI2.nsh"

; The name of the installer
!ifdef x64
    Name "${PRODUCT_NAME} ${PRODUCT_VERSION} (64-bit)"
!else
    Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
!endif

; Disable the Nullsoft Installer branding text at the bottom.
BrandingText " "

; The file to write and default installation directory
!ifdef x64
    OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-x64.exe"
    ;InstallDir "$PROGRAMFILES64\${PRODUCT_NAME}"
!else
    OutFile "${PRODUCT_NAME}-${PRODUCT_VERSION}-x86.exe"
!endif

; Registry key to check for directory (so if you install again, it will 
; overwrite the old one automatically)
;InstallDirRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_DIR_REGKEY}" ""

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP "${BASE_BUILD_DIR}\res\images\mixxx_install_logo.bmp"
!define MUI_ICON "${BASE_BUILD_DIR}\res\images\icon.ico"

; Pages
!insertmacro MUI_PAGE_LICENSE "${BASE_BUILD_DIR}\LICENSE"
!insertmacro MULTIUSER_PAGE_INSTALLMODE
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
        
    !insertmacro MULTIUSER_INIT

FunctionEnd

;--------------------------------
; The stuff to install

Section "Mixxx (required)" SecMixxx

  SectionIn RO
  
  ; Set output path to the installation directory.
  SetOutPath $INSTDIR
  
  ; Put binary files there
  File "${BASE_BUILD_DIR}\dist\mixxx.exe"
  File "${BASE_BUILD_DIR}\dist\*.dll"
  
  ; Put other files there
  File "${BASE_BUILD_DIR}\dist\*.xml"
  
  ; NOTE: you need to check the mixxx.exe.manifest file in the win??_build directory
  ; and place the appropriate versions of the listed DLL files and their manifest files
  ; into the mixxx-win[64]lib-msvc directory for packaging before making the installer
  ; (Visual C++ 2005 is msvc?80.dll and Microsoft.VC80.CRT.manifest,
  ;  Visual C++ 2008 is msvc?90.dll and Microsoft.VC90.CRT.manifest)
  ;
  ; See http://mixxx.org/wiki/doku.php/build_windows_installer for full details.
  
  !ifdef x64    ; x64 versions
    File ${BASE_BUILD_DIR}\..\mixxx-win64lib-msvc\msvcr*.dll
    File ${BASE_BUILD_DIR}\..\mixxx-win64lib-msvc\msvcp*.dll
    File /nonfatal ${BASE_BUILD_DIR}\..\mixxx-win64lib-msvc\msvcm*.dll
    File ${BASE_BUILD_DIR}\..\mixxx-win64lib-msvc\Microsoft.VC*.CRT.manifest
  !else         ; x86 versions
    File ${BASE_BUILD_DIR}\..\mixxx-win32lib-msvc\msvcr*.dll
    File ${BASE_BUILD_DIR}\..\mixxx-win32lib-msvc\msvcp*.dll
    File /nonfatal ${BASE_BUILD_DIR}\..\mixxx-win32lib-msvc\msvcm*.dll
    File ${BASE_BUILD_DIR}\..\mixxx-win32lib-msvc\Microsoft.VC*.CRT.manifest
  !endif

  ; And documentation, licence etc.
  File "${BASE_BUILD_DIR}\Mixxx-Manual.pdf"
  File "${BASE_BUILD_DIR}\LICENSE"
  File "${BASE_BUILD_DIR}\README"
  File "${BASE_BUILD_DIR}\COPYING"

  SetOutPath $INSTDIR\midi
  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\midi\*.*

  SetOutPath $INSTDIR\promo\${PRODUCT_VERSION}
  File /nonfatal /r "${BASE_BUILD_DIR}\dist\promo\${PRODUCT_VERSION}\*"

  SetOutPath $INSTDIR\keyboard
  File "${BASE_BUILD_DIR}\dist\keyboard\Standard.kbd.cfg"
  File "${BASE_BUILD_DIR}\dist\keyboard\Old.kbd.cfg"

  ; Common skin files
  SetOutPath "$INSTDIR\skins"
  File /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\*.*
  
  ; Just the default skin
  SetOutPath "$INSTDIR\skins\${DEFAULT_SKIN}"
  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\${DEFAULT_SKIN}\*.*

  ; Write the installation path into the registry
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Mixxx.exe"
  
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

; Optional sections (can be disabled by the user)

Section "Minimalist skins" SecBasicSkins
  
  SetOutPath "$INSTDIR\skins"
  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\outline*
  
  ;SetOutPath "$INSTDIR\skins\outline"
  ;File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\outline\*.*
  ;SetOutPath "$INSTDIR\skins\outlineClose"
  ;File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\outlineClose\*.*
  ;SetOutPath "$INSTDIR\skins\outlineSmall"
  ;File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\outlineSmall\*.*
  ;SetOutPath "$INSTDIR\skins\outlineMini"
  ;File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist\skins\outlineMini\*.*
  
SectionEnd

Section "Additional skins (recommended)" SecFancySkins

  SetOutPath "$INSTDIR\skins"
  File /r /x ".svn" /x ".bzr" /x "outline*" ${BASE_BUILD_DIR}\dist\skins\*.*
  
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu

  CreateDirectory "$SMPROGRAMS\Mixxx"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Mixxx\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Manual.lnk" "$INSTDIR\Mixxx-Manual.pdf" "" "$INSTDIR\Mixxx-Manual.pdf" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Uninstall.lnk" "$INSTDIR\uninst.exe" "" "$INSTDIR\uninst.exe" 0
  
SectionEnd

Section "Desktop Shortcut" SecDesktop

  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  
SectionEnd

;--------------------------------
; Descriptions

  ;Language strings
  LangString DESC_SecMixxx ${LANG_ENGLISH} "Mixxx itself with the default wide-format skin (1024x600) using the Outline theme, and mappings for all supported controllers"
  LangString DESC_SecBasicSkins ${LANG_ENGLISH} "Additional skins using the Outline theme (featuring a clear, clean and simple layout,) including one for 800x600 screens"
  LangString DESC_SecFancySkins ${LANG_ENGLISH} "Additional skins with different themes, flashier graphics, and some for screens larger than 1024x768"
  LangString DESC_SecStartMenu ${LANG_ENGLISH} "Mixxx program group containing useful shortcuts appearing under the [All] Programs section under the Start menu"
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Shortcut to Mixxx placed on the Desktop"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMixxx} $(DESC_SecMixxx)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBasicSkins} $(DESC_SecBasicSkins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecFancySkins} $(DESC_SecFancySkins)
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
    !insertmacro MULTIUSER_UNINIT
FunctionEnd

Section "Uninstall"

  ; Remove files and uninstaller
  Delete $INSTDIR\mixxx.exe
  Delete $INSTDIR\mixxx.log
  Delete $INSTDIR\*.dll
  Delete $INSTDIR\*.xml
  Delete $INSTDIR\*.manifest
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
  Delete $INSTDIR\promo\${PRODUCT_VERSION}\*.*
  Delete $INSTDIR\promo\*.*
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
  RMDir /r "$INSTDIR\promo\${PRODUCT_VERSION}"
  RMDir "$INSTDIR\promo"


  ; Remove shortcuts, if any
  Delete "$SMPROGRAMS\Mixxx\*.*"
  Delete "$DESKTOP\Mixxx.lnk"

  ; Remove directories used
  RMDir "$SMPROGRAMS\Mixxx"
  RMDir "$INSTDIR"

  ; Remove registry keys
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}"
  DeleteRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_DIR_REGKEY}"
  SetAutoClose true
  
SectionEnd
