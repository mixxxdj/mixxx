import Mixxx 1.0 as Mixxx
import QtQuick

Item {
    id: root

    required property string controlKey
    property var dragImage: null
    required property Item dragItem
    required property Mixxx.EffectSlotProxy effectSlot
    readonly property int parameterSlotNumber: {
        const slotNumber = parseInt(root.controlKey.replace(/\D/g, ""));
        return Number.isNaN(slotNumber) ? -1 : slotNumber - 1;
    }
    required property int parameterType
    property EffectParameterReorderModel previewModel: null
    property EffectParameterReorderModel reorderModel: null
    property Mixxx.EffectSlotProxy swapDestination: null

    function draggedParameter(drag) {
        if (!root.effectSlot.loaded || !drag.hasText) {
            return null;
        }
        const payload = drag.text.split(/\r?\n/);
        if (payload.length !== 3 || !/^Mixxx effect parameter [01]$/.test(payload[0]) || payload[1] !== root.effectSlot.uniqueEffectId || !/^\d+$/.test(payload[2])) {
            return null;
        }
        const slotNumber = Number(payload[2]);
        return Number.isSafeInteger(slotNumber) ? {
            type: Number(payload[0].slice(-1)),
            slot: slotNumber
        } : null;
    }

    Drag.active: parameterDragHandler.active && root.dragImage !== null
    Drag.dragType: Drag.Automatic
    Drag.imageSource: root.dragImage ? root.dragImage.url : ""
    Drag.mimeData: ({
            "text/plain": "Mixxx effect parameter " + root.parameterType + "\n" + root.effectSlot.uniqueEffectId + "\n" + root.parameterSlotNumber
        })
    Drag.proposedAction: Qt.MoveAction
    Drag.source: root
    Drag.supportedActions: Qt.MoveAction

    Drag.onDragFinished: dropAction => {
        const destination = root.swapDestination;
        root.swapDestination = null;
        if (root.previewModel) {
            root.previewModel.restore();
            root.previewModel = null;
        }
        if (destination) {
            Qt.callLater(() => destination.completeParameterSwap(dropAction === Qt.MoveAction));
        }
    }
    Drag.onDragStarted: {
        if (root.reorderModel && root.reorderModel.begin(root.parameterType, root.parameterSlotNumber)) {
            root.previewModel = root.reorderModel;
        }
    }

    Binding {
        property: "opacity"
        target: root.dragItem
        value: 0
        when: root.Drag.active
    }
    DragHandler {
        id: parameterDragHandler

        acceptedButtons: Qt.LeftButton
        enabled: root.effectSlot.loaded
        target: null

        onActiveChanged: {
            if (!active) {
                root.dragImage = null;
                return;
            }
            const hotSpot = root.dragItem.mapFromItem(root, centroid.pressPosition.x, centroid.pressPosition.y);
            root.dragItem.grabToImage(result => {
                if (!parameterDragHandler.active) {
                    return;
                }
                root.Drag.hotSpot = hotSpot;
                root.dragImage = result;
            });
        }
    }
    HoverHandler {
        cursorShape: parameterDragHandler.active ? Qt.ClosedHandCursor : Qt.OpenHandCursor
        enabled: root.effectSlot.loaded && root.parameterSlotNumber >= 0
    }
    DropArea {
        property bool externalDrag: false

        anchors.fill: parent
        parent: root.dragItem

        onDropped: drop => {
            const source = root.draggedParameter(drop);
            if (source === null) {
                return;
            }
            if (root.reorderModel) {
                root.reorderModel.queueMove(root.effectSlot, source.type, source.slot);
            } else {
                root.effectSlot.queueParameterSwap(root.parameterType, root.parameterSlotNumber, source.slot, source.type);
            }
            if (drop.source instanceof EffectParameterDragArea) {
                drop.source.swapDestination = root.effectSlot;
            } else {
                const destination = root.effectSlot;
                if (root.reorderModel) {
                    root.reorderModel.restore();
                }
                Qt.callLater(() => destination.completeParameterSwap(true));
            }
            drop.acceptProposedAction();
        }
        onEntered: drag => {
            drag.accepted = root.parameterSlotNumber >= 0 && root.draggedParameter(drag) !== null;
            if (drag.accepted && root.reorderModel) {
                const source = root.draggedParameter(drag);
                const sourceArea = drag.source as EffectParameterDragArea;
                externalDrag = !sourceArea;
                if (sourceArea) {
                    if (sourceArea.previewModel && sourceArea.previewModel !== root.reorderModel) {
                        sourceArea.previewModel.restore();
                    }
                    sourceArea.previewModel = root.reorderModel;
                }
                root.reorderModel.preview(source.type, source.slot, root.controlKey);
            }
        }
        onExited: {
            if (externalDrag && root.reorderModel) {
                root.reorderModel.restore();
            }
        }
    }
}
