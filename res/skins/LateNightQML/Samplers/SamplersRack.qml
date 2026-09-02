pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    readonly property Item loadedRack: (mode === 0 ? fourSamplerLoader.item : samplerRowsLoader.item) as Item
    readonly property int mode: Math.max(0, Math.min(5, Math.round(samplerRowsControl.value)))
    readonly property bool modeControlsInitialized: show4SamplersControl.initialized && show8SamplersControl.initialized && show16SamplersControl.initialized && show32SamplersControl.initialized && show48SamplersControl.initialized && show64SamplersControl.initialized
    readonly property int selectedSamplerCount: [4, 8, 16, 32, 48, 64][mode]
    property bool synchronizingMode: false

    function normalizeMode() {
        if (!root.modeControlsInitialized)
            return;
        root.selectMode(root.mode);
    }
    function selectMode(mode) {
        if (!root.modeControlsInitialized || mode < 0 || mode > 5)
            return;
        root.synchronizingMode = true;
        show4SamplersControl.value = mode === 0 ? 1 : 0;
        show8SamplersControl.value = mode === 1 ? 1 : 0;
        show16SamplersControl.value = mode === 2 ? 1 : 0;
        show32SamplersControl.value = mode === 3 ? 1 : 0;
        show48SamplersControl.value = mode === 4 ? 1 : 0;
        show64SamplersControl.value = mode === 5 ? 1 : 0;
        samplerRowsControl.value = mode;
        root.synchronizingMode = false;
        const samplerCount = [4, 8, 16, 32, 48, 64][mode];
        if (numSamplersControl.initialized && numSamplersControl.value < samplerCount)
            numSamplersControl.value = samplerCount;
    }

    implicitHeight: loadedRack?.implicitHeight ?? 0

    Loader {
        id: fourSamplerLoader

        active: numSamplersControl.initialized && numSamplersControl.value >= 4
        height: visible ? (item?.implicitHeight ?? 0) : 0
        sourceComponent: fourSamplers
        visible: root.mode === 0
        width: root.width
    }
    Loader {
        id: samplerRowsLoader

        active: numSamplersControl.initialized && numSamplersControl.value >= 8
        height: visible ? (item?.implicitHeight ?? 0) : 0
        sourceComponent: samplerRows
        visible: root.mode !== 0
        width: root.width
    }
    Mixxx.ControlProxy {
        id: numSamplersControl

        group: "[App]"
        key: "num_samplers"
    }
    Mixxx.ControlProxy {
        id: samplerRowsControl

        group: "[Skin]"
        key: "sampler_rows"
    }
    Mixxx.ControlProxy {
        id: show4SamplersControl

        group: "[Skin]"
        key: "show_4samplers"

        onInitializedChanged: root.normalizeMode()
        onValueChanged: {
            if (!root.synchronizingMode && value > 0.5)
                root.selectMode(0);
        }
    }
    Mixxx.ControlProxy {
        id: show8SamplersControl

        group: "[Skin]"
        key: "show_8samplers"

        onInitializedChanged: root.normalizeMode()
        onValueChanged: {
            if (!root.synchronizingMode && value > 0.5)
                root.selectMode(1);
        }
    }
    Mixxx.ControlProxy {
        id: show16SamplersControl

        group: "[Skin]"
        key: "show_16samplers"

        onInitializedChanged: root.normalizeMode()
        onValueChanged: {
            if (!root.synchronizingMode && value > 0.5)
                root.selectMode(2);
        }
    }
    Mixxx.ControlProxy {
        id: show32SamplersControl

        group: "[Skin]"
        key: "show_32samplers"

        onInitializedChanged: root.normalizeMode()
        onValueChanged: {
            if (!root.synchronizingMode && value > 0.5)
                root.selectMode(3);
        }
    }
    Mixxx.ControlProxy {
        id: show48SamplersControl

        group: "[Skin]"
        key: "show_48samplers"

        onInitializedChanged: root.normalizeMode()
        onValueChanged: {
            if (!root.synchronizingMode && value > 0.5)
                root.selectMode(4);
        }
    }
    Mixxx.ControlProxy {
        id: show64SamplersControl

        group: "[Skin]"
        key: "show_64samplers"

        onInitializedChanged: root.normalizeMode()
        onValueChanged: {
            if (!root.synchronizingMode && value > 0.5)
                root.selectMode(5);
        }
    }
    Mixxx.ControlProxy {
        id: show8HotcuesControl

        group: "[Skin]"
        key: "show_8_hotcues"
    }
    Mixxx.ControlProxy {
        id: showSamplerFxControl

        group: "[Skin]"
        key: "show_sampler_fx"
    }
    Component {
        id: fourSamplers

        SamplerGroup {
            count: 4
            expandKey: "expand_samplers_1-4"
            firstSampler: 1
            show8Hotcues: show8HotcuesControl.value > 0
            showFxAssignments: showSamplerFxControl.value > 0
        }
    }
    Component {
        id: samplerRows

        ColumnLayout {
            id: rows

            property int preloadIndex: 0

            function advancePreload() {
                while (rows.preloadIndex < samplerGroups.count && samplerGroups.itemAt(rows.preloadIndex)?.expandedContentReady)
                    ++rows.preloadIndex;
            }

            spacing: 4

            Repeater {
                id: samplerGroups

                model: numSamplersControl.initialized ? Math.min(8, Math.floor(numSamplersControl.value / 8)) : 0

                onItemAdded: rows.advancePreload()

                SamplerGroup {
                    required property int index

                    Layout.fillWidth: true
                    count: 8
                    expandKey: "expand_samplers_" + firstSampler + "-" + (firstSampler + 7)
                    firstSampler: 1 + index * 8
                    preloadExpandedContent: index <= rows.preloadIndex
                    show8Hotcues: show8HotcuesControl.value > 0
                    showFxAssignments: showSamplerFxControl.value > 0
                    visible: index < root.selectedSamplerCount / 8

                    onExpandedContentReadyChanged: rows.advancePreload()
                }
            }
        }
    }
}
