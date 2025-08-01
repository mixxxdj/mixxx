import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import "." as E

Item {
    id: root
    width: mixer.implicitWidth
    height: mixer.implicitHeight

    RowLayout {
        spacing: 0
        id: mixer
        E.Mixer {
            Layout.fillHeight: true
        }
        Item {
            Layout.fillHeight: true
            width: childrenRect.width
            E.MainMix {
                height: parent.height / 2
            }
            E.HeadphoneMix {
                y: parent.height / 2
                height: parent.height / 2
            }
        }
    }
}
