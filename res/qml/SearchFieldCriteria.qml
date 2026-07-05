import QtQuick

FocusScope {
    id: root
    width: filedLabel.width + 18 + field.width
    height: 24
    required property string field
    property var query: undefined
    property bool interactive: false
    property alias textInput: searchField

    readonly property var regExp: new RegExp(root.query, "i")

    signal edited
    signal deleted

    TapHandler {
        onTapped: searchField.forceActiveFocus()
    }

    Rectangle {
        width: parent.width
        height: parent.height
        focus: true

        radius: 7
        color: '#2D4EA1'
        Text {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 5
            id: filedLabel
            text: (root.query ? root.field.replace(root.regExp, `**${root.query}**`) : root.field) + ":"

            textFormat: Text.MarkdownText
        }
        Rectangle {
            id: field
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 5
            width: searchField.activeFocus ? 162 : fontMetrics.advanceWidth(searchField.text.length ? searchField.text : "...") + 4
            height: 18
            radius: 7
            color: '#D9D9D9'
            FontMetrics {
                id: fontMetrics
                font: searchField.font
            }
            Text {
                visible: !root.interactive
                anchors.fill: parent
                text: searchField.text ?? "..."
            }
            Item {
                anchors.fill: parent
                visible: root.interactive
                TextInput {
                    id: searchField
                    anchors.fill: parent
                    focus: true
                    clip: true
                    color: "#808080"
                    horizontalAlignment: TextInput.AlignLeft

                    Keys.onPressed: (event) => {
                        if (event.key == Qt.Key_Backspace && searchField.text.length == 0) {
                            root.deleted()
                            event.accepted = true
                        }
                    }

                    onActiveFocusChanged: {
                        if (!activeFocus && text.length == 0){
                            root.deleted()
                        }
                    }

                    onTextEdited: {
                        root.edited()
                    }
                }
                Text {
                    id: searchPlaceholder
                    visible: searchField.text.length == 0
                    anchors.fill: parent
                    color: "#808080"
                    text: "Press “=” for an exact match"
                    elide: Text.ElideRight
                }
            }
        }
    }
}
