import Mixxx 0.1 as Mixxx
import Qt.labs.qmlmodels 1.0
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import "Theme"

Window {
    id: root

    title: "Developer Tools"
    color: Theme.toolbarBackgroundColor

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 5

        TextField {
            id: searchField

            Layout.fillWidth: true
            placeholderText: "Search Term..."
        }

        TableView {
            id: tableView

            Layout.fillHeight: true
            Layout.fillWidth: true
            columnSpacing: 1
            rowSpacing: 1
            clip: true
            onWidthChanged: forceLayout()

            model: Mixxx.ControlModel {
            }

            delegate: DelegateChooser {
                DelegateChoice {
                    column: 0

                    delegate: Rectangle {
                        implicitWidth: (root.width - 10) * 0.3
                        implicitHeight: groupName.contentHeight
                        color: root.color

                        Text {
                            id: groupName

                            anchors.fill: parent
                            text: display
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: Theme.deckTextColor
                        }

                    }

                }

                DelegateChoice {
                    column: 1

                    delegate: Rectangle {
                        implicitWidth: (root.width - 10) * 0.5
                        implicitHeight: keyName.contentHeight
                        color: root.color

                        Text {
                            id: keyName

                            anchors.fill: parent
                            text: display
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: Theme.deckTextColor
                        }

                    }

                }

                DelegateChoice {
                    column: 2

                    delegate: Rectangle {
                        implicitWidth: (root.width - 10) * 0.2
                        implicitHeight: valueField.implicitHeight
                        color: root.color

                        TextField {
                            // TODO: Make editing work

                            id: valueField

                            anchors.fill: parent
                            text: display

                            validator: DoubleValidator {
                            }

                        }

                    }

                }

            }

        }

    }

}
