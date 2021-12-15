import "." as Skin
import Mixxx 0.1 as Mixxx
import Qt.labs.qmlmodels 1.0
import QtQuick 2.12
import QtQuick.Controls 2.15
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

        Skin.TextField {
            id: searchField

            Layout.fillWidth: true
            placeholderText: "Search Term..."
            onTextChanged: controlModel.setFilterFixedString(text)
        }

        HorizontalHeaderView {
            id: horizontalHeader

            Layout.fillWidth: true
            syncView: tableView

            delegate: Item {
                implicitHeight: columnName.contentHeight + 5
                implicitWidth: columnName.contentWidth + 5
                visible: column < tableView.columnWidths.length

                BorderImage {
                    anchors.fill: parent
                    horizontalTileMode: BorderImage.Stretch
                    verticalTileMode: BorderImage.Stretch
                    source: Theme.imgPopupBackground

                    border {
                        top: 10
                        left: 20
                        right: 20
                        bottom: 10
                    }

                }

                Text {
                    id: columnName

                    text: display
                    anchors.fill: parent
                    anchors.margins: 5
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font.family: Theme.fontFamily
                    font.capitalization: Font.AllUppercase
                    font.bold: true
                    font.pixelSize: Theme.buttonFontPixelSize
                    color: Theme.buttonNormalColor
                }

                Text {
                    id: sortIndicator

                    text: controlModel.sortDescending ? "▲" : "▼"
                    visible: controlModel.sortColumn == column
                    anchors.fill: parent
                    anchors.margins: 5
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignRight
                    verticalAlignment: Text.AlignVCenter
                    font.family: Theme.fontFamily
                    font.capitalization: Font.AllUppercase
                    font.bold: true
                    font.pixelSize: Theme.buttonFontPixelSize
                    color: Theme.buttonNormalColor
                }

                TapHandler {
                    onTapped: controlModel.toggleSortColumn(column)
                }

            }

        }

        TableView {
            id: tableView

            property var columnWidths: [0.3, 0.4, 0.15, 0.15]

            Layout.fillHeight: true
            Layout.fillWidth: true
            columnSpacing: 1
            rowSpacing: 1
            clip: true
            onWidthChanged: forceLayout()
            boundsBehavior: Flickable.StopAtBounds

            model: Mixxx.ControlSortFilterModel {
                id: controlModel

                function toggleSortColumn(column) {
                    const descending = (sortColumn == column) ? !sortDescending : false;
                    sortByColumn(column, descending);
                }

                Component.onCompleted: {
                    // First order by key, then by group.
                    sortByColumn(1, false);
                    sortByColumn(0, false);
                }
            }

            delegate: DelegateChooser {
                DelegateChoice {
                    column: 0

                    delegate: Rectangle {
                        implicitWidth: (root.width - 10) * tableView.columnWidths[column]
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
                        implicitWidth: (root.width - 10) * tableView.columnWidths[column]
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
                        implicitWidth: (root.width - 10) * tableView.columnWidths[column]
                        implicitHeight: valueField.implicitHeight
                        color: root.color

                        Skin.TextField {
                            id: valueField

                            anchors.fill: parent
                            text: display
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onEditingFinished: {
                                const idx = controlModel.index(row, column);
                                controlModel.setData(idx, parseFloat(text));
                            }
                            color: Theme.textColor

                            background: Rectangle {
                                anchors.fill: parent
                                color: Theme.embeddedBackgroundColor
                                radius: 5
                            }

                            validator: DoubleValidator {
                            }

                        }

                    }

                }

                DelegateChoice {
                    column: 3

                    delegate: Rectangle {
                        implicitWidth: (root.width - 10) * tableView.columnWidths[column]
                        implicitHeight: valueField.implicitHeight
                        color: root.color

                        Skin.TextField {
                            id: valueField

                            anchors.fill: parent
                            text: display
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onEditingFinished: {
                                const idx = controlModel.index(row, column);
                                controlModel.setData(idx, parseFloat(text));
                            }
                            color: Theme.textColor

                            background: Rectangle {
                                anchors.fill: parent
                                color: Theme.embeddedBackgroundColor
                                radius: 5
                            }

                            validator: DoubleValidator {
                            }

                        }

                    }

                }

            }

        }

    }

}
