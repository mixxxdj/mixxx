@echo off

for /F %%d IN (subdirs.txt) DO (
  heat dir ..\..\dist32\%%d -nologo -sfrag -suid -ag -srd -cg %%dComp -dr %%dDir -out %%d.wxs -sw5150 -var var.%%dVar  
  candle -nologo -dPlatform=x86 -d%%dVar=..\..\dist32\%%d -arch x86 %%d.wxs
)

candle -nologo -dPlatform=x86 -arch x86 mixxx.wxs

light -nologo -sw1076 -ext WixUIExtension -cultures:en-us -loc Localization\en-us\mixxx_en-us.wxl -out mixxx-32.msi *.wixobj