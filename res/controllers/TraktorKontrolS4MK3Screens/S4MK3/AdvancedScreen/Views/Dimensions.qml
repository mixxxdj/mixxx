import QtQuick 2.15

QtObject {

    readonly property real infoBoxesWidth: 150
    readonly property real firstRowHeight: 33
    readonly property real secondRowHeight: 72
    readonly property real thirdRowHeight: 72
    readonly property real spacing: 6
    readonly property real largeBoxWidth: 2*infoBoxesWidth + spacing
    readonly property real cornerRadius: 5
    readonly property real screenTopMargin: 3 // might need to be adapted based on the tolerances of hardware manufacturing
    readonly property real screenLeftMargin: spacing // might need to be adapted based on the tolerances of hardware manufacturing
    readonly property real titleTextMargin: spacing
}
