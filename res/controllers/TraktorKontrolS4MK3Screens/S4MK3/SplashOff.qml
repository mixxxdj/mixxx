import QtQuick 2.15

Rectangle {
    id: root
    anchors.fill: parent
    color: "black"

    Image {
        anchors.centerIn: parent
        width: root.width*0.8
        height: root.height
        fillMode: Image.PreserveAspectFit
        source: engine.getSetting("idleBackground") || "../../../images/templates/logo_mixxx.png"
    }
}
