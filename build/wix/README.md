To build the windows installer package
==========================================

Mixxx installation package is built with Wix Toolset v3.9+
All the build logic is called from Sconscript, which calls Wix Toolset executables
to build a multilingual MSI package for Mixxx.

`scons mixxx makerelease`

To test your multilingual package
=================================

`msiexec /i mixxx-full-name.msi ProductLanguage=xxxx`

where xxxx is a LCID in decimal notation taken from http://www.science.co.il/Language/Locale-codes.asp and available as a translation in build/wix/Localization
