To build the windows installer package
==========================================

install & configure Wix toolset

once done :

1. Build Mixxx
2. Package, calling build.bat <bitwidth>

For example, to build 64 bits package:
.\build.bat 64

or to build 32 bits package:
.\build.bat 32

To test your multilingual package
=================================

`msiexec /i mixxx-full-name.msi ProductLanguage=xxxx`

where xxxx is a LCID in decimal notation taken from http://www.science.co.il/Language/Locale-codes.asp and available as a translation in build/wix/Localization
