To build hte windows installer package
==========================================

install & configure Wix toolset

once done :

    ...\mixxx\build\wix>candle -dPlatform=x64 mixxx.wxs

    ...\mixxx\build\wix>light -ext WixUIExtension -cultures:en-us -loc Localization\en-us\mixxx_en-us.wxl -out maxxx.msi mixxx.wixobj
