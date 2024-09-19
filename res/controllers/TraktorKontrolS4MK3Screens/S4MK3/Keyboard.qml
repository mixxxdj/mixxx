/*
This module is used render the keyboard scale, originating from C major (do).
*/
import QtQuick 2.15
import QtQuick.Shapes 1.4
import QtQuick.Layouts 1.3

import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls

import "." as S4MK3

Item {
    id: root

    required property string group

    Mixxx.ControlProxy {
        id: keyProxy
        group: root.group
        key: "key"
    }

    readonly property int key: keyProxy.value

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle { anchors.fill: parent; color: "transparent" }
        }
        Repeater {
            id: whiteKeys

            model: 7

            property variant keyMap: [
                                      S4MK3.KeyIndicator.Key.OneD,
                                      S4MK3.KeyIndicator.Key.ThreeD,
                                      S4MK3.KeyIndicator.Key.FiveD,
                                      S4MK3.KeyIndicator.Key.TwelveD,
                                      S4MK3.KeyIndicator.Key.SecondD,
                                      S4MK3.KeyIndicator.Key.FourD,
                                      S4MK3.KeyIndicator.Key.SixD,
                                      S4MK3.KeyIndicator.Key.TenM,
                                      S4MK3.KeyIndicator.Key.TwelveM,
                                      S4MK3.KeyIndicator.Key.TwoM,
                                      S4MK3.KeyIndicator.Key.NineM,
                                      S4MK3.KeyIndicator.Key.ElevenM,
                                      S4MK3.KeyIndicator.Key.OneM,
                                      S4MK3.KeyIndicator.Key.ThreeM
            ]

            Rectangle {
                Layout.preferredWidth: 21
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                radius: 2
                border.width: 1
                border.color: root.key == whiteKeys.keyMap[index] || root.key == whiteKeys.keyMap[index + 7] ? "red" : "black"
                color: root.key == whiteKeys.keyMap[index] || root.key == whiteKeys.keyMap[index + 7] ? "#aaaaaa" : "white"
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle { anchors.fill: parent; color: "transparent" }
        }
    }
    RowLayout {
        anchors.fill: parent
        spacing: 0

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle { anchors.fill: parent; color: "transparent" }
        }
        Repeater {
            id: blackKeys

            model: 5

            property variant keyMap: [
                                      S4MK3.KeyIndicator.Key.EightD,
                                      S4MK3.KeyIndicator.Key.TenD,
                                      S4MK3.KeyIndicator.Key.SevenD,
                                      S4MK3.KeyIndicator.Key.NineD,
                                      S4MK3.KeyIndicator.Key.ElevenD,
                                      S4MK3.KeyIndicator.Key.FiveM,
                                      S4MK3.KeyIndicator.Key.SevenM,
                                      S4MK3.KeyIndicator.Key.FourM,
                                      S4MK3.KeyIndicator.Key.SixM,
                                      S4MK3.KeyIndicator.Key.EightM,
            ]

            Item {
                Layout.fillHeight: true
                Layout.preferredWidth: index == 1 ? 42 : index == 4 ? 12 : 21
                Rectangle {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    width: 12
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    color: "transparent"
                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 0
                        Rectangle {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            radius: 2
                            border.width: 1
                            color: root.key == blackKeys.keyMap[index] || root.key == blackKeys.keyMap[index + blackKeys.model] ? "#aaaaaa" : "black"
                        }
                        Item {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            Rectangle { anchors.fill: parent; color: "transparent" }
                        }
                    }
                }
            }
        }
        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Rectangle { anchors.fill: parent; color: "transparent" }
        }
    }
}
