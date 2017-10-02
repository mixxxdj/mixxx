@echo off

REM create messagebox helper vbs

:uniqGet
set "msgbx=%tmp%\messagebox~%RANDOM%.vbs"
if exist "%msgbx%" goto :uniqGet
REM echo === %msgbx% ===
echo(  > %msgbx%
echo Set objArgs = WScript.Arguments >> %msgbx%
echo messageText = objArgs(0) >> %msgbx%
echo MsgBox messageText, vbOKOnly+vbExclamation, "Previous version uninstallation" >> %msgbx%


if exist "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe" (
  cscript /NoLogo %msgbx% "An old version has been detected in %PROGRAMFILES%. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  "%PROGRAMFILES%\Mixxx\UninstallMixxx.exe" /S
  IF ERRORLEVEL 1 (
    cscript /NoLogo %msgbx% "There was a problem uninstalling previous version. Unable to continue Mixxx installation."
    CALL :cleanup
    EXIT /B 1
  )
)

if exist "%PROGRAMFILES(x86)%\Mixxx\UninstallMixxx.exe" (
  cscript /NoLogo %msgbx% "An old version has been detected in %PROGRAMFILES(x86)%. You need to uninstall it first. We will launch the uninstaller and all you have to do is to click yes or next to uninstall it."
  "%PROGRAMFILES%(x86)\Mixxx\UninstallMixxx.exe" /S
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
