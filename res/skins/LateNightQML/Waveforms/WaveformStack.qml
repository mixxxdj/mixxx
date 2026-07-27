import "../LateNightTheme"
import "../../../qml" as Shared
import QtQuick

Item {
    id: root

    property string deck1Group: "[Channel1]"
    property string deck2Group: "[Channel2]"
    property string deck3Group: "[Channel3]"
    property string deck4Group: "[Channel4]"
    property bool show4decks: false

    Loader {
        id: deck3waveform

        readonly property string group: root.deck3Group

        active: root.show4decks
        anchors.top: parent.top
        height: parent.height / 4
        width: root.width

        sourceComponent: Component {
            DeckWaveform {
                group: deck3waveform.group

                Shared.FadeBehavior on visible {
                    fadeTarget: deck3waveform
                }
            }
        }
    }
    DeckWaveform {
        id: deck1waveform

        anchors.top: root.show4decks ? deck3waveform.bottom : parent.top
        group: root.deck1Group
        height: parent.height / (root.show4decks ? 4 : 2)
        width: root.width
    }
    DeckWaveform {
        id: deck2waveform

        anchors.bottom: root.show4decks ? deck4waveform.top : parent.bottom
        group: root.deck2Group
        height: parent.height / (root.show4decks ? 4 : 2)
        width: root.width
    }
    Loader {
        id: deck4waveform

        readonly property string group: root.deck4Group

        active: root.show4decks
        anchors.bottom: parent.bottom
        height: parent.height / 4
        width: root.width

        sourceComponent: Component {
            DeckWaveform {
                group: deck4waveform.group

                Shared.FadeBehavior on visible {
                    fadeTarget: deck4waveform
                }
            }
        }
    }
    Rectangle {
        width: 125

        gradient: Gradient {
            orientation: Gradient.Horizontal

            GradientStop {
                color: LateNightTheme.darkGray
                position: 0
            }
            GradientStop {
                color: "transparent"
                position: 1
            }
        }

        anchors {
            bottom: parent.bottom
            left: parent.left
            top: parent.top
        }
    }
    Rectangle {
        width: 125

        gradient: Gradient {
            orientation: Gradient.Horizontal

            GradientStop {
                color: "transparent"
                position: 0
            }
            GradientStop {
                color: LateNightTheme.darkGray
                position: 1
            }
        }

        anchors {
            bottom: parent.bottom
            right: parent.right
            top: parent.top
        }
    }
}
