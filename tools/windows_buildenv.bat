@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION

CALL :REALPATH "%~dp0\.."
SET MIXXX_ROOT=%RETVAL%

REM Detect host architecture
IF /I "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
    SET "HOST_ARCH=x64"
) else IF /I "%PROCESSOR_ARCHITECTURE%"=="ARM64" (
    SET "HOST_ARCH=arm64"
) else (
    echo ^Error: Unknown processor architecture: %PROCESSOR_ARCHITECTURE%
    PAUSE
    EXIT /B 1
)

IF NOT DEFINED PLATFORM (
    ECHO ^Info: The PLATFORM environment variable is not defined. Using host's PROCESSOR_ARCHITECTURE=%PROCESSOR_ARCHITECTURE%.
    SET "PLATFORM=%HOST_ARCH%"
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

IF /I "%PLATFORM%"=="arm64" (
    IF DEFINED BUILDENV_RELEASE (
        SET BUILDENV_BRANCH=2.6-rel
        SET VCPKG_TARGET_TRIPLET=arm64-windows-release
        SET BUILDENV_NAME=mixxx-deps-2.6-arm64-windows-rel-da4c207
        SET BUILDENV_SHA256=1225f0a71a1dd624b14f6b2148cc01305496f11ba2bcbff8638ecf2e1e995702
    ) ELSE (
        SET BUILDENV_BRANCH=2.6
        SET VCPKG_TARGET_TRIPLET=arm64-windows
        SET BUILDENV_NAME=mixxx-deps-2.6-arm64-windows-c2def9b
        SET BUILDENV_SHA256=9918615b607045f5907e051d84f40180ec7392b84e46bed571dc6bf97438303d
    )
) ELSE IF /I "%PLATFORM%"=="x64" (
    IF DEFINED BUILDENV_RELEASE (
        SET BUILDENV_BRANCH=2.6-rel
        SET VCPKG_TARGET_TRIPLET=x64-windows-release
        SET BUILDENV_NAME=mixxx-deps-2.6-x64-windows-rel-da4c207
        SET BUILDENV_SHA256=62d4d7249a7e49ef96d4b96b380e23426dd714eaa9ae415e7a66a587a71e9a27
    ) ELSE (
        SET BUILDENV_BRANCH=2.6
        SET VCPKG_TARGET_TRIPLET=x64-windows
        SET BUILDENV_NAME=mixxx-deps-2.6-x64-windows-c2def9b
        SET BUILDENV_SHA256=01df9fdc8154f96184281a934e73eb4202e4f29452ecc888053c747f7a745d4f
    )
) ELSE (
    ECHO ^ERROR: Unsupported PLATFORM: %PLATFORM%
    ECHO ^Please refer to the following guide to manually build the vcpkg environment:"
    ECHO ^https://github.com/mixxxdj/mixxx/wiki/Compiling-dependencies-for-macOS-arm64"
    PAUSE
    EXIT /B 1
)
SET BUILDENV_URL=https://downloads.mixxx.org/dependencies/!BUILDENV_BRANCH!/Windows/!BUILDENV_NAME!.zip

IF "%~1"=="" (
    REM In case of manual start by double click no arguments are specified: Default to COMMAND_setup
    CALL :COMMAND_setup
    PAUSE
) ELSE (
    CALL :COMMAND_%1
)

REM Make These permanent, not local to the batch script.
ENDLOCAL & SET "MIXXX_VCPKG_ROOT=%MIXXX_VCPKG_ROOT%" & SET "VCPKG_DEFAULT_TRIPLET=%VCPKG_DEFAULT_TRIPLET%" & SET "X_VCPKG_APPLOCAL_DEPS_INSTALL=%X_VCPKG_APPLOCAL_DEPS_INSTALL%" & SET "CMAKE_GENERATOR=%CMAKE_GENERATOR%" & SET "BUILDENV_BASEPATH=%BUILDENV_BASEPATH%" & SET "BUILDENV_URL=%BUILDENV_URL%" & SET "BUILDENV_SHA256=%BUILDENV_SHA256%"

EXIT /B 0

:COMMAND_name
    IF DEFINED GITHUB_ENV (
        ECHO BUILDENV_NAME=%BUILDENV_NAME% >> !GITHUB_ENV!
    )
    GOTO :EOF

:COMMAND_setup
    SET BUILDENV_PATH=%BUILDENV_BASEPATH%\%BUILDENV_NAME%
    ECHO ^Build environment path: !BUILDENV_PATH!

    SET "MIXXX_VCPKG_ROOT=!BUILDENV_PATH!"
    SET "CMAKE_GENERATOR=Ninja"
    SET "CMAKE_PREFIX_PATH=!BUILDENV_PATH!\installed\!VCPKG_TARGET_TRIPLET!"

    ECHO ^Environment Variables:
    ECHO ^- MIXXX_VCPKG_ROOT='!MIXXX_VCPKG_ROOT!'
    ECHO ^- CMAKE_GENERATOR='!CMAKE_GENERATOR!'
    ECHO ^- BUILDENV_BASEPATH='!BUILDENV_BASEPATH!'
    ECHO ^- BUILDENV_URL='!BUILDENV_URL!'
    ECHO ^- BUILDENV_SHA256='!BUILDENV_SHA256!'

    IF DEFINED GITHUB_ENV (
        ECHO MIXXX_VCPKG_ROOT=!MIXXX_VCPKG_ROOT!>>!GITHUB_ENV!
        ECHO CMAKE_GENERATOR=!CMAKE_GENERATOR!>>!GITHUB_ENV!
        ECHO VCPKG_TARGET_TRIPLET=!VCPKG_TARGET_TRIPLET!>>!GITHUB_ENV!
        ECHO BUILDENV_BASEPATH=!BUILDENV_BASEPATH!>>!GITHUB_ENV!
        ECHO BUILDENV_URL=!BUILDENV_URL!>>!GITHUB_ENV!
        ECHO BUILDENV_SHA256=!BUILDENV_SHA256!>>!GITHUB_ENV!
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
    echo ^cmake -DCMAKE_TOOLCHAIN_FILE=!MIXXX_VCPKG_ROOT!\scripts\buildsystems\vcpkg.cmake %MIXXX_ROOT%

    CALL :RESTORECONSOLE %OLDCODEPAGE%
    GOTO :EOF

:Configuration2CMakeSettings_JSON <optimize> <configurationtype>
    >>"%CMakeSettings%" echo     {
    >>"%CMakeSettings%" echo       "name": "!PLATFORM!__%1",
    >>"%CMakeSettings%" echo       "buildRoot": "!BUILD_ROOT:\=\\!\\${name}",
    >>"%CMakeSettings%" echo       "configurationType": "%2",
    >>"%CMakeSettings%" echo       "enableClangTidyCodeAnalysis": true,
    >>"%CMakeSettings%" echo       "generator": "Ninja",
    REM <compiler>_<architecture>_<host_arch>
    >>"%CMakeSettings%" echo       "inheritEnvironments": [ "msvc_!PLATFORM!_!HOST_ARCH!" ],
    >>"%CMakeSettings%" echo       "installRoot": "!INSTALL_ROOT:\=\\!\\${name}",
    >>"%CMakeSettings%" echo       "cmakeToolchain": "!MIXXX_VCPKG_ROOT:\=\\!\\scripts\\buildsystems\\vcpkg.cmake",
    REM <platform>-<compiler>-<architecture>
    >>"%CMakeSettings%" echo       "intelliSenseMode": "windows-msvc-!PLATFORM!",
    >>"%CMakeSettings%" echo       "variables": [
    SET variableElementTermination=,
    CALL :AddCMakeVar2CMakeSettings_JSON "MIXXX_VCPKG_ROOT"                   "STRING"   "!MIXXX_VCPKG_ROOT:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "BUILDENV_BASEPATH"                  "STRING"   "!BUILDENV_BASEPATH:\=\\!"
    CALL :AddCMakeVar2CMakeSettings_JSON "BUILDENV_URL"                       "STRING"   "!BUILDENV_URL!"
    CALL :AddCMakeVar2CMakeSettings_JSON "BUILDENV_SHA256"                    "STRING"   "!BUILDENV_SHA256:\=\\!"
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
