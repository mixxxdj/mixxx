@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

CALL :REALPATH "%~dp0\.."
SET MIXXX_ROOT=%RETVAL%

IF NOT DEFINED PLATFORM (
    SET PLATFORM=x64
)

IF NOT DEFINED BUILDENV_BASEPATH (
    REM  SET BUILDENV_BASEPATH=%MIXXX_ROOT%\buildenv
	SET BUILDENV_BASEPATH=B:\\vcpkg-git\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake
)

IF NOT DEFINED BUILD_ROOT (
    SET BUILD_ROOT=%MIXXX_ROOT%\build
)

IF NOT DEFINED INSTALL_ROOT (
    SET INSTALL_ROOT=%MIXXX_ROOT%\install
)

REM IF DEFINED BUILDENV_RELEASE (
    SET BUILDENV_BRANCH=2.6-rel
    SET VCPKG_TARGET_TRIPLET=x64-windows-release
    REM vcpkg_update_main
    REM SET BUILDENV_NAME=mixxx-deps-2.6-x64-windows-release-98fd50c
    REM SET BUILDENV_SHA256=dff7d5a8141ae2a4c13eb85fe45e6f2915b24c11329af52a11b38b430e6b1961
REM ) ELSE (
    REM SET BUILDENV_BRANCH=2.6
    REM SET VCPKG_TARGET_TRIPLET=x64-windows
    REM SET BUILDENV_NAME=mixxx-deps-2.6-x64-windows-12239ed
    REM SET BUILDENV_SHA256=5c60b2c61d6448a99979d7cc997e92f2fd5b4b65f65e2439abfca3fa6fd30f8d
)

IF "%~1"=="" (
    REM In case of manual start by double click no arguments are specified: Default to COMMAND_setup
    CALL :COMMAND_setup
    PAUSE
) ELSE (
    CALL :COMMAND_%1
)

REM Make These permanent, not local to the batch script.
REM ENDLOCAL & SET "MIXXX_VCPKG_ROOT=%MIXXX_VCPKG_ROOT%" & SET "VCPKG_DEFAULT_TRIPLET=%VCPKG_DEFAULT_TRIPLET%" & SET "X_VCPKG_APPLOCAL_DEPS_INSTALL=%X_VCPKG_APPLOCAL_DEPS_INSTALL%" & SET "CMAKE_GENERATOR=%CMAKE_GENERATOR%"
ENDLOCAL & SET "MIXXX_VCPKG_ROOT=%MIXXX_VCPKG_ROOT%" & SET "VCPKG_DEFAULT_TRIPLET=%VCPKG_DEFAULT_TRIPLET%" & SET "X_VCPKG_APPLOCAL_DEPS_INSTALL=%X_VCPKG_APPLOCAL_DEPS_INSTALL%" & SET "CMAKE_GENERATOR=%CMAKE_GENERATOR%"


EXIT /B 0

:COMMAND_name
    IF DEFINED GITHUB_ENV (
        ECHO BUILDENV_NAME=%BUILDENV_NAME% >> !GITHUB_ENV!
    )
    GOTO :EOF

:COMMAND_setup
    SET BUILDENV_PATH=%BUILDENV_BASEPATH%\%BUILDENV_NAME%

    IF NOT EXIST "%BUILDENV_BASEPATH%" (
        ECHO ^Creating "buildenv" directory...
        MD "%BUILDENV_BASEPATH%"
    )

    IF NOT EXIST "%BUILDENV_PATH%" (
        SET BUILDENV_URL=https://downloads.mixxx.org/dependencies/!BUILDENV_BRANCH!/Windows/!BUILDENV_NAME!.zip
        IF NOT EXIST "!BUILDENV_PATH!.zip" (
            ECHO ^Download prebuilt build environment from "!BUILDENV_URL!" to "!BUILDENV_PATH!.zip"...
            REM TODO: The /DYNAMIC parameter is required because our server does not yet support HTTP range headers
            BITSADMIN /transfer buildenvjob /download /priority normal /DYNAMIC !BUILDENV_URL! "!BUILDENV_PATH!.zip"
            ECHO ^Download complete.
            certutil -hashfile "!BUILDENV_PATH!.zip" SHA256 | FIND /C "!BUILDENV_SHA256!"
            IF errorlevel 1 (
                ECHO ^ERROR: Download did not match expected SHA256 checksum!
                certutil -hashfile "!BUILDENV_PATH!.zip" SHA256
                echo ^Expected: "!BUILDENV_SHA256!"
                EXIT /B 1
            )
        ) else (
            ECHO ^Using cached archive at "!BUILDENV_PATH!.zip".
        )

        CALL :DETECT_SEVENZIP
        IF !RETVAL!=="" (
            ECHO ^Unpacking "!BUILDENV_PATH!.zip" using powershell...
            CALL :UNZIP_POWERSHELL "!BUILDENV_PATH!.zip" "!BUILDENV_BASEPATH!"
        ) ELSE (
            ECHO ^Unpacking "!BUILDENV_PATH!.zip" using 7z...
            CALL :UNZIP_SEVENZIP !RETVAL! "!BUILDENV_PATH!.zip" "!BUILDENV_BASEPATH!"
        )
        IF NOT EXIST "%BUILDENV_PATH%" (
            ECHO ^Error: Unpacking failed. The downloaded archive might be broken, consider removing "!BUILDENV_PATH!.zip" to force redownload.
            EXIT /B 1
        )

        ECHO ^Unpacking complete.
        DEL /f /q "%BUILDENV_PATH%.zip"
    )

    ECHO ^Build environment path: !BUILDENV_PATH!

    SET "MIXXX_VCPKG_ROOT=!BUILDENV_PATH!"
    SET "CMAKE_GENERATOR=Ninja"
    SET "CMAKE_PREFIX_PATH=!BUILDENV_PATH!\installed\!VCPKG_TARGET_TRIPLET!"

    ECHO ^Environment Variables:
    ECHO ^- MIXXX_VCPKG_ROOT='!MIXXX_VCPKG_ROOT!'
    ECHO ^- CMAKE_GENERATOR='!CMAKE_GENERATOR!'

    IF DEFINED GITHUB_ENV (
        ECHO MIXXX_VCPKG_ROOT=!MIXXX_VCPKG_ROOT!>>!GITHUB_ENV!
        ECHO CMAKE_GENERATOR=!CMAKE_GENERATOR!>>!GITHUB_ENV!
    ) ELSE (
        ECHO ^Generating "CMakeSettings.json"...
        CALL :GENERATE_CMakeSettings_JSON

        IF NOT EXIST "%BUILD_ROOT%" (
            ECHO ^Creating subdirectory "build"...
            MD "%BUILD_ROOT%"
        )

        IF NOT EXIST "%INSTALL_ROOT%" (
            ECHO ^Creating subdirectory "install"...
            MD "%INSTALL_ROOT%"
        )
    )
    GOTO :EOF


:DETECT_SEVENZIP
    SET SEVENZIP_PATH="7z.exe"
    !SEVENZIP_PATH! --help >NUL 2>NUL
    IF errorlevel 1 (
        SET SEVENZIP_PATH="c:\Program Files\7-Zip\7z.exe"
        !SEVENZIP_PATH! --help >NUL 2>NUL
        IF errorlevel 1 (
            SET SEVENZIP_PATH="c:\Program Files (x86)\7-Zip\7z.exe"
            !SEVENZIP_PATH! --help >NUL 2>NUL
            if errorlevel 1 (
                SET SEVENZIP_PATH=""
            )
        )
    )
    SET RETVAL=!SEVENZIP_PATH!
    GOTO :EOF


:UNZIP_SEVENZIP <7zippath> <newzipfile> <ExtractTo>
    %1 x "-o%~3" %2
    GOTO :EOF


:UNZIP_POWERSHELL <newzipfile> <ExtractTo>
    SET SCRIPTDIR=%~dp0
    powershell -executionpolicy bypass -File "%SCRIPTDIR%\unzip.ps1" %1 %2
    GOTO :EOF


:REALPATH
    SET RETVAL=%~f1
    GOTO :EOF


:GENERATE_CMakeSettings_JSON
REM Generate CMakeSettings.json which is read by MS Visual Studio to determine the supported CMake build environments
    SET CMakeSettings=%MIXXX_ROOT%\CMakeSettings.json
    IF EXIST "%CMakeSettings%" (
        FOR /f "delims=" %%a in ('wmic OS Get localdatetime ^| find "."') do set DateTime=%%a
        SET CMakeSettingsBackup=CMakeSettings_!DateTime:~0,4!-!DateTime:~4,2!-!DateTime:~6,2!_!DateTime:~8,2!-!DateTime:~10,2!-!DateTime:~12,2!.json
        ECHO CMakeSettings.json already exists, creating backup at "!CMakeSettingsBackup!"...
        REN "%CMakeSettings%" "!CMakeSettingsBackup!"
    )

    CALL :SETANSICONSOLE
    SET OLDCODEPAGE=%RETVAL%
    REM Start with a UTF-8-BOM
    REM Correct values are EF BB BF (&iuml; &raquo; &iquest;)
    REM Make sure the BOM is not removed from UTF-8-BOM.txt
    copy "%MIXXX_ROOT%\tools\UTF-8-BOM.txt" "%CMakeSettings%"
    CALL :SETUTF8CONSOLE

    >>"%CMakeSettings%" echo {
    >>"%CMakeSettings%" echo   "configurations": [
    SET configElementTermination=,
    IF NOT DEFINED BUILDENV_RELEASE (
        CALL :Configuration2CMakeSettings_JSON off       Debug
    )
    CALL :Configuration2CMakeSettings_JSON legacy    RelWithDebInfo
    CALL :Configuration2CMakeSettings_JSON portable  RelWithDebInfo
    SET configElementTermination=
    CALL :Configuration2CMakeSettings_JSON native    Release
    >>"%CMakeSettings%" echo   ]
    >>"%CMakeSettings%" echo }

    echo ^You can now open CMakeSetting.json from Visual Studio
    echo ^or configure cmake from the command line in an EMPTY build directory via:
    echo ^cmake -DCMAKE_TOOLCHAIN_FILE=B:\vcpkg-git\vcpkg\scripts\buildsystems\vcpkg.cmake %MIXXX_ROOT%

    CALL :RESTORECONSOLE %OLDCODEPAGE%
    GOTO :EOF

:Configuration2CMakeSettings_JSON <optimize> <configurationtype>
    >>"%CMakeSettings%" echo     {
    >>"%CMakeSettings%" echo       "name": "!PLATFORM!__%1",
    >>"%CMakeSettings%" echo       "buildRoot": "!BUILD_ROOT:\=\\!\\${name}",
    >>"%CMakeSettings%" echo       "configurationType": "%2",
    >>"%CMakeSettings%" echo       "enableClangTidyCodeAnalysis": true,
    >>"%CMakeSettings%" echo       "generator": "Ninja",
    >>"%CMakeSettings%" echo       "inheritEnvironments": [ "msvc_!PLATFORM!_!PLATFORM!" ],
    >>"%CMakeSettings%" echo       "installRoot": "!INSTALL_ROOT:\=\\!\\${name}",
    >>"%CMakeSettings%" echo       "cmakeToolchain": "!MIXXX_VCPKG_ROOT:\=\\!\\scripts\\buildsystems\\vcpkg.cmake",
    >>"%CMakeSettings%" echo       "intelliSenseMode": "windows-msvc-!PLATFORM!",
    >>"%CMakeSettings%" echo       "variables": [
    SET variableElementTermination=,
    CALL :AddCMakeVar2CMakeSettings_JSON "MIXXX_VCPKG_ROOT"                   "STRING"   "!MIXXX_VCPKG_ROOT:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "BATTERY"                            "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "BROADCAST"                          "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "BULK"                               "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_EXPORT_COMPILE_COMMANDS"      "BOOL"   "True"
    REM Replace all \ by \\ in CMAKE_PREFIX_PATH
    REM CALL :AddCMakeVar2CMakeSettings_JSON "CMAKE_PREFIX_PATH"                  "STRING"   "!CMAKE_PREFIX_PATH:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "DEBUG_ASSERTIONS_FATAL"             "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "FFMPEG"                             "BOOL"   "True"
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
    CALL :AddCMakeVar2CMakeSettings_JSON "QT6"                                "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "QTKEYCHAIN"                         "BOOL"   "True"
    CALL :AddCMakeVar2CMakeSettings_JSON "STATIC_DEPS"                        "BOOL"   "False"
    CALL :AddCMakeVar2CMakeSettings_JSON "VCPKG_TARGET_TRIPLET"               "STRING"  "!VCPKG_TARGET_TRIPLET!"
    CALL :AddCMakeVar2CMakeSettings_JSON "VINYLCONTROL"                       "BOOL"   "True"
    SET variableElementTermination=
    CALL :AddCMakeVar2CMakeSettings_JSON "WAVPACK"                            "BOOL"   "True"
    >>"%CMakeSettings%" echo       ]
    >>"%CMakeSettings%" echo     }!configElementTermination!
  GOTO :EOF

:AddCMakeVar2CMakeSettings_JSON <varname> <vartype> <value>
    >>"%CMakeSettings%" echo         {
    >>"%CMakeSettings%" echo           "name": %1,
    >>"%CMakeSettings%" echo           "value": %3,
    >>"%CMakeSettings%" echo           "type": %2
    >>"%CMakeSettings%" echo         }!variableElementTermination!
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
