import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import "../LateNightTheme"

ComboBox {
    id: root

    property int popupMaxItem: 44
    property int popupWidth: 160
    required property Mixxx.EffectSlotProxy slot

    function syncCurrentEffect() {
        for (let index = 0; index < count; ++index) {
            if (model.get(index).effectId === slot.effectId) {
                currentIndex = index;
                return;
            }
        }
        currentIndex = 0;
    }

    font.family: "Open Sans"
    font.pixelSize: LateNightTheme.isClassic ? 12 : 14
    font.weight: LateNightTheme.isClassic ? Font.Bold : Font.Medium
    implicitHeight: 24
    model: Mixxx.EffectsManager.visibleEffectsModel
    textRole: "display"

    background: BorderImage {
        border.bottom: 2
        border.left: 2
        border.right: 2
        border.top: 2
        horizontalTileMode: BorderImage.Stretch
        source: root.popup.visible ? LateNightTheme.assetFxSelectorActiveBorder : LateNightTheme.assetFxSelectorBorder
        verticalTileMode: BorderImage.Stretch
    }
    contentItem: Text {
        color: LateNightTheme.mixerQuickEffectSelectorTextColor
        elide: Text.ElideRight
        font.pixelSize: 11
        leftPadding: 5
        rightPadding: 18
        text: root.displayText
        verticalAlignment: Text.AlignVCenter
    }
    delegate: ItemDelegate {
        id: effectDelegate

        required property int index

        checkable: false
        checked: root.currentIndex === index
        height: 20
        highlighted: root.highlightedIndex === index
        padding: 0
        width: ListView.view ? ListView.view.width : root.popupWidth

        background: Rectangle {
            color: effectDelegate.highlighted ? (LateNightTheme.isClassic ? "#5e4507" : "#2c454f") : "transparent"
            radius: effectDelegate.highlighted ? 1 : 0
        }
        contentItem: Text {
            color: effectDelegate.checked || effectDelegate.highlighted ? "#ffffff" : LateNightTheme.mixerQuickEffectSelectorTextColor
            elide: Text.ElideRight
            font: root.font
            leftPadding: 20
            rightPadding: 4
            text: root.textAt(effectDelegate.index)
            verticalAlignment: Text.AlignVCenter
        }
    }
    indicator: Image {
        anchors.right: parent.right
        anchors.rightMargin: 3
        anchors.verticalCenter: parent.verticalCenter
        height: 8
        source: LateNightTheme.assetFxSelectorDownButton
        width: 12
    }
    popup: Popup {
        height: Math.min(contentItem.contentHeight, root.popupMaxItem * 20) + 2
        padding: 1
        width: root.popupWidth
        x: 0
        y: root.height

        background: Rectangle {
            border.color: LateNightTheme.isClassic ? "#888888" : "#333333"
            border.width: 1
            color: LateNightTheme.isClassic ? "#0f0f0f" : "#151517"
            radius: LateNightTheme.isClassic ? 2 : 1
        }
        contentItem: ListView {
            id: effectList

            clip: true
            currentIndex: root.highlightedIndex
            implicitHeight: contentHeight
            model: root.popup.visible ? root.delegateModel : null

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOff
            }
        }

        onOpened: effectList.contentY = 0
    }

    Component.onCompleted: syncCurrentEffect()
    onActivated: index => {
        const effectId = model.get(index).effectId || "";
        if (slot.effectId !== effectId) {
            slot.effectId = effectId;
        }
    }
    onCountChanged: syncCurrentEffect()

    Connections {
        function onEffectIdChanged() {
            root.syncCurrentEffect();
        }

        target: root.slot
    }
}
