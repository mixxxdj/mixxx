import QtQuick
import Mixxx 1.0 as Mixxx
import "../LateNightTheme"

Item {
    id: root

    required property string group
    required property var track
    required property string text
    required property string displayProperty
    property string editProperty: displayProperty
    property bool contextMenuEnabled: true
    property bool editable: true
    property bool showTrackPropertiesOnDoubleClick: true
    property color textColor: LateNightTheme.textColor
    property int pixelSize: 18
    property int horizontalAlignment: Text.AlignLeft
    property int horizontalPadding: 0
    property int selectedTimeoutMs: 2000

    signal doubleClicked()

    property bool selected: false
    property bool editing: false

    implicitWidth: textLabel.implicitWidth + horizontalPadding * 2
    implicitHeight: textLabel.implicitHeight

    function propertyValue(propertyName) {
        if (!root.track) {
            return "";
        }
        switch (propertyName) {
        case "title":
            return root.track.title || "";
        case "artist":
            return root.track.artist || "";
        case "album":
            return root.track.album || "";
        default:
            return "";
        }
    }

    function setPropertyValue(propertyName, value) {
        if (!root.track) {
            return;
        }
        switch (propertyName) {
        case "title":
            root.track.title = value;
            break;
        case "artist":
            root.track.artist = value;
            break;
        case "album":
            root.track.album = value;
            break;
        default:
            break;
        }
    }

    function startSelectedClickTimer() {
        root.selected = true;
        selectedClickTimer.restart();
    }

    function openEditor() {
        if (!root.editable || !root.track) {
            return;
        }

        selectedClickTimer.stop();
        root.selected = false;
        editor.text = root.propertyValue(root.editProperty);
        if (root.displayProperty === "titleInfo" && editor.text.length === 0) {
            editor.text = qsTr("title");
        }
        root.editing = true;
        editor.forceActiveFocus();
        editor.selectAll();
    }

    function showContextMenu() {
        if (!root.track) {
            return;
        }

        Mixxx.Library.showDeckTrackMenu(root.track,
                root.group,
                root.displayProperty,
                -1,
                -1);
        root.selected = false;
        selectedClickTimer.stop();
        pendingEditorTimer.stop();
    }

    function showTrackProperties() {
        if (!root.track) {
            return;
        }

        Mixxx.Library.showDeckTrackProperties(root.track,
                root.group,
                root.displayProperty);
        root.selected = false;
        selectedClickTimer.stop();
        pendingEditorTimer.stop();
    }

    function commitEditor() {
        if (!root.editing) {
            return;
        }

        root.editing = false;
        root.setPropertyValue(root.editProperty, editor.text);
    }

    function cancelEditor() {
        root.editing = false;
    }

    Rectangle {
        anchors.fill: parent
        color: root.selected || mouseArea.containsMouse || root.editing ? "#151515" : "transparent"
        border.width: root.editing ? 1 : 0
        border.color: LateNightTheme.secondaryWaveformSignalColor
        visible: root.selected || mouseArea.containsMouse || root.editing
    }

    Text {
        id: textLabel
        anchors.fill: parent
        anchors.leftMargin: root.horizontalPadding
        anchors.rightMargin: root.horizontalPadding
        text: root.text
        font.family: "Open Sans"
        font.pixelSize: root.pixelSize
        font.weight: Font.Normal
        color: root.textColor
        elide: Text.ElideRight
        horizontalAlignment: root.horizontalAlignment
        verticalAlignment: Text.AlignVCenter
        visible: !root.editing
    }

    TextInput {
        id: editor
        anchors.fill: parent
        anchors.leftMargin: Math.max(2, root.horizontalPadding)
        anchors.rightMargin: Math.max(2, root.horizontalPadding)
        font.family: "Open Sans"
        font.pixelSize: root.pixelSize
        color: root.textColor
        selectedTextColor: "#111111"
        selectionColor: "#d9d9d9"
        horizontalAlignment: root.horizontalAlignment
        verticalAlignment: TextInput.AlignVCenter
        selectByMouse: true
        visible: root.editing

        Keys.onPressed: event => {
            if (event.key === Qt.Key_Escape) {
                root.cancelEditor();
                event.accepted = true;
            } else if (event.key === Qt.Key_Return || event.key === Qt.Key_Enter) {
                root.commitEditor();
                event.accepted = true;
            }
        }

        onActiveFocusChanged: {
            if (!activeFocus) {
                root.commitEditor();
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        enabled: !root.editing
        hoverEnabled: true

        onClicked: mouse => {
            if (!root.track) {
                return;
            }

            if (mouse.button === Qt.RightButton && root.contextMenuEnabled) {
                root.showContextMenu();
                return;
            }

            if (!root.editable) {
                return;
            }

            if (root.selected && selectedClickTimer.running) {
                pendingEditorTimer.restart();
                return;
            }

            root.startSelectedClickTimer();
        }

        onDoubleClicked: mouse => {
            if (!root.track || mouse.button !== Qt.LeftButton) {
                return;
            }

            if (root.showTrackPropertiesOnDoubleClick) {
                root.showTrackProperties();
            } else {
                root.doubleClicked();
            }
        }
    }

    Timer {
        id: selectedClickTimer
        interval: root.selectedTimeoutMs
        repeat: false

        onTriggered: {
            root.selected = false;
        }
    }

    Timer {
        id: pendingEditorTimer
        interval: 260
        repeat: false

        onTriggered: root.openEditor()
    }
}
