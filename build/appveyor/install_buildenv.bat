REM @ECHO OFF

SET BUILDENVBASEDIR=C:\MIXXX-BUILDSERVER

REM Build envs to install. You can specify more than one separated by spaces (no quotes)
SET BUILDENVS=2.0-%1-%2-minimal

REM precompiled compressed build env base URL
SET BASEURL=https://downloads.mixxx.org/builds/appveyor/environments/2.0

REM ---------------------------

REM main()

REM uncomment the following line if you want to empty cache and
REM force buildenv (and build script) download
REM RMDIR /S /Q %BUILDENVBASEDIR%

REM Install build env base dir
IF NOT EXIST %BUILDENVBASEDIR% MKDIR %BUILDENVBASEDIR%

REM Install build envs
FOR %%G IN (%BUILDENVS%) DO CALL :installbuildenv %%G

REM End of main()
GOTO:EOF

REM ---------------------------

REM FUNCTION installbuildenv(build_env_name)
:installbuildenv
SETLOCAL
IF EXIST "%BUILDENVBASEDIR%\%1" GOTO BUILDENVEXISTS
ECHO Installing build env %1
CD %BUILDENVBASEDIR%
echo ..Downloading %1.zip
curl -fsS -L -o %1.zip %BASEURL%/%1.zip
echo ..unzipping %1.zip
7z x %1.zip
DEL %1.zip
GOTO ENDFUNC
:BUILDENVEXISTS
ECHO "Build env %1 seems to already exist. Leaving without doing anything"
:ENDFUNC
ENDLOCAL
GOTO:EOF
