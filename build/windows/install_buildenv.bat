@ECHO OFF
SETLOCAL enableDelayedExpansion

REM Base URL to download winlibs from.
SET BASEURL=%1

REM The winlib name to install.
SET WINLIB_NAME=%2

REM The local directory to cache build environments in.
SET WINLIBS_PATH=%3

IF NOT EXIST "%WINLIBS_PATH%\%WINLIB_NAME%" (

  REM Install build env base dir
  IF NOT EXIST "%WINLIBS_PATH%" MKDIR "%WINLIBS_PATH%"

  where /q 7za
  IF ERRORLEVEL 1 (
    where /q 7z
    if ERRORLEVEL 1 (
      where /q powershell
      if ERRORLEVEL 1 (
         echo Could not find 7z or 7za on the path, and powershell is not available. 
         echo Either install http://7-zip.org and retry
         echo or download %BASEURL%/%WINLIB_NAME%.zip
         echo yourself and unzip it at %WINLIBS_PATH%
         ENDLOCAL && EXIT /b 1
      ) else (
        set POWERSHELL=powershell
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
    echo curl not found. Please download %WINLIB_NAME%.zip from %BASEURL%/ and unzip it yourself at %WINLIBS_PATH%
    ENDLOCAL && EXIT /b 1
  )
  curl -fsS -L -o %WINLIB_NAME%.zip %BASEURL%/%WINLIB_NAME%.zip
  IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
  REM TODO(XXX) check fingerprint on zip file.
  echo ..unzipping %WINLIB_NAME%.zip
  IF "!POWERSHELL!" == "powershell" (
    powershell -Command "& {Expand-Archive \"%WINLIBS_PATH%/%WINLIB_NAME%.zip\" \"%WINLIBS_PATH%\\\"}"
    IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
  ) ELSE (
    !ZIP! x %WINLIB_NAME%.zip
    IF ERRORLEVEL 1 ENDLOCAL && EXIT /b 1
  )
  DEL %WINLIB_NAME%.zip
) else (
  echo %WINLIB_NAME% already exists at %WINLIBS_PATH%
  REM TODO(XXX) check fingerprint on build environment?
)

ENDLOCAL
