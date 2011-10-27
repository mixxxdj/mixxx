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
!define PRODUCT_PUBLISHER "The Mixxx Team"
!define PRODUCT_WEB_SITE "http://www.mixxx.org"

!define DEFAULT_SKIN "Deere1280x800-WXGA"

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
    !define BITWIDTH "64"
!else
    Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
    !define BITWIDTH "32"
!endif

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

;--------------------------------
; The stuff to install

Section "Mixxx (required)" SecMixxx

  SectionIn RO

  ; Set output path to the installation directory.
  SetOutPath $INSTDIR

  ; Put binary files there
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\mixxx.exe"
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\*.dll"

  ; Put other files there
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\*.xml"

  ; NOTE: you need to check the mixxx.exe.manifest file in the win??_build directory
  ; and place the appropriate versions of the listed DLL files and their manifest files
  ; into the mixxx-win[64]lib-msvc directory for packaging before making the installer
  ; (Visual C++ 2005 is msvc?80.dll and Microsoft.VC80.CRT.manifest,
  ;  Visual C++ 2008 is msvc?90.dll and Microsoft.VC90.CRT.manifest)
  ;
  ; See http://mixxx.org/wiki/doku.php/build_windows_installer for full details.
  ;
  ; All the MSVC files are located here if you have MSVC 2008 installed. (x86)
  File "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\*"
  ;File "$%VCINSTALLDIR%\redist\x86\Microsoft.VC90.CRT\*"
  ;File "$%VS90COMNTOOLS%\..\..\VC\redist\x86\Microsoft.VC90.CRT\*"

  ; NOTE: The Microsoft Visual C++ 2010 Runtime gets rid of the manifest file, so it
  ;         is no longer necessary if we switch to deploying with MSVC 2010. - Albert

  ; If you have the msvc DLLs & manifest elsewhere,
  ; copy them to the WINLIB_PATH and uncomment these:
  ;File ..\${WINLIB_PATH}\msvcr*.dll        ; Required
  ;File ..\${WINLIB_PATH}\msvcp*.dll        ; Required
  ;File /nonfatal ..\${WINLIB_PATH}\msvcm*.dll    ; Not (currently) required, so nonfatal
  ;File ..\${WINLIB_PATH}\Microsoft.VC*.CRT.manifest    ; Required on MSVC < 2010, apparently

  ; And documentation, licence etc.
  File "${BASE_BUILD_DIR}\Mixxx-Manual.pdf"
  File "${BASE_BUILD_DIR}\LICENSE"
  File "${BASE_BUILD_DIR}\README"
  File "${BASE_BUILD_DIR}\COPYING"

  SetOutPath $INSTDIR\promo\${PRODUCT_VERSION}
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\promo\${PRODUCT_VERSION}\*"

  SetOutPath $INSTDIR\sqldrivers
  File /nonfatal /r "${BASE_BUILD_DIR}\dist${BITWIDTH}\sqldrivers\*"

  SetOutPath $INSTDIR\keyboard
  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\keyboard\Standard.kbd.cfg"

  ; MIDI mapping tools (mappings are below) & common script file
  SetOutPath $INSTDIR\midi
  File /r /x ".svn" /x ".bzr" /x "*.midi.xml" /x "*.js" ${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\*.*
  File ${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\midi-mappings-scripts.js

  ; Common skin files
  SetOutPath "$INSTDIR\skins"
  File /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*.*

  ; Just the default skin
  SetOutPath "$INSTDIR\skins\${DEFAULT_SKIN}"
  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\${DEFAULT_SKIN}\*.*

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

SectionGroup "MIDI controller mappings" SecControllerMappings

  SectionGroup "Certified mappings" SecCertifiedMappings

	Section "Hercules DJ Console Mk2"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Hercules DJ Console Mk2.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Hercules-DJ-Console-Mk2-scripts.js"
	SectionEnd

	Section "Hercules DJ Console RMX"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Hercules DJ Console RMX.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Hercules-DJ-Console-RMX-scripts.js"
	SectionEnd

	Section "Hercules DJ Control MP3 e2"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Hercules DJ Control MP3 e2.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Hercules DJ Control MP3 e2-scripts.js"
	SectionEnd

	Section "Stanton SCS.3d"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton SCS.3d.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton-SCS3d-scripts.js"
	SectionEnd

	Section "Stanton SCS.3m"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton SCS.3m.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton-SCS3m-scripts.js"
	SectionEnd

    Section "Stanton SCS.1d" SecSCS1d
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton SCS.1d.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton-SCS1d-scripts.js"
	SectionEnd

	Section "Stanton SCS.1m" SecSCS1m
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton SCS.1m.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\Stanton-SCS1m-scripts.js"
	SectionEnd

	Section "DJ TechTools MIDIFighter"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\DJTechTools MIDI Fighter.midi.xml"
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\DJTechTools-MIDIFighter-scripts.js"
	SectionEnd

	Section "M-Audio X-Session Pro"
	  SetOutPath $INSTDIR\midi
	  File "${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\M-Audio_Xsession_pro.midi.xml"
	SectionEnd

  SectionGroupEnd

  Section "Community-supported mappings" SecCommunityMappings
    SetOutPath $INSTDIR\midi
	File ${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\*.midi.xml
	File ${BASE_BUILD_DIR}\dist${BITWIDTH}\midi\*.js
  SectionEnd

SectionGroupEnd

SectionGroup "Additional Skins" SecAddlSkins

	Section "Minimalist skins" SecBasicSkins

	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\Outline*

	  SetOutPath "$INSTDIR\skins\Outline800x480-WVGA"
	  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\Outline800x480-WVGA\*.*
	  SetOutPath "$INSTDIR\skins\Outline1024x768-XGA"
	  File /r /x ".svn" /x ".bzr" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\Outline1024x768-XGA\*.*

	SectionEnd

	Section "Netbook-size (1024x600)" SecNetbookSkins
	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" /x "Outline*" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*-Netbook*
	SectionEnd

	Section "XGA-size (1024x768)" SecXGASkins
	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" /x "Outline*" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*-XGA*
	SectionEnd

	Section "WXGA-size (1280x800)" SecWXGASkins
	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" /x "Outline*" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*-WXGA*
	SectionEnd

	Section "SXGA-size (1280x1024)" SecSXGASkins
	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" /x "Outline*" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*-SXGA*
	SectionEnd

	Section "UXGA-size (1600x1200)" SecUXGASkins
	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" /x "Outline*" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*-UXGA*
	SectionEnd

	Section "WSXGA-size (1680x1050)" SecWSXGASkins
	  SetOutPath "$INSTDIR\skins"
	  File /r /x ".svn" /x ".bzr" /x "Outline*" ${BASE_BUILD_DIR}\dist${BITWIDTH}\skins\*-WSXGA*
	SectionEnd

SectionGroupEnd

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

  ; Language strings
  LangString DESC_SecMixxx ${LANG_ENGLISH} "Mixxx itself with the default wide-format netbook-sized skin (1024x600) using the Outline theme"
  LangString DESC_SecStartMenu ${LANG_ENGLISH} "Mixxx program group containing useful shortcuts appearing under the [All] Programs section under the Start menu"
  LangString DESC_SecDesktop ${LANG_ENGLISH} "Shortcut to Mixxx placed on the Desktop"

  ; Controller mapping descriptions
  LangString DESC_SecControllerMappings ${LANG_ENGLISH} "Mappings that enable popular MIDI controllers to be used with Mixxx"
  LangString DESC_SecCertifiedMappings ${LANG_ENGLISH} "Mappings developed and supported by the Mixxx team."
  LangString DESC_SecCommunityMappings ${LANG_ENGLISH} "User-developed mappings that the Mixxx team is unable to directly support. Please visit the Mixxx forum for help with these: http://mixxx.org/forums/"
  LangString DESC_SecSCS1d ${LANG_ENGLISH} "Mapping for the Stanton SCS.1d. DaRouter must be closed to use it."
  LangString DESC_SecSCS1m ${LANG_ENGLISH} "Mapping for the Stanton SCS.1m. DaRouter must be closed to use it."

  ; Skin group descriptions
  LangString DESC_SecBasicSkins ${LANG_ENGLISH} "Additional skins using the Outline theme (featuring a clear, clean and simple layout,) including one for 800 pixel-wide screens"
  LangString DESC_SecAddlSkins ${LANG_ENGLISH} "Additional good-looking skins with varying themes and larger screen sizes."
  LangString DESC_SecNetbookSkins ${LANG_ENGLISH} "Includes Shade and Shade Dark"
  LangString DESC_SecXGASkins ${LANG_ENGLISH} "Includes Shade and Shade Dark"
  LangString DESC_SecWXGASkins ${LANG_ENGLISH} "Includes Deere, Late Night (Blues)"
  LangString DESC_SecSXGASkins ${LANG_ENGLISH} "Includes Deere, Late Night (Blues)"
  LangString DESC_SecUXGASkins ${LANG_ENGLISH} "Includes Phoney and Phoney Dark"
  LangString DESC_SecWSXGASkins ${LANG_ENGLISH} "Includes Phoney and Phoney Dark"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecMixxx} $(DESC_SecMixxx)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecStartMenu} $(DESC_SecStartMenu)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} $(DESC_SecDesktop)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecControllerMappings} $(DESC_SecControllerMappings)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCertifiedMappings} $(DESC_SecCertifiedMappings)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecCommunityMappings} $(DESC_SecCommunityMappings)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAddlSkins} $(DESC_SecAddlSkins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBasicSkins} $(DESC_SecBasicSkins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecNetbookSkins} $(DESC_SecNetbookSkins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecXGASkins} $(DESC_SecXGASkins)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecWXGASkins} $(DESC_SecWXGASkins)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecSXGASkins} $(DESC_SecSXGASkins)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecUXGASkins} $(DESC_SecUXGASkins)
	!insertmacro MUI_DESCRIPTION_TEXT ${SecWSXGASkins} $(DESC_SecWSXGASkins)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSCS1d} $(DESC_SecSCS1d)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSCS1m} $(DESC_SecSCS1m)
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
  Delete $INSTDIR\sqldrivers\*.dll

  ; Remove skins, keyboard, midi mappings, promos
  Delete "$INSTDIR\skins\${DEFAULT_SKIN}\*.*"
  Delete "$INSTDIR\skins\Deere1280x1024-SXGA\*.*"
  Delete "$INSTDIR\skins\Deere1280x800-WXGA\*.*"
  Delete "$INSTDIR\skins\Deere1440x900-WXGA+\*.*"
  Delete "$INSTDIR\skins\Deere1920x1080-FullHD\*.*"
  Delete "$INSTDIR\skins\Deere1920x1200-WUXGA\*.*"
  Delete "$INSTDIR\skins\LateNight1280x1024-SXGA\*.*"
  Delete "$INSTDIR\skins\LateNight1280x800-WXGA\*.*"
  Delete "$INSTDIR\skins\LateNight1366x768-WXGA\*.*"
  Delete "$INSTDIR\skins\LateNightBlues1280x1024-SXGA\*.*"
  Delete "$INSTDIR\skins\LateNightBlues1280x800-WXGA\*.*"
  Delete "$INSTDIR\skins\LateNightBlues1366x768-WXGA\*.*"
  Delete "$INSTDIR\skins\Outline1024x600-Netbook\*.*"
  Delete "$INSTDIR\skins\Outline800x480-WVGA\*.*"
  Delete "$INSTDIR\skins\Outline1024x768-XGA\*.*"
  Delete "$INSTDIR\skins\Phoney1600x1200-UXGA\*.*"
  Delete "$INSTDIR\skins\Phoney1680x1050-WSXGA\*.*"
  Delete "$INSTDIR\skins\PhoneyDark1600x1200-UXGA\*.*"
  Delete "$INSTDIR\skins\PhoneyDark1680x1050-WSXGA\*.*"
  Delete "$INSTDIR\skins\Shade1024x600-Netbook\*.*"
  Delete "$INSTDIR\skins\Shade1024x768-XGA\*.*"
  Delete "$INSTDIR\skins\ShadeDark1024x600-Netbook\*.*"
  Delete "$INSTDIR\skins\ShadeDark1024x768-XGA\*.*"
  Delete $INSTDIR\skins\*.*

  Delete $INSTDIR\keyboard\*.*
  Delete $INSTDIR\midi\*.*
  Delete $INSTDIR\promo\${PRODUCT_VERSION}\*.*
  Delete $INSTDIR\promo\*.*

  RMDir "$INSTDIR\skins\${DEFAULT_SKIN}"
  RMDir "$INSTDIR\skins\Deere1280x1024-SXGA"
  RMDir "$INSTDIR\skins\Deere1280x800-WXGA"
  RMDir "$INSTDIR\skins\Deere1440x900-WXGA+"
  RMDir "$INSTDIR\skins\Deere1920x1080-FullHD"
  RMDir "$INSTDIR\skins\Deere1920x1200-WUXGA"
  RMDir "$INSTDIR\skins\LateNight1280x1024-SXGA"
  RMDir "$INSTDIR\skins\LateNight1280x800-WXGA"
  RMDir "$INSTDIR\skins\LateNight1366x768-WXGA"
  RMDir "$INSTDIR\skins\LateNightBlues1280x1024-SXGA"
  RMDir "$INSTDIR\skins\LateNightBlues1280x800-WXGA"
  RMDir "$INSTDIR\skins\LateNightBlues1366x768-WXGA"
  RMDir "$INSTDIR\skins\Outline1024x600-Netbook"
  RMDir "$INSTDIR\skins\Outline800x480-WVGA"
  RMDir "$INSTDIR\skins\Outline1024x768-XGA"
  RMDir "$INSTDIR\skins\Phoney1600x1200-UXGA"
  RMDir "$INSTDIR\skins\Phoney1680x1050-WSXGA"
  RMDir "$INSTDIR\skins\PhoneyDark1600x1200-UXGA"
  RMDir "$INSTDIR\skins\PhoneyDark1680x1050-WSXGA"
  RMDir "$INSTDIR\skins\Shade1024x600-Netbook"
  RMDir "$INSTDIR\skins\Shade1024x768-XGA"
  RMDir "$INSTDIR\skins\ShadeDark1024x600-Netbook"
  RMDir "$INSTDIR\skins\ShadeDark1024x768-XGA"
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
