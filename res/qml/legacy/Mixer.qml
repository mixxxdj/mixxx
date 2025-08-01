import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as E

E.Frame {
    leftPadding: 4
    rightPadding: 5
    color: "#202020"
    ColumnLayout {
        height: parent.height
        spacing: 4
        RowLayout {
            Layout.fillHeight: false
            Layout.alignment: Qt.AlignTop
            // EQ section left deck
            E.EqFxSection {
            }
            // Gain + Level section left deck
            E.GainLevel {
                Layout.alignment: Qt.AlignTop
            }
            // PFL + metering
            ColumnLayout {
                // PLF section
                spacing: 0
                Layout.alignment: Qt.AlignTop
                RowLayout {
                    spacing: 4
                    Layout.preferredHeight: 36
                    E.PflButton {
                    }
                    E.PflButton {
                    }
                }
                Item {
                    implicitHeight: 4
                }
                // Metering section
                RowLayout {
                    spacing: 6
                    Layout.alignment: Qt.AlignHCenter
                    // Meter left deck
                    Rectangle {
                        color: "#606060"
                        implicitWidth: 8
                        Layout.fillHeight: true
                    }
                    // Main output meters
                    RowLayout {
                        spacing: 0
                        Rectangle {
                            color: "#606060"
                            implicitWidth: 8
                            Layout.fillHeight: true
                        }
                        Rectangle {
                            color: "#606060"
                            implicitWidth: 8
                            Layout.fillHeight: true
                        }
                    }
                    // Meter right deck
                    Rectangle {
                        color: "#606060"
                        implicitWidth: 8
                        Layout.fillHeight: true
                    }
                }
                Item {
                    implicitHeight: 4
                }
            }
            // Gain + Level section right deck
            E.GainLevel {
                Layout.alignment: Qt.AlignTop
            }
            // EQ section right deck
            E.EqFxSection {
                layoutDirection: Qt.RightToLeft
                Layout.alignment: Qt.AlignTop
            }
        }
        Item {
            Layout.fillHeight: true
        }
        // XFader section
        RowLayout {
            Layout.fillWidth: false
            Layout.alignment: Qt.AlignCenter
            E.XFaderOrientationSwitch {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.fillWidth: true
            }
            E.Slider {
                implicitWidth: 115
                implicitHeight: 40
                from: -1
                to: 1
                Layout.columnSpan: 3
                Layout.alignment: Qt.AlignHCenter
                backgroundSource: "image://svgmodifier/sliders/xfader/background.svg"
                handleSource: "image://svgmodifier/sliders/xfader/handle.svg"
            }
            E.XFaderOrientationSwitch {
                Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                Layout.fillWidth: true
                value: 2
            }
        }
    }
}
