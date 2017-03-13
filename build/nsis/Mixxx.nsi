; Mixxx.nsi
;
; Mixxx NSI install script.
; This has uninstall support, optional mutli-user install support,
;	and optionally installs start menu shortcuts, controller mappings and additional skins.
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
!define PRODUCT_PUBLISHER "The Mixxx Development Team"
!define PRODUCT_WEB_SITE "http://www.mixxx.org"

; Assumes this script is locaed in <base>\mixxx\build\nsis
!define BASE_BUILD_DIR "..\.."

!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\Mixxx.exe"
!define PRODUCT_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME} (${PRODUCT_VERSION})"

; The name of the installer
!ifdef x64
    Name "${PRODUCT_NAME} ${PRODUCT_VERSION} (64-bit)"
    !define BITWIDTH "64"
    !define ARCH "x64"
; In order for the below line to work, you must patch your C:\Program Files (x86)\NSIS\Include\MultiUser.nsh file with the one given at this link:
;   http://sourceforge.net/tracker/?func=detail&atid=373085&aid=2355677&group_id=22049
    !define MULTIUSER_USE_PROGRAMFILES64 "True"
!else
    Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
    !define BITWIDTH "32"
    !define ARCH "x86"
!endif

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

; Disable the Nullsoft Installer branding text at the bottom.
BrandingText " "

; The file to write and default installation directory. This is provided by the
; SConscript. Write to the base directory (assuming we are in <root>/build/nsis/
OutFile "${BASE_BUILD_DIR}\${PACKAGE_NAME}"

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
;InstallDirRegKey ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_DIR_REGKEY}" ""

;--------------------------------
; Interface Settings

!define MUI_ABORTWARNING

!define MUI_HEADERIMAGE
;!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP_NOSTRETCH
!define MUI_HEADERIMAGE_BITMAP ${BASE_BUILD_DIR}\res\images\mixxx_install_logo.bmp
!define MUI_ICON "${BASE_BUILD_DIR}\res\images\ic_mixxx.ico"

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

;-------------------------------
; Install the VC 2010 redistributable DLLs if they're not already.
Function InstallVCRedist
  Push $R0
  Call CheckVCRedist
  Pop $R0
  StrCmp $R0 "-1" 0 VCRedistDone

  ; Install them
  SetOutPath $TEMP

  ; Put the VC redist installer files there
  File ${WINLIB_PATH}\vcredist_${ARCH}.exe

  ClearErrors
  ; Call it & wait for it to install
  ExecWait 'vcredist_${ARCH}.exe /quiet /install'
  Delete "$TEMP\vc_redist_${ARCH}.exe"
  IfErrors 0 VCRedistDone
  MessageBox MB_ICONSTOP|MB_OK "There was a problem installing the Microsoft Visual C++ libraries.$\r$\nYou may need to run this installer as an administrator."
  Abort

  ; OLD VC stuff below

  ; NOTE: you need to check the mixxx.exe.manifest file in the win??_build directory
  ; and place the appropriate versions of the listed DLL files and their manifest files
  ; into the mixxx-win[64]lib-msvc directory for packaging before making the installer
  ; (Visual C++ 2005 is msvc?80.dll and Microsoft.VC80.CRT.manifest,
  ;  Visual C++ 2008 is msvc?90.dll and Microsoft.VC90.CRT.manifest)
  ;
  ; See http://mixxx.org/wiki/doku.php/build_windows_installer for full details.
  ;
  ; All the MSVC files are located here if you have MSVC 2008 installed. (x86)
  ;File "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*"
  ;File "$%VCINSTALLDIR%\redist\x86\Microsoft.VC90.CRT\*"
  ;File "$%VS90COMNTOOLS%\..\..\VC\redist\x86\Microsoft.VC90.CRT\*"

  ; NOTE: The Microsoft Visual C++ 2010 Runtime gets rid of the manifest file, so it
  ;         is no longer necessary if we switch to deploying with MSVC 2010. - Albert

  ; If you have the msvc DLLs & manifest elsewhere,
  ; copy them to the WINLIB_PATH and uncomment these:
  ;File ${WINLIB_PATH}\msvcr*.dll        ; Required
  ;File ${WINLIB_PATH}\msvcp*.dll        ; Required
  ;File /nonfatal ${WINLIB_PATH}\msvcm*.dll    ; Not (currently) required, so nonfatal
  ;File ${WINLIB_PATH}\Microsoft.VC*.CRT.manifest    ; Required on MSVC < 2010, apparently

  VCRedistDone:
    Exch $R0

FunctionEnd

;-------------------------------
; Test if Visual C++ Redistributables 10.0 are installed
; Returns -1 if they're not
Function CheckVCRedist
   Push $R0
   ClearErrors
   ReadRegDword $R0 HKLM "SOFTWARE\Wow6432Node\Microsoft\VisualStudio\12.0\VC\Runtimes\${ARCH}" "Installed"
   ; Old way:
   ;   x64
   ;ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{DA5E371C-6333-3D8A-93A4-6FD5B20BCC6E}" "Version"
   ;   x86
   ;ReadRegDword $R0 HKLM "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{196BB40D-1578-3D01-B289-BEFC77A11A1E}" "Version"

   IfErrors 0 VSRedistInstalled
   StrCpy $R0 "-1"

VSRedistInstalled:
   Exch $R0

FunctionEnd

;--------------------------------
; The stuff to install

Section "Mixxx (required)" SecMixxx

  SectionIn RO

  Call InstallVCRedist

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put binary files there
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\mixxx.exe"
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\*.dll"

  ; Put other files there
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\*.xml"

  ; And documentation, licence etc.
  File "${BASE_BUILD_DIR}\Mixxx-Manual.pdf"
  File "${BASE_BUILD_DIR}\LICENSE"
  File "${BASE_BUILD_DIR}\README"
  File "${BASE_BUILD_DIR}\COPYING"

  SetOutPath $INSTDIR\promo\${PRODUCT_VERSION}
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\promo\${PRODUCT_VERSION}\*"

  SetOutPath $INSTDIR\sqldrivers
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\sqldrivers\*"

  SetOutPath $INSTDIR\plugins
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\plugins\*.dll"

  SetOutPath $INSTDIR\plugins\soundsource
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\plugins\soundsource\*.dll"

  SetOutPath $INSTDIR\plugins\vamp
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\plugins\vamp\*.dll"

  SetOutPath $INSTDIR\keyboard
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\keyboard\*.kbd.cfg"

  ; HID/MIDI controller presets
  SetOutPath $INSTDIR\controllers
  File ${BASE_BUILD_DIR}\dist${BITWIDTH}\controllers\*.xml
  File ${BASE_BUILD_DIR}\dist${BITWIDTH}\controllers\*.js

  ; Skins
  SetOutPath "$INSTDIR\skins"
  File /r ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*

  ; Write the installation path into the registry
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_DIR_REGKEY}" "" "$INSTDIR\Mixxx.exe"

  ; Write the uninstall keys for Windows
  WriteUninstaller "$INSTDIR\UninstallMixxx.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayName" "$(^Name)"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "UninstallString" "$INSTDIR\UninstallMixxx.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Mixxx.exe"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "DisplayVersion" "${PRODUCT_VERSION}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "URLInfoAbout" "${PRODUCT_WEB_SITE}"
  WriteRegStr ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "Publisher" "${PRODUCT_PUBLISHER}"
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoModify" 1
  WriteRegDWORD ${PRODUCT_UNINST_ROOT_KEY} "${PRODUCT_UNINST_KEY}" "NoRepair" 1

SectionEnd

; Optional sections (can be disabled by the user)

Section "Translations" SecTranslations
    SetOutPath "$INSTDIR\translations"
    File /r ${BASE_BUILD_DIR}\dist${BITWIDTH}\translations\*.qm
    File /r ${QTDIR}\translations\qt_*.qm
SectionEnd

Section "Start Menu Shortcuts" SecStartMenu
  CreateDirectory "$SMPROGRAMS\Mixxx"
  SetOutPath $INSTDIR
  CreateShortCut "$SMPROGRAMS\Mixxx\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
  CreateShortCut "$SMPROGRAMS\Mixxx\Manual.lnk" "$INSTDIR\Mixxx-Manual.pdf" "" "$INSTDIR\Mixxx-Manual.pdf" 0
SectionEnd

Section "Desktop Shortcut" SecDesktop
  SetOutPath $INSTDIR
  CreateShortCut "$DESKTOP\Mixxx.lnk" "$INSTDIR\mixxx.exe" "" "$INSTDIR\mixxx.exe" 0
SectionEnd

;--------------------------------
; Descriptions

  ; Language strings
  LangString DESC_SecMixxx ${LANG_ENGLISH} "Mixxx itself in US English"
  LangString DESC_SecStartMenu ${LANG_ENGLISH} "Mixxx program group containing useful shortcuts appearing under the [All] Programs section under the Start menu"
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Shortcut to Mixxx placed on the Desktop"
  LangString DESC_SecTranslations ${LANG_ENGLISH} "Translations for all available languages"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMixxx} $(DESC_SecMixxx)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecTranslations} $(DESC_SecTranslations)
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
  Delete $INSTDIR\schema.xml
  Delete $INSTDIR\*.manifest
  Delete $INSTDIR\UninstallMixxx.exe
  Delete $INSTDIR\Mixxx-Manual.pdf
  Delete $INSTDIR\LICENSE
  Delete $INSTDIR\README
  Delete $INSTDIR\COPYING
  Delete $INSTDIR\sqldrivers\*.dll
  RMDir "$INSTDIR\sqldrivers"
  Delete $INSTDIR\plugins\soundsource\*
  RMDir "$INSTDIR\plugins\soundsource"
  Delete $INSTDIR\plugins\vamp\*
  RMDir "$INSTDIR\plugins\vamp"
  Delete $INSTDIR\plugins\*
  RMDir "$INSTDIR\plugins"
  Delete $INSTDIR\translations\*
  RMDir "$INSTDIR\translations"

  ; Remove keyboard mappings
  ; TODO(XXX): Only delete files that were not changed since install
  ; Custom Keyboard mappings should be placed in Custom.kbd.cfg in user folder
  Delete $INSTDIR\keyboard\*.kbd.cfg
  RMDir "$INSTDIR\keyboard" ; No /r flag means remove the directory only if it's empty

  ; Remove midi mappings/scripts that we may have installed
  ; TODO: Only delete files that were not changed since install
  ; Get this list with dir /b /s <build_dir>\res\controllers >> filestodelete.txt  and creative search & replace
  Delete "$INSTDIR\controllers\Akai MPD24.midi.xml"
  Delete "$INSTDIR\controllers\American Audio RADIUS 2000 CH1.midi.xml"
  Delete "$INSTDIR\controllers\American Audio RADIUS 2000 CH2.midi.xml"
  Delete "$INSTDIR\controllers\American-Audio-RADIUS-2000-scripts.js"
  Delete "$INSTDIR\controllers\American Audio VMS4.midi.xml"
  Delete "$INSTDIR\controllers\American-Audio-VMS4-scripts.js"
  Delete "$INSTDIR\controllers\Behringer BCD3000.midi.xml"
  Delete "$INSTDIR\controllers\Behringer-BCD3000-scripts.js"
  Delete "$INSTDIR\controllers\convertToXMLSchemaV1.php"
  Delete "$INSTDIR\controllers\DJ-Tech i-Mix Reload.midi.xml"
  Delete "$INSTDIR\controllers\DJ-Tech-i-Mix-Reload-scripts.js"
  Delete "$INSTDIR\controllers\DJTechTools MIDI Fighter.midi.xml"
  Delete "$INSTDIR\controllers\DJTechTools-MIDIFighter-scripts.js"
  Delete "$INSTDIR\controllers\Evolution_Xsession.midi.xml"
  Delete "$INSTDIR\controllers\FaderFoxDJ2.midi.xml"
  Delete "$INSTDIR\controllers\Hercules DJ Console Mac Edition.midi.xml"
  Delete "$INSTDIR\controllers\Hercules DJ Console Mk2.midi.xml"
  Delete "$INSTDIR\controllers\Hercules-DJ-Console-Mk2-scripts.js"
  Delete "$INSTDIR\controllers\Hercules DJ Console Mk4.midi.xml"
  Delete "$INSTDIR\controllers\Hercules-DJ-Console-Mk4-scripts.js"
  Delete "$INSTDIR\controllers\Hercules DJ Console RMX Advanced.midi.xml"
  Delete "$INSTDIR\controllers\Hercules DJ Console RMX.midi.xml"
  Delete "$INSTDIR\controllers\Hercules-DJ-Console-RMX-scripts.js"
  Delete "$INSTDIR\controllers\Hercules DJ Control MP3 e2.midi.xml"
  Delete "$INSTDIR\controllers\Hercules DJ Control MP3 e2-scripts.js"
  Delete "$INSTDIR\controllers\Hercules DJ Control MP3.midi.xml"
  Delete "$INSTDIR\controllers\Hercules-DJ-Control-MP3-scripts.js"
  Delete "$INSTDIR\controllers\Hercules DJ Control Steel.midi.xml"
  Delete "$INSTDIR\controllers\Hercules-DJ-Control-Steel-scripts.js"
  Delete "$INSTDIR\controllers\Ion Discover DJ.midi.xml"
  Delete "$INSTDIR\controllers\Ion-Discover-DJ-scripts.js"
  Delete "$INSTDIR\controllers\M-Audio_Xponent.midi.xml"
  Delete "$INSTDIR\controllers\M-Audio-Xponent-scripts.js"
  Delete "$INSTDIR\controllers\M-Audio_Xsession_pro.midi.xml"
  Delete "$INSTDIR\controllers\Midi-Keyboard.midi.xml"
  Delete "$INSTDIR\controllers\common-controller-scripts.js"
  Delete "$INSTDIR\controllers\MidiTech-MidiControl.midi.xml"
  Delete "$INSTDIR\controllers\Mixman DM2 (Linux).js"
  Delete "$INSTDIR\controllers\Mixman DM2 (Linux).midi.xml"
  Delete "$INSTDIR\controllers\Mixman DM2 (OS X).js"
  Delete "$INSTDIR\controllers\Mixman DM2 (OS X).midi.xml"
  Delete "$INSTDIR\controllers\Mixman DM2 (Windows).midi.xml"
  Delete "$INSTDIR\controllers\Numark MIXTRACK.midi.xml"
  Delete "$INSTDIR\controllers\Numark-MixTrack-scripts.js"
  Delete "$INSTDIR\controllers\Numark NS7.midi.xml"
  Delete "$INSTDIR\controllers\Numark-NS7-scripts.js"
  Delete "$INSTDIR\controllers\Numark Total Control.midi.xml"
  Delete "$INSTDIR\controllers\Numark-Total-Control-scripts.js"
  Delete "$INSTDIR\controllers\Pioneer CDJ-2000.midi.xml"
  Delete "$INSTDIR\controllers\Pioneer-CDJ-2000-scripts.js"
  Delete "$INSTDIR\controllers\Pioneer CDJ-350 Ch1.midi.xml"
  Delete "$INSTDIR\controllers\Pioneer CDJ-350 Ch2.midi.xml"
  Delete "$INSTDIR\controllers\Pioneer-CDJ-350-scripts.js"
  Delete "$INSTDIR\controllers\Pioneer CDJ-850.midi.xml"
  Delete "$INSTDIR\controllers\Pioneer-CDJ-850-scripts.js"
  Delete "$INSTDIR\controllers\README.txt"
  Delete "$INSTDIR\controllers\Reloop Digital Jockey 2 Controller Edition.midi.xml"
  Delete "$INSTDIR\controllers\Reloop-Digital-Jockey2-Controller-scripts.js"
  Delete "$INSTDIR\controllers\Stanton SCS.1d.midi.xml"
  Delete "$INSTDIR\controllers\Stanton-SCS1d-scripts.js"
  Delete "$INSTDIR\controllers\Stanton SCS.1m.midi.xml"
  Delete "$INSTDIR\controllers\Stanton-SCS1m-scripts.js"
  Delete "$INSTDIR\controllers\Stanton SCS.3d.midi.xml"
  Delete "$INSTDIR\controllers\Stanton-SCS3d-scripts.js"
  Delete "$INSTDIR\controllers\Stanton SCS.3m.midi.xml"
  Delete "$INSTDIR\controllers\Stanton-SCS3m-scripts.js"
  Delete "$INSTDIR\controllers\TrakProDJ iPad.midi.xml"
  Delete "$INSTDIR\controllers\TrakProDJ-iPad-scripts.js"
  Delete "$INSTDIR\controllers\us428.midi.xml"
  Delete "$INSTDIR\controllers\Vestax Spin.midi.xml"
  Delete "$INSTDIR\controllers\Vestax-Spin-scripts.js"
  Delete "$INSTDIR\controllers\Vestax Typhoon.midi.xml"
  Delete "$INSTDIR\controllers\Vestax-Typhoon-scripts.js"
  Delete "$INSTDIR\controllers\Vestax VCI-100.midi.xml"
  Delete "$INSTDIR\controllers\Vestax-VCI-100-scripts.js"
  Delete "$INSTDIR\controllers\Vestax VCI-400.midi.xml"
  Delete "$INSTDIR\controllers\Vestax-VCI-400-scripts.js"
  Delete "$INSTDIR\controllers\Wireless DJ App.midi.xml"
  Delete "$INSTDIR\controllers\Wireless-DJ-scripts.js"
  ;Delete $INSTDIR\controllers\*.* ; Avoid this since it will delete customized files too
  RMDir "$INSTDIR\controllers"

  ; Remove promos
  Delete $INSTDIR\promo\${PRODUCT_VERSION}\*.*
  Delete $INSTDIR\promo\*.*
  RMDir /r "$INSTDIR\promo\${PRODUCT_VERSION}"
  RMDir "$INSTDIR\promo"

  ; Remove skins we (might have) installed
  Delete $INSTDIR\skins\*.* ; This just deletes files at the root of the skins directory
  RMDir /r "$INSTDIR\skins\Deere"
  RMDir /r "$INSTDIR\skins\LateNight"
  RMDir /r "$INSTDIR\skins\Shade"
  ; The lack of the /r prevents deleting any sub-directories we didn't explicitly delete above
  RMDir "$INSTDIR\skins"

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
