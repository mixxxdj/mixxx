REM Clean up after old builds.
del /q /f *.exe
rmdir /s /q dist32
rmdir /s /q dist64

REM XP Compatibility requires the v7.1A SDK
set MSSDK_DIR="c:\Program Files (x86)\Microsoft SDKs\Windows\v7.1A"

REM this can be either release or debug. For development you want to use debug
set BUILD_TYPE=release

REM This determines if you build a 32bit or 64bit version of mixxx. 
REM 32bit = i386, 64bit = amd64
set ARCHITECTURE=i386

REM set this to the folder where you build the dependencies
set WINLIB_PATH= "C:\mixxx\environments\prototype"

if "%ARCHITECTURE%" == "i386" (
  set TARGET_MACHINE=x86
  set VCVARS_ARCH=x86
) else ( 
  set TARGET_MACHINE=amd64
  set VCVARS_ARCH=x86_amd64
)

call "c:\Program Files (x86)\Microsoft Visual Studio 12.0\VC\vcvarsall.bat" %VCVARS_ARCH%

scons winlib=C:\mixxx\environments\prototype qtdir=C:\mixxx\environments\prototype\build\qt-everywhere-opensource-src-4.8.6 hss1394=1 mediafoundation=1 opus=0 build=%BUILD_TYPE% machine=%TARGET_MACHINE% toolchain=msvs virtualize=0 test=1 sqlitedll=0 mssdk_dir=%MSSDK_DIR% force32=1