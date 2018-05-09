To build the windows installer package
==========================================

Mixxx installation package is built with Wix Toolset v3.9+
All the build logic is called from Sconscript, which calls Wix Toolset executables
to first build a multilingual MSI package for Mixxx, then create an executable
bundle embedding Visual Studio C++ redistributables and Mixxx's MSI.

`scons mixxx makerelease`

To test your multilingual package
=================================

* MSI package: `msiexec /i mixxx-full-name.msi ProductLanguage=xxxx`
* EXE bundle: `mixxx-full-name.exe -lang xxxx`

where xxxx is a LCID in decimal notation taken from http://www.science.co.il/Language/Locale-codes.asp and available as a translation in build/wix/Localization

Bootstrapper (bundle) command line options
==========================================
```
-q, -quiet, -s, -silent = silent install
-passive = progress bar only install
-norestart = suppress any restarts
-forcerestart = restart no matter what (I don't know why this is still around)
-promptrestart = prompt if a restart is required (default)
-layout = create a local image of the bootstrapper (i.e. download files so they can be burned to DVD)
-l, -log = log to a specific file (default is controlled by bundle developer)
-uninstall = uninstall
-repair = repair (or install if not installed)
-package,-update = install (default if no -uninstall or -repair)
-lang xxxx = apply language with lcid xxxx to bootstrapper
```
