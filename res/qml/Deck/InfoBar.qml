import ".." as Skin
import Mixxx 1.0 as Mixxx
import Mixxx.Controls 1.0 as MixxxControls
import Qt5Compat.GraphicalEffects
import QtQuick 2.12
import QtQuick.Shapes
import QtQuick.Layouts
import QtQml.Models // DelegateChoice for Qt >= 6.9
import Qt.labs.qmlmodels // DelegateChooser
import "../Theme"

Rectangle {
    id: root
    property bool minimized: false

    required property string group
    required property int rightColumnWidth
    property var deckPlayer: Mixxx.PlayerManager.getPlayer(group)
    readonly property var currentTrack: deckPlayer.currentTrack
    property color lineColor: Theme.deckLineColor

    property bool editMode: false

    border.width: 2
    border.color: Theme.deckBackgroundColor
    radius: 5

    Image {
        id: coverArt

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 5
        width: height
        source: root.currentTrack.coverArtUrl
        visible: false
        asynchronous: true
    }

    Rectangle {
        id: coverArtCircle

        anchors.fill: coverArt
        radius: 4
        visible: false
    }

    OpacityMask {
        id: coverArtMask
        visible: root.deckPlayer.isLoaded && !root.minimized
        anchors.fill: coverArt
        source: coverArt
        maskSource: coverArtCircle

        Skin.FadeBehavior on visible {
            fadeTarget: coverArtMask
        }
    }

    component Cell: Item {
        id: cell

        required property int index
        property alias item: data
        property bool showSeparator: index != parent.model.count - 1 && root.deckPlayer.isLoaded
        readonly property bool isTop: !!parent.isTop

        Layout.fillWidth: index == 0
        Layout.preferredWidth: index == 0 ? 0 : rightColumnWidth
        Layout.fillHeight: true
        Layout.leftMargin: index == 0 ? 10 : 0
        Skin.EmbeddedText {
            id: data
            anchors.fill: parent
            visible: root.deckPlayer.isLoaded
            horizontalAlignment: index == 0 ? Text.AlignLeft : Text.AlignHCenter
            font.bold: isTop
            font.pixelSize: isTop ? 18 : Theme.textFontPixelSize
            color: Theme.white

            Skin.FadeBehavior on visible {
                fadeTarget: data
            }
        }

        Rectangle {
            id: separator
            visible: showSeparator
            anchors.top: cell.top
            anchors.bottom: cell.bottom
            anchors.right: cell.right
            anchors.topMargin: isTop ? 5 : 0
            anchors.bottomMargin: isTop ? 0 : 5
            width: 2
            color: root.lineColor

            Skin.FadeBehavior on visible {
                fadeTarget: separator
            }
        }
    }

    DelegateChooser {
        id: cellDelegate
        role: "type"
        DelegateChoice {
            roleValue: "title"
            Cell {
                item.visible: true
                item.font.bold: false
                item.font.weight: root.deckPlayer.isLoaded ? Font.DemiBold : Font.Thin
                item.text: root.deckPlayer.isLoaded ? root.currentTrack.title : "No track loaded"
            }
        }
        DelegateChoice {
            roleValue: "artist"
            Cell {
                item.text: root.currentTrack.artist
            }
        }
        DelegateChoice {
            roleValue: "year"
            Cell {
                item.text: root.currentTrack.year
            }
        }
        DelegateChoice {
            roleValue: "remaining"
            Cell {
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

                readonly property real remaining: durationControl.value * (1 - playPositionControl.value)

                item.text: `-${parseInt(remaining / 60).toString().padStart(2, '0')}:${parseInt(remaining % 60).toString().padStart(2, '0')}.${(remaining % 1).toFixed(1)}`
            }
        }
        DelegateChoice {
            roleValue: "rating"
            Item {
                id: cell

                required property int index
                property bool showSeparator: index != parent.model.count - 1 && root.deckPlayer.isLoaded

                property real ratio: ((rateRatioControl.value - 1) * 100).toPrecision(2)

                Mixxx.ControlProxy {
                    id: rateRatioControl

                    group: root.group
                    key: "rate_ratio"
                }

                Layout.fillWidth: index == 0
                Layout.preferredWidth: index == 0 ? 0 : rightColumnWidth
                Layout.fillHeight: true

                Row {
                    id: stars
                    visible: root.deckPlayer.isLoaded
                    anchors.centerIn: parent
                    spacing: 0
                    Repeater {
                        model: 5
                        Shape {
                            id: star
                            antialiasing: true
                            layer.enabled: true
                            layer.samples: 4
                            // Layout.preferredWidth: 16
                            // Layout.preferredHeight: 14
                            width: 16
                            height: 14
                            ShapePath {
                                fillColor: mouse.containsMouse && !(mouse.pressedButtons & Qt.RightButton) && mouse.mouseX > star.x + stars.x ? "#3a60be" : (!mouse.containsMouse || mouse.pressedButtons & Qt.RightButton) && root.currentTrack.stars > index ? (mouse.containsMouse ? "#7D3B3B" : '#D9D9D9') : '#96d9d9d9'
                                strokeColor: 'transparent'
                                startX: 8; startY: 0
                                PathLine { x: 9.78701; y: 5.18237; }
                                PathLine { x: 15.3496; y: 5.18237; }
                                PathLine { x: 10.8494; y: 8.38525; }
                                PathLine { x: 12.5683; y: 13.5676; }
                                PathLine { x: 8.06808; y: 10.3647; }
                                PathLine { x: 3.56787; y: 13.5676; }
                                PathLine { x: 5.2868; y: 8.38525; }
                                PathLine { x: 0.786587; y: 5.18237; }
                                PathLine { x: 6.34915; y: 5.18237; }
                                PathLine { x: 8.06808; y: 0; }
                            }
                        }
                    }
                }
                MouseArea {
                    id: mouse
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: (event) => {
                        let selectedStars = Math.ceil((mouseX - stars.x)/16);
                        if (event.button === Qt.RightButton) {
                            root.currentTrack.stars = 0
                        } else if (selectedStars >= 0 && selectedStars <= 5) {
                            root.currentTrack.stars = selectedStars;
                            console.warn(root.currentTrack.stars)
                        }
                    }
                }

                Rectangle {
                    id: separator
                    visible: showSeparator
                    anchors.top: cell.top
                    anchors.bottom: cell.bottom
                    anchors.right: cell.right
                    anchors.topMargin: 5
                    width: 2
                    color: root.lineColor

                    Skin.FadeBehavior on visible {
                        fadeTarget: separator
                    }
                }
            }
        }
    }

    property list<string> availableData: [
        "none",
        "title",
        "year",
        "remaining",
        "artist",
        "rating"
    ]

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
            required property string type
            readonly property var modelData: parent.model

            model: root.availableData
            currentIndex: root.availableData.indexOf(type)

            onCurrentIndexChanged: {
                modelData.setProperty(index, "type", root.availableData[currentIndex])
            }

            Layout.fillWidth: index == 0
            Layout.preferredWidth: index == 0 ? 0 : rightColumnWidth
            Layout.fillHeight: true
        }
    }

    ColumnLayout {
        anchors.top: root.top
        anchors.left: coverArt.right
        anchors.right: root.right
        anchors.bottom: root.bottom
        spacing: 0
        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            id: topRow
            property var model: topRowModel
            readonly property bool isTop: true
            Repeater {
                model: parent.model
                delegate: root.editMode ? editCellDelegate : cellDelegate
            }
        }
        Rectangle {
            Layout.fillWidth: true
            visible: !root.minimized

            height: 2
            color: root.lineColor
        }
        RowLayout {
            Layout.fillHeight: true
            Layout.fillWidth: true
            id: bottomRow
            visible: !root.minimized
            property var model: bottomRowModel
            Repeater {
                model: parent.model
                delegate: root.editMode ? editCellDelegate : cellDelegate
            }
        }
    }

    gradient: Gradient {
        orientation: Gradient.Horizontal

        GradientStop {
            position: 0
            color: {
                const trackColor = root.currentTrack.color;
                if (!trackColor.valid)
                    return Theme.deckBackgroundColor;

                return Qt.darker(root.currentTrack.color, 2);
            }
        }

        GradientStop {
            position: 1
            color: Theme.deckBackgroundColor
        }
    }
}
