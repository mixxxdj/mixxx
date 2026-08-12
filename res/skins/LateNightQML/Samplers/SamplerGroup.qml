pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property int count
    required property string expandKey
    readonly property bool expanded: expandControl.value > 0
    required property int firstSampler
    readonly property Item loadedItem: content.item as Item
    property bool show8Hotcues: true
    property bool showFxAssignments: true

    implicitHeight: loadedItem?.implicitHeight ?? 0

    Loader {
        id: content

        height: root.loadedItem?.implicitHeight ?? 0
        sourceComponent: root.expanded ? (root.count === 4 ? expandedFour : expandedEight) : minimizedRow
        width: root.width
    }
    Mixxx.ControlProxy {
        id: expandControl

        group: "[LateNight]"
        key: root.expandKey
    }
    Component {
        id: minimizedRow

        RowLayout {
            spacing: 4

            Repeater {
                model: root.count / 2

                SamplerMini {
                    required property int index

                    Layout.fillWidth: true
                    group: "[Sampler" + (root.firstSampler + index) + "]"
                }
            }
            SamplerExpandButton {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 20
                controlKey: root.expandKey
            }
            Repeater {
                model: root.count / 2

                SamplerMini {
                    required property int index

                    Layout.fillWidth: true
                    group: "[Sampler" + (root.firstSampler + root.count / 2 + index) + "]"
                }
            }
        }
    }
    Component {
        id: expandedFour

        RowLayout {
            spacing: 4

            Repeater {
                model: 2

                SamplerFull {
                    required property int index

                    Layout.fillWidth: true
                    group: "[Sampler" + (root.firstSampler + index) + "]"
                    show8Hotcues: root.show8Hotcues
                    showFxAssignments: root.showFxAssignments
                }
            }
            SamplerExpandButton {
                Layout.alignment: Qt.AlignTop
                Layout.preferredWidth: 20
                controlKey: root.expandKey
            }
            Repeater {
                model: 2

                SamplerFull {
                    required property int index

                    Layout.fillWidth: true
                    group: "[Sampler" + (root.firstSampler + 2 + index) + "]"
                    show8Hotcues: root.show8Hotcues
                    showFxAssignments: root.showFxAssignments
                }
            }
        }
    }
    Component {
        id: expandedEight

        ColumnLayout {
            spacing: 4

            RowLayout {
                Layout.fillWidth: true
                spacing: 4

                Repeater {
                    model: [0, 1]

                    SamplerFull {
                        required property int modelData

                        Layout.fillWidth: true
                        group: "[Sampler" + (root.firstSampler + modelData) + "]"
                        show8Hotcues: root.show8Hotcues
                        showFxAssignments: root.showFxAssignments
                    }
                }
                SamplerExpandButton {
                    Layout.alignment: Qt.AlignTop
                    Layout.preferredWidth: 20
                    controlKey: root.expandKey
                }
                Repeater {
                    model: [4, 5]

                    SamplerFull {
                        required property int modelData

                        Layout.fillWidth: true
                        group: "[Sampler" + (root.firstSampler + modelData) + "]"
                        show8Hotcues: root.show8Hotcues
                        showFxAssignments: root.showFxAssignments
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                spacing: 4

                Repeater {
                    model: [2, 3]

                    SamplerFull {
                        required property int modelData

                        Layout.fillWidth: true
                        group: "[Sampler" + (root.firstSampler + modelData) + "]"
                        show8Hotcues: root.show8Hotcues
                        showFxAssignments: root.showFxAssignments
                    }
                }
                Item {
                    Layout.preferredWidth: 20
                }
                Repeater {
                    model: [6, 7]

                    SamplerFull {
                        required property int modelData

                        Layout.fillWidth: true
                        group: "[Sampler" + (root.firstSampler + modelData) + "]"
                        show8Hotcues: root.show8Hotcues
                        showFxAssignments: root.showFxAssignments
                    }
                }
            }
        }
    }
}
