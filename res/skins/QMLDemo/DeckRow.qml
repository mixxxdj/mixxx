import Mixxx 0.1 as Mixxx
import Mixxx.Controls 0.1 as MixxxControls
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.11

Item {
    id: root

    required property string leftDeck
    required property string rightDeck

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        Layout.margins: 10

        Deck {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            group: root.leftDeck
        }

        Rectangle {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            implicitWidth: 200
            Layout.fillHeight: true
            color: "#1e1e20"
            border.color: "#121213"
            border.width: 2
            radius: 1

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10

                EqColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.leftDeck
                }

                MixerColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.leftDeck
                }

                MixerColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.rightDeck
                }

                EqColumn {
                    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
                    group: root.rightDeck
                }

            }

        }

        Deck {
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.fillWidth: true
            Layout.fillHeight: true
            group: root.rightDeck
        }

    }

}
