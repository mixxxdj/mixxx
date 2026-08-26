pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../../../qml" as Shared
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property int unitNumber
    property string group: unitNumber === 1 ? "[Microphone]" : "[Microphone" + unitNumber + "]"
    readonly property bool inputConfigured: inputConfiguredControl.value > 0
    readonly property Item loadedUnit: unitLoader.item as Item

    implicitHeight: loadedUnit?.implicitHeight ?? 0
    implicitWidth: loadedUnit?.implicitWidth ?? 0

    Loader {
        id: unitLoader

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
            color: LateNightTheme.micAuxPanelColor
            implicitHeight: 78
            implicitWidth: 132

            RowLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 43
                    spacing: 1

                    Text {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 20
                        color: LateNightTheme.textColor
                        font.family: "Open Sans"
                        font.pixelSize: 10
                        horizontalAlignment: Text.AlignHCenter
                        text: "mic " + root.unitNumber
                        verticalAlignment: Text.AlignVCenter
                    }
                    Item {
                        Layout.fillHeight: true
                    }
                    DeckControls.LateNightControlButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: 26
                        Layout.preferredWidth: 42
                        activeColor: LateNightTheme.activePlayCueColor
                        group: root.group
                        iconSource: LateNightTheme.assetMicTalkButton
                        key: "talkover"
                        stretchIcon: true
                        toggleable: true
                    }
                }
                Shared.VuMeter {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 5
                    group: root.group
                    key: "vu_meter"
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    spacing: 1

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 34
                        spacing: 3

                        Controls.Knob {
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 35
                            backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
                            displayArc: true
                            displayArcColor: LateNightTheme.samplerGainColor
                            displayArcStart: Controls.Knob.ArcStart.Minimum
                            group: root.group
                            indicatorColor: "orange"
                            indicatorKind: "small"
                            key: "pregain"
                        }
                        DeckControls.LateNightControlButton {
                            Layout.preferredHeight: 26
                            Layout.preferredWidth: 26
                            activeColor: LateNightTheme.samplerColor
                            group: root.group
                            iconSource: LateNightTheme.lateNightAsset("buttons", "btn__pfl.svg")
                            key: "pfl"
                            stretchIcon: true
                            toggleable: true
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 22
                        spacing: 1

                        Repeater {
                            model: 4

                            DeckControls.LateNightControlButton {
                                required property int fxUnitIdx

                                Layout.fillWidth: true
                                Layout.preferredHeight: 20
                                activeColor: LateNightTheme.samplerColor
                                group: "[EffectRack1_EffectUnit" + (fxUnitIdx + 1) + "]"
                                key: "group_" + root.group + "_enable"
                                label: String(fxUnitIdx + 1)
                                labelPixelSize: 8
                                toggleable: true
                            }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: unconfiguredUnit

        Controls.Panel {
            color: LateNightTheme.micAuxPanelColor
            implicitHeight: 78
            implicitWidth: 72

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 4
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 21
                    color: LateNightTheme.micAuxUnconfiguredTextColor
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    horizontalAlignment: Text.AlignHCenter
                    text: "mic " + root.unitNumber
                    verticalAlignment: Text.AlignVCenter
                }
                Item {
                    Layout.fillHeight: true
                }
                DeckControls.LateNightControlButton {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: 26
                    Layout.preferredWidth: 26
                    backgroundSource: LateNightTheme.assetMicAuxUnconfiguredBackground
                    group: root.group
                    iconSource: LateNightTheme.assetMicAuxAddButton
                    key: "talkover"
                    stretchIcon: true
                }
            }
        }
    }
}
