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
    SET configElementTermination=,
    CALL :Configuration2CMakeSettings_JSON off       Release
    CALL :Configuration2CMakeSettings_JSON legacy    Release
    CALL :Configuration2CMakeSettings_JSON portable  Release
    CALL :Configuration2CMakeSettings_JSON fastbuild RelWithDebInfo
    SET configElementTermination=
    CALL :Configuration2CMakeSettings_JSON native    Release
    >>%CMakeSettings% echo   ]
    >>%CMakeSettings% echo }
    GOTO :EOF

:Configuration2CMakeSettings_JSON <optimize> <configurationtype>
    >>%CMakeSettings% echo     {
    >>%CMakeSettings% echo       "buildRoot": "${projectDir}\\build_!PLATFORM!__%1",
    >>%CMakeSettings% echo       "configurationType": "%2",
    >>%CMakeSettings% echo       "enableClangTidyCodeAnalysis": true,
    >>%CMakeSettings% echo       "generator": "Ninja",
    >>%CMakeSettings% echo       "inheritEnvironments": [ "msvc_!PLATFORM!_!PLATFORM!" ],
    >>%CMakeSettings% echo       "installRoot": "${projectDir}\\dist_!PLATFORM!__%1",
    >>%CMakeSettings% echo       "intelliSenseMode": "windows-msvc-!PLATFORM!",
    >>%CMakeSettings% echo       "name": "!PLATFORM!__%1",
    >>%CMakeSettings% echo       "variables": [
    SET variableElementTermination=,
    CALL :AddCMakeVar2CMakeSettings_JSON "BATTERY"                            "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "BROADCAST"                          "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "BULK"                               "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_EXPORT_COMPILE_COMMANDS"      "BOOL"   "true"
    REM Replace all \ by \\ in CMAKE_PREFIX_PATH
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_PREFIX_PATH"                  "PATH"   "!CMAKE_PREFIX_PATH:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "DEBUG_ASSERTIONS_FATAL"             "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "HID"                                "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "HSS1394"                            "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "KEYFINDER"                          "BOOL"   "false"
    CALL :AddCMakeVar2CMakeSettings_JSON "LOCALECOMPARE"                      "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "LILV"                               "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "MAD"                                "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "MEDIAFOUNDATION"                    "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "OPUS"                               "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "OPTIMIZE"                           "STRING" "%1"
    CALL :AddCMakeVar2CMakeSettings_JSON "QTKEYCHAIN"                         "BOOL"   "true"
    CALL :AddCMakeVar2CMakeSettings_JSON "STATIC_DEPS"                        "BOOL"   "true"
    SET variableElementTermination=
    CALL :AddCMakeVar2CMakeSettings_JSON "VINYLCONTROL"                       "BOOL"   "true"
    >>%CMakeSettings% echo       ]
    >>%CMakeSettings% echo     }!configElementTermination!
  GOTO :EOF

:AddCMakeVar2CMakeSettings_JSON <varname> <vartype> <value>
    >>%CMakeSettings% echo         {
    >>%CMakeSettings% echo           "name": %1,
    >>%CMakeSettings% echo           "type": %2,
    >>%CMakeSettings% echo           "value": %3
    >>%CMakeSettings% echo         }!variableElementTermination!
  GOTO :EOF
