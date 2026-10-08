import QtQuick

Item {
    id: root

    property string field: ""
    property string value: ""
    property bool active: false
    property bool interactive: true

    property alias valueEditorHost: valueArea

    signal activated()
    signal deleted()

    height: 24
    width: labelText.width + valueArea.width + 14

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
            text: root.field.length > 0 ? root.field + ":" : root.field
            color: '#FFFFFF'
            font.pixelSize: 14
        }

        Rectangle {
            id: valueArea
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5
            width: root.active ? 132 : Math.max(valueMetrics.advanceWidth(root.value.length ? root.value : "..."), 20) + 10

            Behavior on width {
                PropertyAnimation { duration: 200 }
            }

            height: 18
            radius: 7
            color: '#D9D9D9'

            FontMetrics {
                id: valueMetrics
                font: valueText.font
            }

            Text {
                id: exactIndicator
                visible: !root.active && root.value.startsWith('=')
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                text: "="
                color: '#404040'
                opacity: 0.4
                font.pixelSize: 14
                font.weight: Font.Light
            }

            Text {
                id: valueText
                visible: !root.active
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.right: parent.right
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                leftPadding: exactIndicator.visible ? exactIndicator.width : 0
                clip: true
                color: root.value.length === 0 ? '#808080' : '#404040'
                font.pixelSize: 14
                font.italic: root.value.length === 0
                text: root.value.length === 0
                        ? "..."
                        : (root.value.startsWith('=') ? root.value.slice(1) : root.value)
            }
        }
    }
}
