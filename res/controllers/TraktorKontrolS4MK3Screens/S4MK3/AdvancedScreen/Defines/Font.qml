import QtQuick 2.15

QtObject {

// currently mapped to unity but you can use to bulk scale fonsize if needed
    function scale(fontSize) { return fontSize; }

// Font Size Variables
    readonly property int miniFontSize: 			scale(10)
    readonly property int smallFontSize: 			scale(12)
    readonly property int middleFontSize: 			scale(15)
    readonly property int largeFontSize: 			scale(18)
    readonly property int largeValueFontSize: 	scale(21)
    readonly property int moreLargeValueFontSize: 	scale(33)
    readonly property int extraLargeValueFontSize: scale(45)
    readonly property int superLargeValueFontSize: scale(55)
}
