import QtQml.Models

DelegateModel {
    id: root

    property string draggedKey: ""
    property int originalIndex: -1
    required property var sourceModel

    function begin(type, slot) {
        const key = (type === 1 ? "button_parameter" : "parameter") + (slot + 1);
        const index = root.indexOf(key);
        if (index < 0) {
            return false;
        }
        root.draggedKey = key;
        root.originalIndex = index;
        return true;
    }
    function indexOf(key) {
        for (let index = 0; index < root.items.count; ++index) {
            if (root.parameter(index).controlKey === key) {
                return index;
            }
        }
        return -1;
    }
    function parameter(index) {
        const data = root.items.get(index).model;
        return data.modelData || data;
    }
    function preview(type, slot, targetKey) {
        if (root.originalIndex < 0 && !root.begin(type, slot)) {
            return false;
        }
        const from = root.indexOf(root.draggedKey);
        const to = root.indexOf(targetKey);
        if (from >= 0 && to >= 0 && from !== to) {
            root.items.move(from, to);
        }
        return to >= 0;
    }
    function queueMove(effectSlot, type, slot) {
        const index = root.indexOf(root.draggedKey);
        if (index < 0) {
            return false;
        }
        let targetIndex = index + 1;
        while (targetIndex < root.items.count && !root.shown(targetIndex)) {
            ++targetIndex;
        }
        const after = targetIndex === root.items.count;
        if (after) {
            targetIndex = index - 1;
            while (targetIndex >= 0 && !root.shown(targetIndex)) {
                --targetIndex;
            }
        }
        if (targetIndex >= 0) {
            const target = root.parameter(targetIndex);
            const targetSlot = Number(target.controlKey.replace(/\D/g, "")) - 1;
            effectSlot.queueParameterMove(type, slot, target.type, targetSlot, after);
        }
        return true;
    }
    function restore() {
        const index = root.indexOf(root.draggedKey);
        if (index >= 0 && root.originalIndex >= 0 && index !== root.originalIndex) {
            root.items.move(index, root.originalIndex);
        }
        root.originalIndex = -1;
        root.draggedKey = "";
    }
    function shown(index) {
        const data = root.parameter(index);
        const slot = Number(data.controlKey.replace(/\D/g, ""));
        return data.loaded !== false && slot > 0 && slot <= 8;
    }

    model: root.sourceModel

    onSourceModelChanged: {
        root.originalIndex = -1;
        root.draggedKey = "";
    }
}
