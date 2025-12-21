import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts
import QtMultimedia
import ".." as Skin
import "../Theme"

Row {
    id: root

    required property var screens

    spacing: 10

    Repeater {
        model: root.screens
        Rectangle {
            width: {width=modelData.size.width}
            height: {height=modelData.size.height}
            color: 'black'
            Text {
                anchors.centerIn: parent
                color: Theme.white
                font.pixelSize: 14
                text: qsTr("Off")
            }

            VideoOutput {
                id: videoOutput
                Layout.column: 0
                Layout.row: 0
                Layout.columnSpan: 2
                Component.onCompleted: {
                    modelData.connectVideoSink(videoOutput.videoSink)
                }
            }
            Rectangle {
                anchors {
                    bottom: parent.bottom
                    left: parent.left
                }
                height: identifierText.implicitHeight
                width: identifierText.implicitWidth
                color: 'black'
                Text {
                    id: identifierText
                    color: Theme.white
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    text: modelData.identifier
                }
            }
            Text {
                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }
                height: fpsText.implicitHeight
                width: fpsText.implicitWidth
                color: 'black'
                Text {
                    id: fpsText
                    color: Theme.white
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    text: qsTr("FPS: %0/%1").arg(modelData.currentFps).arg(modelData.targetFps)
                }
            }
        }
    }
}
