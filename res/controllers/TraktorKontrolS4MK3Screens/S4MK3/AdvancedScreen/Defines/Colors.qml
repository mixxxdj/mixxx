import QtQuick 2.15

QtObject {

    function rgba(r,g,b,a) { return Qt.rgba(  neutralizer(r)/255. ,  neutralizer(g)/255. ,  neutralizer(b)/255. , neutralizer(a)/255. ) }

  // this categorizes any rgb value to multiples of 8 for each channel to avoid unbalanced colors on the display (r5-g6-b5 bit)
  // function neutralizer(value) { if(value%8 > 4) { return  value - value%8 + 8} else { return  value - value%8 }}
    function neutralizer(value) { return value}

    property variant colorBlack: rgba (0, 0, 0, 255)
    property variant colorBlack94: rgba (0, 0, 0, 240)
    property variant colorBlack88: rgba (0, 0, 0, 224)
    property variant colorBlack85: rgba (0, 0, 0, 217)
    property variant colorBlack81: rgba (0, 0, 0, 207)
    property variant colorBlack78: rgba (0, 0, 0, 199)
    property variant colorBlack75: rgba (0, 0, 0, 191)
    property variant colorBlack69: rgba (0, 0, 0, 176)
    property variant colorBlack66: rgba (0, 0, 0, 168)
    property variant colorBlack63: rgba (0, 0, 0, 161)
    property variant colorBlack60: rgba (0, 0, 0, 153) // from 59 - 61%
    property variant colorBlack56: rgba (0, 0, 0, 143) //
    property variant colorBlack53: rgba (0, 0, 0, 135) // from 49 - 51%
    property variant colorBlack50: rgba (0, 0, 0, 128) // from 49 - 51%
    property variant colorBlack47: rgba (0, 0, 0, 120) // from 46 - 48%
    property variant colorBlack44: rgba (0, 0, 0, 112) // from 43 - 45%
    property variant colorBlack41: rgba (0, 0, 0, 105) // from 40 - 42%
    property variant colorBlack38: rgba (0, 0, 0, 97) // from 37 - 39%
    property variant colorBlack35: rgba (0, 0, 0, 89) // from 33 - 36%
    property variant colorBlack31: rgba (0, 0, 0, 79) // from 30 - 32%
    property variant colorBlack28: rgba (0, 0, 0, 71) // from 27 - 29%
    property variant colorBlack25: rgba (0, 0, 0, 64) // from 24 - 26%
    property variant colorBlack22: rgba (0, 0, 0, 56) // from 21 - 23%
    property variant colorBlack19: rgba (0, 0, 0, 51) // from 18 - 20%
    property variant colorBlack16: rgba (0, 0, 0, 41) // from 15 - 17%
    property variant colorBlack12: rgba (0, 0, 0, 31) // from 11 - 13%
    property variant colorBlack09: rgba (0, 0, 0, 23) // from 8 - 10%
    property variant colorBlack0: rgba (0, 0, 0, 0)

    property variant colorWhite: rgba (255, 255, 255, 255)

    property variant colorWhite75: rgba (255, 255, 255, 191)
    property variant colorWhite85: rgba (255, 255, 255, 217)

  // property variant colorWhite60:                rgba (255, 255, 255, 153) // from 59 - 61%
    property variant colorWhite50: rgba (255, 255, 255, 128) // from 49 - 51%
  // property variant colorWhite47:                rgba (255, 255, 255, 120) // from 46 - 48%
  // property variant colorWhite44:                rgba (255, 255, 255, 112) // from 43 - 45%
    property variant colorWhite41: rgba (255, 255, 255, 105) // from 40 - 42%
  // property variant colorWhite38:                rgba (255, 255, 255, 97) // from 37 - 39%
    property variant colorWhite35: rgba (255, 255, 255, 89) // from 33 - 36%
  // property variant colorWhite31:                rgba (255, 255, 255, 79) // from 30 - 32%
    property variant colorWhite28: rgba (255, 255, 255, 71) // from 27 - 29%
    property variant colorWhite25: rgba (255, 255, 255, 64) // from 24 - 26%
    property variant colorWhite22: rgba (255, 255, 255, 56) // from 21 - 23%
    property variant colorWhite19: rgba (255, 255, 255, 51) // from 18 - 20%
    property variant colorWhite16: rgba (255, 255, 255, 41) // from 15 - 17%
    property variant colorWhite12: rgba (255, 255, 255, 31) // from 11 - 13%
    property variant colorWhite09: rgba (255, 255, 255, 23) // from 8 - 10%
  // property variant colorWhite06:                rgba (255, 255, 255, 15) // from 5 - 7%
  // property variant colorWhite03:                rgba (255, 255, 255, 8) // from 2 - 4%

    property variant colorGrey232: rgba (232, 232, 232, 255)
    property variant colorGrey216: rgba (216, 216, 216, 255)
    property variant colorGrey208: rgba (208, 208, 208, 255)
    property variant colorGrey200: rgba (200, 200, 200, 255)
    property variant colorGrey192: rgba (192, 192, 192, 255)
    property variant colorGrey152: rgba (152, 152, 152, 255)
    property variant colorGrey128: rgba (128, 128, 128, 255)
    property variant colorGrey120: rgba (120, 120, 120, 255)
    property variant colorGrey112: rgba (112, 112, 112, 255)
    property variant colorGrey104: rgba (104, 104, 104, 255)
    property variant colorGrey96: rgba (96, 96, 96, 255)
    property variant colorGrey88: rgba (88, 88, 88, 255)
    property variant colorGrey80: rgba (80, 80, 80, 255)
    property variant colorGrey72: rgba (72, 72, 72, 255)
    property variant colorGrey64: rgba (64, 64, 64, 255)
    property variant colorGrey56: rgba (56, 56, 56, 255)
    property variant colorGrey48: rgba (48, 48, 48, 255)
    property variant colorGrey40: rgba (40, 40, 40, 255)
    property variant colorGrey32: rgba (32, 32, 32, 255)
    property variant colorGrey24: rgba (24, 24, 24, 255)
    property variant colorGrey16: rgba (16, 16, 16, 255)
    property variant colorGrey08: rgba (08, 08, 08, 255)

    property variant cueColors: [
                                 red,
                                 darkOrange,
                                 lightOrange,
                                 colorWhite,
                                 yellow,
                                 lime,
                                 green,
                                 mint,
                                 cyan,
                                 turquoise,
                                 blue,
                                 plum,
                                 violet,
                                 purple,
                                 magenta,
                                 fuchsia,
                                 warmYellow
    ]

    property variant cueColorsDark: [
                                     Qt.darker(red, 0.15),
                                     Qt.darker(darkOrange, 0.15),
                                     Qt.darker(lightOrange, 0.15),
                                     Qt.darker(colorWhite, 0.15),
                                     Qt.darker(yellow, 0.15),
                                     Qt.darker(lime, 0.15),
                                     Qt.darker(green, 0.15),
                                     Qt.darker(mint, 0.15),
                                     Qt.darker(cyan, 0.15),
                                     Qt.darker(turquoise, 0.15),
                                     Qt.darker(blue, 0.15),
                                     Qt.darker(plum, 0.15),
                                     Qt.darker(violet, 0.15),
                                     Qt.darker(purple, 0.15),
                                     Qt.darker(magenta, 0.15),
                                     Qt.darker(fuchsia, 0.15),
                                     Qt.darker(warmYellow, 0.15)
    ]

    property variant colorOrange: rgba(208, 104, 0, 255) // FX Selection; FX Faders etc
    property variant colorOrangeDimmed: rgba(96, 48, 0, 255)

    property variant colorRed: rgba(255, 0, 0, 255)
    property variant colorRed70: rgba(185, 6, 6, 255)

  // Playmarker
    property variant colorRedPlaymarker: rgba(255, 0, 0, 255)
    property variant colorRedPlaymarker75: rgba(255, 56, 26, 191)
    property variant colorRedPlaymarker06: rgba(255, 56, 26, 31)

  // Playmarker
    property variant colorBluePlaymarker: rgba(96, 184, 192, 255) //rgba(136, 224, 232, 255)

    property variant colorGreen: rgba(0, 255, 0, 255)
    property variant colorGreen50: rgba(0, 255, 0, 128)
    property variant colorGreen12: rgba(0, 255, 0, 31) // used for loop bg (in WaveformCues.qml)
    property variant colorGreenLoopOverlay: rgba(96, 192, 128, 16)
    property variant colorGreenMint: 		rgba(0, 219, 138, 255)

    property variant colorGreen08: rgba(0, 255, 0, 20)
    property variant colorGreen50Full: rgba(0, 51, 0, 255)

    property variant colorGreenGreyMix: rgba(139, 240, 139, 82)

  // font colors
    property variant colorFontsListBrowser: colorGrey72
    property variant colorFontsListFx: colorGrey56
    property variant colorFontBrowserHeader: colorGrey88
    property variant colorFontFxHeader: colorGrey80 // also for FX header, FX select buttons

  // headers & footers backgrounds
    property variant colorBgEmpty: colorGrey16 // also for empty decks & Footer Small (used to be colorGrey08)
    property variant colorBrowserHeader: colorGrey24
    property variant colorFxHeaderBg: colorGrey16 // also for large footer; fx overlay tabs
    property variant colorFxHeaderLightBg: colorGrey24

    property variant colorProgressBg: colorGrey32
    property variant colorProgressBgLight: colorGrey48
    property variant colorDivider: colorGrey40

    property variant colorIndicatorBg: rgba(20, 20, 20, 255)
    property variant colorIndicatorBg2: rgba(31, 31, 31, 255)

    property variant colorIndicatorLevelGrey: rgba(51, 51, 51, 255)
    property variant colorIndicatorLevelOrange: rgba(247, 143, 30, 255)

    property variant colorCenterOverlayHeadline: colorGrey88

// blue
    property variant colorDeckBlueBright: rgba(0, 136, 184, 255)
    property variant colorDeckBlueDark: rgba(0, 64, 88, 255)
    property variant colorDeckBlueBright20: rgba(0, 174, 239, 51)
    property variant colorDeckBlueBright50Full: rgba(0, 87, 120, 255)
    property variant colorDeckBlueBright12Full: rgba(0, 8, 10, 255) //rgba(0, 23, 31, 255)
    property variant colorBrowserBlueBright: rgba(0, 187, 255, 255)
    property variant colorBrowserBlueBright56Full:rgba(0, 114, 143, 255)

    property color footerBackgroundBlue: "#011f26"

  // fx Select overlay colors
    property variant fxSelectHeaderTextRGB: rgba( 96, 96, 96, 255)
    property variant fxSelectHeaderNormalRGB: rgba( 32, 32, 32, 255)
    property variant fxSelectHeaderNormalBorderRGB: rgba( 32, 32, 32, 255)
    property variant fxSelectHeaderHighlightRGB: rgba( 64, 64, 48, 255)
    property variant fxSelectHeaderHighlightBorderRGB: rgba(128, 128, 48, 255)

  // 16 Colors Palette (Bright)
    property variant color01Bright: rgba (255, 0, 0, 255)
    property variant color02Bright: rgba (255, 16, 16, 255)
    property variant color03Bright: rgba (255, 120, 0, 255)
    property variant color04Bright: rgba (255, 184, 0, 255)
    property variant color05Bright: rgba (255, 255, 0, 255)
    property variant color06Bright: rgba (144, 255, 0, 255)
    property variant color07Bright: rgba ( 40, 255, 40, 255)
    property variant color08Bright: rgba ( 0, 208, 128, 255)
    property variant color09Bright: rgba ( 0, 184, 232, 255)
    property variant color10Bright: rgba ( 0, 120, 255, 255)
    property variant color11Bright: rgba ( 0, 72, 255, 255)
    property variant color12Bright: rgba (128, 0, 255, 255)
    property variant color13Bright: rgba (160, 0, 200, 255)
    property variant color14Bright: rgba (240, 0, 200, 255)
    property variant color15Bright: rgba (255, 0, 120, 255)
    property variant color16Bright: rgba (248, 8, 64, 255)

  // 16 Colors Palette (Mid)
    property variant color01Mid: rgba (112, 8, 8, 255)
    property variant color02Mid: rgba (112, 24, 8, 255)
    property variant color03Mid: rgba (112, 56, 0, 255)
    property variant color04Mid: rgba (112, 80, 0, 255)
    property variant color05Mid: rgba (96, 96, 0, 255)
    property variant color06Mid: rgba (56, 96, 0, 255)
    property variant color07Mid: rgba (8, 96, 8, 255)
    property variant color08Mid: rgba (0, 90, 60, 255)
    property variant color09Mid: rgba (0, 77, 77, 255)
    property variant color10Mid: rgba (0, 84, 108, 255)
    property variant color11Mid: rgba (32, 56, 112, 255)
    property variant color12Mid: rgba (72, 32, 120, 255)
    property variant color13Mid: rgba (80, 24, 96, 255)
    property variant color14Mid: rgba (111, 12, 149, 255)
    property variant color15Mid: rgba (122, 0, 122, 255)
    property variant color16Mid: rgba (130, 1, 43, 255)

  // 16 Colors Palette (Dark)
    property variant color01Dark: rgba (16, 0, 0, 255)
    property variant color02Dark: rgba (16, 8, 0, 255)
    property variant color03Dark: rgba (16, 8, 0, 255)
    property variant color04Dark: rgba (16, 16, 0, 255)
    property variant color05Dark: rgba (16, 16, 0, 255)
    property variant color06Dark: rgba (8, 16, 0, 255)
    property variant color07Dark: rgba (8, 16, 8, 255)
    property variant color08Dark: rgba (0, 16, 8, 255)
    property variant color09Dark: rgba (0, 8, 16, 255)
    property variant color10Dark: rgba (0, 8, 16, 255)
    property variant color11Dark: rgba (0, 0, 16, 255)
    property variant color12Dark: rgba (8, 0, 16, 255)
    property variant color13Dark: rgba (8, 0, 16, 255)
    property variant color14Dark: rgba (16, 0, 16, 255)
    property variant color15Dark: rgba (16, 0, 8, 255)
    property variant color16Dark: rgba (16, 0, 8, 255)

  //--------------------------------------------------------------------------------------------------------------------

  //  Browser

  //--------------------------------------------------------------------------------------------------------------------

    property variant browser:
        QtObject {
        property color prelisten: rgba(223, 178, 30, 255)
        property color prevPlayed: rgba(32, 32, 32, 255)
    }

  //--------------------------------------------------------------------------------------------------------------------

  //  Hotcues

  //--------------------------------------------------------------------------------------------------------------------

    property variant hotcue:
        QtObject {
        property color grid: colorWhite
        property color hotcue: colorDeckBlueBright
        property color fade: color03Bright
        property color load: color05Bright
        property color loop: color07Bright
        property color temp: "grey"
    }

  //--------------------------------------------------------------------------------------------------------------------

  //  Freeze & Slicer

  //--------------------------------------------------------------------------------------------------------------------

    property variant freeze:
        QtObject {
        property color box_inactive: "#199be7ef"
        property color box_active: "#ff9be7ef"
        property color marker: "#4DFFFFFF"
        property color slice_overlay: "white" // flashing rectangle
    }

    property variant slicer:
        QtObject {
        property color box_active: rgba(20,195,13,255)
        property color box_inrange: rgba(20,195,13,90)
        property color box_inactive: rgba(20,195,13,25)
        property color marker_default: rgba(20,195,13,77)
        property color marker_beat: rgba(20,195,13,150)
        property color marker_edge: box_active
    }

  //--------------------------------------------------------------------------------------------------------------------

  //  Musical Key coloring for the browser

  //--------------------------------------------------------------------------------------------------------------------
    property variant color01MusicalKey: rgba (255, 0, 0, 255) // not yet in use
    property variant color02MusicalKey: rgba (255, 64, 0, 255)
    property variant color03MusicalKey: rgba (255, 120, 0, 255) // not yet in use
    property variant color04MusicalKey: rgba (255, 200, 0, 255)
    property variant color05MusicalKey: rgba (255, 255, 0, 255)
    property variant color06MusicalKey: rgba (210, 255, 0, 255) // not yet in use
    property variant color07MusicalKey: rgba ( 0, 255, 0, 255)
    property variant color08MusicalKey: rgba ( 0, 255, 128, 255)
  //property variant color09MusicalKey: rgba (  0, 200, 232, 255)
    property variant color09MusicalKey: colorDeckBlueBright // use the same color as for the browser selection
    property variant color10MusicalKey: rgba ( 0, 100, 255, 255)
    property variant color11MusicalKey: rgba ( 0, 40, 255, 255)
    property variant color12MusicalKey: rgba (128, 0, 255, 255)
    property variant color13MusicalKey: rgba (160, 0, 200, 255) // not yet in use
    property variant color14MusicalKey: rgba (240, 0, 200, 255)
    property variant color15MusicalKey: rgba (255, 0, 120, 255) // not yet in use
    property variant color16MusicalKey: rgba (248, 8, 64, 255)

    property variant color01MusicalKey2: rgba (255, 0, 0, 120) // not yet in use
    property variant color02MusicalKey2: rgba (255, 64, 0, 120)
    property variant color03MusicalKey2: rgba (255, 120, 0, 120) // not yet in use
    property variant color04MusicalKey2: rgba (255, 200, 0, 120)
    property variant color05MusicalKey2: rgba (255, 255, 0, 120)
    property variant color06MusicalKey2: rgba (210, 255, 0, 120) // not yet in use
    property variant color07MusicalKey2: rgba ( 0, 255, 0, 120)
    property variant color08MusicalKey2: rgba ( 0, 255, 128, 120)
  //property variant color09MusicalKey2: rgba (  0, 200, 232, 120)
    property variant color09MusicalKey2: colorDeckBlueBright // use the same color as for the browser selection
    property variant color10MusicalKey2: rgba ( 0, 100, 255, 120)
    property variant color11MusicalKey2: rgba ( 0, 40, 255, 120)
    property variant color12MusicalKey2: rgba (128, 0, 255, 120)
    property variant color13MusicalKey2: rgba (160, 0, 200, 120) // not yet in use
    property variant color14MusicalKey2: rgba (240, 0, 200, 120)
    property variant color15MusicalKey2: rgba (255, 0, 120, 120) // not yet in use
    property variant color16MusicalKey2: rgba (248, 8, 64, 120)

  // 16 Colors Palette (Bright)
    property variant color01Bright2: rgba (255, 0, 0, 120)
    property variant color02Bright2: rgba (255, 16, 16, 120)
    property variant color03Bright2: rgba (255, 120, 0, 120)
    property variant color04Bright2: rgba (255, 184, 0, 120)
    property variant color05Bright2: rgba (255, 255, 0, 120)
    property variant color06Bright2: rgba (144, 255, 0, 120)
    property variant color07Bright2: rgba ( 40, 255, 40, 120)
    property variant color08Bright2: rgba ( 0, 208, 128, 120)
    property variant color09Bright2: rgba ( 0, 184, 232, 120)
    property variant color10Bright2: rgba ( 0, 120, 255, 120)
    property variant color11Bright2: rgba ( 0, 72, 255, 120)
    property variant color12Bright2: rgba (128, 0, 255, 120)
    property variant color13Bright2: rgba (160, 0, 200, 120)
    property variant color14Bright2: rgba (240, 0, 200, 120)
    property variant color15Bright2: rgba (255, 0, 120, 120)
    property variant color16Bright2: rgba (248, 8, 64, 120)

    property variant musicalKeyColors: [
                                        'grey',               //0 No key
                                        color15Bright,        //1   -11 c
                                        color06Bright,        //2   -4  c#, db
                                        color11MusicalKey,    //3   -13 d
                                        color03Bright,        //4   -6  d#, eb
                                        color09MusicalKey,    //5   -16 e
                                        color01Bright,        //6   -9  f
                                        color07MusicalKey,    //7   -2  f#, gb
                                        color13Bright,        //8   -12 g
                                        color04MusicalKey,    //9   -5  g#, ab
                                        color10MusicalKey,    //10   -15 a
                                        color02MusicalKey,    //11  -7  a#, bb
                                        color08MusicalKey,    //12  -1  b
                                        color03Bright,        //13  -6  cm
                                        color09MusicalKey,    //14  -16 c#m, dbm
                                        color01Bright,        //15  -9  dm
                                        color07MusicalKey,    //16  -2  d#m, ebm
                                        color13Bright,        //17  -12 em
                                        color04MusicalKey,    //18  -5  fm
                                        color10MusicalKey,    //19  -15 f#m, gbm
                                        color02MusicalKey,    //20  -7  gm
                                        color08MusicalKey,    //21  -1  g#m, abm
                                        color15Bright,        //22  -11 am
                                        color06Bright,        //23  -4  a#m, bbm
                                        color11MusicalKey     //24  -13 bm
    ]

    property variant musicalKeyColorsDark: [
                                            'grey',                             //0  No key
                                            Qt.darker(color15Bright, 5),        //1   -11 c
                                            Qt.darker(color06Bright, 5),        //2   -4  c#, db
                                            Qt.darker(color11MusicalKey, 5),    //3   -13 d
                                            Qt.darker(color03Bright, 5),        //4   -6  d#, eb
                                            Qt.darker(color09MusicalKey, 5),    //5   -16 e
                                            Qt.darker(color01Bright, 5),        //6   -9  f
                                            Qt.darker(color07MusicalKey, 5),    //7   -2  f#, gb
                                            Qt.darker(color13Bright, 5),        //8   -12 g
                                            Qt.darker(color04MusicalKey, 5),    //9   -5  g#, ab
                                            Qt.darker(color10MusicalKey, 5),    //10   -15 a
                                            Qt.darker(color02MusicalKey, 5),    //11  -7  a#, bb
                                            Qt.darker(color08MusicalKey, 5),    //12  -1  b
                                            Qt.darker(color03Bright, 5),        //13  -6  cm
                                            Qt.darker(color09MusicalKey, 5),    //14  -16 c#m, dbm
                                            Qt.darker(color01Bright, 5),        //15  -9  dm
                                            Qt.darker(color07MusicalKey, 5),    //16  -2  d#m, ebm
                                            Qt.darker(color13Bright, 5),        //17  -12 em
                                            Qt.darker(color04MusicalKey, 5),    //18  -5  fm
                                            Qt.darker(color10MusicalKey, 5),    //19  -15 f#m, gbm
                                            Qt.darker(color02MusicalKey, 5),    //20  -7  gm
                                            Qt.darker(color08MusicalKey, 5),    //21  -1  g#m, abm
                                            Qt.darker(color15Bright, 5),        //22  -11 am
                                            Qt.darker(color06Bright, 5),        //23  -4  a#m, bbm
                                            Qt.darker(color11MusicalKey, 5)     //24  -13 bm
    ]

  //--------------------------------------------------------------------------------------------------------------------

  //  Waveform coloring

  //--------------------------------------------------------------------------------------------------------------------

    property color defaultBackground: "black"
    property color defaultTextColor: "white"
    property color loopActiveColor: rgba(0,255,70,255)
    property color loopFlashColor: rgba ( 20, 235, 165, 120)

    property color loopActiveDimmedColor: rgba(0,255,70,190)
    property color grayBackground: "#ff333333"

    property variant colorDeckBrightGrey: rgba (85, 85, 85, 255)
    property variant colorDeckGrey: rgba (70, 70, 70, 255)
    property variant colorDeckDarkGrey: rgba (40, 40, 40, 255)

    property variant colorDeckOrangeBright: rgba (253, 186, 16, 255)

    property variant colorQuantizeOn: rgba ( 20, 255, 255, 170)
    property variant colorQuantizeOff: Qt.darker(colorQuantizeOn, 0.7)

    property color red: "#ff0000"
    property color darkOrange: "#ff8c00"
    property color lightOrange: "#fccf3e"
    property color warmYellow: "#f9d71c"
    property color yellow: "#ffff00"
    property color lime: "#effd5f"
    property color green: "#00FF00"
    property color mint: "#98ff98"
    property color cyan: "#00FFFF"
    property color turquoise: "#40e0d0"
    property color blue: "#0080FF"
    property color plum: "#ff7eff"
    property color violet: "#ee82ee"
    property color purple: "#9f00c5"
    property color magenta: "#ff6fff"
    property color fuchsia: "#ff0080"
    property color white: "#ff0080"
    property color phaseColor: 			 "#90550C"

  //--------------------------------------------------------------------------------------------------------------------

  //  Waveform coloring

  //--------------------------------------------------------------------------------------------------------------------

    property variant low1: settings.low1
    property variant low2: settings.low2
    property variant mid1: settings.mid1
    property variant mid2: settings.mid2
    property variant high1: settings.high1
    property variant high2: settings.high2

    function getWaveformColors(colorId) {
        if (colorId <= 17) {
            return waveformColorsMap[colorId];
        }

        return waveformColorsMap[0];
    }

    function palette(brightness, colorId) {
        if ( brightness >= 0.666 && brightness <= 1.0 ) { // bright color
            switch(colorId) {
                case 0: return defaultBackground // default color for this palette!
                case 1: return color01Bright
                case 2: return color02Bright
                case 3: return color03Bright
                case 4: return color04Bright
                case 5: return color05Bright
                case 6: return color06Bright
                case 7: return color07Bright
                case 8: return color08Bright
                case 9: return color09Bright
                case 10: return color10Bright
                case 11: return color11Bright
                case 12: return color12Bright
                case 13: return color13Bright
                case 14: return color14Bright
                case 15: return color15Bright
                case 16: return color16Bright
                case 17: return "grey"
                case 18: return colorGrey232
            }
        } else if ( brightness >= 0.333 && brightness < 0.666 ) { // mid color
            switch(colorId) {
                case 0: return defaultBackground // default color for this palette!
                case 1: return color01Mid
                case 2: return color02Mid
                case 3: return color03Mid
                case 4: return color04Mid
                case 5: return color05Mid
                case 6: return color06Mid
                case 7: return color07Mid
                case 8: return color08Mid
                case 9: return color09Mid
                case 10: return color10Mid
                case 11: return color11Mid
                case 12: return color12Mid
                case 13: return color13Mid
                case 14: return color14Mid
                case 15: return color15Mid
                case 16: return color16Mid
                case 17: return "grey"
                case 18: return colorGrey232
            }
        } else if ( brightness >= 0 && brightness < 0.333 ) { // dimmed color
            switch(colorId) {
                case 0: return defaultBackground // default color for this palette!
                case 1: return color01Dark
                case 2: return color02Dark
                case 3: return color03Dark
                case 4: return color04Dark
                case 5: return color05Dark
                case 6: return color06Dark
                case 7: return color07Dark
                case 8: return color08Dark
                case 9: return color09Dark
                case 10: return color10Dark
                case 11: return color11Dark
                case 12: return color12Dark
                case 13: return color13Dark
                case 14: return color14Dark
                case 15: return color15Dark
                case 16: return color16Dark
                case 17: return "grey"
                case 18: return colorGrey232
            }
        } else if ( brightness < 0) { // color Off
            return defaultBackground;
        }
        return defaultBackground;  // default color if no palette is set
    }
}
