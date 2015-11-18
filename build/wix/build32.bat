@echo off

REM set this to the folder where you build the dependencies
set WINLIB_PATH=D:\mixxx-buildserver32

echo "*** Cleaning"
del *.wixobj
del *.wixpdb
del /Q subdirs\*.*

echo "*** Building intermediate files"

FOR %%d IN (controllers,fonts,imageformats,keyboard,plugins,skins,translations) DO (
  "%WIX%"\bin\heat.exe dir ..\..\dist32\%%d -nologo -sfrag -suid -ag -srd -cg %%dComp -dr %%dDir -out subdirs\%%d.wxs -sw5150 -var var.%%dVar  
  "%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x86 -d%%dVar=..\..\dist32\%%d -arch x86 -out subdirs\%%d.wixobj subdirs\%%d.wxs
)

SET promo=no

IF EXIST ..\..\dist32\promo (
  SET promo=yes
  "%WIX%"\bin\heat.exe dir ..\..\dist32\promo -nologo -sfrag -suid -ag -srd -cg promoComp -dr promoDir -out subdirs\promo.wxs -sw5150 -var var.promoVar  
  "%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x86 -dpromoVar=..\..\dist32\promo -arch x86 -out subdirs\promo.wixobj subdirs\promo.wxs
)

REM Harvest main DLL from install dir
"%WIX%"\bin\heat.exe dir ..\..\dist32 -nologo -sfrag -suid -ag -srd -cg mainDLLCompGroup -dr INSTALLDIR -out subdirs\mainDLL.wxs -sw5150 -var var.SourceDir -t only-dll.xslt
"%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x86 -dSourceDir=..\..\dist32 -arch x86 -out subdirs\mainDLL.wixobj subdirs\mainDLL.wxs

"%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x86 -arch x86 warningDlg.wxs
"%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x86 -dPromo=%promo% -arch x86 mixxx.wxs

FOR %%G IN (Localization\*.wxl) DO (
  REM skip 19 chars (Localization\mixxx_), keep until end -4 char (.wxl)
  set _locfile=%%G
  set _locale=!_locfile:~19,-4!
  echo "*** Building package for locale !_locale!"
  "%WIX%"\bin\light.exe -nologo -sw1076 -ext WixUIExtension -cultures:!_locale! -loc %%G -out mixxx-32-!_locale!.msi *.wixobj subdirs\*.wixobj
)