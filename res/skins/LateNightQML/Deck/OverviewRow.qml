import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"
import "../../../qml/Mixxx/Controls" as MixxxControls

Item {
    id: root

    required property string group
    readonly property bool useSecondaryDeckColors: root.group === "[Channel3]" || root.group === "[Channel4]"
    readonly property color overviewBackgroundColor: useSecondaryDeckColors ? LateNightTheme.secondaryOverviewBackgroundColor : LateNightTheme.primaryOverviewBackgroundColor
    readonly property color waveformSignalColor: useSecondaryDeckColors ? LateNightTheme.secondaryWaveformSignalColor : LateNightTheme.primaryWaveformSignalColor
    readonly property int waveformOverviewType: Math.round(waveformOverviewTypeProxy.value)
    readonly property bool useFilteredOverview: waveformOverviewType === 0

    Mixxx.ControlProxy {
        id: waveformOverviewTypeProxy
        group: "[Waveform]"
        key: "WaveformOverviewType"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: root.overviewBackgroundColor

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: LateNightTheme.overviewBorderTopColor
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: LateNightTheme.overviewBorderLeftColor
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: LateNightTheme.overviewBorderBottomColor
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: LateNightTheme.overviewBorderRightColor
            }

            MixxxControls.WaveformOverview {
                id: waveformOverview

                anchors.fill: parent
                anchors.margins: 1
                group: root.group
                colorLow: root.useFilteredOverview ? root.waveformSignalColor : "#0000ff"
                colorMid: root.useFilteredOverview ? root.waveformSignalColor : "#00ff00"
                colorHigh: root.useFilteredOverview ? root.waveformSignalColor : "#ff0000"
                renderer: root.useFilteredOverview ? Mixxx.WaveformOverview.Renderer.Filtered : Mixxx.WaveformOverview.Renderer.RGB
            }

            Item {
                id: overviewOverlay

                anchors.fill: parent
                anchors.margins: 1

                Mixxx.ControlProxy {
                    id: trackSamplesProxy
                    group: root.group
                    key: "track_samples"
                }

                Mixxx.ControlProxy {
                    id: cuePointProxy
                    group: root.group
                    key: "cue_point"
                }

                Mixxx.ControlProxy {
                    id: loopStartProxy
                    group: root.group
                    key: "loop_start_position"
                }

                Mixxx.ControlProxy {
                    id: loopEndProxy
                    group: root.group
                    key: "loop_end_position"
                }

                Mixxx.ControlProxy {
                    id: loopEnabledProxy
                    group: root.group
                    key: "loop_enabled"
                }

                Mixxx.ControlProxy {
                    id: introStartProxy
                    group: root.group
                    key: "intro_start_position"
                }

                Mixxx.ControlProxy {
                    id: introEndProxy
                    group: root.group
                    key: "intro_end_position"
                }

                Mixxx.ControlProxy {
                    id: outroStartProxy
                    group: root.group
                    key: "outro_start_position"
                }

                Mixxx.ControlProxy {
                    id: outroEndProxy
                    group: root.group
                    key: "outro_end_position"
                }

                Mixxx.ControlProxy {
                    id: showIntroOutroCuesProxy
                    group: "[Skin]"
                    key: "show_intro_outro_cues"
                }

                readonly property string cueColor: LateNightTheme.isPaleMoon ? "#ff7a01" : "#ff001c"
                readonly property string loopColor: LateNightTheme.isPaleMoon ? "#00b400" : "#00ff00"
                readonly property string introOutroColor: LateNightTheme.isPaleMoon ? "#2c5c9a" : "#0000ff"

                function mapX(pos) {
                    if (trackSamplesProxy.value <= 0 || pos < 0) {
                        return -9999;
                    }
                    return (overviewOverlay.width * pos) / trackSamplesProxy.value;
                }

                Repeater {
                    model: 8

                    delegate: Item {
                        id: hotcueMarker

                        required property int index
                        readonly property int hotcueNumber: index + 1
                        readonly property real markerX: overviewOverlay.mapX(positionProxy.value)
                        readonly property color markerColor: colorProxy.value >= 0
                                ? "#" + Math.round(colorProxy.value).toString(16).padStart(6, "0")
                                : overviewOverlay.cueColor

                        anchors.fill: parent
                        visible: statusProxy.value > 0 && positionProxy.value >= 0
                        z: 1

                        Rectangle {
                            x: Math.max(0, Math.min(parent.width - width, hotcueMarker.markerX))
                            y: 0
                            width: 1
                            height: parent.height
                            color: hotcueMarker.markerColor
                        }

                        Text {
                            x: Math.max(0, Math.min(parent.width - width, hotcueMarker.markerX + 2))
                            y: 2
                            width: 12
                            height: 10
                            text: hotcueMarker.hotcueNumber
                            color: "#FFFFFF"
                            font.family: "Open Sans"
                            font.pixelSize: 10
                            font.bold: true
                            verticalAlignment: Text.AlignVCenter
                        }

                        Mixxx.ControlProxy {
                            id: statusProxy

                            group: root.group
                            key: "hotcue_" + hotcueMarker.hotcueNumber + "_status"
                        }

                        Mixxx.ControlProxy {
                            id: positionProxy

                            group: root.group
                            key: "hotcue_" + hotcueMarker.hotcueNumber + "_position"
                        }

                        Mixxx.ControlProxy {
                            id: colorProxy

                            group: root.group
                            key: "hotcue_" + hotcueMarker.hotcueNumber + "_color"
                        }
                    }
                }

                // --- Loop Range and Marks ---
                readonly property real loopStartX: mapX(loopStartProxy.value)
                readonly property real loopEndX: mapX(loopEndProxy.value)
                readonly property bool loopValid: loopStartProxy.value >= 0 && loopEndProxy.value >= 0 && loopEnabledProxy.value > 0

                Rectangle {
                    x: Math.min(overviewOverlay.loopStartX, overviewOverlay.loopEndX)
                    y: 0
                    width: Math.max(1, Math.abs(overviewOverlay.loopEndX - overviewOverlay.loopStartX))
                    height: parent.height
                    color: overviewOverlay.loopColor
                    opacity: 0.7
                    visible: overviewOverlay.loopValid
                }

                Rectangle {
                    x: overviewOverlay.loopStartX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.loopColor
                    visible: loopStartProxy.value >= 0
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: overviewOverlay.loopStartX + 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    text: "↻"
                    color: "#FFFFFF"
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    font.bold: true
                    visible: loopStartProxy.value >= 0
                }

                Rectangle {
                    x: overviewOverlay.loopEndX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.loopColor
                    visible: loopEndProxy.value >= 0
                }

                // --- Cue Mark ---
                readonly property real cueX: mapX(cuePointProxy.value)

                Rectangle {
                    x: overviewOverlay.cueX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.cueColor
                    visible: cuePointProxy.value >= 0
                }

                Text {
                    anchors.right: parent.left
                    anchors.rightMargin: -overviewOverlay.cueX + 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    text: "C"
                    color: "#FFFFFF"
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    font.bold: true
                    visible: cuePointProxy.value >= 0
                }

                // --- Intro Range and Marks ---
                readonly property real introStartX: mapX(introStartProxy.value)
                readonly property real introEndX: mapX(introEndProxy.value)
                readonly property bool introValid: introStartProxy.value >= 0 && introEndProxy.value >= 0 && showIntroOutroCuesProxy.value > 0

                Rectangle {
                    x: Math.min(overviewOverlay.introStartX, overviewOverlay.introEndX)
                    y: 0
                    width: Math.max(1, Math.abs(overviewOverlay.introEndX - overviewOverlay.introStartX))
                    height: parent.height
                    color: overviewOverlay.introOutroColor
                    opacity: 0.6
                    visible: overviewOverlay.introValid
                }

                Rectangle {
                    x: overviewOverlay.introStartX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.introOutroColor
                    visible: introStartProxy.value >= 0 && showIntroOutroCuesProxy.value > 0
                }

                Rectangle {
                    x: overviewOverlay.introEndX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.introOutroColor
                    visible: introEndProxy.value >= 0 && showIntroOutroCuesProxy.value > 0
                }

                Text {
                    anchors.left: parent.left
                    anchors.leftMargin: overviewOverlay.introEndX + 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    text: "◢"
                    color: "#FFFFFF"
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    font.bold: true
                    visible: introEndProxy.value >= 0 && showIntroOutroCuesProxy.value > 0
                }

                // --- Outro Range and Marks ---
                readonly property real outroStartX: mapX(outroStartProxy.value)
                readonly property real outroEndX: mapX(outroEndProxy.value)
                readonly property bool outroValid: outroStartProxy.value >= 0 && outroEndProxy.value >= 0 && showIntroOutroCuesProxy.value > 0

                Rectangle {
                    x: Math.min(overviewOverlay.outroStartX, overviewOverlay.outroEndX)
                    y: 0
                    width: Math.max(1, Math.abs(overviewOverlay.outroEndX - overviewOverlay.outroStartX))
                    height: parent.height
                    color: overviewOverlay.introOutroColor
                    opacity: 0.6
                    visible: overviewOverlay.outroValid
                }

                Rectangle {
                    x: overviewOverlay.outroStartX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.introOutroColor
                    visible: outroStartProxy.value >= 0 && showIntroOutroCuesProxy.value > 0
                }

                Rectangle {
                    x: overviewOverlay.outroEndX
                    y: 0
                    width: 1
                    height: parent.height
                    color: overviewOverlay.introOutroColor
                    visible: outroEndProxy.value >= 0 && showIntroOutroCuesProxy.value > 0
                }

                Text {
                    anchors.right: parent.left
                    anchors.rightMargin: -overviewOverlay.outroStartX + 2
                    anchors.top: parent.top
                    anchors.topMargin: 2
                    text: "◣"
                    color: "#FFFFFF"
                    font.family: "Open Sans"
                    font.pixelSize: 10
                    font.bold: true
                    visible: outroStartProxy.value >= 0 && showIntroOutroCuesProxy.value > 0
                }
            }
        }

        // Settings panel.
        Rectangle {
            Layout.preferredWidth: 76
            Layout.fillHeight: true
            color: LateNightTheme.overviewSettingsBackgroundColor

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 1
                color: LateNightTheme.overviewBorderTopColor
            }

            Rectangle {
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: LateNightTheme.overviewBorderLeftColor
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 1
                color: LateNightTheme.overviewBorderBottomColor
            }

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: LateNightTheme.overviewBorderRightColor
            }

            DeckSettingsPlaceholder {
                anchors.fill: parent
                group: root.group
            }
        }
    }
}
