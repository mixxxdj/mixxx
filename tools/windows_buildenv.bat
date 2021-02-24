@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
REM this í is just to force some editors to recognize this file as ANSI, not UTF8.

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
    SET CMAKE_TOOLCHAIN_FILE=%BUILDENV_PATH%\scripts\buildsystems\vcpkg.cmake

    IF NOT EXIST %BUILDENV_BASEPATH% (
        ECHO ### Create subdirectory buildenv ###
        MD %BUILDENV_BASEPATH%
    )

    IF NOT EXIST %BUILDENV_PATH% (
        ECHO ### Download prebuild build environment ###
        SET BUILDENV_URL=https://downloads.mixxx.org/dependencies/2.3/Windows/!BUILDENV_NAME!.zip
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
    SET CMAKE_PREFIX_PATH=!BUILDENV_PATH!

    ECHO ^Environent Variables:
    ECHO ^- PATH=!PATH!
    ECHO ^CMake Configuration:
    ECHO ^- CMAKE_PREFIX_PATH=!CMAKE_PREFIX_PATH!

    IF DEFINED GITHUB_ENV (
        ECHO CMAKE_ARGS_EXTRA=-DX_VCPKG_APPLOCAL_DEPS_INSTALL=ON -DCMAKE_TOOLCHAIN_FILE=!CMAKE_TOOLCHAIN_FILE! -DVCPKG_TARGET_TRIPLET=x64-windows>>!GITHUB_ENV!
        ECHO PATH=!PATH!>>!GITHUB_ENV!
    ) else (
        CALL :GENERATE_CMakeSettings_JSON
        echo WARNING: CMakeSettings.json will include an invalid CMAKE_PREFIX_PATH
        echo          for settings other than %CONFIGURATION% .

        IF NOT EXIST %BUILD_ROOT% (
            ECHO ### Create subdirectory build ###
            MD %BUILD_ROOT%
        )

        IF NOT EXIST %INSTALL_ROOT% (
            ECHO ### Create subdirectory install ###
            MD %INSTALL_ROOT%
        )
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

    CALL :SETANSICONSOLE
    SET OLDCODEPAGE=%RETVAL%
    REM set byte order mark (BOM) for the file .
    REM WARNING: Ensure that the script is saved as ANSI, or these characters will not
    REM contain the correct values. Correct values are EF BB BF (&iuml; &raquo; &iquest;) .
    REM The last character is an actual character for the file, the start "{"
    >"%CMakeSettings%" echo ï»¿{
    CALL :SETUTF8CONSOLE

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
    CALL :RESTORECONSOLE %OLDCODEPAGE%
    GOTO :EOF

:Configuration2CMakeSettings_JSON <optimize> <configurationtype>
    >>%CMakeSettings% echo     {
    >>%CMakeSettings% echo       "name": "!PLATFORM!__%1",
    >>%CMakeSettings% echo       "buildRoot": "!BUILD_ROOT:\=\\!\\${name}",
    >>%CMakeSettings% echo       "configurationType": "%2",
    >>%CMakeSettings% echo       "enableClangTidyCodeAnalysis": true,
    >>%CMakeSettings% echo       "generator": "Ninja",
    >>%CMakeSettings% echo       "inheritEnvironments": [ "msvc_!PLATFORM!_!PLATFORM!" ],
    >>%CMakeSettings% echo       "installRoot": "!INSTALL_ROOT:\=\\!\\${name}",
    >>%CMakeSettings% echo       "cmakeToolchain": "!BUILDENV_PATH:\=\\!\\scripts\\buildsystems\\vcpkg.cmake",
    >>%CMakeSettings% echo       "intelliSenseMode": "windows-msvc-!PLATFORM!",
    >>%CMakeSettings% echo       "variables": [
    SET variableElementTermination=,
    CALL :AddCMakeVar2CMakeSettings_JSON "X_VCPKG_APPLOCAL_DEPS_INSTALL"      "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "VCPKG_TARGET_TRIPLET"               "STRING" "x64-windows"
    CALL :AddCMakeVar2CMakeSettings_JSON "BATTERY"                            "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "BROADCAST"                          "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "BULK"                               "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_EXPORT_COMPILE_COMMANDS"      "BOOL"   "True"
    REM Replace all \ by \\ in CMAKE_PREFIX_PATH
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_PREFIX_PATH"                  "STRING"   "!CMAKE_PREFIX_PATH:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "DEBUG_ASSERTIONS_FATAL"             "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "HID"                                "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "HSS1394"                            "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "KEYFINDER"                          "BOOL"   "False"
    CALL :AddCMakeVar2CMakeSettings_JSON "LOCALECOMPARE"                      "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "LILV"                               "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "MAD"                                "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "MEDIAFOUNDATION"                    "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "OPUS"                               "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "OPTIMIZE"                           "STRING" "%1"
    CALL :AddCMakeVar2CMakeSettings_JSON "QTKEYCHAIN"                         "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "STATIC_DEPS"                        "BOOL"   "False"
    SET variableElementTermination=
    CALL :AddCMakeVar2CMakeSettings_JSON "VINYLCONTROL"                       "BOOL"   "True"
    >>%CMakeSettings% echo       ]
    >>%CMakeSettings% echo     }!configElementTermination!
  GOTO :EOF

:AddCMakeVar2CMakeSettings_JSON <varname> <vartype> <value>
    >>%CMakeSettings% echo         {
    >>%CMakeSettings% echo           "name": %1,
    >>%CMakeSettings% echo           "value": %3,
    >>%CMakeSettings% echo           "type": %2
    >>%CMakeSettings% echo         }!variableElementTermination!
  GOTO :EOF

:SETANSICONSOLE
    for /f "tokens=2 delims=:" %%I in ('chcp') do set "_codepage=%%I"

    >NUL chcp 1252

    SET RETVAL=%_codepage%
  GOTO :EOF

:SETUTF8CONSOLE
    >NUL chcp 65001
  GOTO :EOF

:RESTORECONSOLE <codepage>
    >NUL chcp %1
  GOTO :EOF
