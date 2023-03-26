@ECHO OFF
set DEFINED BUILDENV_RELEASE=TRUE && call "%~dp0windows_buildenv.bat" %* & set DEFINED BUILDENV_RELEASE=
