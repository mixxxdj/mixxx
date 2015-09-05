@echo off

REM set this to the folder where you build the dependencies
set WINLIB_PATH=D:\mixxx-buildserver64

del *.wixobj
del *.wixpdb
del /Q subdirs\*.*

for /F %%d IN (subdirs.txt) DO (
  "%WIX%"\bin\heat.exe dir ..\..\dist64\%%d -nologo -sfrag -suid -ag -srd -cg %%dComp -dr %%dDir -out subdirs\%%d.wxs -sw5150 -var var.%%dVar  
  "%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x64 -d%%dVar=..\..\dist64\%%d -arch x64 -out subdirs\%%d.wixobj subdirs\%%d.wxs
)

SET promo=no

IF EXIST ..\..\dist64\promo (
  SET promo=yes
  "%WIX%"\bin\heat.exe dir ..\..\dist64\promo -nologo -sfrag -suid -ag -srd -cg promoComp -dr promoDir -out subdirs\promo.wxs -sw5150 -var var.promoVar  
  "%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x64 -dpromoVar=..\..\dist64\promo -arch x64 -out subdirs\promo.wixobj subdirs\promo.wxs
)

"%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x64 -arch x64 warningDlg.wxs
"%WIX%"\bin\candle.exe -nologo -dWINLIBPATH=%WINLIB_PATH% -dPlatform=x64 -dPromo=%promo% -arch x64 mixxx.wxs

"%WIX%"\bin\light.exe -nologo -sw1076 -ext WixUIExtension -cultures:en-us -loc Localization\en-us\mixxx_en-us.wxl -out mixxx-64.msi *.wixobj subdirs\*.wixobj