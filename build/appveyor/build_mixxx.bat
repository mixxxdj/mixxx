@echo off
SETLOCAL ENABLEDELAYEDEXPANSION

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
SET PARAM_VCVARSVER=14.1
SET "BUILDTOOLS_PATH=%PROGRAMFILES_PATH%\Microsoft Visual Studio\2017\BuildTools\VC"
SET BUILDTOOLS_SCRIPT=Auxiliary\Build\vcvarsall.bat

IF EXIST "%BUILDTOOLS_PATH%" (
echo Building with preconfigured path at: "%BUILDTOOLS_PATH%"
) ELSE (
call :function_get_product
IF ERRORLEVEL 1 (
echo.
echo Could not find "%BUILDTOOLS_PATH%" and the detection of product didn't work
echo Edit the %~nx0 file and/or install the required software
echo http://landinghub.visualstudio.com/visual-cpp-build-tools
echo https://www.microsoft.com/en-us/download/details.aspx?id=8279
exit /b 1
)
REM END NO PRODUCT
)
REM END EXIST BUILDTOOLS_PATH

REM Check whether we have a 64-bit compiler available.
call :function_has_64bit
IF ERRORLEVEL 1 (
echo Using 32-bit compiler.
SET COMPILER_X86=x86
SET COMPILER_X64=x86_amd64
) ELSE (
echo Using 64-bit compiler.
SET COMPILER_X86=amd64_x86
SET COMPILER_X64=amd64
)

REM ==================================
REM Parameter reading and variable setup
REM ==================================
REM ^ is the escape character.
if "%3" == "" (
  echo Missing parameters. Usage: ^(Note: keep same case and order^)
  echo.
  echo build_mixxx.bat x64^|x86 debug^|release^|release-fastbuild ^<winlib-path^> [skiptest] [skipinstaller]
  echo.
  echo skiptest means that we don't want to build and execute the mixxx-test.
  echo skipinstaller means that we don't want to generate the installer after the build.
  echo.
  echo Example: build_mixxx.bat x64 release c:\mixxx\environments\2.3-j00013-x64-release-static-36f44bd2-minimal
  exit /b 1
)

set MACHINE_X86="%1" == "x86"
if "%2" == "release" (
  set CONFIG_RELEASE=1==1
  set PARAM_OPTIMIZE=portable
)
if "%2" == "release-fastbuild" (
  set CONFIG_RELEASE=1==1
  set PARAM_OPTIMIZE=fastbuild
)
if "%2" == "debug" (
  set CONFIG_RELEASE=0==1
  set PARAM_OPTIMIZE=portable
)
if "%4" == "skiptest" (
  set PARAM_TEST=0
) else (
  set PARAM_TEST=1
)
if "%4" == "skipinstaller" (
   set PARAM_INSTALLER=
) else (
    if "%5" == "skipinstaller" (
       set PARAM_INSTALLER=
    ) else (
       set PARAM_INSTALLER=makerelease
    )
)
set WINLIB_DIR=%3

SET BIN_DIR=%WINLIB_DIR%\bin
SET LIB_DIR=%WINLIB_DIR%\lib
SET INCLUDE_DIR=%WINLIB_DIR%\include
FOR /D %%G IN (%WINLIB_DIR%\Qt-*) DO SET QTDIR=%%G
IF "!QTDIR!" EQU "" (
echo QT not found on %WINLIB_DIR%
exit /b 1
)



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
call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" %COMPILER_X86% -vcvars_ver=%PARAM_VCVARSVER%
set MACHINE_TYPE=x86
) else (
call "%BUILDTOOLS_PATH%\%BUILDTOOLS_SCRIPT%" %COMPILER_X64% -vcvars_ver=%PARAM_VCVARSVER%
set MACHINE_TYPE=x86_64
)

if %CONFIG_RELEASE% (
set BUILD_TYPE=release
) else (
set BUILD_TYPE=debug
)

rem /MP Use all CPU cores.
rem /FS force synchronous PDB writes (prevents PDB corruption with /MP)
rem /EHsc Do not handle SEH in try / except blocks.
set CXXFLAGS=/MP /FS /EHsc
set CFLAGS=/MP /FS /EHsc

REM Now build Mixxx.
set PATH=%BIN_DIR%;%PATH%
scons.py %SCONS_NUMBER_PROCESSORS% mixxx %PARAM_INSTALLER% toolchain=msvs winlib=%WINLIB_DIR% build=%BUILD_TYPE% staticlibs=1 staticqt=1 debug_assertions_fatal=1 verbose=0 machine=%MACHINE_TYPE% qtdir=%QTDIR% hss1394=1 mediafoundation=1 opus=1 localecompare=1 optimize=%PARAM_OPTIMIZE% virtualize=0 test=%PARAM_TEST% qt_sqlite_plugin=0 build_number_in_title_bar=0 bundle_pdbs=0

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
EXIT /b 0

:function_get_product
FOR %%Y IN (2019,2017) DO (
  FOR %%P IN (Community,Professional,Enterprise) DO (
    SET "LOCAL_VS_PATH=%PROGRAMFILES_PATH%\Microsoft Visual Studio\%%Y\%%P\VC"
    IF EXIST "!LOCAL_VS_PATH!" (
      SET "BUILDTOOLS_PATH=!LOCAL_VS_PATH!"
      ECHO Using Visual Studio %%Y %%P at: !LOCAL_VS_PATH!
      EXIT /B 0
    )
  )
  REM FOR
  SET "LOCAL_BT_PATH=%PROGRAMFILES_PATH%\Microsoft Visual Studio\%%Y\BuildTools\VC"
  IF EXIST "!LOCAL_BT_PATH!" (
    SET "BUILDTOOLS_PATH=!LOCAL_BT_PATH!"
    ECHO Using BuildTools %%Y at: !LOCAL_BT_PATH!
    EXIT /B 0
  )
  REM BT
)
REM FOR
EXIT /B 1

:function_has_64bit
FOR /F %%G IN ('dir "%BUILDTOOLS_PATH%\Tools\MSVC\%PARAM_VCVARSVER%*" /b /ad-h /o-n') DO (
  set "LOCAL_64_CL=%BUILDTOOLS_PATH%\Tools\MSVC\%%G\bin\Hostx64\x64\cl.exe"
  if EXIST "!LOCAL_64_CL!" (
    EXIT /B 0
  )
)
EXIT /B 1
