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

REM Make These permanent, not local to the batch script.
ENDLOCAL & SET "VCPKG_ROOT=%VCPKG_ROOT%" & SET "VCPKG_DEFAULT_TRIPLET=%VCPKG_DEFAULT_TRIPLET%" & SET "X_VCPKG_APPLOCAL_DEPS_INSTALL=%X_VCPKG_APPLOCAL_DEPS_INSTALL%" & SET "CMAKE_GENERATOR=%CMAKE_GENERATOR%"

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

    IF NOT EXIST %BUILDENV_BASEPATH% (
        ECHO ^Creating "buildenv" directory...
        MD %BUILDENV_BASEPATH%
    )

    IF NOT EXIST %BUILDENV_PATH% (
        SET BUILDENV_URL=https://downloads.mixxx.org/dependencies/2.3/Windows/!BUILDENV_NAME!.zip
        IF NOT EXIST !BUILDENV_PATH!.zip (
            ECHO ^Download prebuilt build environment from "!BUILDENV_URL!" to "!BUILDENV_PATH!.zip"...
            BITSADMIN /transfer buildenvjob /download /priority normal !BUILDENV_URL! !BUILDENV_PATH!.zip
            REM TODO: verify download using sha256sum?
            ECHO ^Download complete.
        ) else (
            ECHO ^Using cached archive at "!BUILDENV_PATH!.zip".
        )

        CALL :DETECT_SEVENZIP
        IF !RETVAL!=="" (
            ECHO ^Unpacking "!BUILDENV_PATH!.zip" using powershell...
            CALL :UNZIP_POWERSHELL "!BUILDENV_PATH!.zip" "!BUILDENV_BASEPATH!"
        ) ELSE (
            ECHO ^Unpacking "!BUILDENV_PATH!.zip" using 7z...
            CALL :UNZIP_SEVENZIP "!RETVAL!" "!BUILDENV_PATH!.zip" "!BUILDENV_BASEPATH!"
        )
        IF NOT EXIST %BUILDENV_PATH% (
            ECHO ^Error: Unpacking failed. The downloaded archive might be broken, consider removing "!BUILDENV_PATH!.zip" to force redownload.
            EXIT /B 1
        )

        ECHO ^Unpacking complete.
        DEL /f /q %BUILDENV_PATH%.zip
    )

    ECHO ^Build environment path: !BUILDENV_PATH!

    SET "VCPKG_ROOT=!BUILDENV_PATH!"
    SET "VCPKG_DEFAULT_TRIPLET=x64-windows"
    SET "X_VCPKG_APPLOCAL_DEPS_INSTALL=ON"
    SET "CMAKE_GENERATOR=Ninja"

    ECHO ^Environment Variables:
    ECHO ^- VCPKG_ROOT='!VCPKG_ROOT!'
    ECHO ^- VCPKG_DEFAULT_TRIPLET='!VCPKG_DEFAULT_TRIPLET!'
    ECHO ^- X_VCPKG_APPLOCAL_DEPS_INSTALL='!X_VCPKG_APPLOCAL_DEPS_INSTALL!'
    ECHO ^- CMAKE_GENERATOR='!CMAKE_GENERATOR!'

    IF DEFINED GITHUB_ENV (
        ECHO VCPKG_ROOT=!VCPKG_ROOT!>>!GITHUB_ENV!
        ECHO VCPKG_DEFAULT_TRIPLET=!VCPKG_DEFAULT_TRIPLET!>>!GITHUB_ENV!
        ECHO X_VCPKG_APPLOCAL_DEPS_INSTALL=!X_VCPKG_APPLOCAL_DEPS_INSTALL!>>!GITHUB_ENV!
        ECHO CMAKE_GENERATOR=!CMAKE_GENERATOR!>>!GITHUB_ENV!
    ) ELSE (
        ECHO ^Generating "CMakeSettings.json"...
        CALL :GENERATE_CMakeSettings_JSON

        IF NOT EXIST %BUILD_ROOT% (
            ECHO ^Creating subdirectory "build"...
            MD %BUILD_ROOT%
        )

        IF NOT EXIST %INSTALL_ROOT% (
            ECHO ^Creating subdirectory "install"...
            MD %INSTALL_ROOT%
        )
    )
    GOTO :EOF


:DETECT_SEVENZIP
    SET SEVENZIP_PATH=7z.exe
    !SEVENZIP_PATH! --help >NUL 2>NUL
    IF errorlevel 1 (
        SET SEVENZIP_PATH="c:\Program Files\7-Zip\7z.exe"
        !SEVENZIP_PATH! --help >NUL 2>NUL
        IF errorlevel 1 (
            SET SEVENZIP_PATH="c:\Program Files (x86)\7-Zip\7z.exe"
            !SEVENZIP_PATH! --help >NUL 2>NUL
            if errorlevel 1 (
                SET SEVENZIP_PATH=
            )
        )
    )
    SET RETVAL="!SEVENZIP_PATH!"
    GOTO :EOF


:UNZIP_SEVENZIP <7zippath> <newzipfile> <ExtractTo>
    %1 x -o"%3" "%2"
    GOTO :EOF


:UNZIP_POWERSHELL <newzipfile> <ExtractTo>
    SET SCRIPTDIR=%~dp0
    powershell -executionpolicy bypass -File "%SCRIPTDIR%\unzip.ps1" %1 %2
    GOTO :EOF


:REALPATH
    SET RETVAL=%~f1
    GOTO :EOF


:READ_ENVNAME
    ECHO Reading name of prebuild environment from "%MIXXX_ROOT%\packaging\windows\build_environment"
    SET /P BUILDENV_NAME=<%MIXXX_ROOT%\packaging\windows\build_environment
    SET BUILDENV_NAME=!BUILDENV_NAME:PLATFORM=%PLATFORM%!
    SET BUILDENV_NAME=!BUILDENV_NAME:CONFIGURATION=%CONFIGURATION%!
    SET RETVAL=%BUILDENV_NAME%
    ECHO Environment name: %RETVAL%
    GOTO :EOF

:GENERATE_CMakeSettings_JSON
REM Generate CMakeSettings.json which is read by MS Visual Studio to determine the supported CMake build environments
    SET CMakeSettings=%MIXXX_ROOT%\CMakeSettings.json
    IF EXIST %CMakeSettings% (
        FOR /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do set DateTime=%%a
        SET CMakeSettingsBackup=CMakeSettings_!DateTime:~0,4!-!DateTime:~4,2!-!DateTime:~6,2!_!DateTime:~8,2!-!DateTime:~10,2!-!DateTime:~12,2!.json
        ECHO CMakeSettings.json already exists, creating backup at "!CMakeSettingsBackup!"...
        REN %CMakeSettings% !CMakeSettingsBackup!
    )

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
    CALL :AddCMakeVar2CMakeSettings_JSON "BULK"                               "BOOL"   "False"
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_EXPORT_COMPILE_COMMANDS"      "BOOL"   "True"
    REM Replace all \ by \\ in CMAKE_PREFIX_PATH
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_PREFIX_PATH"                  "STRING"   "!CMAKE_PREFIX_PATH:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "DEBUG_ASSERTIONS_FATAL"             "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "FFMPEG"                             "BOOL"   "False"
    CALL :AddCMakeVar2CMakeSettings_JSON "HID"                                "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "HSS1394"                            "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "KEYFINDER"                          "BOOL"   "False"
    CALL :AddCMakeVar2CMakeSettings_JSON "LOCALECOMPARE"                      "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "LILV"                               "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "MAD"                                "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "MEDIAFOUNDATION"                    "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "MODPLUG"                            "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "OPUS"                               "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "OPTIMIZE"                           "STRING" "%1"
    CALL :AddCMakeVar2CMakeSettings_JSON "QTKEYCHAIN"                         "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "STATIC_DEPS"                        "BOOL"   "False"
    CALL :AddCMakeVar2CMakeSettings_JSON "VINYLCONTROL"                       "BOOL"   "True"
    SET variableElementTermination=
    CALL :AddCMakeVar2CMakeSettings_JSON "WAVPACK"                            "BOOL"   "True"
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
    REM Remove suffix dot
    set _codepage=%_codepage:.=%
    SET RETVAL=%_codepage%
  GOTO :EOF

:SETUTF8CONSOLE
    >NUL chcp 65001
  GOTO :EOF

:RESTORECONSOLE <codepage>
    >NUL chcp %1
  GOTO :EOF
