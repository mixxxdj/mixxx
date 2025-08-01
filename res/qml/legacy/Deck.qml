import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as E

E.Frame {
    color: "#202020"
    Layout.minimumWidth: 200
    padding: 4
    ColumnLayout {
        height: parent.height
        width: parent.width
        spacing: 0
        Item {
            Layout.fillHeight: true
        }
        // Playback/Cue
        RowLayout {
            // Playback/Cue section
            GridLayout {
                columnSpacing: 2
                rowSpacing: 2
                columns: 2
                // Cue
                Button {
                    text: "CUE"
                    color: "#141414"
                }
                // Reverse
                Button {
                    color: "#141414"
                    implicitWidth: 24
                }
                // Play/Pause
                Button {
                    color: "#141414"
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                }
            }
            // Spacer
            Item {
                Layout.fillWidth: true
            }
            // Hotcues section
            GridLayout {
                columnSpacing: 2
                rowSpacing: 2
                columns: 4
                CueButton {
                    text: "1"
                }
                CueButton {
                    text: "2"
                }
                CueButton {
                    text: "3"
                }
                CueButton {
                    text: "4"
                }
                CueButton {
                    text: "5"
                }
                CueButton {
                    text: "6"
                }
                CueButton {
                    text: "7"
                }
                CueButton {
                    text: "8"
                }
            }
            // spacer
            Item {
                Layout.fillWidth: true
            }
            // Intro/Outro section
            GridLayout {
                columnSpacing: 2
                rowSpacing: 2
                columns: 2
                IntroOutroButton {
                }
                IntroOutroButton {
                }
                IntroOutroButton {
                }
                IntroOutroButton {
                }
            }
            // spacer
            Item {
                Layout.fillWidth: true
            }
            // Beatloop section
            ColumnLayout {
                spacing: 2
                RowLayout {
                    spacing: 2
                    LoopButton {
                    }
                    SpinBox {
                        editable: true
                        from: 0
                        to: 512
                    }
                }
                RowLayout {
                    Layout.maximumWidth: childrenRect.width
                    spacing: 2
                    LoopToggleButton {
                    }
                    LoopButton {
                    }
                    LoopButton {
                    }
                }
            }
            // spacer
            Item {
                Layout.fillWidth: true
            }
            // Loopjump section
            ColumnLayout {
                spacing: 2
                RowLayout {
                    SpinBox {
                        editable: true
                        from: 0
                        to: 512
                    }
                }
                RowLayout {
                    spacing: 2
                    Layout.maximumWidth: childrenRect.width
                    LoopButton {
                    }
                    LoopButton {
                    }
                }
            }
        }
    }
}
