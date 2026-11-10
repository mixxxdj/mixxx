import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Controls
import "../LateNightTheme"

ComboBox {
    id: root

    property int popupMaxItem: Math.max(1, Math.floor((Screen.height - 2) / popupRowHeight))
    readonly property int popupRowHeight: Math.round(popupFontMetrics.height) + 2
    property int popupWidth: 162
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
    font.pixelSize: LateNightTheme.isClassic ? 13 : 14
    font.weight: LateNightTheme.isClassic ? Font.Bold : Font.Medium
    implicitHeight: 24
    model: Mixxx.EffectsManager.visibleEffectsModel
    padding: 0
    rightPadding: 0
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
        bottomPadding: LateNightTheme.isClassic ? 4 : 3
        color: LateNightTheme.mixerQuickEffectSelectorTextColor
        elide: Text.ElideRight
        font: root.font
        leftPadding: 7
        renderType: Text.NativeRendering
        rightPadding: 18
        text: root.displayText
        topPadding: LateNightTheme.isClassic ? 2 : 5
        verticalAlignment: Text.AlignVCenter
    }
    delegate: ItemDelegate {
        id: effectDelegate

        required property int index

        checkable: false
        checked: root.currentIndex === index
        height: root.popupRowHeight
        highlighted: root.highlightedIndex === index
        padding: 0
        width: ListView.view ? ListView.view.width : root.popupWidth

        background: Rectangle {
            color: effectDelegate.highlighted ? (effectDelegate.checked ? (LateNightTheme.isClassic ? "#2a1e03" : "#2f2f2f") : (LateNightTheme.isClassic ? "#5e4507" : "#2c454f")) : "transparent"
            radius: effectDelegate.highlighted ? 1 : 0
        }
        contentItem: Text {
            color: effectDelegate.checked || effectDelegate.highlighted ? "#ffffff" : LateNightTheme.mixerQuickEffectSelectorTextColor
            elide: Text.ElideMiddle
            font: root.font
            leftPadding: 20
            renderType: Text.NativeRendering
            rightPadding: 4
            text: root.textAt(effectDelegate.index)
            verticalAlignment: Text.AlignVCenter
        }

        Image {
            anchors.verticalCenter: parent.verticalCenter
            height: 10
            source: LateNightTheme.lateNightAsset("buttons", LateNightTheme.isClassic ? "btn__lib_checkmark_orange.svg" : "btn__effect_selected.svg")
            sourceSize: Qt.size(10, 10)
            visible: effectDelegate.checked
            width: 10
            x: LateNightTheme.isClassic ? 8 : 6
        }
    }
    indicator: Image {
        anchors.right: parent.right
        anchors.rightMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        height: 24
        source: LateNightTheme.isClassic && arrowHover.hovered ? LateNightTheme.lateNightAsset("buttons", "btn__fx_selector_down_pressed.svg") : LateNightTheme.assetFxSelectorDownButton
        sourceSize: Qt.size(16, 24)
        width: 16

        HoverHandler {
            id: arrowHover
        }
    }
    popup: Popup {
        height: Math.min(contentItem.contentHeight, root.popupMaxItem * root.popupRowHeight) + 2
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

    FontMetrics {
        id: popupFontMetrics

        font: root.font
    }
    Connections {
        function onEffectIdChanged() {
            root.syncCurrentEffect();
        }

        target: root.slot
    }
}
