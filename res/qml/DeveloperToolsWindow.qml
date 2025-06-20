import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQml.Models // DelegateChoice for Qt >= 6.9
import Qt.labs.qmlmodels // DelegateChooser
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

        // FIXME: Header items currently get squeezed when the table is empty
        // (i.e. if a non-existing search term was entered).
        HorizontalHeaderView {
            id: horizontalHeader

            Layout.fillWidth: true
            syncView: tableView

            delegate: Item {
                id: headerDlgt

                required property int column
                required property string display

                implicitHeight: columnName.contentHeight + 5
                implicitWidth: columnName.contentWidth + 5
                visible: this.column < tableView.columnWidths.length

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

                    text: headerDlgt.display
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
                    visible: controlModel.sortColumn == headerDlgt.column
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
                    onTapped: controlModel.toggleSortColumn(headerDlgt.column)
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
                        id: groupDelegate

                        required property int column
                        required property string display

                        implicitWidth: (root.width - 10) * tableView.columnWidths[groupDelegate.column]
                        implicitHeight: groupName.contentHeight
                        color: root.color

                        Text {
                            id: groupName

                            anchors.fill: parent
                            text: groupDelegate.display
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: Theme.deckTextColor
                        }
                    }
                }

                DelegateChoice {
                    column: 1

                    delegate: Rectangle {
                        id: keyDelegate

                        required property int column
                        required property string display

                        implicitWidth: (root.width - 10) * tableView.columnWidths[keyDelegate.column]
                        implicitHeight: keyName.contentHeight
                        color: root.color

                        Text {
                            id: keyName

                            anchors.fill: parent
                            text: keyDelegate.display
                            verticalAlignment: Text.AlignVCenter
                            elide: Text.ElideRight
                            color: Theme.deckTextColor
                        }
                    }
                }

                DelegateChoice {
                    column: 2

                    delegate: Rectangle {
                        id: valueDelegate

                        required property int row
                        required property int column
                        required property string display

                        implicitWidth: (root.width - 10) * tableView.columnWidths[valueDelegate.column]
                        implicitHeight: valueField.implicitHeight
                        color: root.color

                        Skin.TextField {
                            id: valueField

                            anchors.fill: parent
                            text: valueDelegate.display
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onEditingFinished: {
                                const idx = controlModel.index(valueDelegate.row, valueDelegate.column);
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
                        id: parameterDelegate

                        required property int row
                        required property int column
                        required property string display

                        implicitWidth: (root.width - 10) * tableView.columnWidths[parameterDelegate.column]
                        implicitHeight: valueField.implicitHeight
                        color: root.color

                        Skin.TextField {
                            id: valueField

                            anchors.fill: parent
                            text: parameterDelegate.display
                            inputMethodHints: Qt.ImhFormattedNumbersOnly
                            onEditingFinished: {
                                const idx = controlModel.index(parameterDelegate.row, parameterDelegate.column);
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
