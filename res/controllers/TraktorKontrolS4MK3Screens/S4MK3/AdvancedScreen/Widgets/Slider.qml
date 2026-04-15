import QtQuick 2.5

Item {
    id: item
    property color backgroundColor: "grey"
    property color sliderColor: "red"
    property color cursorColor: "white"
    property color centerColor: "black"

    property real min: 0
    property real max: 1
    property real value: 0.5
    property real radius: 0
    property real cursorWidth: 5
    property bool centered: false

    Item {
        id: toBeMasked_noCenter
        anchors.fill: parent

        property real cursorPosition: (parent.width - item.cursorWidth) * ( item.value / (item.max-item.min) )

    //background
        Rectangle {
            anchors.fill: parent
            color: item.backgroundColor
        }

    //colored part of the slider
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            width: toBeMasked_noCenter.cursorPosition
            height: parent.height

            color: item.sliderColor
        }

    //cursor
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: toBeMasked_noCenter.cursorPosition
            width: item.cursorWidth
            height: parent.height
            color: item.cursorColor
        }

        visible: false
    }

    Item {
        id: toBeMasked_centered
        anchors.fill: parent
        property real x0: (parent.width - item.cursorWidth)/2
        property real cursorPosition_left: Math.min( toBeMasked_noCenter.cursorPosition, x0)
        property real cursorPosition_right: Math.max( toBeMasked_noCenter.cursorPosition, x0)

    //cursor background
        Rectangle {
            id: background
            anchors.fill: parent
            color: item.backgroundColor
        }

    //filled slider
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: toBeMasked_centered.cursorPosition_left
            width: toBeMasked_centered.cursorPosition_right - toBeMasked_centered.cursorPosition_left
            height: parent.height
            color: item.sliderColor
        }

    //center
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: toBeMasked_centered.x0
            width: item.cursorWidth
            height: parent.height
            color: item.centerColor
        }

    //cursor
        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: toBeMasked_noCenter.cursorPosition
            width: item.cursorWidth
            height: parent.height
            color: item.cursorColor
        }

        visible: false
    }

    Rectangle {
        id: mask_noCenter
        anchors.fill: parent
        radius: item.radius
        visible: false
    }

  // OpacityMask {
  //     anchors.fill: parent
  //     maskSource: mask_noCenter
  //     source: item.centered ? toBeMasked_centered : toBeMasked_noCenter
  // }
}
