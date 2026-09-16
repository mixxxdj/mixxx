import QtQuick

Item {
    id: root

    property string field: ""
    property string value: ""
    property bool exact: false
    property bool active: false
    property bool interactive: true

    property alias valueEditorHost: valueArea

    readonly property real valueAreaX: width - 5 - valueArea.width

    signal activated()
    signal deleted()

    height: 24
    width: labelText.width + valueArea.width + 18

    TapHandler {
        enabled: root.interactive
        onTapped: root.activated()
        onDoubleTapped: root.deleted()
    }

    Rectangle {
        anchors.fill: parent
        radius: 7
        color: root.active ? '#3A60BE' : '#2D4EA1'
        border.color: root.active ? '#5C82D6' : 'transparent'
        border.width: 1

        Text {
            id: labelText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 5
            text: root.field + ":"
            color: '#FFFFFF'
            font.pixelSize: 14
        }

        Rectangle {
            id: valueArea
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5
            width: root.active ? 162 : Math.max(valueMetrics.advanceWidth(root.value.length ? (root.exact ? "=" : "") + root.value : "..."), 20) + 8
            height: 18
            radius: 7
            color: '#D9D9D9'

            FontMetrics {
                id: valueMetrics
                font: valueText.font
            }

            Text {
                id: valueText
                visible: !root.active
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignLeft
                clip: true
                color: '#404040'
                font.pixelSize: 14
                text: root.value.length ? (root.exact ? "=" : "") + root.value : "..."
            }
        }
    }
}
