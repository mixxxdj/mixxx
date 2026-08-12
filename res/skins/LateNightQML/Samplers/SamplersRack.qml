pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    readonly property Item loadedRack: rackLoader.item as Item
    readonly property int mode: Math.max(0, Math.min(2, Math.round(samplerRowsControl.value)))

    function synchronizeModeControls() {
        if (!show4SamplersControl.initialized || !show8SamplersControl.initialized || !show16SamplersControl.initialized)
            return;
        show4SamplersControl.value = root.mode === 0 ? 1 : 0;
        show8SamplersControl.value = root.mode === 1 ? 1 : 0;
        show16SamplersControl.value = root.mode === 2 ? 1 : 0;
    }

    implicitHeight: loadedRack?.implicitHeight ?? 0

    Loader {
        id: rackLoader

        height: root.loadedRack?.implicitHeight ?? 0
        sourceComponent: root.mode === 0 ? fourSamplers : root.mode === 1 ? eightSamplers : sixteenSamplers
        width: root.width
    }
    Mixxx.ControlProxy {
        id: samplerRowsControl

        group: "[LateNight]"
        key: "sampler_rows"

        onInitializedChanged: root.synchronizeModeControls()
        onValueChanged: root.synchronizeModeControls()
    }
    Mixxx.ControlProxy {
        id: show4SamplersControl

        group: "[LateNight]"
        key: "show_4samplers"

        onValueChanged: {
            if (value > 0 && root.mode !== 0)
                samplerRowsControl.value = 0;
        }
    }
    Mixxx.ControlProxy {
        id: show8SamplersControl

        group: "[LateNight]"
        key: "show_8samplers"

        onValueChanged: {
            if (value > 0 && root.mode !== 1)
                samplerRowsControl.value = 1;
        }
    }
    Mixxx.ControlProxy {
        id: show16SamplersControl

        group: "[LateNight]"
        key: "show_16samplers"

        onValueChanged: {
            if (value > 0 && root.mode !== 2)
                samplerRowsControl.value = 2;
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
        id: eightSamplers

        SamplerGroup {
            count: 8
            expandKey: "expand_samplers_1-8"
            firstSampler: 1
            show8Hotcues: show8HotcuesControl.value > 0
            showFxAssignments: showSamplerFxControl.value > 0
        }
    }
    Component {
        id: sixteenSamplers

        ColumnLayout {
            spacing: 4

            SamplerGroup {
                Layout.fillWidth: true
                count: 8
                expandKey: "expand_samplers_1-8"
                firstSampler: 1
                show8Hotcues: show8HotcuesControl.value > 0
                showFxAssignments: showSamplerFxControl.value > 0
            }
            SamplerGroup {
                Layout.fillWidth: true
                count: 8
                expandKey: "expand_samplers_9-16"
                firstSampler: 9
                show8Hotcues: show8HotcuesControl.value > 0
                showFxAssignments: showSamplerFxControl.value > 0
            }
        }
    }
}
