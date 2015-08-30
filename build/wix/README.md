To build hte windows installer package
==========================================

install & configure Wix toolset

once done :

1. Build Mixxx
2. Package (example given for x64 platform)

First, generate file list of dist subdirectories
    ...\mixxx\build\wix>heat dir ..\..\dist64\imageformats -nologo -sfrag -suid -ag -srd -cg imageformatsComp -dr imageformatsDir -out imageformats.wxs -sw5150

Then, compile    
    ...\mixxx\build\wix>candle -dPlatform=x64 -arch x64 mixxx.wxs imageformats.wxs

And finally, link    
    ...\mixxx\build\wix>light -sw1076 -b ..\..\dist64 -b ..\..\dist64\imageformats -ext WixUIExtension -cultures:en-us -loc Localization\en-us\mixxx_en-us.wxl -out mixxx.msi mixxx.wixobj imageformats.wixobj
