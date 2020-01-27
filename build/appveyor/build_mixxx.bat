@echo on
SETLOCAL

REM ==================================
REM Path setup and  initial checks
REM ==================================

IF "%ProgramW6432%" =="" (
SET PROGRAMFILES_PATH=%ProgramFiles%
) else (
REM doublequote the whole SET command prevents an error with parentheses
SET "PROGRAMFILES_PATH=%ProgramFiles(x86)%"
)

rem ====== Edit to suit your environment =========
SET VCVERSION=141
SET "MSVC_PATH=%PROGRAMFILES_PATH%\Microsoft Visual Studio\2017\Community\VC"
SET "BUILDTOOLS_PATH=%PROGRAMFILES_PATH%\Microsoft Visual Studio\2017\BuildTools\VC"
SET BUILDTOOLS_SCRIPT=Auxiliary\Build\vcvarsall.bat

IF EXIST "%MSVC_PATH%" (
echo Building with Microsoft Visual Studio 2017 Community Edition.
SET "BUILDTOOLS_PATH=%MSVC_PATH%"
) ELSE (
IF EXIST "%BUILDTOOLS_PATH%" (
echo Building with Microsoft Visual Studio 2017 Build Tools.
) ELSE (
echo.
echo Could not find "%MSVC_PATH%" nor "%BUILDTOOLS_PATH%".
echo Edit the build_environment.bat file and/or install the required software
echo http://landinghub.visualstudio.com/visual-cpp-build-tools
echo https://www.microsoft.com/en-us/download/details.aspx?id=8279
exit /b 1
)
REM END EXIST BUILDTOOLS
)
REM END EXIST MSVC_PATH

REM Check whether we have a 64-bit compiler available.
REM TODO(rryan): Remove hard-coding of compiler version in this path?
IF EXIST "%BUILDTOOLS_PATH%\Tools\MSVC\14.15.26726\bin\Hostx64\x64\cl.exe" (
echo Using 64-bit compiler.
SET COMPILER_X86=amd64_x86
SET COMPILER_X64=amd64
) ELSE (
echo Using 32-bit compiler.
SET COMPILER_X86=x86
SET COMPILER_X64=x86_amd64
)

REM ==================================
REM Parameter reading and variable setup
REM ==================================
REM ^ is the escape character.
if "%3" == "" (
  echo Missing parameters. Usage: ^(Note: keep same case and order^)
  echo.
  echo build_mixxx.bat x64^|x86 debug^|release^|release-fastbuild ^<winlib-path^>
  echo.
  echo Example: build_mixxx.bat x64 release c:\mixxx\environments\msvc15-static-x86-release
  exit /b 1
)

set MACHINE_X86="%1" == "x86"
if "%2" == "release" (
  set CONFIG_RELEASE=1==1
)
if "%2" == "release-fastbuild" (
  set CONFIG_RELEASE=1==1
)
if "%2" == "debug" (
  set CONFIG_RELEASE=0==1
)

set WINLIB_DIR=%3

SET BIN_DIR=%WINLIB_DIR%\bin
SET LIB_DIR=%WINLIB_DIR%\lib
SET INCLUDE_DIR=%WINLIB_DIR%\include
REM TODO(rryan): Remove hard-coding of Qt version.
set QT_VERSION=5.12.0
SET QTDIR=%WINLIB_DIR%\Qt-%QT_VERSION%

if NOT EXIST "%BIN_DIR%\scons.py" (
echo.
echo You need to obtain and copy SCons to the folder:
echo %BIN_DIR%
exit /b 1
)

if NOT EXIST "%QTDIR%" (
echo.
echo Could not find Qt %QT_VERSION% at "%QT_DIR%".
exit /b 1
)

REM Everything prepared. Setup the compiler.
if %MACHINE_X86% (
call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" %COMPILER_X86%
) else (
call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" %COMPILER_X64%
)

REM Now build Mixxx.

if %CONFIG_RELEASE% (
set BUILD_TYPE=release
) else (
set BUILD_TYPE=debug
)

if %MACHINE_X86% (
set MACHINE_TYPE=x86
set DISTDIR=dist32
) else (
set MACHINE_TYPE=x86_64
set DISTDIR=dist64
)

REM Clean up after old builds.
REM del /q /f *.exe *.msi 2>NUL
REM rmdir /s /q %DISTDIR%

rem /MP Use all CPU cores.
rem /FS force synchronous PDB writes (prevents PDB corruption with /MP)
rem /EHsc Do not handle SEH in try / except blocks.
set CXXFLAGS=/MP /FS /EHsc
set CFLAGS=/MP /FS /EHsc

set PATH=%BIN_DIR%;%PATH%
scons.py mixxx makerelease toolchain=msvs winlib=%WINLIB_DIR% build=%BUILD_TYPE% staticlibs=1 staticqt=1 debug_assertions_fatal=1 verbose=0 machine=%MACHINE_TYPE% qtdir=%QTDIR% hss1394=1 mediafoundation=1 opus=1 localecompare=1 optimize=fastbuild virtualize=0 test=1 qt_sqlite_plugin=0 build_number_in_title_bar=0 bundle_pdbs=0

IF ERRORLEVEL 1 (
echo ==============================
echo.
echo Building Mixxx failed.
echo.
REM For debugging, print the configuration log.
REM echo Printing config.log:
REM type config.log
ENDLOCAL
exit /b 1
) else (
echo Mixxx built successfully
ENDLOCAL
)
