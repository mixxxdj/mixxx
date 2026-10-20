pragma ComponentBehavior: Bound

import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Layouts 1.12
import "Theme"

Row {
    id: root

    required property int unitNumber
    property int fxUnitCount: 4
    property string group: "[Auxiliary" + unitNumber + "]"
    readonly property bool inputConfigured: inputConfiguredControl.value > 0

    signal fxAssignmentChanged(int unitNumber, bool enabled)

    spacing: 5

    Loader {
        sourceComponent: root.inputConfigured ? configuredComponent : unconfiguredComponent
    }
    Mixxx.ControlProxy {
        id: inputConfiguredControl

        group: root.group
        key: "input_configured"
    }
    Component {
        id: configuredComponent

        Row {
            layoutDirection: root.layoutDirection
            spacing: root.spacing

            Skin.VuMeter {
                group: root.group
                height: parent.height
                key: "vu_meter"
                width: 4
            }
            Rectangle {
                color: Theme.knobBackgroundColor
                height: 52
                radius: 5
                width: 52

                Skin.ControlKnob {
                    anchors.centerIn: parent
                    arcStart: Knob.ArcStart.Minimum
                    color: Theme.gainKnobColor
                    group: root.group
                    height: 48
                    key: "pregain"
                    width: 48
                }
            }
            Column {
                Skin.SectionText {
                    height: 26
                    text: "AUX " + root.unitNumber
                    width: 52
                }
                Skin.ControlButton {
                    activeColor: Theme.pflActiveButtonColor
                    group: root.group
                    key: "pfl"
                    text: "PFL"
                    toggleable: true
                }
            }
            Skin.EmbeddedBackground {
                height: parent.height
                width: Math.max(56, fxAssignments.implicitWidth)

                ColumnLayout {
                    anchors.fill: parent
                    spacing: 0

                    RowLayout {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        spacing: 0

                        Skin.OrientationToggleButton {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            color: Theme.crossfaderOrientationColor
                            group: root.group
                            key: "orientation"
                        }
                        Skin.ControlButton {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            activeColor: Theme.deckActiveColor
                            group: root.group
                            key: "main_mix"
                            text: "Main"
                            toggleable: true
                        }
                    }
                    RowLayout {
                        id: fxAssignments

                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        spacing: 0

                        Repeater {
                            model: Math.max(0, root.fxUnitCount)

                            Skin.ControlButton {
                                required property int index

                                Layout.fillHeight: true
                                Layout.fillWidth: true
                                activeColor: Theme.effectUnitColor
                                group: "[EffectRack1_EffectUnit" + (index + 1) + "]"
                                implicitWidth: 28
                                key: "group_" + root.group + "_enable"
                                text: "FX" + (index + 1)
                                toggleable: true

                                onHighlightChanged: root.fxAssignmentChanged(index + 1, highlight)
                            }
                        }
                    }
                }
            }
        }
    }
    Component {
        id: unconfiguredComponent

        Column {
            spacing: 2

            Skin.SectionText {
                color: Theme.darkGray3
                height: 21
                text: "Aux " + root.unitNumber
                width: 80
            }
            Skin.ControlButton {
                group: root.group
                key: "main_mix"
                text: "Configure"
                width: 80
            }
        }
    }
}
