@echo off

REM set this to the folder where you build the dependencies
set WINLIB_PATH64D=C:\mixxx-buildserver\2.0-x64-debug-minimal
set WINLIB_PATH64R=C:\mixxx-buildserver\2.0-x64-release-minimal
set WINLIB_PATH32D=C:\mixxx-buildserver\2.0-x86-debug-minimal
set WINLIB_PATH32R=C:\mixxx-buildserver\2.0-x86-release-minimal

REM XP Compatibility requires the v7.1A SDK
set MSSDK_DIR="c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A"

IF "%2"=="" (
echo "Usage: build_mixxx.bat <bitwidth> <buildtype>"
echo.
echo "For example : .\build.bat x64 release"
exit /B 1
)

set ARCHITECTURE=%1
set BUILD_TYPE=%2

if "%ARCHITECTURE%" == "x86" set res32=T
if "%ARCHITECTURE%" == "x32" set res32=T


if "%res32%" == "T" (
  set TARGET_MACHINE=x86
  set VCVARS_ARCH=x86
  set DISTDIR=dist32
  if "%BUILD_TYPE%" == "release" (
    set WINLIB_PATH=%WINLIB_PATH32R%
  ) else (
    set WINLIB_PATH=%WINLIB_PATH32D%
  )
  echo "****************************"
  echo "** Building 32 bits Mixxx **"
  echo "****************************"
) else (
  set TARGET_MACHINE=amd64
  set VCVARS_ARCH=x86_amd64
  set DISTDIR=dist64
  if "%BUILD_TYPE%" == "release" (
    set WINLIB_PATH=%WINLIB_PATH64R%
  ) else (
    set WINLIB_PATH=%WINLIB_PATH64D%
  )
  echo "****************************"
  echo "** Building 64 bits Mixxx **"
  echo "****************************"
)

REM Clean up after old builds.
REM del /q /f *.exe *.msi 2>NUL
REM rmdir /s /q %DISTDIR%

call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %VCVARS_ARCH%

REM set multiprocessor to build faster (together with scons -j4)
set CL=/MP /FS /EHsc

scons mixxx makerelease verbose=0 winlib=%WINLIB_PATH% qtdir=%WINLIB_PATH%\build\qt-everywhere-opensource-src-4.8.6 hss1394=1 mediafoundation=1 opus=1 localecompare=1 optimize=portable build=%BUILD_TYPE% machine=%TARGET_MACHINE% toolchain=msvs virtualize=0 test=1 qt_sqlite_plugin=0 mssdk_dir=%MSSDK_DIR% build_number_in_title_bar=0 bundle_pdbs=1
