import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import ".." as Skin
import "." as Setting
import "../Theme"

Category {
    id: root

    property bool dirty: false
    property bool stemsEnabled: true
    property bool writeNextToTrack: true
    property bool pauseWhilePlaying: true
    property int codecIndex: 0
    property int modelIndex: 0
    property int providerIndex: 0
    property string pythonPath: ""
    property string cacheDirectory: ""
    readonly property var codecs: ["aac", "mp3", "alac"]
    readonly property var models: ["htdemucs", "htdemucs_ft"]
    readonly property var providers: ["auto", "cuda", "cpu"]

    label: "Stems"

    function markDirty() {
        root.dirty = true;
    }

    function load() {
        stemsEnabled = Mixxx.Core.stemGenerator.enabled;
        writeNextToTrack = Mixxx.Core.stemGenerator.writeNextToTrack;
        pauseWhilePlaying = Mixxx.Core.stemGenerator.pauseDuringPlayback;
        pythonPath = Mixxx.Core.stemGenerator.pythonPath;
        cacheDirectory = Mixxx.Core.stemGenerator.cacheDirectory;
        codecIndex = Math.max(0, codecs.indexOf(Mixxx.Core.stemGenerator.codec));
        modelIndex = Math.max(0, models.indexOf(Mixxx.Core.stemGenerator.model));
        providerIndex = Math.max(0, providers.indexOf(Mixxx.Core.stemGenerator.provider));
        Mixxx.Core.stemGenerator.probe();
    }

    function save() {
        Mixxx.Core.stemGenerator.enabled = stemsEnabled;
        Mixxx.Core.stemGenerator.writeNextToTrack = writeNextToTrack;
        Mixxx.Core.stemGenerator.pauseDuringPlayback = pauseWhilePlaying;
        Mixxx.Core.stemGenerator.pythonPath = pythonPath;
        Mixxx.Core.stemGenerator.cacheDirectory = cacheDirectory;
        Mixxx.Core.stemGenerator.codec = codecs[codecIndex];
        Mixxx.Core.stemGenerator.model = models[modelIndex];
        Mixxx.Core.stemGenerator.provider = providers[providerIndex];
        root.dirty = false;
    }

    ColumnLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        spacing: 10

        Text {
            Layout.fillWidth: true
            color: Theme.white
            font.pixelSize: 14
            font.weight: Font.DemiBold
            text: "Stem generation"
        }
        Text {
            Layout.fillWidth: true
            color: Theme.midGray
            text: "Creates a Native Instruments style .stem.mp4 file with four stems (drums, bass, other, vocals) so the track can be played and remixed stem by stem. Add tracks from the library context menu: right click a selection and choose Stems > Generate stems. The jobs run one at a time and, unless disabled below, are postponed while a deck is playing."
            wrapMode: Text.WordWrap
        }
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.darkGray3
        }

        // --- graphical hardware warning -------------------------------------
        Text {
            Layout.fillWidth: true
            color: Theme.white
            font.pixelSize: 14
            font.weight: Font.DemiBold
            text: "Hardware"
        }
        Text {
            Layout.fillWidth: true
            color: Mixxx.Core.stemGenerator.gpuAvailable ? "#7bd88f" : "#ffb347"
            text: {
                if (Mixxx.Core.stemGenerator.probing)
                    return "Checking the environment...";
                if (Mixxx.Core.stemGenerator.gpuAvailable)
                    return "GPU detected" + (Mixxx.Core.stemGenerator.gpuName !== "" ? ": " + Mixxx.Core.stemGenerator.gpuName : "") + ". Separation runs on CUDA at roughly 10x real time, about 20 seconds for a four minute track.";
                return "No CUDA capable GPU found, so the separation would run on the CPU: it uses a lot of CPU time and takes much longer, roughly 2.4x real time, that is about 100 seconds for a four minute track (some five times slower than with a GPU). Running it while mixing may cause dropouts, so it is postponed while a deck is playing.";
            }
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            color: "#ffb347"
            text: Mixxx.Core.stemGenerator.probeErrors.length > 0 ? "Setup required:\n" + Mixxx.Core.stemGenerator.probeErrors.join("\n") : ""
            visible: Mixxx.Core.stemGenerator.probeErrors.length > 0
            wrapMode: Text.WordWrap
        }
        Text {
            Layout.fillWidth: true
            color: Theme.midGray
            text: "The worker needs a Python interpreter with demucs-onnx and onnxruntime-gpu installed, plus ffmpeg. Example:\n    python -m venv <folder>\\venv\n    <folder>\\venv\\Scripts\\pip install demucs-onnx onnxruntime-gpu\n    <folder>\\venv\\Scripts\\pip install nvidia-cublas nvidia-cuda-runtime nvidia-cuda-nvrtc nvidia-cufft nvidia-nvjitlink nvidia-cudnn-cu13\n    then set that interpreter below."
            visible: Mixxx.Core.stemGenerator.probeErrors.length > 0
            wrapMode: Text.WordWrap
        }

        RowLayout {
            Layout.fillWidth: true

            Setting.RatioChoice {
                id: enabledInput

                options: ["on", "off"]

                onSelectedChanged: root.markDirty()
            }
            Text {
                color: Theme.white
                text: "Enable stem generation"
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.preferredWidth: 220
                color: Theme.white
                text: "Interpreter"
            }
            TextField {
                id: pythonPathInput

                Layout.fillWidth: true
                placeholderText: "python"
                text: root.pythonPath

                onTextEdited: {
                    root.pythonPath = text;
                    root.markDirty();
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.preferredWidth: 220
                color: Theme.white
                text: "Stem folder"
            }
            TextField {
                id: cacheDirectoryInput

                Layout.fillWidth: true
                placeholderText: "%1/stems".arg(Mixxx.Application.settingsDirectoryUrl)
                text: root.cacheDirectory

                onTextEdited: {
                    root.cacheDirectory = text;
                    root.markDirty();
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Setting.RatioChoice {
                id: nextToTrackInput

                options: ["on", "off"]

                onSelectedChanged: root.markDirty()
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                text: "Write the stem file next to the track (otherwise it goes to the stem folder above)"
                wrapMode: Text.WordWrap
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Setting.RatioChoice {
                id: pauseInput

                options: ["on", "off"]

                onSelectedChanged: root.markDirty()
            }
            Text {
                Layout.fillWidth: true
                color: Theme.white
                text: "Pause generation while a deck is playing"
                wrapMode: Text.WordWrap
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.preferredWidth: 220
                color: Theme.white
                text: "Quality"
            }
            Skin.ComboBox {
                id: modelInput

                Layout.preferredWidth: 220
                model: ["htdemucs (fast)", "htdemucs_ft (best)"]

                onCurrentIndexChanged: root.markDirty()
            }
            Text {
                Layout.fillWidth: true
                color: Theme.midGray
                text: "The fast model is about 30% quicker and slightly less accurate."
                wrapMode: Text.WordWrap
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.preferredWidth: 220
                color: Theme.white
                text: "Format"
            }
            Skin.ComboBox {
                id: codecInput

                Layout.preferredWidth: 220
                model: ["aac (smallest)", "mp3 (most compatible)", "alac (lossless, big)"]

                onCurrentIndexChanged: root.markDirty()
            }
        }
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.preferredWidth: 220
                color: Theme.white
                text: "Processing device"
            }
            Skin.ComboBox {
                id: providerInput

                Layout.preferredWidth: 220
                model: ["auto", "cuda (GPU)", "cpu"]

                onCurrentIndexChanged: root.markDirty()
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.darkGray3
        }

        // --- queue -----------------------------------------------------------
        RowLayout {
            Layout.fillWidth: true

            Text {
                Layout.fillWidth: true
                color: Theme.white
                font.pixelSize: 14
                font.weight: Font.DemiBold
                text: "Queue"
            }
            Text {
                color: Theme.midGray
                text: Mixxx.Core.stemGenerator.queueStatus
            }
        }
        ListView {
            id: queueList

            Layout.fillWidth: true
            Layout.preferredHeight: 200
            clip: true
            model: Mixxx.Core.stemGenerator.queue
            spacing: 2

            delegate: Rectangle {
                required property int index
                required property string title
                required property string status
                required property int progress
                required property int state
                required property string error

                color: index % 2 === 0 ? Theme.darkGray4 : Theme.darkGray2
                height: 34
                width: queueList.width

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 6
                    anchors.rightMargin: 6
                    spacing: 8

                    Text {
                        Layout.fillWidth: true
                        color: state === 3 ? "#ff8a80" : Theme.white
                        elide: Text.ElideRight
                        text: title
                    }
                    Text {
                        color: Theme.midGray
                        elide: Text.ElideRight
                        text: error !== "" ? error : status
                    }
                    ProgressBar {
                        Layout.preferredWidth: 120

                        from: 0
                        to: 100
                        value: progress
                        visible: state === 1 || state === 2
                    }
                }
            }
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            FormButton {
                text: "Apply"

                onClicked: root.save()
            }
            FormButton {
                text: "Probe setup"

                onClicked: Mixxx.Core.stemGenerator.probe()
            }
            FormButton {
                text: "Open stem folder"

                onClicked: Mixxx.Core.stemGenerator.openCacheFolder()
            }
            FormButton {
                text: "Rescan library"

                onClicked: Mixxx.Core.stemGenerator.rescanLibrary()
            }
            FormButton {
                text: "Clear finished"

                onClicked: Mixxx.Core.stemGenerator.clearFinished()
            }
            FormButton {
                text: "Cancel all"

                onClicked: Mixxx.Core.stemGenerator.cancel()
            }
        }
    }
}
