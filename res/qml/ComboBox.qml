import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

ComboBox {
    id: root

    property alias popupWidth: popup.width
    property bool clip: false

    background: Skin.EmbeddedBackground {
    }

    delegate: ItemDelegate {
        id: itemDlgt

        required property int index

        width: parent.width
        highlighted: root.highlightedIndex === this.index
        text: root.textAt(this.index)

        contentItem: Text {
            text: itemDlgt.text
            color: Theme.deckTextColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            radius: 5
            border.width: itemDlgt.highlighted ? 1 : 0
            border.color: Theme.deckLineColor
            color: "transparent"
        }
    }

    contentItem: Text {
        leftPadding: 5
        rightPadding: root.indicator.width + root.spacing
        text: root.displayText
        font: root.font
        color: Theme.deckTextColor
        verticalAlignment: Text.AlignVCenter
        elide: root.clip ? Text.ElideNone : Text.ElideRight
        clip: root.clip
    }

    popup: Popup {
        id: popup
        y: root.height
        width: root.width
        implicitHeight: contentItem.implicitHeight

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null
            currentIndex: root.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator {
            }
        }

        background: Skin.EmbeddedBackground {
        }
    }
}
