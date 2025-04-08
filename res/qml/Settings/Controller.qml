import QtQuick
import QtQuick.Layouts
import Mixxx 1.0 as Mixxx
import ".." as Skin

Mixxx.SettingGroup {
    label: "Controllers"

    Mixxx.SettingParameter {
        label: "A orange square"

        Rectangle {
            color: 'orange'
            height: 20
            width: 20
        }
    }
    ColumnLayout {
        anchors.fill: parent
        GridLayout {
            columns: 2
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                color: '#181818'
                radius: 22
                ColumnLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 34
                    anchors.rightMargin: 7
                    anchors.topMargin: 17
                    anchors.bottomMargin: 13
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            color: '#FFFFFF'
                            text: "Native Instruments S4 Mk3"
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Text {
                            color: '#FFFFFF'
                            text: "Supported since 2.4"
                            font.pixelSize: 10
                            font.weight: Font.Light
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Layout.rightMargin: 7
                        spacing: 7
                        Rectangle {
                            color: 'red'
                            Layout.preferredWidth: 162
                            Layout.preferredHeight: 100
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        Skin.Button {
                            Layout.alignment: Qt.AlignBottom
                            id: showPreferencesButton
                            activeColor: Theme.white
                            checked: settingsPopup.opened
                            icon.height: 16
                            icon.source: "images/gear.svg"
                            icon.width: 16
                            implicitWidth: implicitHeight
                        }
                        Skin.ComboBox {
                            Layout.alignment: Qt.AlignBottom
                            Layout.minimumWidth: 180
                            spacing: 2
                            indicator.width: 0
                            popupWidth: 150
                            clip: true

                            opacity: fxControl.value ? 1 : 0.5
                            textRole: "display"
                            font.pixelSize: 10
                            model: ["Foo"]
                        }
                        Skin.Button {
                            Layout.alignment: Qt.AlignBottom
                            id: show4DecksButton
                            activeColor: Theme.white
                            checkable: true
                            text: "4 Decks"
                        }
                    }
                }
            }
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 180
                color: '#181818'
                radius: 22
                ColumnLayout {
                    anchors.fill: parent
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Native Instruments S4 Mk3"
                            font.pixelSize: 16
                            font.weight: Font.DemiBold
                        }
                        Text {
                            text: "Supported since 2.4"
                            font.pixelSize: 10
                            font.weight: Font.Light
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        Rectangle {
                            color: 'red'
                            Layout.preferredWidth: 162
                            Layout.preferredHeight: 100
                        }
                        Skin.Button {
                            Layout.alignment: Qt.AlignBottom
                            id: showPreferencesButton2
                            activeColor: Theme.white
                            checked: settingsPopup.opened
                            icon.height: 16
                            icon.source: "images/gear.svg"
                            icon.width: 16
                            implicitWidth: implicitHeight
                        }
                        Skin.ComboBox {
                            Layout.alignment: Qt.AlignBottom
                            spacing: 2
                            indicator.width: 0
                            popupWidth: 150
                            clip: true

                            opacity: fxControl.value ? 1 : 0.5
                            textRole: "display"
                            font.pixelSize: 10
                            model: ["Foo"]
                        }
                        Skin.Button {
                            Layout.alignment: Qt.AlignBottom
                            id: show4DecksButton2
                            activeColor: Theme.white
                            checkable: true
                            text: "4 Decks"
                        }
                    }
                }
            }
        }
        Item {
            Layout.fillHeight: true
        }
        ListView {
            height: 200
            Layout.fillWidth: true

            header: Rectangle {
                // implicitWidth: 120
                implicitHeight: 32
                width: ListView.view.width
                color: "#161616"
                Text {
                    color: '#FFFFFF'
                    anchors.fill: parent
                    anchors.margins: 10
                    anchors.leftMargin: 20
                    anchors.rightMargin: 20
                    text: 'Other detected devices'
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }
            }

            model: 4
            delegate: Rectangle {
                implicitHeight: 32
                width: ListView.view.width
                required property int index
                color: index % 2 == 0 ? '#0C0C0C' : '#272727'
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 7
                    Rectangle {
                        color: '#393939'
                        Layout.preferredWidth: 35
                        Layout.preferredHeight: 18
                        radius: 4
                        Text {
                            anchors.centerIn: parent
                            text: 'HID'
                            color: '#FFFFFF'
                            font.pixelSize: 14
                            font.weight: Font.Medium
                        }
                    }
                    Text {
                        Layout.fillWidth: true
                        text: '<b>Name:</b> '
                        color: '#FFFFFF'
                        font.pixelSize: 14
                        font.weight: Font.Medium
                    }
                }
            }
        }
    }
}
