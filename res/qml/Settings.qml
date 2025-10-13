import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "Theme"

Popup {
    id: root

    property int activeCategoryIndex: 0
    property list<string> sections: ["SoundHardware", "Library", "Controller", "Interface", "MixerEffect", "AutoDJ", "Broadcast", "Recording", "Analyzer", "StatsPerformance"]

    readonly property var manager: managerItem

    // FIXME change to `final` when supported
    readonly property real smallScreenWidth: 1200

    background: Rectangle {
        anchors.fill: parent
        color: Theme.darkGray2
        opacity: parent.radius < 0 ? Math.max(0.1, 1 + parent.radius / 8) : 1
        radius: 8
    }
    contentItem: Item {
        anchors.centerIn: parent
        height: parent.height - 40
        width: parent.width - 40

        Rectangle {
            id: categories

            width: Math.min(280, root.width*0.25)
            visible: root.width > root.smallScreenWidth || showCategories.checked
            z: 100

            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
            }

            border.color: Theme.darkGray3
            border.width: 6
            color: Theme.darkGray

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 6
                spacing: 0

                Rectangle {
                    id: searchSetting

                    property bool active: false
                    property alias input: searchInput

                    Layout.fillWidth: true
                    color: Theme.midGray
                    height: 30

                    Text {
                        id: searchInputPlaceholder
                        anchors.verticalCenter: parent.verticalCenter
                        color: Theme.white
                        text: 'Search...'
                        visible: !parent.active
                    }
                    TextInput {
                        id: searchInput
                        anchors.verticalCenter: parent.verticalCenter
                        visible: parent.active
                        width: parent.width

                        onActiveFocusChanged: {
                            parent.active = activeFocus;
                        }
                        onTextEdited: {
                            root.manager.search(text);
                        }
                    }
                    TapHandler {
                        onTapped: {
                            parent.active = true;
                            searchInput.forceActiveFocus();
                        }
                    }
                }
                ListView {
                    id: categoryList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    focus: true
                    model: sectionProperties
                    visible: !searchSetting.active

                    delegate: Rectangle {
                        required property int index
                        required property var label

                        color: ListView.isCurrentItem ? Theme.darkGray3 : Theme.darkGray2
                        height: 38
                        width: ListView.view.width

                        Image {
                            id: handleImage
                            anchors.left: parent.left
                            anchors.leftMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            fillMode: Image.PreserveAspectFit
                            height: 24
                            source: "images/gear.svg"
                            visible: false
                        }
                        ColorOverlay {
                            anchors.fill: handleImage
                            antialiasing: true
                            color: parent.ListView.isCurrentItem ? Theme.accentColor : Theme.midGray
                            source: handleImage
                        }
                        Text {
                            anchors.left: handleImage.right
                            anchors.leftMargin: 8
                            anchors.verticalCenter: parent.verticalCenter
                            color: Theme.white
                            font.bold: parent.ListView.isCurrentItem
                            text: label
                        }
                        TapHandler {
                            onTapped: {
                                categoryList.currentIndex = index;
                                showCategories.checked = false
                            }
                        }
                    }
                }
                ListView {
                    id: settingResultList
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    clip: true
                    focus: true
                    model: root.manager.model
                    visible: searchSetting.active

                    delegate: Rectangle {
                        required property var display
                        required property int index
                        required property var toolTip
                        required property var whatsThis

                        color: Theme.darkGray2
                        height: 40
                        width: ListView.view.width

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 4

                            Text {
                                Layout.fillWidth: true
                                Layout.preferredHeight: implicitHeight
                                color: Theme.white
                                text: searchSetting.input.text ? display.replace(searchSetting.input.text, `<b>${searchSetting.input.text}</b>`) : display
                                textFormat: Text.RichText
                            }
                            Text {
                                Layout.fillWidth: true
                                Layout.preferredHeight: implicitHeight
                                color: Theme.midGray
                                font.pixelSize: 10
                                text: searchSetting.input.text ? whatsThis.replace(searchSetting.input.text, `<b>${searchSetting.input.text}</b>`) : whatsThis
                                textFormat: Text.RichText
                            }
                        }
                        TapHandler {
                            onTapped: {
                                for (let setting of toolTip) {
                                    setting.activated();
                                }
                                parent.forceActiveFocus();
                                showCategories.checked = false
                            }
                        }
                    }
                }
            }
        }
        ColumnLayout {
            id: content
            anchors {
                right: parent.right
                left: categories.right
                top: parent.top
                bottom: parent.bottom
            }

            states: [
                State {
                    when: root.width < root.smallScreenWidth
                    AnchorChanges { target: content; anchors.left: parent.left}
                }
            ]

            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                Skin.Button {
                    id: showCategories
                    visible: root.width < root.smallScreenWidth
                    activeColor: Theme.white
                    checkable: true
                    text: "M"
                }

                Text {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 36
                    color: Theme.white
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    text: "Settings"
                    horizontalAlignment: Text.AlignHCenter
                }

                Skin.Button {
                    visible: root.width < root.smallScreenWidth
                    activeColor: Theme.white
                    text: "X"
                    onPressed: {
                        root.close()
                    }
                }
            }
            Rectangle {
                id: tabBar

                readonly property var categoryItem: categoriesLoader.itemAt(categoryList.currentIndex) ? categoriesLoader.itemAt(categoryList.currentIndex).item : null
                readonly property int selectedIndex: categoryItem && categoryItem.selectedIndex !== undefined ? categoryItem.selectedIndex : 0
                readonly property var tabs: categoryItem ? categoryItem.tabs : []

                Layout.fillWidth: true
                Layout.preferredHeight: 30
                color: Theme.darkGray3
                visible: tabs?.length > 0

                RowLayout {
                    anchors.fill: parent

                    Repeater {
                        model: tabBar.tabs

                        Skin.Button {
                            required property int index
                            required property string modelData

                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredHeight: 22
                            Layout.preferredWidth: parent.width / (tabBar.tabs.length + 2)
                            activeColor: Theme.white
                            checked: tabBar.selectedIndex == index
                            text: modelData

                            onPressed: {
                                categoriesLoader.itemAt(categoryList.currentIndex).item.selectedIndex = index;
                            }
                        }
                    }
                }
            }

            Mixxx.SettingParameterManager {
                id: managerItem
                clip: true
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.leftMargin: 20

                Repeater {
                    id: categoriesLoader
                    model: root.sections

                    Loader {
                        id: category

                        required property int index
                        required property var modelData

                        anchors.fill: parent
                        source: `Settings/${modelData}.qml`
                        visible: categoryList.currentIndex == index

                        // asynchronous: true // Unsupported
                        onLoaded: {
                            for (let i = sectionProperties.count; i < index; i++)
                                sectionProperties.append({});
                            sectionProperties.set(index, {
                                    "label": category.item.label
                            });
                        }

                        Connections {
                            function onActivated() {
                                categoryList.currentIndex = index;
                            }

                            target: category.item
                        }
                    }
                }
            }
        }
    }

    ListModel {
        id: sectionProperties
    }
}
