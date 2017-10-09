REM @echo off

set "LOG=%tmp%\mixxx-uninstallprevious~%RANDOM%.log"
set "SCRIPTPATH=%~dp0"
set "SCRIPTNAME=%~nx0"
echo "Starting" >>%LOG%
set "INSTALLDIR="
set "check64=yes"

REM Arguments parsing
:parse
IF "%~1"=="" GOTO endparse
IF "%~1"=="nocheck" (
    set "check64=no"
    SHIFT
    GOTO parse
)

set "INSTALLDIR=%1"
SHIFT
:loop
if "%1"=="" goto after_loop
set "INSTALLDIR=%INSTALLDIR% %1"
SHIFT
GOTO loop
:after_loop
GOTO endparse
:endparse

IF NOT defined INSTALLDIR (
    echo "No param" >>%LOG%
    echo Usage: %SCRIPTNAME% [nocheck] INSTALLDIR
    exit /B 1
) ELSE (
    echo "INSTALLDIR set to %INSTALLDIR%" >>%LOG%
)

if "%check64%"=="no"  GOTO nocheck

echo "Checking if we run in 64-bits mode" >>%LOG%
REM re-launch in 64-bits mode if launched in 32-bits mode under a 64-bits OS
REM Ugly hack to escape 32-bits jail and access 64-bits %ProgramFiles%
reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set "OSB=32BIT" || set "OSB=64BIT"
echo "%OSB% OS detected" >>%LOG%

if "%OSB%"=="32BIT" GOTO bit32
REM we run on a 64-bits OS
echo "Doing 64-bits stuff" >>%LOG%
echo "%PROGRAMFILES(x86)%"

IF "%PROGRAMFILES(x86)%"=="%PROGRAMFILES%" (
  REM We run in 32-bits mode. Get out of this 32-bit jail...
  echo "Relaunching in 64-bits mode" >>%LOG%
  %SYSTEMROOT%\sysnative\cmd.exe /c ""%SCRIPTPATH%%SCRIPTNAME%" nocheck %INSTALLDIR%"
  exit /B 0
) ELSE (
    echo "Yes, We run in 64-bits mode" >>%LOG%
)
goto bit64

:bit32
echo "Doing 32-bits stuff" >>%LOG%

:bit64

:nocheck

REM create messagebox helper vbs
:uniqGet
set "msgbx=%tmp%\messagebox~%RANDOM%.vbs"
if exist "%msgbx%" goto :uniqGet
REM echo === %msgbx% ===

echo(  > %msgbx%
echo Set objArgs = WScript.Arguments >> %msgbx%
echo messageText = objArgs(0) >> %msgbx%
echo MsgBox messageText, vbOKOnly+vbExclamation, "Previous version uninstallation" >> %msgbx%

echo "vbs message box helper in %msgbx%" >>%LOG%

set "tmpuninst=%tmp%\nsu~%RANDOM%.exe"

echo "Checking INSTALLDIR %INSTALLDIR%\UninstallMixxx.exe" >>%LOG%
REM Check for previous version in INSTALLDIR
if exist "%INSTALLDIR%\UninstallMixxx.exe" (
  echo "%INSTALLDIR%\UninstallMixxx.exe Found" >>%LOG%
  echo "...Warning User" >>%LOG%
  cscript /NoLogo %msgbx% "An old version has been detected in %INSTALLDIR%. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  echo "...Copying uninstaller to %tmpuninst%" >>%LOG%
  COPY "%INSTALLDIR%\UninstallMixxx.exe" "%tmpuninst%"
  echo "...Launching uninstaller" >>%LOG%
  ""%tmpuninst%" /S _?=%INSTALLDIR%"
  IF ERRORLEVEL 1 (
    echo "===There was a failure during uninstallation" >>%LOG%
    cscript /NoLogo %msgbx% "There was a problem uninstalling previous version. Unable to continue Mixxx installation."
    CALL :cleanup
    EXIT /B 1
  ) ELSE (
    echo "===no error during uninstallation" >>%LOG%
  )
) ELSE (
    echo "%INSTALLDIR%\UninstallMixxx.exe not found" >>%LOG%
)

echo "aaa"
echo "Checking PROGRAMFILES %PROGRAMFILES%\Mixxx\UninstallMixxx.exe" >>%LOG%
REM Check for previous version in 64-bits program files
if exist "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe" (
  echo "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe Found" >>%LOG%
  echo "...Warning User" >>%LOG%
  cscript /NoLogo %msgbx% "An old version has been detected in %PROGRAMFILES%\Mixxx. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  echo "...Copying uninstaller to %tmpuninst%" >>%LOG%
  COPY "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe" "%tmpuninst%"
  echo "...Launching uninstaller" >>%LOG%
  "%tmpuninst%" /S _?=%PROGRAMFILES%\Mixxx\
  IF ERRORLEVEL 1 (
    echo "===There was a failure during uninstallation" >>%LOG%
    cscript /NoLogo %msgbx% "There was a problem uninstalling previous version. Unable to continue Mixxx installation."
    CALL :cleanup
    EXIT /B 1
  ) ELSE (
    echo "===no error during uninstallation" >>%LOG%
  )
) ELSE (
    echo "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe not found" >>%LOG%
)

IF NOT defined PROGRAMFILES(x86) GOTO cleanup

echo "Checking PROGRAMFILES32 %PROGRAMFILES(x86)%\Mixxx\UninstallMixxx.exe" >>%LOG%
REM Check for previous version in 32-bits program files
if exist "%PROGRAMFILES(x86)%\Mixxx\UninstallMixxx.exe" (
  echo "%PROGRAMFILES(x86)%\Mixxx\UninstallMixxx.exe Found" >>%LOG%
  echo "...Warning User" >>%LOG%
  cscript /NoLogo %msgbx% "An old version has been detected in %PROGRAMFILES(x86)%\Mixxx. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  echo "...Copying uninstaller to %tmpuninst%" >>%LOG%
  COPY "%PROGRAMFILES%(x86)\Mixxx\UninstallMixxx.exe" "%tmpuninst%"
  echo "...Launching uninstaller" >>%LOG%
  REM Another Ugly hack, not working otherwise. don't ask me why...
  set "DIRTOREMOVE=%PROGRAMFILES(x86)%\Mixxx\"
  ""%tmpuninst%" /S _?=%DIRTOREMOVE%"
  IF ERRORLEVEL 1 (
    echo "===There was a failure during uninstallation" >>%LOG%
    cscript /NoLogo %msgbx% "There was a problem uninstalling previous version. Unable to continue Mixxx installation."
    CALL :cleanup
    EXIT /B 1
  ) ELSE (
      echo "===no error during uninstallation" >>%LOG%
  )
) ELSE (
    echo "%PROGRAMFILES(x86)%\Mixxx\UninstallMixxx.exe not found" >>%LOG%
)

REM === Some Cleanup ===
:cleanup
echo "Cleanup" >>%LOG%
DEL %msgbx%
IF EXIST %tmpuninst% DEL %tmpuninst%
EXIT /B 0
