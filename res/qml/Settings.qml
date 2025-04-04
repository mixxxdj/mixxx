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
    property var sections: [
                            "SoundHardware",
                            "Library",
                            "Controller",
                            "Interface",
                            "MixerEffect",
                            "AutoDJ",
                            "Broadcast",
                            "Recording",
                            "Analyzer",
                            "StatsPerformance"
    ]

    ListModel {
        id: sectionProperties
    }

    Mixxx.SettingParameterManager {
        id: manager
    }
    Instantiator {
        id: categories
        model: root.sections
        asynchronous: true
        delegate: Loader {
            source: `Settings/${modelData}.qml`
        }
        onObjectAdded: (index, object) => {
            for (let i = sectionProperties.count; i < index; i++)
                sectionProperties.append({})
            sectionProperties.set(index, {"tabs": object.item.tabs || null, "label":object.item.label})
            manager.indexParameters(index, object)
            object.destroy();
        }
    }

    contentItem: Item {
        width: parent.width - 40
        height: parent.height - 40
        anchors.centerIn: parent
        RowLayout {
            anchors.fill: parent
            spacing: 0
            Rectangle {
                Layout.preferredWidth: 280
                Layout.fillHeight: true
                color: '#0E0E0E'
                border.color: '#3F3F3F'
                border.width: 6
                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 6
                    spacing: 0
                    Rectangle {
                        id: searchSetting
                        Layout.fillWidth: true
                        height: 30
                        color: '#626262'
                        property bool active: false
                        property alias input: searchInput
                        Text {
                            id: searchInputPlaceholder
                            visible: !parent.active
                            anchors.verticalCenter: parent.verticalCenter
                            color: '#D9D9D9'
                            text: 'Search...'
                        }
                        TextInput {
                            id: searchInput
                            anchors.verticalCenter: parent.verticalCenter
                            width: parent.width
                            visible: parent.active
                            onActiveFocusChanged: {
                                parent.active = activeFocus
                            }
                            onTextEdited: {
                                manager.search(text)
                            }
                        }
                        TapHandler {
                            onTapped: {
                                parent.active = true
                                searchInput.forceActiveFocus()
                            }
                        }
                    }
                    ListView {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        id: categoryList
                        model: sectionProperties
                        clip: true
                        visible: !searchSetting.active
                        delegate: Rectangle {
                            required property int index
                            required property var label
                            width: ListView.view.width
                            height: 38
                            color: ListView.isCurrentItem ? '#3F3F3F' : '#2B2B2B'
                            Image {
                                id: handleImage
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 8
                                height: 24
                                visible: false
                                source: "images/gear.svg"
                                fillMode: Image.PreserveAspectFit
                            }
                            ColorOverlay {
                                anchors.fill: handleImage
                                source: handleImage
                                color: ListView.isCurrentItem ? '#3392E6' : "#696968"
                                antialiasing: true
                            }
                            Text {
                                anchors.left: handleImage.right
                                anchors.leftMargin: 8
                                anchors.verticalCenter: parent.verticalCenter
                                color: '#D9D9D9'
                                text: label
                                font.bold: ListView.isCurrentItem
                            }
                            TapHandler {
                                onTapped: {
                                    categoryList.currentIndex = index
                                }
                            }
                        }

                        focus: true
                    }
                    ListView {
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        id: settingResultList
                        model: manager.model
                        clip: true
                        visible: searchSetting.active
                        delegate: Rectangle {
                            required property int index
                            required property var display
                            required property var whatsThis
                            required property var toolTip
                            width: ListView.view.width
                            height: 40
                            color: '#2B2B2B'
                            ColumnLayout {
                                anchors.fill: parent
                                anchors.margins: 4

                                Text {
                                    Layout.preferredHeight: implicitHeight
                                    Layout.fillWidth: true
                                    color: '#D9D9D9'
                                    text: searchSetting.input.text ? display.replace(searchSetting.input.text, `<b>${searchSetting.input.text}</b>`) : display
                                    textFormat: Text.RichText
                                }
                                Text {
                                    Layout.preferredHeight: implicitHeight
                                    Layout.fillWidth: true
                                    color: '#686868'
                                    text: searchSetting.input.text ? whatsThis.replace(searchSetting.input.text, `<b>${searchSetting.input.text}</b>`) : whatsThis
                                    textFormat: Text.RichText
                                    font.pixelSize: 10
                                }
                            }
                            TapHandler {
                                onTapped: {
                                    categoryList.currentIndex = toolTip
                                    parent.forceActiveFocus()
                                }
                            }
                        }

                        focus: true
                    }
                }
            }
            ColumnLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Text {
                    Layout.preferredHeight: 36
                    Layout.alignment: Qt.AlignHCenter
                    color: '#D9D9D9'
                    text: "Settings"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }
                Rectangle {
                    id: tabBar
                    Layout.preferredHeight: 30
                    Layout.fillWidth: true
                    color: '#3F3F3F'

                    property int selectedIndex: 0
                    readonly property var tabsData: sectionProperties.get(categoryList.currentIndex).tabs
                    readonly property var tabs: tabsData ? Object.values(tabsData) : []
                    visible: !!tabs

                    RowLayout {
                        anchors.fill: parent
                        Repeater {
                            model: tabBar.tabs

                            Skin.Button {
                                required property int index
                                required property string modelData

                                text: modelData
                                activeColor: Theme.white
                                Layout.alignment: Qt.AlignHCenter
                                Layout.preferredHeight: 22
                                Layout.preferredWidth: parent.width / (tabBar.tabs.length + 2)

                                checked: tabBar.selectedIndex == index

                                onPressed: {
                                    tabBar.selectedIndex = index
                                }
                            }
                        }
                    }
                }
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.leftMargin: 20

                    Loader {
                        id: settingCategory
                        anchors.fill: parent
                        source: `Settings/${root.sections[categoryList.currentIndex]}.qml`
                    }
                }
                Connections {
                    target: tabBar
                    function onSelectedIndexChanged() {
                        if (!settingCategory.item || settingCategory.item.selectedIndex === undefined)
                            return
                        settingCategory.item.selectedIndex = tabBar.selectedIndex
                    }
                }
            }
        }
    }
    background: Rectangle {
        color: '#2B2B2B'
        anchors.fill: parent
        opacity: parent.radius < 0 ? Math.max(0.1, 1 + parent.radius / 8) : 1
        radius: 8
    }
}
