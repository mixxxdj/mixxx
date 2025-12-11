import QtQuick
import Mixxx 1.0 as Mixxx

Mixxx.SettingGroup {
    id: root

    signal activated;
    signal deactivated;

    property int selectedIndex: 0
    property list<string> tabs: []
}
