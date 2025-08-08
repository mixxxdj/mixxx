import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Rectangle {
    id: root

    required property string group

    color: '#626262'

    Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Loop"
        color: '#3F3F3F'
    }
    RowLayout {
        anchors {
            left: parent.left
            leftMargin: 6
            right: parent.right
            rightMargin: 6
            top: parent.top
            topMargin: 22
        }

        Skin.ControlButton {
            Layout.fillWidth: true
            Layout.minimumWidth: 28
            id: loopInButton

            implicitHeight: 26

            group: root.group
            key: "loop_in"
            text: "In"
        }

        Skin.ControlButton {
            Layout.fillWidth: true
            Layout.minimumWidth: 28
            id: loopOutButton

            implicitHeight: 26

            group: root.group
            key: "loop_out"
            text: "Out"
        }

        Skin.ControlButton {
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            id: loopRecallButton

            implicitHeight: 26

            Mixxx.ControlProxy {
                id: loopEnabled

                group: root.group
                key: "loop_enabled"
            }

            group: root.group
            toggleable: loopEnabled.value
            key: loopEnabled.value ? "loop_enabled" : "reloop_toggle"
            text: loopEnabled.value ? "exit" : "Recall"
        }
    }
    RowLayout {
        anchors {
            bottom: parent.bottom
            bottomMargin: 6
            left: parent.left
            leftMargin: 6
            right: parent.right
            rightMargin: 6
        }
        Skin.ControlButton {
            id: loopSizeHalfButton

            implicitWidth: 22
            implicitHeight: 28

            group: root.group
            key: "loop_halve"
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

        Mixxx.ControlProxy {
            id: beatloopSize

            group: root.group
            key: "beatloop_size"
        }

        Mixxx.ControlProxy {
            id: beatloopActivate

            group: root.group
            key: "beatloop_activate"
        }

        // property list<double> loopValues: {
        //     for
        // }
        // property int loopOffset: beatloopActivate.value ? beatloopSize.value * 32 : 0

        Repeater {

            model: Math.min(Math.max(1, parseInt((root.width - 56)/50)), 4)

            Skin.ControlButton {
                required property int index
                property double currentSize: beatloopSize.value

                id: loopSizeOpt1Button

                implicitHeight: 28
                implicitWidth: 49
                toggleable: true

                group: root.group
                key: `beatloop_${currentSize}_activate`
                text: currentSize < 1 ? `1/${1/currentSize}` : currentSize
                activeColor: Theme.deckActiveColor
            }
        }

        Skin.ControlButton {
            id: loopSizeDoubleButton

            implicitWidth: 22
            implicitHeight: 28

            group: root.group
            key: "loop_double"
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
}
