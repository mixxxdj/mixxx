@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

CALL :REALPATH %~dp0\..
SET MIXXX_ROOT=%RETVAL%

IF "%PLATFORM%"=="" (
    SET PLATFORM=x64
)

IF "%CONFIGURATION%"=="" (
    SET CONFIGURATION=release-fastbuild
)

IF "%BUILDENV_BASEPATH%"=="" (
    SET BUILDENV_BASEPATH="%MIXXX_ROOT%\buildenv"
)

IF "%BUILDENV_BASEPATH%"=="" (
    SET BUILDENV_BASEPATH="%MIXXX_ROOT%\buildenv"
)

CALL :COMMAND_%1
EXIT /B 0

:COMMAND_name
    CALL :READ_ENVNAME
    ECHO "%RETVAL%"
    IF "%CI%" == "true" (
        ECHO BUILDENV_NAME=!RETVAL! >> %GITHUB_ENV%
    )
    GOTO :EOF

:COMMAND_setup
    CALL :READ_ENVNAME
    SET BUILDENV_NAME=%RETVAL%
    SET BUILDENV_PATH=%BUILDENV_BASEPATH%\%BUILDENV_NAME%
    MD %BUILDENV_BASEPATH%

    IF NOT EXIST %BUILDENV_PATH% (
        SET BUILDENV_URL=https://downloads.mixxx.org/builds/buildserver/2.3.x-windows/%BUILDENV_NAME%.zip
        ECHO Downloading !BUILDENV_URL!
        BITSADMIN /transfer buildenvjob /download /priority normal !BUILDENV_URL! !BUILDENV_PATH!.zip
        REM TODO: verify download using sha256sum?
        ECHO Unpacking %BUILDENV_PATH%.zip
        CALL :UNZIP "%BUILDENV_PATH%.zip" "%BUILDENV_BASEPATH%"
        ECHO Unpacking complete.
        DEL /f /q %BUILDENV_PATH%.zip
    )

    ECHO Using build environment: %BUILDENV_PATH%
    ENDLOCAL

    SET PATH=!BUILDENV_PATH!\bin;!PATH!
    # Remove C:\Program Files\Git\usr\bin from the PATH
    # This works around an issue on the windows-2016 builder provided by GitHub
    # Actions, see this for details:
    # - https://github.com/actions/cache/issues/333
    # - https://github.com/actions/virtual-environments/issues/480
    SET PATH=%PATH:C:\Program Files\Git\usr\bin;=%

    FOR /D %%G IN (%BUILDENV_PATH%\Qt-*) DO (SET Qt5_DIR=%%G)
    SET CMAKE_PREFIX_PATH=!BUILDENV_PATH!;!Qt5_DIR!

    ECHO ^Environent Variables:
    ECHO ^- PATH=!PATH!
    ECHO ^CMake Configuration:
    ECHO ^- CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!

    IF NOT "%GITHUB_ENV%" == "" (
        ECHO CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!>>%GITHUB_ENV%
        ECHO PATH=!PATH!>>%GITHUB_ENV%
    )
    GOTO :EOF


:UNZIP <newzipfile> <ExtractTo>
  SET vbs="%temp%\_.vbs"
  IF EXIST %vbs% del /f /q %vbs%
  >%vbs%  echo Set fso = CreateObject("Scripting.FileSystemObject")
  >>%vbs% echo If NOT fso.FolderExists(%2) Then
  >>%vbs% echo fso.CreateFolder(%2)
  >>%vbs% echo End If
  >>%vbs% echo Set objShell = CreateObject("Shell.Application")
  >>%vbs% echo Set FilesInZip=objShell.NameSpace(%1).items
  >>%vbs% echo objShell.NameSpace(%2).CopyHere(FilesInZip)
  >>%vbs% echo Set fso = Nothing
  >>%vbs% echo Set objShell = Nothing
  cscript //nologo %vbs%
  IF EXIST %vbs% DEL /f /q %vbs%
  GOTO :EOF


:REALPATH
    SET RETVAL=%~f1
    GOTO :EOF


:READ_ENVNAME
    SET /P BUILDENV_NAME=<%MIXXX_ROOT%\cmake\windows_build_environment_name
    SET BUILDENV_NAME=!BUILDENV_NAME:PLATFORM=%PLATFORM%!
    SET BUILDENV_NAME=!BUILDENV_NAME:CONFIGURATION=%CONFIGURATION%!
    SET RETVAL=%BUILDENV_NAME%
    GOTO :EOF
