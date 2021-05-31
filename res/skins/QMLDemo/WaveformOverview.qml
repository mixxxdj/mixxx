import Mixxx 0.1 as Mixxx
import QtQuick 2.12
import QtQuick.Shapes 1.12
import "Theme"

Item {
    id: root

    property string group // required

    states: [
        State {
            when: passthroughControl.value == 0

            PropertyChanges {
                target: waveformContainer
                opacity: 1
                visible: true
            }

            PropertyChanges {
                target: passthroughContainer
                opacity: 0
                visible: false
            }

        },
        State {
            when: passthroughControl.value != 0

            PropertyChanges {
                target: waveformContainer
                opacity: 0
                visible: false
            }

            PropertyChanges {
                target: passthroughContainer
                opacity: 1
                visible: true
            }

        }
    ]
    transitions: [
        Transition {
            enabled: passthroughContainer.visible

            SequentialAnimation {
                PropertyAction {
                    target: waveformContainer
                    property: "visible"
                }

                ParallelAnimation {
                    NumberAnimation {
                        target: waveformContainer
                        property: "opacity"
                        duration: 150
                    }

                    NumberAnimation {
                        target: passthroughContainer
                        property: "opacity"
                        duration: 150
                    }

                }

                PropertyAction {
                    target: passthroughContainer
                    property: "visible"
                }

            }

        },
        Transition {
            enabled: waveformContainer.visible

            SequentialAnimation {
                PropertyAction {
                    target: passthroughContainer
                    property: "visible"
                }

                ParallelAnimation {
                    NumberAnimation {
                        target: waveformContainer
                        property: "opacity"
                        duration: 150
                    }

                    NumberAnimation {
                        target: passthroughContainer
                        property: "opacity"
                        duration: 150
                    }

                }

                PropertyAction {
                    target: waveformContainer
                    property: "visible"
                }

            }

        }
    ]

    Mixxx.ControlProxy {
        id: passthroughControl

        group: root.group
        key: "passthrough"
    }

    Rectangle {
        id: waveformContainer

        anchors.fill: parent
        color: Theme.deckBackgroundColor

        Text {
            anchors.centerIn: parent
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize
            color: Theme.deckTextColor
            text: "Waveform Placeholder"
        }

    }

    Rectangle {
        id: passthroughContainer

        anchors.fill: parent
        color: "transparent"

        Text {
            anchors.centerIn: parent
            font.family: Theme.fontFamily
            font.pixelSize: Theme.textFontPixelSize
            color: Theme.deckTextColor
            text: "Passthrough Enabled"
        }

    }

}
