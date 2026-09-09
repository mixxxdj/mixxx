import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "." as Setting
import "../Theme"

Category {
    id: root

    property bool dirty: false
    label: "Waveforms"
    readonly property string graphicsApiName: {
        switch (GraphicsInfo.api) {
        case GraphicsInfo.OpenGL:
            return "OpenGL";
        case GraphicsInfo.Metal:
            return "Metal";
        case GraphicsInfo.Direct3D11:
            return "Direct3D 11";
        case GraphicsInfo.Direct3D12:
            return "Direct3D 12";
        case GraphicsInfo.Vulkan:
            return "Vulkan";
        case GraphicsInfo.Software:
            return "Software";
        case GraphicsInfo.Null:
            return "Null";
        default:
            return "Unknown";
        }
    }
    readonly property list<int> waveformTypes: [Mixxx.WaveformDisplay.Type.Simple, Mixxx.WaveformDisplay.Type.Filtered, Mixxx.WaveformDisplay.Type.HSV, Mixxx.WaveformDisplay.Type.RGB, Mixxx.WaveformDisplay.Type.Stacked]

    function load() {
        overviewTypeInput.currentIndex = Math.max(0, Math.min(2, Mixxx.Config.waveformOverviewType));
        overviewStereoInput.selected = Mixxx.Config.waveformOverviewStereo ? "on" : "off";
        overviewMinuteMarkersInput.selected = Mixxx.Config.waveformOverviewMinuteMarkers ? "on" : "off";
        overviewScalingInput.selected = Mixxx.Config.waveformOverviewNormalized ? "normalize" : "global gain";
        waveformEnabledInput.selected = Mixxx.Config.waveformEnabled ? "on" : "off";
        frameRateInput.value = Mixxx.Config.waveformFrameRate;
        waveformTypeInput.currentIndex = root.waveformTypes.indexOf(Mixxx.Config.waveformType);
        if (waveformTypeInput.currentIndex < 0)
            waveformTypeInput.currentIndex = 3;
        zoomSynchronizationInput.selected = Mixxx.Config.waveformZoomSynchronization ? "on" : "off";
        var options = Mixxx.Config.waveformOptions;
        splitStereoSignalInput.selected = (options & Mixxx.WaveformDisplay.Option.SplitStereoSignal) ? "on" : "off";
        highDetailInput.selected = (options & Mixxx.WaveformDisplay.Option.HighDetail) ? "on" : "off";
        defaultZoomInput.value = Mixxx.Config.waveformDefaultZoom;
        endOfTrackWarningInput.value = Mixxx.Config.waveformEndOfTrackWarningTime;
        beatGridAlphaInput.value = Mixxx.Config.waveformBeatGridAlpha;
        playMarkerPositionInput.value = Mixxx.Config.waveformPlayMarkerPosition * 100;
        visualGainAllInput.value = Mixxx.Config.waveformVisualGainAll;
        visualGainLowInput.value = Mixxx.Config.waveformVisualGainLow;
        visualGainMidInput.value = Mixxx.Config.waveformVisualGainMedium;
        visualGainHighInput.value = Mixxx.Config.waveformVisualGainHigh;
        untilMarkShowBeatsInput.selected = Mixxx.Config.waveformUntilMarkShowBeats ? "on" : "off";
        untilMarkShowTimeInput.selected = Mixxx.Config.waveformUntilMarkShowTime ? "on" : "off";
        untilMarkAlignInput.currentIndex = Math.max(0, Math.min(2, Mixxx.Config.waveformUntilMarkAlign));
        untilMarkTextSizeInput.value = Mixxx.Config.waveformUntilMarkTextPointSize;
        stemOpacityInput.value = Mixxx.Config.waveformStemOpacity;
        stemOutlineOpacityInput.value = Mixxx.Config.waveformStemOutlineOpacity;
        stemReorderInput.selected = Mixxx.Config.waveformStemReorderOnChange ? "on" : "off";
        stemDisplayModeInput.selected = Mixxx.Config.waveformStemSplitTracks ? "stacked" : "overlap";
        cachingInput.selected = Mixxx.Config.waveformCachingEnabled ? "on" : "off";
        generateWithAnalysisInput.selected = Mixxx.Config.waveformGenerationWithAnalysisEnabled ? "on" : "off";
        root.dirty = false;
    }
    function markDirty() {
        if (!root.dirty)
            root.dirty = true;
    }
    function reset() {
        overviewTypeInput.currentIndex = 2;
        overviewStereoInput.selected = "on";
        overviewMinuteMarkersInput.selected = "on";
        overviewScalingInput.selected = "global gain";
        waveformTypeInput.currentIndex = 3;
        zoomSynchronizationInput.selected = "on";
        waveformEnabledInput.selected = "on";
        frameRateInput.value = 60;
        splitStereoSignalInput.selected = "off";
        highDetailInput.selected = "off";
        defaultZoomInput.value = 3;
        endOfTrackWarningInput.value = 30;
        beatGridAlphaInput.value = 90;
        playMarkerPositionInput.value = 50;
        visualGainAllInput.value = 2;
        visualGainLowInput.value = 1;
        visualGainMidInput.value = 1;
        visualGainHighInput.value = 1;
        untilMarkShowBeatsInput.selected = "off";
        untilMarkShowTimeInput.selected = "off";
        untilMarkAlignInput.currentIndex = 1;
        untilMarkTextSizeInput.value = 24;
        stemOpacityInput.value = 0.75;
        stemOutlineOpacityInput.value = 0.15;
        stemReorderInput.selected = "on";
        stemDisplayModeInput.selected = "overlap";
        cachingInput.selected = "on";
        generateWithAnalysisInput.selected = "off";
        root.dirty = true;
    }
    function save() {
        Mixxx.Config.waveformOverviewType = overviewTypeInput.currentIndex;
        Mixxx.Config.waveformOverviewStereo = overviewStereoInput.selected === "on";
        Mixxx.Config.waveformOverviewMinuteMarkers = overviewMinuteMarkersInput.selected === "on";
        Mixxx.Config.waveformOverviewNormalized = overviewScalingInput.selected === "normalize";
        Mixxx.Config.waveformType = root.waveformTypes[waveformTypeInput.currentIndex];
        Mixxx.Config.waveformEnabled = waveformEnabledInput.selected === "on";
        Mixxx.Config.waveformZoomSynchronization = zoomSynchronizationInput.selected === "on";
        Mixxx.Config.waveformFrameRate = frameRateInput.value;
        var options = 0;
        if (waveformTypeInput.currentIndex === 3 && splitStereoSignalInput.selected === "on")
            options |= Mixxx.WaveformDisplay.Option.SplitStereoSignal;
        if (highDetailInput.selected === "on")
            options |= Mixxx.WaveformDisplay.Option.HighDetail;
        Mixxx.Config.waveformOptions = options;
        Mixxx.Config.waveformDefaultZoom = defaultZoomInput.value;
        Mixxx.Config.waveformEndOfTrackWarningTime = endOfTrackWarningInput.value;
        Mixxx.Config.waveformBeatGridAlpha = beatGridAlphaInput.value;
        Mixxx.Config.waveformPlayMarkerPosition = playMarkerPositionInput.value / 100;
        Mixxx.Config.waveformVisualGainAll = visualGainAllInput.value;
        Mixxx.Config.waveformVisualGainLow = visualGainLowInput.value;
        Mixxx.Config.waveformVisualGainMedium = visualGainMidInput.value;
        Mixxx.Config.waveformVisualGainHigh = visualGainHighInput.value;
        Mixxx.Config.waveformUntilMarkShowBeats = untilMarkShowBeatsInput.selected === "on";
        Mixxx.Config.waveformUntilMarkShowTime = untilMarkShowTimeInput.selected === "on";
        Mixxx.Config.waveformUntilMarkAlign = untilMarkAlignInput.currentIndex;
        Mixxx.Config.waveformUntilMarkTextPointSize = untilMarkTextSizeInput.value;
        Mixxx.Config.waveformStemOpacity = stemOpacityInput.value;
        Mixxx.Config.waveformStemOutlineOpacity = stemOutlineOpacityInput.value;
        Mixxx.Config.waveformStemReorderOnChange = stemReorderInput.selected === "on";
        Mixxx.Config.waveformStemSplitTracks = stemDisplayModeInput.selected === "stacked";
        Mixxx.Config.waveformCachingEnabled = cachingInput.selected === "on";
        Mixxx.Config.waveformGenerationWithAnalysisEnabled = generateWithAnalysisInput.selected === "on";
        root.load();
    }

    Component.onCompleted: {
        Mixxx.Library.refreshWaveformCacheDiskUsage();
        root.load();
    }

    ScrollView {
        anchors.bottom: buttonActions.top
        anchors.bottomMargin: 18
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        clip: true

        ColumnLayout {
            id: content

            anchors.left: parent.left
            anchors.margins: 14
            anchors.right: parent.right
            anchors.top: parent.top
            spacing: 10

            Text {
                Layout.fillWidth: true
                color: Theme.white
                font.pixelSize: 14
                font.weight: Font.DemiBold
                text: "Overview Waveforms"
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Type"
                }
                Skin.ComboBox {
                    id: overviewTypeInput

                    Layout.preferredWidth: 180
                    model: ["Filtered", "HSV", "RGB"]

                    onCurrentIndexChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Stereo channels"
                }
                RatioChoice {
                    id: overviewStereoInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Minute markers"
                }
                RatioChoice {
                    id: overviewMinuteMarkersInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Gain scaling"
                }
                RatioChoice {
                    id: overviewScalingInput

                    options: ["normalize", "global gain"]

                    onSelectedChanged: root.markDirty()
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.darkGray3
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                font.pixelSize: 14
                font.weight: Font.DemiBold
                text: "Scrolling Waveforms"
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Type"
                }
                Skin.ComboBox {
                    id: waveformTypeInput

                    Layout.preferredWidth: 180
                    model: ["Simple", "Filtered", "HSV", "RGB", "Stacked"]

                    onCurrentIndexChanged: root.markDirty()
                }
            }
            Text {
                Layout.fillWidth: true
                color: Theme.midGray
                text: "Simple, Filtered, HSV, RGB and Stacked are supported by the QML renderer."
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Enabled"
                }
                RatioChoice {
                    id: waveformEnabledInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            WaveformSliderRow {
                id: frameRateInput

                decimals: 0
                label: "Frame rate"
                max: 240
                min: 10
                suffix: " fps"

                onValueChanged: root.markDirty()
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Sync deck zoom"
                }
                RatioChoice {
                    id: zoomSynchronizationInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            WaveformSliderRow {
                id: defaultZoomInput

                decimals: 0
                label: "Default zoom"
                max: 10
                min: 1
                suffix: "x"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: endOfTrackWarningInput

                decimals: 0
                label: "End-of-track warning"
                max: 120
                min: 0
                suffix: "s"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: beatGridAlphaInput

                decimals: 0
                label: "Beat-grid opacity"
                max: 100
                min: 0
                suffix: "%"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: playMarkerPositionInput

                decimals: 0
                label: "Play-marker position"
                max: 100
                min: 0
                suffix: "%"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: visualGainAllInput

                decimals: 2
                label: "Visual gain (all)"
                max: 3
                min: 0
                suffix: "x"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: visualGainLowInput

                decimals: 2
                label: "Visual gain (low)"
                max: 3
                min: 0
                suffix: "x"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: visualGainMidInput

                decimals: 2
                label: "Visual gain (mid)"
                max: 3
                min: 0
                suffix: "x"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: visualGainHighInput

                decimals: 2
                label: "Visual gain (high)"
                max: 3
                min: 0
                suffix: "x"

                onValueChanged: root.markDirty()
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Split stereo signal"
                }
                RatioChoice {
                    id: splitStereoSignalInput

                    enabled: waveformTypeInput.currentIndex === 3
                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            Text {
                Layout.fillWidth: true
                color: Theme.midGray
                text: "Split stereo signal is used by the RGB renderer."
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "High detail (legacy)"
                }
                RatioChoice {
                    id: highDetailInput

                    enabled: false
                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            Text {
                Layout.fillWidth: true
                color: Theme.midGray
                text: "High detail is retained for compatibility but is unavailable in the QML scene-graph renderer."
                wrapMode: Text.WordWrap
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Show beats until marker"
                }
                RatioChoice {
                    id: untilMarkShowBeatsInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Show time until marker"
                }
                RatioChoice {
                    id: untilMarkShowTimeInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Marker alignment"
                }
                Skin.ComboBox {
                    id: untilMarkAlignInput

                    Layout.preferredWidth: 180
                    model: ["Top", "Center", "Bottom"]

                    onCurrentIndexChanged: root.markDirty()
                }
            }
            WaveformSliderRow {
                id: untilMarkTextSizeInput

                decimals: 0
                label: "Marker text size"
                max: 48
                min: 6
                suffix: "px"

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: stemOpacityInput

                decimals: 2
                label: "Stem channel opacity"
                max: 1
                min: 0

                onValueChanged: root.markDirty()
            }
            WaveformSliderRow {
                id: stemOutlineOpacityInput

                decimals: 2
                label: "Stem outline opacity"
                max: 1
                min: 0

                onValueChanged: root.markDirty()
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Reorder stems when volume changes"
                }
                RatioChoice {
                    id: stemReorderInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Stem display mode"
                }
                RatioChoice {
                    id: stemDisplayModeInput

                    options: ["overlap", "stacked"]

                    onSelectedChanged: root.markDirty()
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.darkGray3
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                font.pixelSize: 14
                font.weight: Font.DemiBold
                text: "Caching"
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Enable waveform caching"
                }
                RatioChoice {
                    id: cachingInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                WaveformSettingParameter {
                    label: "Generate while analyzing"
                }
                RatioChoice {
                    id: generateWithAnalysisInput

                    options: ["on", "off"]

                    onSelectedChanged: root.markDirty()
                }
            }
            RowLayout {
                Layout.fillWidth: true

                Text {
                    Layout.fillWidth: true
                    color: Theme.white
                    text: "Cached waveforms occupy " + (Mixxx.Library.waveformCacheDiskUsage || "unknown") + " on disk."
                }
                Skin.Button {
                    text: "Clear cached waveforms"

                    onPressed: Mixxx.Library.clearCachedWaveforms()
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 1
                color: Theme.darkGray3
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                font.pixelSize: 14
                font.weight: Font.DemiBold
                text: "Graphics API Status"
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                text: "Qt Quick renderer: " + root.graphicsApiName
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                text: Mixxx.Config.waveformAverageFrameRate > 0
                        ? "Average frame rate: " + Number(Mixxx.Config.waveformAverageFrameRate).toFixed(2) + " fps"
                        : "Average frame rate: Measuring…"
            }
            Text {
                Layout.fillWidth: true
                color: Theme.midGray
                text: GraphicsInfo.api === GraphicsInfo.OpenGL ? "OpenGL " + GraphicsInfo.majorVersion + "." + GraphicsInfo.minorVersion : "Runtime API reported by Qt Quick"
            }
        }
    }
    Item {
        id: buttonActions

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        height: 20

        Setting.FormButton {
            activeColor: "#999999"
            anchors.left: parent.left
            backgroundColor: "#7D3B3B"
            opacity: enabled ? 1.0 : 0.5
            text: "Reset"

            onPressed: {
                root.reset();
            }
        }
        Row {
            anchors.right: parent.right
            spacing: 10

            Text {
                id: errorMessage

                Layout.alignment: Qt.AlignVCenter
                Layout.rightMargin: 16
                color: "#7D3B3B"
                text: ""
            }
            Setting.FormButton {
                activeColor: "#999999"
                backgroundColor: "#3F3F3F"
                opacity: enabled ? 1.0 : 0.5
                text: "Cancel"
                visible: root.dirty

                onPressed: {
                    root.load();
                }
            }
            Setting.FormButton {
                activeColor: "#999999"
                backgroundColor: root.dirty ? "#3a60be" : "#3F3F3F"
                enabled: root.dirty
                opacity: enabled ? 1.0 : 0.5
                text: "Save"

                onPressed: {
                    errorMessage.text = "";
                    root.save();
                }
            }
        }
    }

    component WaveformSettingParameter: Mixxx.SettingParameter {
        Layout.fillWidth: true
        implicitHeight: 30

        Text {
            anchors.fill: parent
            color: Theme.white
            font.pixelSize: 14
            text: parent.label
            verticalAlignment: Text.AlignVCenter
        }
    }
    component WaveformSliderRow: RowLayout {
        property alias decimals: slider.decimals
        required property string label
        property alias max: slider.max
        property alias min: slider.min
        property alias suffix: slider.suffix
        property alias value: slider.value

        Layout.fillWidth: true

        WaveformSettingParameter {
            label: parent.label
        }
        Setting.Slider {
            id: slider

            Layout.preferredWidth: 260
        }
    }
}
