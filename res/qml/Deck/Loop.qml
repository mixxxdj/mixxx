import ".." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Rectangle {
    id: root

    property color buttonColor: trackLoadedControl.value > 0 ? Theme.buttonActiveColor : Theme.buttonDisableColor
    required property string group

    color: Theme.deckLoopBackgroundColor

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        color: Theme.deckLoopLabelColor
        text: "Loop"
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
            id: loopInButton

            Layout.fillWidth: true
            Layout.minimumWidth: 28
            activeColor: root.buttonColor
            group: root.group
            implicitHeight: 26
            key: "loop_in"
            normalColor: root.buttonColor
            text: "In"
        }
        Skin.ControlButton {
            id: loopOutButton

            Layout.fillWidth: true
            Layout.minimumWidth: 28
            activeColor: root.buttonColor
            group: root.group
            implicitHeight: 26
            key: "loop_out"
            normalColor: root.buttonColor
            text: "Out"
        }
        Skin.ControlButton {
            id: loopRecallButton

            Layout.fillWidth: true
            Layout.minimumWidth: 40
            activeColor: root.buttonColor
            group: root.group
            implicitHeight: 26
            key: loopEnabled.value ? "loop_enabled" : "reloop_toggle"
            normalColor: root.buttonColor
            text: loopEnabled.value ? "exit" : "Recall"
            toggleable: loopEnabled.value
        }
    }
    MouseArea {
        anchors {
            bottom: parent.bottom
            bottomMargin: 6
            left: parent.left
            leftMargin: 6
            right: parent.right
            rightMargin: 6
        }
        onWheel: mouse => {
            if (mouse.angleDelta.y < 0 && loopSizeRepeater.selectedIndex > 0) {
                loopSizeRepeater.selectedIndex -= 1
            } else if (mouse.angleDelta.y > 0 && loopSizeRepeater.selectedIndex < loopSizeRepeater.values.length - 1) {
                loopSizeRepeater.selectedIndex += 1
            }
        }
        implicitHeight: 28
        RowLayout {
            anchors.fill: parent
            Skin.Button {
                id: loopSizeHalfButton

                activeColor: root.buttonColor
                implicitHeight: 28
                implicitWidth: 22

                contentItem: Item {
                    anchors.fill: parent

                    Shape {
                        anchors.centerIn: parent
                        antialiasing: true
                        height: 10
                        layer.enabled: true
                        layer.samples: 4
                        width: 12

                        ShapePath {
                            fillColor: root.buttonColor
                            startX: 0
                            startY: 5
                            strokeColor: 'transparent'

                            PathLine {
                                x: 12
                                y: 0
                            }
                            PathLine {
                                x: 12
                                y: 10
                            }
                            PathLine {
                                x: 0
                                y: 5
                            }
                        }
                    }
                }

                MouseArea {
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent

                    onPressed: mouse => {
                        if (loopEnabled.value ^ mouse.button == Qt.RightButton) {
                            loopHalve.trigger();
                        }
                        loopSizeRepeater.selectedIndex = Math.max(0, loopSizeRepeater.selectedIndex - 1);
                    }
                }
            }

            Repeater {
                id: loopSizeRepeater

                property int selectedIndex: 0
                property int valueCount: Math.min(Math.max(1, parseInt((root.width - 56) / 40)), 4)
                property list<double> values: {
                    let values = [1 / 32];
                    while (values[values.length - 1] < 512) {
                        values.push(values[values.length - 1] * 2);
                    }
                    return values;
                }

                function update() {
                    let values = [this.values[selectedIndex]];
                    let appendMode = values[0] <= this.values[0];
                    while (values.length < valueCount) {
                        if (appendMode) {
                            values.push(values[values.length - 1] * 2);
                        } else {
                            values = [values[0] / 2, ...values];
                        }
                        if (values[0] == this.values[0]) {
                            appendMode = true;
                        } else if (values[values.length - 1] == this.values[this.values.length - 1]) {
                            appendMode = false;
                        } else {
                            appendMode = !appendMode;
                        }
                    }
                    model = values;
                }

                Component.onCompleted: {
                    update();
                    selectedIndex = values.indexOf(beatloopSize.value);
                    if (selectedIndex < 0) {
                        selectedIndex = values.indexOf(4);
                    }
                }
                onSelectedIndexChanged: update()
                onValueCountChanged: update()

                Connections {
                    function onValueChanged() {
                        if (loopEnabled.value) {
                            parent.selectedIndex = parent.values.indexOf(beatloopSize.value);
                        }
                        parent.update();
                    }

                    target: beatloopSize
                }
                Skin.Button {
                    id: loopSizeOpt1Button

                    property double currentSize: modelData
                    required property int index
                    required property var modelData

                    activeColor: root.buttonColor
                    highlight: currentSize == beatloopSize.value && trackLoadedControl.value > 0
                    implicitHeight: 28
                    implicitWidth: 33
                    text: currentSize < 1 ? `1/${1 / currentSize}` : currentSize

                    onPressed: {
                        if (loopEnabled.value) {
                            beatloopSize.value = currentSize;
                        } else {
                            sizedBeatloopActivate.trigger();
                        }
                    }

                    Mixxx.ControlProxy {
                        id: sizedBeatloopActivate

                        group: root.group
                        key: `beatloop_${currentSize}_activate`
                    }
                }
            }
            Skin.Button {
                id: loopSizeDoubleButton

                Layout.alignment: Qt.AlignRight

                activeColor: root.buttonColor
                implicitHeight: 28
                implicitWidth: 22

                contentItem: Item {
                    anchors.fill: parent

                    Shape {
                        anchors.centerIn: parent
                        antialiasing: true
                        height: 10
                        layer.enabled: true
                        layer.samples: 4
                        width: 12

                        ShapePath {
                            capStyle: ShapePath.RoundCap
                            fillColor: root.buttonColor
                            fillRule: ShapePath.WindingFill
                            startX: 0
                            startY: 0
                            strokeColor: 'transparent'

                            PathLine {
                                x: 12
                                y: 5
                            }
                            PathLine {
                                x: 0
                                y: 10
                            }
                            PathLine {
                                x: 0
                                y: 0
                            }
                        }
                    }
                }

                MouseArea {
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent

                    onPressed: mouse => {
                        if (loopEnabled.value ^ mouse.button == Qt.RightButton) {
                            loopDouble.trigger();
                        }
                        loopSizeRepeater.selectedIndex = Math.min(loopSizeRepeater.values.length - 1, loopSizeRepeater.selectedIndex + 1);
                    }
                }
            }
        }
    }
}
