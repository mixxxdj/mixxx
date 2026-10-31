import QtQuick 2.15

import '../Defines' as Defines
import '../Widgets' as Widgets

import Mixxx 1.0 as Mixxx

//--------------------------------------------------------------------------------------------------------------------
//  FX CONTROLS
//--------------------------------------------------------------------------------------------------------------------

// The FxControls are located on the top of the screen and blend in if one of the top knobs is touched/changed

Item {
    id: topLabels

    property int topMargin: 0

    property string showHideState: "hide"
    property int fxUnit: 0
    property int yPositionWhenHidden: 0 - topLabels.height - headerBlackLine.height - headerShadow.height // also hides black border & shadow
    property int yPositionWhenShown: topMargin

    readonly property color barBgColor: "black"

    property var fxModel: Mixxx.EffectsManager.visibleEffectsModel

    Defines.Colors { id: colors }
    Defines.Durations { id: durations }

    height: 40
    anchors.left: parent.left
    anchors.right: parent.right

  // dark grey background
    Rectangle {
        id: topInfoDetailsPanelDarkBg
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: topLabels.height
        color: colors.colorFxHeaderBg
    // light grey background
        Rectangle {
            id:topInfoDetailsPanelLightBg
            anchors {
                top: parent.top
                left: parent.left
            }
            height: topLabels.height
            width: 80
            color: colors.colorFxHeaderLightBg
        }
    }

//  // dividers
    Rectangle {
        id: fxInfoDivider0
        width:1;
        height:40;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 80
    }

  // dividers
    Rectangle {
        id: fxInfoDivider1
        width:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 160
        height: 40
    }

    Rectangle {
        id: fxInfoDivider2
        width:1;
        color: colors.colorDivider
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.leftMargin: 240
        height: 40
    }

  // Info Details
    Rectangle {
        id: topInfoDetailsPanel

        height: parent.height
        clip: true
        width: parent.width
        color: "transparent"

        anchors.left: parent.left
        anchors.leftMargin: 1

    // AppProperty { id: fxDryWet;      path: "app.traktor.fx." + (fxUnit + 1) + ".dry_wet"          }

        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}]`
            key: `mix`
            id: fxDryWet
            property string description: ""
            property var valueRange: ({isDiscrete: true, steps: 1})
        }

    // AppProperty { id: fxParam1;      path: "app.traktor.fx." + (fxUnit + 1) + ".parameters.1"      }
        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect1]`
            key: `meta`
            id: fxParam1
            property string description: ""
            property var valueRange: ({isDiscrete: false, steps: 0})
        }
        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect1]`
            key: `enabled`
            id: fxEnabled1
        }
        QtObject {
            id: fxKnob1name

            property Mixxx.EffectSlotProxy slot: Mixxx.EffectsManager.getEffectSlot(1, 1)
            property string description: "Description"
            property var value: "---"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
        Mixxx.ControlProxy {
            id: fxSelect1
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect1]`
            key: `loaded_effect`
            onValueChanged: {
                fxKnob1name.value = topLabels.fxModel.get(value).display
            }
        }

        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect2]`
            key: `meta`
            id: fxParam2
            property string description: ""
            property var valueRange: ({isDiscrete: false, steps: 0})
        }
        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect2]`
            key: `enabled`
            id: fxEnabled2
        }
        QtObject {
            id: fxKnob2name

            property Mixxx.EffectSlotProxy slot: Mixxx.EffectsManager.getEffectSlot(1, 2)
            property string description: "Description"
            property var value: "---"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
        Mixxx.ControlProxy {
            id: fxSelect2
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect2]`
            key: `loaded_effect`
            onValueChanged: {
                fxKnob2name.value = topLabels.fxModel.get(value).display
            }
        }

    // AppProperty { id: fxParam3;      path: "app.traktor.fx." + (fxUnit + 1) + ".parameters.3"      }
        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect3]`
            key: `meta`
            id: fxParam3
            property string description: ""
            property var valueRange: ({isDiscrete: false, steps: 0})
        }
        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect3]`
            key: `enabled`
            id: fxEnabled3
        }
        QtObject {
            id: fxKnob3name

            property Mixxx.EffectSlotProxy slot: Mixxx.EffectsManager.getEffectSlot(1, 3)
            property string description: "Description"
            property var value: "---"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
        Mixxx.ControlProxy {
            id: fxSelect3
            group: `[EffectRack1_EffectUnit${fxUnit + 1}_Effect3]`
            key: `loaded_effect`
            onValueChanged: {
                fxKnob3name.value = topLabels.fxModel.get(value).display
            }
        }

        Mixxx.ControlProxy {
            group: `[EffectRack1_EffectUnit${fxUnit + 1}]`
            key: "enabled"
            id: fxOn
            property string description: "Description"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: fxButton1;     path: "app.traktor.fx." + (fxUnit + 1) + ".buttons.1"         }
        QtObject {
            id: fxButton1
            property string description: "Description"
            property var value: fxEnabled1.value
            property var valueRange: ({isDiscrete: true, steps: 1})
        }

    // AppProperty { id: fxButton1name; path: "app.traktor.fx." + (fxUnit + 1) + ".buttons.1.name"    }
        QtObject {
            id: fxButton1name
            property string description: "Description"
            property var value: "ON"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: fxButton2;     path: "app.traktor.fx." + (fxUnit + 1) + ".buttons.2"         }
        QtObject {
            id: fxButton2
            property string description: "Description"
            property var value: fxEnabled2.value
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: fxButton2name; path: "app.traktor.fx." + (fxUnit + 1) + ".buttons.2.name"    }
        QtObject {
            id: fxButton2name
            property string description: "Description"
            property var value: "ON"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: fxButton3;     path: "app.traktor.fx." + (fxUnit + 1) + ".buttons.3"         }
        QtObject {
            id: fxButton3
            property string description: "Description"
            property var value: fxEnabled3.value
            property var valueRange: ({isDiscrete: true, steps: 1})
        }
    // AppProperty { id: fxButton3name; path: "app.traktor.fx." + (fxUnit + 1) + ".buttons.3.name"    }
        QtObject {
            id: fxButton3name
            property string description: "Description"
            property var value: "ON"
            property var valueRange: ({isDiscrete: true, steps: 1})
        }

    // AppProperty { id: fxType;    path: "app.traktor.fx." + (fxUnit + 1) + ".type"             } // singleMode -> fxSelect1.description else "DRY/WET"
        QtObject {
            id: fxType
            property string description: "Description"
            property var value: 0
            property var valueRange: ({isDiscrete: true, steps: 1})
        }

        Row {
            id: controlRow
            TopInfoDetails {
                id: topInfoDetails1
                parameter: fxDryWet
                isOn: fxOn.value
                label: fxType.value == 1 ? ((fxSelect1.description == "Delay") ? "DELAY" : (fxSelect1.description == "Reverb") ? "REVRB" : (fxSelect1.description == "Flanger") ? "FLANG" : (fxSelect1.description == "Flanger Pulse") ? "FLN-P" : (fxSelect1.description == "Flanger Flux") ? "FLN-F" : (fxSelect1.description == "Gater") ? "GATER" : (fxSelect1.description == "Beatmasher 2") ? "BEATM" : (fxSelect1.description == "Delay T3") ? "T3DELAY" : (fxSelect1.description == "Filter LFO") ? "FLT-O" : (fxSelect1.description == "Filter Pulse") ? "FLT-P" : (fxSelect1.description == "Filter") ? "FILTR" : (fxSelect1.description == "Filter:92 Pulse") ? "F92-O" : (fxSelect1.description == "Filter:92 Pulse") ? "F92-P" : (fxSelect1.description == "Filter:92") ? "FLT92" : (fxSelect1.description == "Phaser") ? "PHFXASR" : (fxSelect1.description == "Phaser Pulse") ? "PHS-P" : (fxSelect1.description == "Phaser Flux") ? "PHS-F" : (fxSelect1.description == "Reverse Grain") ? "REVGR" : (fxSelect1.description == "Turntable FX") ? "TTFX" : (fxSelect1.description == "Iceverb") ? "ICEVB" : (fxSelect1.description == "Reverb T3") ? "T3REVRB" : (fxSelect1.description == "Ringmodulator") ? "RINGM" : (fxSelect1.description == "Digital LoFi") ? "LOFI" : (fxSelect1.description == "Mulholland Drive") ? "MHDRV" : (fxSelect1.description == "Transpose Stretch") ? "TRANS" : (fxSelect1.description == "BeatSlicer") ? "SLICER" : (fxSelect1.description == "Formant Filter") ? "FFTR" : (fxSelect1.description == "Peak Filter") ? "PFTR" : (fxSelect1.description == "Tape Delay") ? "TPDELAY" : (fxSelect1.description == "Ramp Delay") ? "RMPDLY" : (fxSelect1.description == "Auto Bouncer") ? "ABOUNCE" : (fxSelect1.description == "Bouncer") ? "BOUNCER" : (fxKnob3name.value == "LASLI") ? "LASLI" : (fxKnob3name.value == "GRANP") ? "GRANP" : (fxKnob3name.value == "B-O-M") ? "B-O-M" : (fxKnob3name.value == "POWIN") ? "POWIN" : (fxKnob3name.value == "EVNHR") ? "EVNHR" : (fxKnob3name.value == "ZZZRP") ? "ZZZRP" : (fxKnob3name.value == "STRRS") ? "STRRS" : (fxKnob3name.value == "STRRF") ? "STRRF" : (fxKnob3name.value == "DARKM") ? "DARKM" : (fxKnob3name.value == "FTEST") ? "FTEST" : fxSelect1.description) : "DRY/WET"
                buttonLabel: fxType.value == 1 ? "ON" : ""
                fxEnabled: (fxType.value != 1) || fxSelect1.value
                barBgColor: topLabels.barBgColor
                isPatternPlayer: (fxType.value == 2 ? true : false)
            }
            TopInfoDetails {
                id: topInfoDetails2
                parameter: fxParam1
                isOn: fxButton1.value
                label: fxKnob1name.value
                buttonLabel: fxButton1name.value
                fxEnabled: (fxSelect1.value || ((fxType.value == 1) && fxSelect1.value) )
                barBgColor: topLabels.barBgColor
                isPatternPlayer: (fxType.value == 2 ? true : false)
            }

            TopInfoDetails {
                id: topInfoDetails3
                parameter: fxParam2
                isOn: fxButton2.value
                label: fxKnob2name.value
                buttonLabel: fxButton2name.value
                fxEnabled: (fxSelect2.value || ((fxType.value == 1) && fxSelect1.value) )
                barBgColor: topLabels.barBgColor
                isPatternPlayer: (fxType.value == 2 ? true : false)
            }

            TopInfoDetails {
                id: topInfoDetails4
                parameter: fxParam3
                isOn: fxButton3.value
                label: fxKnob3name.value
                buttonLabel: fxButton3name.value
                fxEnabled: (fxSelect3.value || ((fxType.value == 1) && fxSelect1.value) )
                barBgColor: topLabels.barBgColor
                isPatternPlayer: (fxType.value == 2 ? true : false)
            }
        }
    }

  // black border & shadow
    Rectangle {
        id: headerBlackLine
        anchors.top: topLabels.bottom
        width: parent.width
        color: colors.colorBlack
        height: 2
    }
    Rectangle {
        id: headerShadow
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: headerBlackLine.bottom
        height: 6
        gradient: Gradient {
            GradientStop { position: 1.0; color: colors.colorBlack0 }
            GradientStop { position: 0.0; color: colors.colorBlack63 }
        }
        visible: false
    }

  //------------------------------------------------------------------------------------------------------------------
  //  STATES
  //------------------------------------------------------------------------------------------------------------------

    Behavior on y { PropertyAnimation { duration: durations.overlayTransition; easing.type: Easing.InOutQuad } }

    Item {
        id: showHide
        state: showHideState
        states: [
            State {
                name: "show";
                PropertyChanges { target: topLabels; y: yPositionWhenShown}
            },
            State {
                name: "hide";
                PropertyChanges { target: topLabels; y: yPositionWhenHidden}
            }
        ]
    }
}
