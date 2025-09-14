import ".." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Rectangle {
    id: root

    required property string group

    property color buttonColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor

    color: Theme.deckLoopBackgroundColor

    Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Loop"
        color: Theme.deckLoopLabelColor
    }

    Mixxx.ControlProxy {
        id: trackLoadedControl

        group: root.group
        key: "track_loaded"
    }

    Mixxx.ControlProxy {
        id: loopEnabled

        group: root.group
        key: "loop_enabled"
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

    Mixxx.ControlProxy {
        id: loopHalve

        group: root.group
        key: "loop_halve"
    }

    Mixxx.ControlProxy {
        id: loopDouble

        group: root.group
        key: "loop_double"
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
            normalColor: root.buttonColor
            activeColor: root.buttonColor
            key: "loop_in"
            text: "In"
        }

        Skin.ControlButton {
            Layout.fillWidth: true
            Layout.minimumWidth: 28
            id: loopOutButton

            implicitHeight: 26

            group: root.group
            normalColor: root.buttonColor
            activeColor: root.buttonColor
            key: "loop_out"
            text: "Out"
        }

        Skin.ControlButton {
            Layout.fillWidth: true
            Layout.minimumWidth: 40
            id: loopRecallButton

            implicitHeight: 26

            group: root.group
            normalColor: root.buttonColor
            activeColor: root.buttonColor
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
        Skin.Button {
            id: loopSizeHalfButton

            implicitWidth: 22
            implicitHeight: 28

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
                        fillColor: root.buttonColor
                        strokeColor: 'transparent'
                        startX: 0; startY: 5
                        PathLine { x: 12; y: 0 }
                        PathLine { x: 12; y: 10 }
                        PathLine { x: 0; y: 5 }
                    }
                }
            }
            activeColor: root.buttonColor
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onPressed: (mouse) => {
                    if (loopEnabled.value ^ mouse.button == Qt.RightButton) {
                        loopHalve.trigger()
                    }
                    loopSizeRepeater.selectedIndex = Math.max(0, loopSizeRepeater.selectedIndex-1)
                }
            }
        }

        Repeater {
            id: loopSizeRepeater
            property int valueCount: Math.min(Math.max(1, parseInt((root.width - 56)/40)), 4);
            property list<double> values: {
                let values = [1/32];
                while (values[values.length-1] < 512) {
                    values.push(values[values.length-1]*2);
                }
                return values
            }
            property int selectedIndex: 0
            Component.onCompleted: {
                update()
                selectedIndex = values.indexOf(beatloopSize.value)
                if (selectedIndex < 0) {
                    selectedIndex = values.indexOf(4)
                }
            }
            onValueCountChanged: update()
            onSelectedIndexChanged: update()

            function update() {
                let values = [this.values[selectedIndex]];
                let appendMode = values[0] <= this.values[0];
                while (values.length < valueCount) {
                    if (appendMode) {
                        values.push(values[values.length-1] * 2);
                    } else {
                        values = [values[0] / 2, ...values];
                    }
                    if (values[0] == this.values[0]) {
                        appendMode = true;
                    } else if (values[values.length-1] == this.values[this.values.length-1]) {
                        appendMode = false;
                    } else {
                        appendMode = !appendMode;
                    }
                }
                model = values;
            }

            Connections {
                target: beatloopSize
                function onValueChanged() {
                    if (loopEnabled.value) {
                        parent.selectedIndex = parent.values.indexOf(beatloopSize.value)
                    }
                    parent.update()
                }
            }

            Skin.Button {
                required property int index
                required property var modelData

                id: loopSizeOpt1Button

                property double currentSize: modelData

                highlight: currentSize == beatloopSize.value && trackLoadedControl.value > 0

                implicitHeight: 28
                implicitWidth: 33
                activeColor: root.buttonColor

                Mixxx.ControlProxy {
                    id: sizedBeatloopActivate

                    group: root.group
                    key: `beatloop_${currentSize}_activate`
                }

                text: currentSize < 1 ? `1/${1/currentSize}` : currentSize

                onPressed: {
                    if (loopEnabled.value) {
                        beatloopSize.value = currentSize;
                    } else {
                        sizedBeatloopActivate.trigger()
                    }
                }
            }
        }

        Skin.Button {
            id: loopSizeDoubleButton

            implicitWidth: 22
            implicitHeight: 28

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
                        fillColor: root.buttonColor
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
            activeColor: root.buttonColor
            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onPressed: (mouse) => {
                    if (loopEnabled.value ^ mouse.button == Qt.RightButton) {
                        loopDouble.trigger()
                    }
                    loopSizeRepeater.selectedIndex = Math.min(loopSizeRepeater.values.length - 1, loopSizeRepeater.selectedIndex+1)
                }
            }
        }
    }
}
