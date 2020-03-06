@ECHO OFF
SETLOCAL enableDelayedExpansion

REM Base URL to download winlibs from.
SET BASEURL=%1

REM The winlib name to install.
SET WINLIB_NAME=%2

REM The local directory to cache build environments in.
SET WINLIBS_PATH=%3

IF EXIST "%WINLIBS_PATH%\%WINLIB_NAME%" (
  echo %WINLIB_NAME% already exists at %WINLIBS_PATH%
  REM TODO(XXX) check fingerprint on build environment?
  ENDLOCAL && EXIT /B 0
)

REM Install build env base dir
IF NOT EXIST "%WINLIBS_PATH%" MKDIR "%WINLIBS_PATH%"

where /q 7za
IF ERRORLEVEL 1 (
  where /q 7z
  IF ERRORLEVEL 1 (
    REM In case it is not present in PATH
    IF EXIST "%PROGRAMFILES%\7-zip\7z.exe" (
      set "ZIP=%PROGRAMFILES%\7-zip\7z.exe"
    ) else (
      call :get_powershell_version
      IF !POWERSHELL! LSS 5 (
        echo Could not find 7z or 7za on the path, and cannot use powershell ^(version !POWERSHELL!^)
        echo Either install http://7-zip.org ^(and add it to the PATH^) and retry
        echo or download %BASEURL%/%WINLIB_NAME%.zip
        echo yourself and unzip it at %WINLIBS_PATH%
        ENDLOCAL && EXIT /b 1
      )
    )
  ) else (
    set ZIP=7z
  )
) else (
  set ZIP=7za
)

echo Installing winlib %WINLIB_NAME%.
CD "%WINLIBS_PATH%"
echo ..Downloading %WINLIB_NAME%.zip
where /q curl
IF ERRORLEVEL 1 (
  call :get_powershell_version
  IF !POWERSHELL! LSS 3 (
    echo curl not found and cannot use powershell ^(version !POWERSHELL!^)
    echo Please download %WINLIB_NAME%.zip 
    echo from %BASEURL%/ 
    echo and unzip it yourself at %WINLIBS_PATH%
    ENDLOCAL && EXIT /b 1
  ) else (
    powershell -Command "& {Invoke-WebRequest -Uri %BASEURL%/%WINLIB_NAME%.zip -OutFile \"%WINLIBS_PATH%/%WINLIB_NAME%.zip\"}"
    IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
  )
) else (
  curl -fsS -L -o %WINLIB_NAME%.zip %BASEURL%/%WINLIB_NAME%.zip
  IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
)
REM TODO(XXX) check fingerprint on zip file.

echo ..unzipping %WINLIB_NAME%.zip
IF "!ZIP!" NEQ "" (
  "!ZIP!" x %WINLIB_NAME%.zip
  IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
) ELSE (
  powershell -Command "& {Expand-Archive \"%WINLIBS_PATH%/%WINLIB_NAME%.zip\" \"%WINLIBS_PATH%\\\"}"
  IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
)
DEL %WINLIB_NAME%.zip

ENDLOCAL
EXIT /B 0

:get_powershell_version
  where /q powershell
  if ERRORLEVEL 1 (
    set POWERSHELL=0
  ) else (
    REM DETECT powershell major version
    FOR /F "tokens=* USEBACKQ" %%F IN (`powershell -Command "& {$PSVersionTable.PSVersion.Major}"`) DO ( set POWERSHELL=%%F )
  )
EXIT /B 1
