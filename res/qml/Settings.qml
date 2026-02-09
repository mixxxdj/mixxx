import "." as Skin
import Mixxx 1.0 as Mixxx
import QtQuick 2.12
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes
import Qt5Compat.GraphicalEffects
import "Theme"
import "Settings" as Settings

Popup {
    id: root

    property var activeCategory: null
    property alias activeCategoryIndex: categoryList.currentIndex
    readonly property var manager: managerItem
    property alias sections: managerItem.data

    function updateActiveCategory() {
        root.activeCategory?.deactivated();
        root.activeCategory = Object.values(managerItem.data)[categoryList.currentIndex] ?? null;
        root.activeCategory?.activated();
    }

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

        RowLayout {
            anchors.fill: parent
            spacing: 0

            Rectangle {
                Layout.fillHeight: true
                Layout.preferredWidth: 280
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
                        currentIndex: 0
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
                                }
                            }
                        }
                    }
                }
            }
            ColumnLayout {
                Layout.fillHeight: true
                Layout.fillWidth: true

                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredHeight: 36
                    color: Theme.white
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                    text: "Settings"
                }
                Rectangle {
                    id: tabBar

                    readonly property int selectedIndex: root.activeCategory?.selectedIndex ?? 0
                    readonly property var tabs: activeCategory?.tabs ?? []

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
                                    if (root.activeCategory?.selectedIndex || root.activeCategory?.selectedIndex === 0) {
                                        root.activeCategory.selectedIndex = index;
                                    }
                                }
                            }
                        }
                    }
                }
                Mixxx.SettingParameterManager {
                    id: managerItem

                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.leftMargin: 20

                    Component.onCompleted: {
                        let activateBuilder = index => function () {
                                categoryList.currentIndex = index;
                            };
                        let visibleBuilder = index => function () {
                                return categoryList.currentIndex == index;
                            };
                        for (let index = 0; index < data.length; index++) {
                            let child = data[index];
                            if (!child.label)
                                continue;
                            sectionProperties.append({
                                label: child.label
                            });
                            child.visible = Qt.binding(visibleBuilder(index));
                            child.activated.connect(activateBuilder(index));
                            child.anchors.fill = this;
                        }
                        // This is needed to ensure the right category is displayed.
                        // It would seems there is a bug, where the component's layout appears out of date.
                        // Setting the value to its current one seems to be triggering a component update which help fixing the layout
                        root.activeCategoryIndex = root.activeCategoryIndex;
                    }

                    Settings.SoundHardware {
                    }
                    Settings.Library {
                    }
                    Settings.Controller {
                    }
                    Settings.Interface {
                    }
                    Settings.MixerEffect {
                    }
                    Settings.AutoDJ {
                    }
                    Settings.Broadcast {
                    }
                    Settings.Recording {
                    }
                    Settings.Analyzer {
                    }
                    Settings.StatsPerformance {
                    }
                }
            }
        }
    }

    onActiveCategoryIndexChanged: updateActiveCategory()
    onSectionsChanged: updateActiveCategory()

    ListModel {
        id: sectionProperties

    }
}
