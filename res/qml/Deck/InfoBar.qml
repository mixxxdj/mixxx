import ".." as Skin
import Mixxx 1.0 as Mixxx
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Layouts
import QtQml.Models // DelegateChoice for Qt >= 6.9
import Qt.labs.qmlmodels // DelegateChooser
import "../Theme"

Rectangle {
    id: root

    property list<string> availableData: ["none", "title", "year", "remaining", "artist", "rating"]
    readonly property var currentTrack: deckPlayer?.currentTrack
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    property bool editMode: false
    required property string group
    property color lineColor: Theme.deckLineColor
    property bool minimized: false
    required property int rightColumnWidth

    radius: 5

    gradient: Gradient {
        orientation: Gradient.Horizontal

        GradientStop {
            color: {
                const trackColor = root.currentTrack?.color;
                if (!trackColor.valid)
                    return Theme.deckInfoBarBackgroundColor;

                return Qt.darker(root.currentTrack?.color, 2);
            }
            position: 0
        }
        GradientStop {
            color: Theme.deckInfoBarBackgroundColor
            position: 1
        }
    }

    Image {
        id: coverArt

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.top: parent.top
        asynchronous: true
        source: root.currentTrack?.coverArtUrl
        visible: false
        width: height
    }
    Rectangle {
        id: coverArtCircle

        anchors.fill: coverArt
        color: Theme.deckEmptyCoverArt
        radius: 4
        visible: !root.deckPlayer?.isLoaded && !root.minimized
    }
    OpacityMask {
        id: coverArtMask

        anchors.fill: coverArt
        maskSource: coverArtCircle
        source: coverArt
        visible: root.deckPlayer?.isLoaded && !root.minimized

        Skin.FadeBehavior on visible {
            fadeTarget: coverArtMask
        }
    }
    DelegateChooser {
        id: cellDelegate

        role: "type"

        DelegateChoice {
            roleValue: "title"

            Cell {
                item.font.bold: false
                item.font.weight: root.deckPlayer?.isLoaded ? Font.DemiBold : Font.Thin
                item.text: root.deckPlayer?.isLoaded ? root.currentTrack?.title : "No track loaded"
                item.visible: true
            }
        }
        DelegateChoice {
            roleValue: "artist"

            Cell {
                item.text: root.currentTrack?.artist
            }
        }
        DelegateChoice {
            roleValue: "year"

            Cell {
                visible: root.width > 500 && root.currentTrack?.year
                item.text: root.currentTrack?.year
            }
        }
        DelegateChoice {
            roleValue: "remaining"

            Cell {
                visible: root.width > 450
                readonly property real remaining: durationControl.value * (1 - playPositionControl.value)

                item.text: `-${parseInt(remaining / 60).toString().padStart(2, '0')}:${parseInt(remaining % 60).toString().padStart(2, '0')}.${(remaining % 1).toFixed(1)}`

                Mixxx.ControlProxy {
                    id: durationControl

                    group: root.group
                    key: "duration"
                }
                Mixxx.ControlProxy {
                    id: playPositionControl

                    group: root.group
                    key: "playposition"
                }
            }
        }
        DelegateChoice {
            roleValue: "rating"

            Item {
                id: cell

                required property int index
                property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)
                property bool showSeparator: index != parent.model.count - 1 && root.deckPlayer?.isLoaded

                Layout.fillHeight: true
                Layout.fillWidth: index == 0
                Layout.preferredWidth: index == 0 ? 0 : rightColumnWidth
                visible: root.width > 400

                Mixxx.ControlProxy {
                    id: rateRatioControl

                    group: root.group
                    key: "rate_ratio"
                }
                Row {
                    id: stars

                    anchors.centerIn: parent
                    spacing: 0
                    visible: root.deckPlayer?.isLoaded

                    Repeater {
                        model: 5

                        Shape {
                            id: star

                            antialiasing: true
                            height: 14
                            layer.enabled: true
                            layer.samples: 4
                            width: 16

                            ShapePath {
                                fillColor: mouse.containsMouse && !(mouse.pressedButtons & Qt.RightButton) && mouse.mouseX > star.x + stars.x ? "#3a60be" : (!mouse.containsMouse || mouse.pressedButtons & Qt.RightButton) && root.currentTrack?.stars > index ? (mouse.containsMouse ? "#7D3B3B" : '#D9D9D9') : '#96d9d9d9'
                                startX: 8
                                startY: 0
                                strokeColor: 'transparent'

                                PathLine {
                                    x: 9.78701
                                    y: 5.18237
                                }
                                PathLine {
                                    x: 15.3496
                                    y: 5.18237
                                }
                                PathLine {
                                    x: 10.8494
                                    y: 8.38525
                                }
                                PathLine {
                                    x: 12.5683
                                    y: 13.5676
                                }
                                PathLine {
                                    x: 8.06808
                                    y: 10.3647
                                }
                                PathLine {
                                    x: 3.56787
                                    y: 13.5676
                                }
                                PathLine {
                                    x: 5.2868
                                    y: 8.38525
                                }
                                PathLine {
                                    x: 0.786587
                                    y: 5.18237
                                }
                                PathLine {
                                    x: 6.34915
                                    y: 5.18237
                                }
                                PathLine {
                                    x: 8.06808
                                    y: 0
                                }
                            }
                        }
                    }
                }
                MouseArea {
                    id: mouse

                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: event => {
                        if (!root.currentTrack) {
                            return;
                        }
                        let selectedStars = Math.ceil((mouseX - stars.x) / 16);
                        if (event.button === Qt.RightButton) {
                            root.currentTrack.stars = 0;
                        } else if (selectedStars >= 0 && selectedStars <= 5) {
                            root.currentTrack.stars = selectedStars;
                        }
                    }
                }
                Rectangle {
                    id: separator

                    anchors.bottom: cell.bottom
                    anchors.right: cell.right
                    anchors.top: cell.top
                    anchors.topMargin: 5
                    color: root.lineColor
                    visible: showSeparator
                    width: 2

                    Skin.FadeBehavior on visible {
                        fadeTarget: separator
                    }
                }
            }
        }
    }
    ListModel {
        id: topRowModel

        ListElement {
            type: "title"
        }
        ListElement {
            type: "year"
        }
        ListElement {
            type: "remaining"
        }
    }
    ListModel {
        id: bottomRowModel

        ListElement {
            type: "artist"
        }
        ListElement {
            type: "none"
        }
        ListElement {
            type: "rating"
        }
    }
    Component {
        id: editCellDelegate

        Skin.ComboBox {
            required property int index
            readonly property var modelData: parent.model
            required property string type

            Layout.fillHeight: true
            Layout.fillWidth: index == 0
            Layout.preferredWidth: index == 0 ? 0 : rightColumnWidth
            currentIndex: root.availableData.indexOf(type)
            model: root.availableData

            onCurrentIndexChanged: {
                modelData.setProperty(index, "type", root.availableData[currentIndex]);
            }
        }
    }
    ColumnLayout {
        anchors.bottom: root.bottom
        anchors.left: coverArt.right
        anchors.right: root.right
        anchors.top: root.top
        spacing: 0

        RowLayout {
            id: topRow

            readonly property bool isTop: true
            property var model: topRowModel

            Layout.fillHeight: true
            Layout.fillWidth: true

            Repeater {
                delegate: root.editMode ? editCellDelegate : cellDelegate
                model: parent.model
            }
        }
        Rectangle {
            Layout.fillWidth: true
            color: root.lineColor
            height: 2
            visible: !root.minimized
        }
        RowLayout {
            id: bottomRow

            property var model: bottomRowModel

            Layout.fillHeight: true
            Layout.fillWidth: true
            visible: !root.minimized

            Repeater {
                delegate: root.editMode ? editCellDelegate : cellDelegate
                model: parent.model
            }
        }
    }

    component Cell: Item {
        id: cell

        required property int index
        readonly property bool isTop: !!parent.isTop
        property alias item: data
        property bool showSeparator: index != parent.model.count - 1 && root.deckPlayer?.isLoaded

        Layout.fillHeight: true
        Layout.fillWidth: index == 0
        Layout.leftMargin: index == 0 ? 10 : 0
        Layout.preferredWidth: index == 0 ? 0 : rightColumnWidth

        Skin.EmbeddedText {
            id: data

            anchors.fill: parent
            color: Theme.white
            font.bold: isTop
            font.pixelSize: isTop ? 18 : Theme.textFontPixelSize
            horizontalAlignment: index == 0 ? Text.AlignLeft : Text.AlignHCenter
            visible: root.deckPlayer?.isLoaded

            Skin.FadeBehavior on visible {
                fadeTarget: data
            }
        }
        Rectangle {
            id: separator

            anchors.bottom: cell.bottom
            anchors.bottomMargin: isTop ? 0 : 5
            anchors.right: cell.right
            anchors.top: cell.top
            anchors.topMargin: isTop ? 5 : 0
            color: root.lineColor
            visible: showSeparator
            width: 2

            Skin.FadeBehavior on visible {
                fadeTarget: separator
            }
        }
    }
}
