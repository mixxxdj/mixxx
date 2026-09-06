import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import "../LateNightTheme"

ComboBox {
    id: root

    property bool arrowOnRight: false
    readonly property bool compact: width <= 40
    required property string group
    property int popupMaxItem: 44
    property int popupWidth: 160
    readonly property string quickEffectGroup: "[QuickEffectRack1_" + group + "]"

    currentIndex: root.count > 0 ? Math.max(0, Math.min(root.count - 1, Math.round(presetControl.value))) : -1
    font.family: "Open Sans"
    font.pixelSize: 13
    font.weight: Font.Medium
    implicitHeight: 18
    implicitWidth: 62
    model: Mixxx.EffectsManager.quickChainPresetModel
    textRole: "display"

    background: Item {
    }
    contentItem: Item {
        implicitHeight: root.implicitHeight
        implicitWidth: root.implicitWidth

        Image {
            id: arrow

            anchors.verticalCenter: parent.verticalCenter
            fillMode: Image.PreserveAspectFit
            height: 24
            source: LateNightTheme.assetToolbarDropdownIcon
            width: 16
            x: root.arrowOnRight ? parent.width - width + 1 : -3
        }
        Text {
            anchors.fill: parent
            anchors.leftMargin: root.compact && root.arrowOnRight ? -6 : 0
            anchors.rightMargin: root.compact && !root.arrowOnRight ? -6 : 0
            color: LateNightTheme.mixerQuickEffectSelectorTextColor
            elide: Text.ElideRight
            font: root.font
            horizontalAlignment: root.arrowOnRight ? Text.AlignRight : Text.AlignLeft
            leftPadding: root.arrowOnRight ? 0 : (root.compact ? 10 : 13)
            rightPadding: root.arrowOnRight ? (root.compact ? 10 : 13) : 0
            text: root.displayText || qsTr("Filter")
            verticalAlignment: Text.AlignVCenter
        }
    }
    delegate: ItemDelegate {
        id: presetDelegate

        required property int index

        checkable: false
        checked: root.currentIndex === index
        height: 20
        highlighted: root.highlightedIndex === index
        padding: 0
        width: ListView.view ? ListView.view.width : root.popupWidth

        background: Rectangle {
            color: presetDelegate.highlighted ? "#2c454f" : "transparent"
            radius: presetDelegate.highlighted ? 1 : 0
        }
        contentItem: Text {
            color: presetDelegate.checked || presetDelegate.highlighted ? "#ffffff" : LateNightTheme.mixerQuickEffectSelectorTextColor
            elide: Text.ElideRight
            font: root.font
            leftPadding: 20
            rightPadding: 4
            text: root.textAt(presetDelegate.index)
            verticalAlignment: Text.AlignVCenter
        }
    }
    indicator: Item {
        width: 0
    }
    popup: Popup {
        height: Math.min(contentItem.contentHeight, root.popupMaxItem * 20) + 2
        padding: 1
        width: root.popupWidth
        x: root.arrowOnRight ? root.width - width : 0
        y: root.height

        background: Rectangle {
            border.color: "#333333"
            border.width: 1
            color: "#151517"
            radius: 1
        }
        contentItem: ListView {
            id: presetList

            clip: true
            currentIndex: root.highlightedIndex
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOff
            }
        }

        onOpened: presetList.contentY = 0
    }

    onActivated: index => presetControl.value = index

    Mixxx.ControlProxy {
        id: presetControl

        group: root.quickEffectGroup
        key: "loaded_chain_preset"
    }
}
