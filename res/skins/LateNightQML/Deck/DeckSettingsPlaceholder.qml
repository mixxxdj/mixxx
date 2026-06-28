import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group

    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(root.group)
    readonly property var currentTrack: deckPlayer?.currentTrack

    readonly property bool isDeck12: root.group === "[Channel1]" || root.group === "[Channel2]"
    readonly property color starsColor: isDeck12 ? LateNightTheme.starsColor12 : LateNightTheme.starsColor34
    readonly property string activeSuffix: isDeck12 ? "active_12" : "active_34"

    implicitWidth: 76
    implicitHeight: 63

    Mixxx.ControlProxy {
        id: bpmLockControl
        group: root.group
        key: "bpmlock"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 5
        spacing: 2

        Item {
            id: starsContainer
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredWidth: 75
            Layout.preferredHeight: 15

            readonly property int hoveredStarCount: {
                if (!mouseStars.containsMouse) {
                    return -1;
                }
                var localX = mouseStars.mouseX - starsRow.x;
                if (localX < 3.75) {
                    return 0;
                }
                if (localX >= 75) {
                    return -1;
                }
                var star = Math.floor(localX / 15) + 1;
                return star > 5 ? 5 : star;
            }

            Row {
                id: starsRow
                anchors.centerIn: parent
                spacing: 0

                Repeater {
                    model: 5

                    delegate: Item {
                        id: starItem
                        width: 15
                        height: 15

                        readonly property bool isFilled: {
                            if (starsContainer.hoveredStarCount >= 0) {
                                return index < starsContainer.hoveredStarCount;
                            }
                            return root.currentTrack && root.currentTrack.stars > index;
                        }

                        Canvas {
                            anchors.fill: parent

                            property bool filled: starItem.isFilled
                            property color shapeColor: root.starsColor

                            renderStrategy: Canvas.Immediate

                            onFilledChanged: requestPaint()
                            onShapeColorChanged: requestPaint()

                            onPaint: {
                                const ctx = getContext("2d");
                                ctx.clearRect(0, 0, width, height);
                                ctx.fillStyle = shapeColor;
                                ctx.beginPath();
                                if (filled) {
                                    ctx.moveTo(15.0, 7.5);
                                    ctx.lineTo(1.4324, 11.9084);
                                    ctx.lineTo(9.8176, 0.3671);
                                    ctx.lineTo(9.8176, 14.6329);
                                    ctx.lineTo(1.4324, 3.0916);
                                } else {
                                    ctx.moveTo(6.0, 7.5);
                                    ctx.lineTo(7.5, 6.0);
                                    ctx.lineTo(9.0, 7.5);
                                    ctx.lineTo(7.5, 9.0);
                                }
                                ctx.closePath();
                                ctx.fill();
                            }

                            Component.onCompleted: requestPaint()
                        }
                    }
                }
            }

            MouseArea {
                id: mouseStars
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton

                onClicked: function(mouse) {
                    if (!root.currentTrack) {
                        return;
                    }
                    if (mouse.button === Qt.RightButton) {
                        root.currentTrack.stars = 0;
                    } else {
                        var clickedStar = starsContainer.hoveredStarCount;
                        if (clickedStar >= 0) {
                            root.currentTrack.stars = clickedStar;
                        }
                    }
                }
            }
        }

        // Row 1: Slip, Quantize, Curpos (lock match)
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4

            // Slip mode toggle
            LateNightControlButton {
                Layout.preferredWidth: 21
                Layout.preferredHeight: 18
                backgroundSource: LateNightTheme.legacyButton("btn__.svg")
                iconSource: LateNightTheme.assetDeckSlipButton
                activeIconSuffix: root.activeSuffix
                stretchIcon: true
                inactiveFillEnabled: false
                group: root.group
                key: "slip_enabled"
                toggleable: true
                activeOpacity: 1.0
                inactiveOpacity: 0.72
            }

            // Quantize toggle
            LateNightControlButton {
                Layout.preferredWidth: 21
                Layout.preferredHeight: 18
                backgroundSource: LateNightTheme.legacyButton("btn__.svg")
                iconSource: LateNightTheme.assetDeckQuantizeButton
                activeIconSuffix: root.activeSuffix
                stretchIcon: true
                inactiveFillEnabled: false
                group: root.group
                key: "quantize"
                toggleable: true
                activeOpacity: 1.0
                inactiveOpacity: 0.72
            }

            // Curpos button
            LateNightControlButton {
                id: curposBtn
                Layout.preferredWidth: 21
                Layout.preferredHeight: 18
                backgroundSource: LateNightTheme.legacyButton("btn__.svg")
                iconSource: LateNightTheme.assetDeckBeatCurposButton
                activeIconSuffix: root.activeSuffix
                stretchIcon: true
                inactiveFillEnabled: false
                group: root.group
                key: "beats_translate_curpos"
                rightClickKey: "beats_translate_match_alignment"
                toggleable: false
                activeOpacity: 1.0
                inactiveOpacity: 0.72

                // Cover rectangle when BPM lock is active
                Rectangle {
                    anchors.fill: parent
                    color: "#b419191a" // rgba(25, 25, 26, 180) cover
                    visible: bpmLockControl.value > 0
                }
            }
        }

        // Row 2: Eject, Repeat, Keylock
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            spacing: 4

            // Eject button: momentary push
            LateNightControlButton {
                Layout.preferredWidth: 21
                Layout.preferredHeight: 18
                backgroundSource: LateNightTheme.legacyButton("btn__.svg")
                iconSource: LateNightTheme.assetDeckEjectButton
                activeIconSuffix: root.activeSuffix
                stretchIcon: true
                inactiveFillEnabled: false
                group: root.group
                key: "eject"
                toggleable: false
                activeOpacity: 0.95
                inactiveOpacity: 0.72
            }

            // Repeat toggle
            LateNightControlButton {
                Layout.preferredWidth: 21
                Layout.preferredHeight: 18
                backgroundSource: LateNightTheme.legacyButton("btn__.svg")
                iconSource: LateNightTheme.assetDeckRepeatButton
                activeIconSuffix: root.activeSuffix
                stretchIcon: true
                inactiveFillEnabled: false
                group: root.group
                key: "repeat"
                toggleable: true
                activeOpacity: 1.0
                inactiveOpacity: 0.72
            }

            // Keylock toggle
            LateNightControlButton {
                Layout.preferredWidth: 21
                Layout.preferredHeight: 18
                backgroundSource: LateNightTheme.legacyButton("btn__.svg")
                iconSource: LateNightTheme.assetDeckKeylockButton
                activeIconSuffix: root.activeSuffix
                stretchIcon: true
                inactiveFillEnabled: false
                group: root.group
                key: "keylock"
                toggleable: true
                activeOpacity: 1.0
                inactiveOpacity: 0.72
            }
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
