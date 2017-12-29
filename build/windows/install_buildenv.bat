@ECHO OFF
SETLOCAL

REM Base URL to download winlibs from.
SET BASEURL=%1

REM The winlib name to install.
SET WINLIB_NAME=%2

REM The local directory to cache build environments in.
SET WINLIBS_PATH=%3

REM Install build env base dir
IF NOT EXIST "%WINLIBS_PATH%" MKDIR "%WINLIBS_PATH%"

IF NOT EXIST "%WINLIBS_PATH%\%WINLIB_NAME%" (
  echo Installing winlib %WINLIB_NAME%.
  CD "%WINLIBS_PATH%"
  echo ..Downloading %WINLIB_NAME%.zip
  curl -fsS -L -o %WINLIB_NAME%.zip %BASEURL%/%WINLIB_NAME%.zip
  REM TODO(XXX) check fingerprint on zip file.
  echo ..unzipping %WINLIB_NAME%.zip
  7z x %WINLIB_NAME%.zip
  DEL %WINLIB_NAME%.zip
) else (
  echo %WINLIB_NAME% already exists at %WINLIBS_PATH%
  REM TODO(XXX) check fingerprint on build environment?
)

ENDLOCAL
