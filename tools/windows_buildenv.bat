@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

CALL :REALPATH %~dp0\..
SET MIXXX_ROOT=%RETVAL%

IF NOT DEFINED PLATFORM (
    SET PLATFORM=x64
)

IF NOT DEFINED CONFIGURATION (
    SET CONFIGURATION=release-fastbuild
)

IF NOT DEFINED BUILDENV_BASEPATH (
    SET BUILDENV_BASEPATH=%MIXXX_ROOT%\buildenv
)

CALL :READ_ENVNAME
SET BUILDENV_NAME=%RETVAL%
SET BUILDENV_PATH=%BUILDENV_BASEPATH%\%BUILDENV_NAME%

CALL :COMMAND_%1


IF NOT DEFINED GITHUB_ENV (
    CALL :GENERATE_CMakeSettings_JSON
)

EXIT /B 0

:COMMAND_name
    ECHO "!BUILDENV_NAME!"
    IF DEFINED GITHUB_ENV (
        ECHO BUILDENV_NAME=!BUILDENV_PATH! >> !GITHUB_ENV!
    )
    GOTO :EOF

:COMMAND_setup
    IF NOT EXIST %BUILDENV_BASEPATH% (
        MD %BUILDENV_BASEPATH%
    )

    IF NOT EXIST %BUILDENV_PATH% (
        SET BUILDENV_URL=https://downloads.mixxx.org/builds/buildserver/2.3.x-windows/%BUILDENV_NAME%.zip
        IF NOT EXIST !BUILDENV_PATH!.zip (
            ECHO Downloading !BUILDENV_URL!
            BITSADMIN /transfer buildenvjob /download /priority normal !BUILDENV_URL! !BUILDENV_PATH!.zip
            REM TODO: verify download using sha256sum?
        )
        ECHO Unpacking !BUILDENV_PATH!.zip
        CALL :UNZIP "!BUILDENV_PATH!.zip" "!BUILDENV_BASEPATH!"
        ECHO Unpacking complete.
        DEL /f /q %BUILDENV_PATH%.zip
    )

    ECHO Using build environment: !BUILDENV_PATH!
    ENDLOCAL

    SET PATH=!BUILDENV_PATH!\bin;!PATH!

    FOR /D %%G IN (%BUILDENV_PATH%\Qt-*) DO (SET Qt5_DIR=%%G)
    SET CMAKE_PREFIX_PATH=!BUILDENV_PATH!;!Qt5_DIR!

    ECHO ^Environent Variables:
    ECHO ^- PATH=!PATH!
    ECHO ^CMake Configuration:
    ECHO ^- CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!

    IF DEFINED GITHUB_ENV (
        ECHO CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!>>!GITHUB_ENV!
        ECHO PATH=!PATH!>>!GITHUB_ENV!
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
    SET /P BUILDENV_NAME=<%MIXXX_ROOT%\packaging\windows\build_environment
    SET BUILDENV_NAME=!BUILDENV_NAME:PLATFORM=%PLATFORM%!
    SET BUILDENV_NAME=!BUILDENV_NAME:CONFIGURATION=%CONFIGURATION%!
    SET RETVAL=%BUILDENV_NAME%
    GOTO :EOF

:GENERATE_CMakeSettings_JSON
REM Generate CMakeSettings.json which is read by MS Visual Studio to determine the supported CMake build environments
    SET CMakeSettings="%MIXXX_ROOT%\CMakeSettings.json"
    IF EXIST %CMakeSettings% del /f /q %CMakeSettings%
    >>%CMakeSettings% echo {
    >>%CMakeSettings% echo   "configurations": [
    >>%CMakeSettings% echo     {
    >>%CMakeSettings% echo       "buildCommandArgs": "",
    >>%CMakeSettings% echo       "buildRoot": "${projectDir}\\cmake_build",
    >>%CMakeSettings% echo       "cmakeCommandArgs": "-DDEBUG_ASSERTIONS_FATAL=ON -DHSS1394=ON -DKEYFINDER=OFF -DLOCALECOMPARE=ON -DMAD=ON -DMEDIAFOUNDATION=ON -DSTATIC_DEPS=ON -DBATTERY=ON -DBROADCAST=ON -DBULK=ON -DHID=ON -DLILV=ON -DOPUS=ON -DQTKEYCHAIN=ON -DVINYLCONTROL=ON",
    >>%CMakeSettings% echo       "configurationType": "Release",
    >>%CMakeSettings% echo       "ctestCommandArgs": "",
    >>%CMakeSettings% echo       "enableClangTidyCodeAnalysis": true,
    >>%CMakeSettings% echo       "generator": "Ninja",
    >>%CMakeSettings% echo       "inheritEnvironments": [ "msvc_!PLATFORM!_!PLATFORM!" ],
    >>%CMakeSettings% echo       "installRoot": "${projectDir}\\cmake_dist",
    >>%CMakeSettings% echo       "intelliSenseMode": "windows-msvc-!PLATFORM!",
    >>%CMakeSettings% echo       "name": "!PLATFORM!-!CONFIGURATION!",
    >>%CMakeSettings% echo       "variables": [
    >>%CMakeSettings% echo         {
    >>%CMakeSettings% echo           "name": "CMAKE_PREFIX_PATH",
    >>%CMakeSettings% echo           "type": "STRING",
    REM Replaces all \ by \\ in CMAKE_PREFIX_PATH
    >>%CMakeSettings% echo           "value": "!CMAKE_PREFIX_PATH:\=\\!"
    >>%CMakeSettings% echo         },
    >>%CMakeSettings% echo         {
    >>%CMakeSettings% echo           "name": "CMAKE_INTERPROCEDURAL_OPTIMIZATION",
    >>%CMakeSettings% echo           "type": "BOOL",
    >>%CMakeSettings% echo           "value": "FALSE"
    >>%CMakeSettings% echo         },
    >>%CMakeSettings% echo         {
    >>%CMakeSettings% echo           "name": "CMAKE_EXPORT_COMPILE_COMMANDS",
    >>%CMakeSettings% echo           "type": "BOOL",
    >>%CMakeSettings% echo           "value": "TRUE"
    >>%CMakeSettings% echo         }
    >>%CMakeSettings% echo       ]
    >>%CMakeSettings% echo     }
    >>%CMakeSettings% echo   ]
    >>%CMakeSettings% echo }
    GOTO :EOF
