import Mixxx 1.0 as Mixxx
import QtQuick

Rectangle {
    id: root

    readonly property bool isClassic: Mixxx.Core.colorScheme === "Classic"
    readonly property string schemeDirectory: isClassic ? "classic" : "palemoon"
    readonly property color backgroundColor: isClassic ? "#1e1e1e" : "#0f0f0f"
    readonly property int logoWidth: isClassic ? 162 : 160
    readonly property int logoHeight: isClassic ? 42 : 40
    readonly property int progressBarWidth: isClassic ? 160 : 164
    readonly property int progressBarHeight: 5

    required property int progress

    color: backgroundColor

    function asset(fileName) {
        return Qt.resolvedUrl("../LateNight/" + schemeDirectory + "/style/" + fileName);
    }

    Accessible.name: Mixxx.Core.initializationService

    Column {
        anchors.centerIn: parent
        spacing: 6

        Image {
            height: root.logoHeight
            source: root.asset("mixxx_logo.svg")
            sourceSize.height: height
            sourceSize.width: width
            width: root.logoWidth
        }

        Item {
            height: root.progressBarHeight
            width: root.progressBarWidth

            Image {
                anchors.fill: parent
                source: root.asset("progressbar_bg.svg")
                sourceSize.height: height
                sourceSize.width: width
            }

            Item {
                clip: true
                height: parent.height
                width: parent.width * Math.min(100, Math.max(0, root.progress)) / 100

                Image {
                    height: parent.height
                    source: root.asset("progressbar.svg")
                    sourceSize.height: height
                    sourceSize.width: root.progressBarWidth
                    width: root.progressBarWidth
                }
            }
        }
    }
}
