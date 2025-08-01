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
            // Main
            ColumnLayout {
                spacing: 0
                E.Dial {
                    color: "#c06020"
                }
                E.Label {
                    text: "MAIN"
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 10
                }
            }
            // Balance
            ColumnLayout {
                spacing: 0
                E.Dial {
                    color: "#a00000"
                    from: -1
                    to: 1
                }
                E.Label {
                    text: "BAL"
                    Layout.alignment: Qt.AlignHCenter
                    font.pixelSize: 10
                }
            }
        }
        E.FxButtonRow {
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
