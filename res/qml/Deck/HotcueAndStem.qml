import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Layouts
import QtQuick.Shapes
import QtQuick.Controls 2.12
import "../Theme"

Item {
    id: root

    required property string group
    required property var currentTrack

    Mixxx.ControlProxy {
        id: stemCountControl

        group: root.group
        key: "stem_count"
    }

    Rectangle {
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
            right: tabs.left
            rightMargin: 6
        }

        color: '#0E0E0E'

        states: [
            State {
                name: "inactive"
                when: trackLoadedControl.value == 0

                PropertyChanges {
                    target: hotcueTabButton
                    checked: false
                    opacity: 0.75
                }

                PropertyChanges {
                    target: stemTabButton
                    checked: false
                    opacity: 0.75
                }

                PropertyChanges {
                    target: stemTab
                    visible: false
                }

                PropertyChanges {
                    target: hotcueTab
                    visible: false
                }
            },
            State {
                name: "hotcue"
                when: hotcueTabButton.checked || stemCountControl.value == 0

                PropertyChanges {
                    target: hotcueTab
                    visible: true
                }

                PropertyChanges {
                    target: stemTab
                    visible: false
                }
            },
            State {
                name: "stem"
                when: stemTabButton.checked && stemCountControl.value != 0

                PropertyChanges {
                    target: stemTab
                    visible: true
                }

                PropertyChanges {
                    target: hotcueTab
                    visible: false
                }
            }
        ]

        Item {
            id: hotcueTab
            anchors.fill: parent
            visible: false
            GridLayout {
                rowSpacing: 6
                columnSpacing: 6
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16
                anchors.topMargin: 8
                anchors.bottomMargin: 8
                Repeater {
                    model: 8
                    Item {
                        required property int index

                        Skin.Hotcue {
                            id: hotcue

                            readonly property var label: isSet ? root.currentTrack.hotcuesModel.get(index).label : null

                            group: root.group
                            hotcueNumber: index + 1
                            activate: activator.pressedButtons == Qt.LeftButton
                            // onIsSetChanged: {
                            //     if (!isSet)
                            //         popup.close();
                            // }
                        }
                        Layout.column: index % 4
                        Layout.row: parseInt(index / 4)
                        Layout.fillWidth: true
                        Layout.fillHeight: true

                        Rectangle {
                            id: backgroundImage

                            anchors.fill: parent
                            color: hotcue.isSet ? hotcue.color : '#2B2B2B'

                            MouseArea {
                                anchors.fill: parent
                                acceptedButtons: Qt.LeftButton | Qt.RightButton
                                id: activator
                            }
                        }

                        DropShadow {
                            id: effect1
                            anchors.fill: backgroundImage
                            source: backgroundImage
                            horizontalOffset: 0
                            verticalOffset: 0
                            radius: 1.0
                            color: "#80000000"
                        }
                        InnerShadow {
                            id: effect2
                            anchors.fill: parent
                            source: effect1
                            spread: 0.2
                            radius: 12
                            samples: 24
                            horizontalOffset: 1
                            verticalOffset: 1
                            color: "#353535"
                        }
                        InnerShadow {
                            anchors.fill: parent
                            source: effect2
                            spread: 0.2
                            radius: 12
                            samples: 24
                            horizontalOffset: -1
                            verticalOffset: -1
                            color: "#353535"
                        }
                        ColumnLayout {
                            anchors.centerIn: backgroundImage
                            spacing: 0
                            Label {
                                Layout.alignment: Qt.AlignHCenter
                                text: `${index+1}`
                                color: "#626262"
                                font.weight: Font.Bold
                                font.pixelSize: 14
                            }
                            Label {
                                Layout.alignment: Qt.AlignHCenter
                                visible: hotcue.label
                                text: hotcue.label ?? ""
                                color: "#626262"
                                font.pixelSize: 12
                            }
                        }
                    }
                }
            }

            Skin.FadeBehavior on visible {
                fadeTarget: hotcueTab
            }
        }
        Item {
            id: stemTab
            anchors.fill: parent
            visible: false
            RowLayout {
                spacing: 9
                anchors.fill: parent
                anchors.leftMargin: 9
                anchors.rightMargin: 9
                anchors.topMargin: 6
                anchors.bottomMargin: 6
                Repeater {
                    model: root.currentTrack.stemsModel
                    Item {
                        id: stem
                        required property int index
                        required property string label
                        required property color color
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        readonly property string group: `${root.group.substr(0, root.group.length-1)}_Stem${index + 1}]`
                        readonly property string fxGroup: `[QuickEffectRack1_${group}]`

                        Item {
                            id: content
                            anchors.fill: parent
                            visible: false
                            Rectangle {
                                id: backgroundColor

                                anchors.fill: parent
                                radius: 1
                                opacity: 0.75
                                color: stem.color
                            }
                        }

                        InnerShadow {
                            anchors.fill: parent
                            horizontalOffset: 0
                            verticalOffset: 4
                            radius: 4.0
                            spread: 0
                            samples: 24
                            color: "#4b000000"
                            smooth: true

                            source: content
                        }
                        Item {
                            id: stemButton
                            width: parent.width / 3 * 2
                            height: parent.height / 3 * 2
                            anchors {
                                top: parent.top
                                left: parent.left
                                bottom: stemFxSelector.top
                            }
                            Item {
                                id: contentStemButton
                                anchors.fill: parent
                                visible: false
                                Rectangle {
                                    anchors.fill: parent
                                    color: stem.color
                                    opacity: stemMute.value ? 0.6 : 1.0
                                }
                            }
                            DropShadow {
                                id: effect1
                                anchors.fill: parent
                                horizontalOffset: 0
                                verticalOffset: 0
                                radius: 2.0
                                spread: 0.5
                                color: "#80000000"
                                source: contentStemButton
                            }
                            InnerShadow {
                                id: effect2
                                anchors.fill: parent
                                horizontalOffset: 2
                                verticalOffset: 2
                                radius: 4.0
                                spread: 0.4
                                samples: 24
                                color: "#353535"
                                smooth: true

                                source: effect1
                            }
                            InnerShadow {
                                anchors.fill: parent
                                horizontalOffset: -2
                                verticalOffset: -2
                                radius: 4.0
                                spread: 0.4
                                samples: 24
                                color: "#353535"
                                smooth: true

                                source: effect2
                            }
                            Item {
                                anchors.fill: parent
                                anchors.margins: 6
                                clip: true
                                Label {
                                    id: stemLabel

                                    readonly property bool rotated: fontMetrics.advanceWidth > parent.width && parent.height >= parent.width

                                    anchors.centerIn: parent
                                    width: rotated ? parent.height : parent.width
                                    height: font.pixelSize

                                    text: stem.label
                                    font.weight: Font.Bold
                                    color: Theme.white
                                    elide: Text.ElideRight
                                    horizontalAlignment: rotated ? Text.AlignLeft : Text.AlignHCenter

                                    TextMetrics  {
                                        id: fontMetrics
                                        font: stemLabel.font
                                        text: stem.label
                                    }

                                    transform: Rotation { origin.x: stemLabel.width/2; origin.y: stemLabel.height/2; angle: stemLabel.rotated ? 90 : 0}
                                }
                            }
                            Mixxx.ControlProxy {
                                id: stemMute

                                group: stem.group
                                key: "mute"
                            }
                            TapHandler {
                                onTapped: stemMute.value = !stemMute.value
                            }
                        }
                        Skin.ComboBox {
                            id: stemFxSelector

                            width: parent.width / 3 * 2
                            height: Math.min(parent.width, parent.height) / 3
                            anchors {
                                bottom: parent.bottom
                                left: parent.left
                            }

                            Mixxx.ControlProxy {
                                id: fxSelect

                                group: stem.fxGroup
                                key: "loaded_chain_preset"
                            }

                            spacing: 2
                            indicator.width: 0
                            popupWidth: 100
                            popupMaxItem: 8
                            clip: true

                            textRole: "display"
                            font.pixelSize: 10
                            model: Mixxx.EffectsManager.quickChainPresetModel
                            currentIndex: fxSelect.value == -1 ? 0 : fxSelect.value
                            onActivated: (index) => {
                                fxSelect.value = index
                            }
                        }
                        Item {
                            id: stemVolume
                            width: parent.width / 3
                            anchors {
                                left: stemButton.right
                                right: parent.right
                                top: parent.top
                                bottom: stemFxKnob.top
                                bottomMargin: 3
                            }

                            // Skin.VuMeter {
                            //     x: 15
                            //     y: (parent.height - height) / 2
                            //     width: 4
                            //     height: parent.height - 40
                            //     group: root.group
                            //     key: "vu_meter_left"
                            // }

                            // Skin.VuMeter {
                            //     x: parent.width - width - 15
                            //     y: (parent.height - height) / 2
                            //     width: 4
                            //     height: parent.height - 40
                            //     group: root.group
                            //     key: "vu_meter_right"
                            // }

                            Skin.ControlFader {
                                id: volumeSlider
                                implicitWidth: 10

                                handleImage {
                                    width: parent.width - 4
                                }

                                anchors.fill: parent
                                group: stem.group
                                key: "volume"
                                barColor: Theme.volumeSliderBarColor
                                bg: Theme.imgVolumeSliderBackground
                            }
                        }
                        Item {
                            id: stemFxKnob
                            width: parent.width / 3
                            height: stemFxSelector.height
                            anchors {
                                right: parent.right
                                bottom: parent.bottom
                            }
                            Skin.QuickFxKnob {
                                anchors.centerIn: parent
                                width: Math.min(parent.width, parent.height)
                                height: width
                                knob {
                                    height: width * 0.8
                                    width: width * 0.8
                                }
                                group: stem.fxGroup
                                knob.arcStyle: ShapePath.DashLine
                                knob.arcStylePattern: [2, 2]
                                knob.color: Theme.eqFxColor
                            }
                        }
                    }
                }
            }

            Skin.FadeBehavior on visible {
                fadeTarget: stemTab
            }
        }
    }

    ColumnLayout {
        id: tabs
        width: stemCountControl.value > 0 ? 36 : 0
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.bottom: parent.bottom
        spacing: 10
        Skin.Button {
            id: hotcueTabButton

            activeColor: Theme.deckActiveColor

            onClicked: {
                stemTabButton.checked = false
                hotcueTabButton.checked = trackLoadedControl.value == 1
            }

            checked: true

            Layout.fillWidth: true
            Layout.margins: 0
            implicitHeight: 30

            contentItem: Shape {
                anchors.fill: parent
                antialiasing: true
                layer.enabled: true
                layer.samples: 4
                ShapePath {
                    fillColor: '#D9D9D9'
                    startX: 10; startY: 4
                    PathLine { x: 26; y: 4 }
                    PathLine { x: 19.5; y: 10 }
                    PathLine { x: 19.5; y: 24 }
                    PathLine { x: 16.5; y: 24 }
                    PathLine { x: 16.5; y: 10 }
                    PathLine { x: 10; y: 4 }
                }
            }
        }
        Skin.Button {
            id: stemTabButton

            activeColor: Theme.deckActiveColor

            onClicked: {
                stemTabButton.checked = trackLoadedControl.value == 1 && stemCountControl.value != 0
                hotcueTabButton.checked = !stemTabButton.checked
            }

            Layout.fillWidth: true
            Layout.margins: 0
            implicitHeight: 30

            contentItem: Item {
                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 5
                        right: parent.right
                        rightMargin: 5
                        top: parent.top
                        topMargin: 6
                    }
                }

                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 5
                        right: parent.right
                        rightMargin: 14
                        top: parent.top
                        topMargin: 12
                    }
                }

                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 5
                        right: parent.right
                        rightMargin: 22
                        bottom: parent.bottom
                        bottomMargin: 12
                    }
                }

                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 22
                        right: parent.right
                        rightMargin: 5
                        bottom: parent.bottom
                        bottomMargin: 13
                    }
                }

                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 5
                        right: parent.right
                        rightMargin: 26
                        bottom: parent.bottom
                        bottomMargin: 6
                    }
                }

                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 14
                        right: parent.right
                        rightMargin: 15
                        bottom: parent.bottom
                        bottomMargin: 6
                    }
                }

                Rectangle {
                    color: '#D9D9D9'
                    height: 1
                    anchors {
                        left: parent.left
                        leftMargin: 26
                        right: parent.right
                        rightMargin: 5
                        bottom: parent.bottom
                        bottomMargin: 6
                    }
                }
            }
        }

        Behavior on width {
            SpringAnimation {
                duration: 500
                spring: 2
                damping: 0.2
            }
        }
    }
}
