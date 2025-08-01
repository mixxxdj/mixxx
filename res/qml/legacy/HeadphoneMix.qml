import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as E

E.Frame {
    color: "#171717"
    padding: 2
    ColumnLayout {
        height: parent.height
        spacing: 2
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            // Headphones
            ColumnLayout {
                spacing: 0
                E.Dial {
                    color: "#c06020"
                }
                E.Label {
                    text: "HEAD"
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 10
                }
            }
            // Mix
            ColumnLayout {
                spacing: 0
                E.Dial {
                    color: "#a00000"
                }
                E.Label {
                    text: "MIX"
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 10
                }
            }
        }
        E.ToggleButton {
            text: "SPLIT"
            font.pixelSize: 12
            implicitHeight: 20
            Layout.alignment: Qt.AlignHCenter
        }
        E.FxButtonRow {
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
