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

IF NOT DEFINED BUILD_ROOT (
    SET BUILD_ROOT=%MIXXX_ROOT%\build
)

IF NOT DEFINED INSTALL_ROOT (
    SET INSTALL_ROOT=%MIXXX_ROOT%\install
)

IF "%~1"=="" (
    REM In case of manual start by double click no arguments are specified: Default to COMMAND_setup
    CALL :COMMAND_setup
    PAUSE
) ELSE (
    CALL :COMMAND_%1
)

EXIT /B 0

:COMMAND_name
    CALL :READ_ENVNAME
    IF DEFINED GITHUB_ENV (
        ECHO BUILDENV_NAME=!RETVAL! >> !GITHUB_ENV!
    )
    GOTO :EOF

:COMMAND_setup
    CALL :READ_ENVNAME
    SET BUILDENV_NAME=%RETVAL%
    SET BUILDENV_PATH=%BUILDENV_BASEPATH%\%BUILDENV_NAME%

    IF NOT DEFINED GITHUB_ENV (
        CALL :GENERATE_CMakeSettings_JSON

        IF NOT EXIST %BUILDENV_BASEPATH% (
            ECHO ### Create subdirectory buildenv ###
            MD %BUILDENV_BASEPATH%
        )

        IF NOT EXIST %BUILD_ROOT% (
            ECHO ### Create subdirectory build ###
            MD %BUILD_ROOT%
        )

        IF NOT EXIST %INSTALL_ROOT% (
            ECHO ### Create subdirectory install ###
            MD %INSTALL_ROOT%
        )
    )

    IF NOT EXIST %BUILDENV_PATH% (
        ECHO ### Download prebuild build environment ###
        SET BUILDENV_URL=https://downloads.mixxx.org/builds/buildserver/2.3.x-windows/!BUILDENV_NAME!.zip
        IF NOT EXIST !BUILDENV_PATH!.zip (
            ECHO ### Download prebuild build environment from !BUILDENV_URL! to !BUILDENV_PATH!.zip ###
            BITSADMIN /transfer buildenvjob /download /priority normal !BUILDENV_URL! !BUILDENV_PATH!.zip
            REM TODO: verify download using sha256sum?
        )
        ECHO ### Unpacking !BUILDENV_PATH!.zip ###
        CALL :UNZIP "!BUILDENV_PATH!.zip" "!BUILDENV_BASEPATH!"
        ECHO ### Unpacking complete. ###
        DEL /f /q %BUILDENV_PATH%.zip
    )

    ECHO ### Build environment path: !BUILDENV_PATH! ###
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
    ECHO ### Read name of prebuild environment from: %MIXXX_ROOT%\packaging\windows\build_environment ###
    SET /P BUILDENV_NAME=<%MIXXX_ROOT%\packaging\windows\build_environment
    SET BUILDENV_NAME=!BUILDENV_NAME:PLATFORM=%PLATFORM%!
    SET BUILDENV_NAME=!BUILDENV_NAME:CONFIGURATION=%CONFIGURATION%!
    SET RETVAL=%BUILDENV_NAME%
    ECHO "%RETVAL%"
    GOTO :EOF

:GENERATE_CMakeSettings_JSON
REM Generate CMakeSettings.json which is read by MS Visual Studio to determine the supported CMake build environments
    SET CMakeSettings=%MIXXX_ROOT%\CMakeSettings.json
    IF EXIST %CMakeSettings% (
        ECHO ### CMakeSettings.json exist: Rename old file to CMakeSettings__YYYY-MM-DD_HH-MM-SS.json ###
        FOR /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do set DateTime=%%a
        REN %CMakeSettings% CMakeSettings__!DateTime:~0,4!-!DateTime:~4,2!-!DateTime:~6,2!_!DateTime:~8,2!-!DateTime:~10,2!-!DateTime:~12,2!.json
    )
    ECHO ### Create new CMakeSettings.json ###
    >>%CMakeSettings% echo {
    >>%CMakeSettings% echo   "configurations": [
    SET configElementTermination=,
    CALL :Configuration2CMakeSettings_JSON off       Debug
    CALL :Configuration2CMakeSettings_JSON legacy    RelWithDebInfo
    CALL :Configuration2CMakeSettings_JSON portable  RelWithDebInfo
    CALL :Configuration2CMakeSettings_JSON fastbuild RelWithDebInfo
    SET configElementTermination=
    CALL :Configuration2CMakeSettings_JSON native    Release
    >>%CMakeSettings% echo   ]
    >>%CMakeSettings% echo }
    GOTO :EOF

:Configuration2CMakeSettings_JSON <optimize> <configurationtype>
    >>%CMakeSettings% echo     {
    >>%CMakeSettings% echo       "buildRoot": "${projectDir}\\build\\!PLATFORM!__%1",
    >>%CMakeSettings% echo       "configurationType": "%2",
    >>%CMakeSettings% echo       "enableClangTidyCodeAnalysis": true,
    >>%CMakeSettings% echo       "generator": "Ninja",
    >>%CMakeSettings% echo       "inheritEnvironments": [ "msvc_!PLATFORM!_!PLATFORM!" ],
    >>%CMakeSettings% echo       "installRoot": "${projectDir}\\install\\!PLATFORM!__%1",
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
