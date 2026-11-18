pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../Mixer" as MixerControls
import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    property string group: "[Auxiliary" + unitNumber + "]"
    readonly property bool inputConfigured: inputConfiguredControl.value > 0
    readonly property Item loadedUnit: unitLoader.status === Loader.Ready ? unitLoader.item as Item : null
    required property int unitNumber

    implicitHeight: loadedUnit?.implicitHeight ?? 0
    implicitWidth: loadedUnit?.implicitWidth ?? 0

    Loader {
        id: unitLoader

        anchors.fill: parent
        sourceComponent: root.inputConfigured ? configuredUnit : unconfiguredUnit
    }
    Mixxx.ControlProxy {
        id: inputConfiguredControl

        group: root.group
        key: "input_configured"
    }
    Component {
        id: configuredUnit

        Controls.Panel {
            bottomBorderColor: LateNightTheme.mixerPanelBorderBottom
            color: LateNightTheme.micAuxPanelColor
            implicitHeight: LateNightTheme.isClassic ? 68 : 67
            implicitWidth: contentLayout.implicitWidth + 2
            leftBorderColor: LateNightTheme.mixerPanelBorderLeft
            rightBorderColor: LateNightTheme.mixerPanelBorderRight
            topBorderColor: LateNightTheme.mixerPanelBorderTop

            Row {
                id: contentLayout

                anchors.fill: parent
                anchors.margins: 1
                spacing: 0

                Item {
                    height: parent.height
                    width: Math.max(LateNightTheme.isClassic ? 43 : 42, Math.ceil(configuredLabel.implicitWidth)) + 3

                    Text {
                        id: configuredLabel

                        color: LateNightTheme.micAuxLabelTextColor
                        font.family: "Open Sans"
                        font.pixelSize: 14
                        font.weight: LateNightTheme.isClassic ? Font.Bold : Font.Medium
                        height: 20
                        horizontalAlignment: Text.AlignHCenter
                        leftPadding: 1
                        renderType: Text.NativeRendering
                        text: "AUX " + root.unitNumber
                        topPadding: 2
                        verticalAlignment: Text.AlignTop
                        width: parent.width - 3
                        x: 2
                    }
                    AuxOrientationButton {
                        group: root.group
                        x: 2 + Math.floor((parent.width - 36) / 2)
                        y: 23
                    }
                    DeckControls.LateNightControlButton {
                        activeBackgroundSuffix: "active"
                        activeColor: LateNightTheme.activePlayCueColor
                        activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                        activeOpacity: 1
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 2
                        backgroundSource: LateNightTheme.lateNightTopRegionButton("medium")
                        fillMargin: 1
                        group: root.group
                        height: 26
                        iconSource: LateNightTheme.assetAuxMainMixButton
                        inactiveOpacity: 1
                        key: "main_mix"
                        powerWindow: true
                        pressedBackgroundSuffix: "active"
                        solidFillEnabled: true
                        stretchIcon: false
                        width: 42
                        x: 2 + Math.floor((parent.width - 45 + (LateNightTheme.isClassic ? 1 : 0)) / 2)
                    }
                }
                Item {
                    height: parent.height
                    width: 11

                    Controls.ImageVuMeter {
                        group: root.group
                        height: 54
                        levelKey: "vu_meter"
                        micAux: true
                        width: 8
                        x: 2
                        y: Math.floor((parent.height - height - (LateNightTheme.isClassic ? 1 : -3)) / 2)
                    }
                }
                Item {
                    height: parent.height
                    width: Math.max(65, fxAssignments.assignmentWidth) + (LateNightTheme.isClassic ? 6 : 5)

                    Row {
                        anchors.left: parent.left
                        anchors.leftMargin: 3
                        height: parent.height - 23
                        spacing: 4

                        Controls.Knob {
                            backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
                            displayArc: true
                            displayArcColor: LateNightTheme.micAuxGainColor
                            displayArcStart: Controls.Knob.ArcStart.Minimum
                            group: root.group
                            height: 30
                            indicatorColor: "orange"
                            indicatorKind: "small"
                            key: "pregain"
                            width: 35
                            y: Math.floor((parent.height - height) / 2)
                        }
                        DeckControls.LateNightControlButton {
                            activeBackgroundSuffix: "active"
                            activeColor: LateNightTheme.mixerPflActiveFillColor
                            backgroundSource: LateNightTheme.assetMixerPflBackground
                            contentOpacity: 1
                            fillMargin: 1
                            group: root.group
                            height: 26
                            iconSource: isActive ? LateNightTheme.assetMixerPflActiveIcon : LateNightTheme.assetMixerPflIcon
                            inactiveColor: LateNightTheme.deckEmbeddedButtonInactiveColor
                            key: "pfl"
                            pressedBackgroundSuffix: "active"
                            solidFillEnabled: true
                            toggleable: true
                            width: 26
                            y: Math.floor((parent.height - height) / 2)
                        }
                    }
                    MixerControls.FxAssignButtons {
                        id: fxAssignments

                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 3
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.horizontalCenterOffset: LateNightTheme.isClassic ? 0 : 0.5
                        groupName: root.group
                    }
                }
            }
        }
    }
    Component {
        id: unconfiguredUnit

        Controls.Panel {
            bottomBorderColor: LateNightTheme.mixerPanelBorderBottom
            color: LateNightTheme.micAuxPanelColor
            implicitHeight: 57
            implicitWidth: Math.ceil(unconfiguredLabel.implicitWidth) + 10
            leftBorderColor: LateNightTheme.mixerPanelBorderLeft
            rightBorderColor: LateNightTheme.mixerPanelBorderRight
            topBorderColor: LateNightTheme.mixerPanelBorderTop

            Text {
                id: unconfiguredLabel

                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.top: parent.top
                anchors.topMargin: 4
                color: LateNightTheme.micAuxUnconfiguredTextColor
                font.family: "Open Sans"
                font.pixelSize: 14
                font.weight: LateNightTheme.isClassic ? Font.Bold : Font.Medium
                height: 21
                horizontalAlignment: Text.AlignHCenter
                renderType: Text.NativeRendering
                text: "AUX " + root.unitNumber
                verticalAlignment: Text.AlignVCenter
            }
            DeckControls.LateNightControlButton {
                anchors.bottom: parent.bottom
                anchors.bottomMargin: 6
                anchors.horizontalCenter: parent.horizontalCenter
                backgroundSource: LateNightTheme.assetMicAuxUnconfiguredBackground
                contentOpacity: 1.0
                group: root.group
                height: 26
                iconSource: LateNightTheme.assetMicAuxAddButton
                inactiveFillEnabled: false
                key: "main_mix"
                stretchIcon: false
                width: 26
            }
        }
    }
}
