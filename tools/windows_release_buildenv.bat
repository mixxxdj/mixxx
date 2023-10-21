@ECHO OFF
set BUILDENV_RELEASE=TRUE && call "%~dp0windows_buildenv.bat" %* & set BUILDENV_RELEASE=
