pragma ComponentBehavior: Bound

import Mixxx 1.0 as Mixxx
import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property int count
    required property string expandKey
    readonly property bool expanded: expandControl.value > 0.5
    readonly property bool expandedContentReady: expandedContent.status === Loader.Ready
    readonly property int expandedHeight: count === 4 ? 98 : 200
    required property int firstSampler
    property bool preloadExpandedContent: true
    property bool show8Hotcues: true
    property bool showFxAssignments: true
    readonly property bool showingExpandedContent: expanded && expandedContentReady

    implicitHeight: showingExpandedContent ? expandedHeight : 40

    Loader {
        id: minimizedContent

        height: 40
        sourceComponent: minimizedRow
        visible: !root.showingExpandedContent
        width: root.width
    }
    Loader {
        id: expandedContent

        active: root.expanded || root.preloadExpandedContent
        asynchronous: !root.expanded
        height: root.expandedHeight
        sourceComponent: root.count === 4 ? expandedFour : expandedEight
        visible: root.showingExpandedContent
        width: root.width
    }
    Mixxx.ControlProxy {
        id: expandControl

        group: "[Skin]"
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
                Layout.fillHeight: true
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
                    waveformsActive: root.expanded
                }
            }
            SamplerExpandButton {
                Layout.fillHeight: true
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
                    waveformsActive: root.expanded
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
                        waveformsActive: root.expanded
                    }
                }
                SamplerExpandButton {
                    Layout.fillHeight: true
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
                        waveformsActive: root.expanded
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
                        waveformsActive: root.expanded
                    }
                }
                SamplerGutter {
                    Layout.fillHeight: true
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
                        waveformsActive: root.expanded
                    }
                }
            }
        }
    }
}
