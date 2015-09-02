@echo off

for /F %%d IN (subdirs.txt) DO (
  "%WIX%"\bin\heat.exe dir ..\..\dist32\%%d -nologo -sfrag -suid -ag -srd -cg %%dComp -dr %%dDir -out %%d.wxs -sw5150 -var var.%%dVar  
  "%WIX%"\bin\candle.exe -nologo -dPlatform=x86 -d%%dVar=..\..\dist32\%%d -arch x86 %%d.wxs
)

"%WIX%"\bin\candle.exe -nologo -dPlatform=x86 -arch x86 warningDlg.wxs
"%WIX%"\bin\candle.exe -nologo -dPlatform=x86 -arch x86 mixxx.wxs

"%WIX%"\bin\light.exe -nologo -sw1076 -ext WixUIExtension -cultures:en-us -loc Localization\en-us\mixxx_en-us.wxl -out mixxx-32.msi *.wixobj