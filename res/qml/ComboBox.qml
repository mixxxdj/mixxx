import "." as Skin
import QtQuick 2.12
import QtQuick.Controls 2.12
import "Theme"

ComboBox {
    id: root

    background: Skin.EmbeddedBackground {
    }

    delegate: ItemDelegate {
        width: parent.width
        highlighted: root.highlightedIndex === index
        text: root.textAt(index)

        contentItem: Text {
            text: parent.text
            color: Theme.deckTextColor
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }

        background: Rectangle {
            radius: 5
            border.width: highlighted ? 1 : 0
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
        elide: Text.ElideRight
    }

    popup: Popup {
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
