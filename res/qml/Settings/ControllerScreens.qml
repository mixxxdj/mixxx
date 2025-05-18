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
            color: 'black'

            Binding on height {
                delayed: true
                value: modelData.size.height
            }
            Binding on width {
                delayed: true
                value: modelData.size.width
            }

            Text {
                anchors.centerIn: parent
                color: Theme.white
                font.pixelSize: 14
                text: qsTr("Off")
            }
            VideoOutput {
                id: videoOutput

                Layout.column: 0
                Layout.columnSpan: 2
                Layout.row: 0

                Component.onCompleted: {
                    modelData.connectVideoSink(videoOutput.videoSink);
                }
            }
            Rectangle {
                color: 'black'
                height: identifierText.implicitHeight
                width: identifierText.implicitWidth

                anchors {
                    bottom: parent.bottom
                    left: parent.left
                }
                Text {
                    id: identifierText

                    color: Theme.white
                    font.pixelSize: 14
                    font.weight: Font.DemiBold
                    text: modelData.identifier
                }
            }
            Text {
                color: 'black'
                height: fpsText.implicitHeight
                width: fpsText.implicitWidth

                anchors {
                    bottom: parent.bottom
                    right: parent.right
                }
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
