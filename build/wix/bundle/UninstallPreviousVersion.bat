@echo off

IF "%1"=="payload" GOTO Payload

REM re-launch in 64-bits mode if launched in 32-bits mode under a 64-bits OS
REM Ugly hack to escape 32-bits jail and access 64-bits %ProgramFiles%
reg Query "HKLM\Hardware\Description\System\CentralProcessor\0" | find /i "x86" > NUL && set OS=32BIT || set OS=64BIT
if %OS%==64BIT (
  REM we run on a 64-bits OS
  IF "%ProgramFiles(x86)%"=="%ProgramFiles%" (
    REM We run in 32-bits mode
    %SYSTEMROOT%\sysnative\cmd.exe /c "%0" payload
    exit /B 0
  )
)

:Payload
REM create messagebox helper vbs
:uniqGet
set "msgbx=%tmp%\messagebox~%RANDOM%.vbs"
if exist "%msgbx%" goto :uniqGet
REM echo === %msgbx% ===

echo(  > %msgbx%
echo Set objArgs = WScript.Arguments >> %msgbx%
echo messageText = objArgs(0) >> %msgbx%
echo MsgBox messageText, vbOKOnly+vbExclamation, "Previous version uninstallation" >> %msgbx%

set "tmpuninst=%tmp%\nsu~%RANDOM%.exe"
if exist "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe" (
  cscript /NoLogo %msgbx% "An old version has been detected in %PROGRAMFILES%. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  COPY "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe" "%tmpuninst%"
  "%tmpuninst%" /S _?=%PROGRAMFILES%\Mixxx\
  IF ERRORLEVEL 1 (
    cscript /NoLogo %msgbx% "There was a problem uninstalling previous version. Unable to continue Mixxx installation."
    CALL :cleanup
    EXIT /B 1
  )
)

if exist "%PROGRAMFILES(x86)%\Mixxx\UninstallMixxx.exe" (
  cscript /NoLogo %msgbx% "An old version has been detected in %PROGRAMFILES(x86)%. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  COPY "%PROGRAMFILES%(x86)\Mixxx\UninstallMixxx.exe" "%tmpuninst%"
  "%tmpuninst%" /S _?=%PROGRAMFILES(x86)%\Mixxx\
  IF ERRORLEVEL 1 (
    cscript /NoLogo %msgbx% "There was a problem uninstalling previous version. Unable to continue Mixxx installation."
    CALL :cleanup
    EXIT /B 1
  )
)

REM === Some Cleanup ===
:cleanup
DEL %msgbx%
EXIT /B 0
