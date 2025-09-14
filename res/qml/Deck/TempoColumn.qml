import ".." as Skin
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import "../Theme"

ColumnLayout {
    required property string group
    required property var currentTrack
    RowLayout {
        height: 22
        Layout.fillWidth: true
        Skin.ControlButton {
            id: pitchDownButton

            implicitWidth: 20
            implicitHeight: 22

            group: root.group
            key: "pitch_down"
            contentItem: Item {
                anchors.fill: parent
                Shape {
                    anchors.centerIn: parent
                    width: 12
                    height: 10
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: '#626262'
                        strokeColor: 'transparent'
                        startX: 0; startY: 5
                        PathLine { x: 12; y: 0 }
                        PathLine { x: 12; y: 10 }
                        PathLine { x: 0; y: 5 }
                    }
                }
            }
            activeColor: Theme.deckActiveColor
        }
        Item {
            id: pitchKey

            Layout.fillWidth: true
            Layout.leftMargin: 0
            Layout.rightMargin: 0

            implicitHeight: 22

            Rectangle {
                id: background2Image

                anchors.fill: parent
                color: '#2B2B2B'
            }

            DropShadow {
                anchors.fill: background2Image
                horizontalOffset: 0
                verticalOffset: 0
                radius: 1.0
                color: "#80000000"
                source: background2Image
            }
            InnerShadow {
                anchors.fill: background2Image
                radius: 1
                samples: 16
                horizontalOffset: -0
                verticalOffset: 0
                color: "#353535"
                source: background2Image
            }
            Label {
                anchors.centerIn: background2Image
                text: trackLoadedControl.value ? root.currentTrack.keyText : "-"
                color: "#626262"
                font.pixelSize: 8
            }
        }

        Skin.ControlButton {
            id: pitchUpButton

            implicitWidth: 20
            implicitHeight: 22

            group: root.group
            key: "pitch_up"
            contentItem: Item {
                anchors.fill: parent
                Shape {
                    anchors.centerIn: parent
                    width: 12
                    height: 10
                    antialiasing: true
                    layer.enabled: true
                    layer.samples: 4
                    ShapePath {
                        fillColor: '#626262'
                        strokeColor: 'transparent'
                        startX: 0; startY: 0
                        fillRule: ShapePath.WindingFill
                        capStyle: ShapePath.RoundCap
                        PathLine { x: 12; y: 5 }
                        PathLine { x: 0; y: 10 }
                        PathLine { x: 0; y: 0 }
                    }
                }
            }
            activeColor: Theme.deckActiveColor
        }
    }
    Skin.ControlFader {
        id: rateSlider

        Layout.fillWidth: true
        Layout.fillHeight: true

        visible: !root.minimized
        width: pitchKey.implicitWidth
        group: root.group
        key: "rate"
        barStart: 0.5
        barMargin: 0
        barColor: Theme.bpmSliderBarColor
        bg: Theme.imgBpmSliderBackground

        Skin.FadeBehavior on visible {
            fadeTarget: rateSlider
        }
    }

    Skin.SyncButton {
        id: syncButton
        Layout.fillWidth: true
        height: 22
        Layout.alignment: Qt.AlignHCenter
        group: root.group
    }
}
