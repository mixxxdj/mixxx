pragma ComponentBehavior: Bound

import "../Controls" as Controls
import "../Deck" as DeckControls
import "../LateNightTheme"
import "../Mixer" as MixerControls
import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    property string group: "[Auxiliary" + unitNumber + "]"
    readonly property bool inputConfigured: inputConfiguredControl.value > 0
    readonly property Item loadedUnit: unitLoader.item as Item
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
            color: LateNightTheme.micAuxPanelColor
            implicitHeight: 68
            implicitWidth: contentLayout.implicitWidth + contentLayout.anchors.leftMargin + contentLayout.anchors.rightMargin

            RowLayout {
                id: contentLayout

                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.preferredWidth: 43
                    spacing: 1

                    Text {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 18
                        color: LateNightTheme.micAuxLabelTextColor
                        font.bold: true
                        font.family: "Open Sans"
                        font.pixelSize: 14
                        horizontalAlignment: Text.AlignHCenter
                        text: "AUX " + root.unitNumber
                        verticalAlignment: Text.AlignVCenter
                    }
                    AuxOrientationButton {
                        Layout.alignment: Qt.AlignHCenter
                        group: root.group
                    }
                    DeckControls.LateNightControlButton {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: 26
                        Layout.preferredWidth: 42
                        activeColor: LateNightTheme.activePlayCueColor
                        activeIconSuffix: LateNightTheme.isPaleMoon ? "active" : ""
                        group: root.group
                        iconSource: LateNightTheme.assetAuxMainMixButton
                        key: "main_mix"
                        powerWindow: true
                        stretchIcon: true
                    }
                }
                Controls.ImageVuMeter {
                    Layout.alignment: Qt.AlignVCenter
                    Layout.preferredHeight: 55
                    Layout.preferredWidth: 8
                    group: root.group
                    levelKey: "vu_meter"
                    micAux: true
                }
                ColumnLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.leftMargin: 3
                    Layout.rightMargin: 3
                    spacing: 1

                    RowLayout {
                        Layout.alignment: Qt.AlignHCenter
                        Layout.preferredHeight: 30
                        spacing: 4

                        Controls.Knob {
                            Layout.preferredHeight: 30
                            Layout.preferredWidth: 35
                            backgroundSource: LateNightTheme.assetMicAuxGainKnobBackground
                            displayArc: true
                            displayArcColor: LateNightTheme.micAuxGainColor
                            displayArcStart: Controls.Knob.ArcStart.Minimum
                            group: root.group
                            indicatorColor: "orange"
                            indicatorKind: "small"
                            key: "pregain"
                        }
                        MixerControls.PflButton {
                            group: root.group
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.preferredHeight: 22
                        spacing: 1

                        MixerControls.FxAssignButtons {
                            id: fxAssignments

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredHeight: 20
                            groupName: root.group
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
            implicitHeight: 57
            implicitWidth: 47

            ColumnLayout {
                anchors.fill: parent
                anchors.bottomMargin: 4
                anchors.leftMargin: 4
                anchors.rightMargin: 4
                anchors.topMargin: 2
                spacing: 2

                Text {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 21
                    color: LateNightTheme.micAuxUnconfiguredTextColor
                    font.bold: true
                    font.family: "Open Sans"
                    font.pixelSize: 14
                    horizontalAlignment: Text.AlignHCenter
                    text: "AUX " + root.unitNumber
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
                    contentOpacity: 1.0
                    group: root.group
                    iconSource: LateNightTheme.assetMicAuxAddButton
                    inactiveFillEnabled: false
                    key: "main_mix"
                    stretchIcon: true
                }
            }
        }
    }
}
