import "." as Skin
import QtQuick 2.12
import QtQuick.Layouts 1.12
import "Theme"

Item {
    id: root

    implicitHeight: 500

    ColumnLayout {
        id: deckCarts

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: mixer.left
        anchors.bottom: parent.verticalCenter

        Repeater {
            model: 4

            Skin.Sampler {
                Layout.fillWidth: true
                Layout.fillHeight: true
                group: "[Channel" + (index + 1) + "]"
            }

        }

    }

    Grid {
        id: mixer

        columns: 20
        anchors.centerIn: parent
        height: parent.height

        Skin.SectionBackground {
            height: parent.height
            width: 66

            Skin.BroadcastMixerColumn {
                anchors.centerIn: parent
                width: 56
                height: parent.height - 10
                group: "[Master]"
                label: "MAIN"
                color: Theme.white
            }

        }

        Skin.SectionBackground {
            height: parent.height
            width: decks.width

            Row {
                id: decks

                height: parent.height
                padding: 5

                Repeater {
                    model: 4

                    Skin.BroadcastMixerColumn {
                        property int i: index + 1

                        width: 56
                        height: parent.height - parent.padding * 2
                        group: "[Channel" + i + "]"
                        label: "DECK " + i
                        color: Theme.blue
                    }

                }

            }

        }

        Skin.SectionBackground {
            height: parent.height
            width: microphones.width

            Row {
                id: microphones

                height: parent.height
                padding: 5

                Repeater {
                    model: 4

                    Skin.BroadcastMixerColumn {
                        property int i: index + 1

                        width: 56
                        height: parent.height - parent.padding * 2
                        group: i == 1 ? "[Microphone]" : "[Microphone" + i + "]"
                        label: "MIC " + i
                        color: Theme.red
                    }

                }

            }

        }

        Skin.SectionBackground {
            height: parent.height
            width: auxiliaries.width

            Row {
                id: auxiliaries

                height: parent.height
                padding: 5

                Repeater {
                    model: 4

                    Skin.BroadcastMixerColumn {
                        property int i: index + 1

                        width: 56
                        height: parent.height - parent.padding * 2
                        group: "[Auxiliary" + i + "]"
                        label: "AUX " + i
                        color: Theme.green
                    }

                }

            }

        }

        Skin.SectionBackground {
            height: parent.height
            width: samplers.width

            Row {
                id: samplers

                height: parent.height
                padding: 5

                Repeater {
                    model: 8

                    Skin.BroadcastMixerColumn {
                        property int i: index + 1

                        width: 56
                        height: parent.height - parent.padding * 2
                        group: "[Sampler" + i + "]"
                        label: "S " + i
                        color: Theme.yellow
                    }

                }

            }

        }

    }

    ColumnLayout {
        id: samplerCarts

        anchors.top: parent.top
        anchors.left: mixer.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        Repeater {
            model: 8

            Skin.Sampler {
                Layout.fillWidth: true
                Layout.fillHeight: true
                group: "[Sampler" + (index + 1) + "]"
            }

        }

    }

}
