@echo off

for /F %%d IN (subdirs.txt) DO (
  "%WIX%"\bin\heat.exe dir ..\..\dist64\%%d -nologo -sfrag -suid -ag -srd -cg %%dComp -dr %%dDir -out %%d.wxs -sw5150 -var var.%%dVar  
  "%WIX%"\bin\candle.exe -nologo -dPlatform=x64 -d%%dVar=..\..\dist64\%%d -arch x64 %%d.wxs
)

"%WIX%"\bin\candle.exe -nologo -dPlatform=x64 -arch x64 mixxx.wxs

"%WIX%"\bin\light.exe -nologo -sw1076 -ext WixUIExtension -cultures:en-us -loc Localization\en-us\mixxx_en-us.wxl -out mixxx-64.msi *.wixobj