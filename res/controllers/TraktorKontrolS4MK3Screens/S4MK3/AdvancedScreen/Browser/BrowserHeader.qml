import QtQuick 2.15

import Mixxx 1.0 as Mixxx

import '../Defines' as Defines
import '../ViewModels' as ViewModels

//------------------------------------------------------------------------------------------------------------------
// BROWSER HEADER - SHOWS THE CURRENT BROWSER PATH
//------------------------------------------------------------------------------------------------------------------
Item {
    id: header

    Defines.Colors { id: colors }

    property int currentDeck: 0
    property int nodeIconId: 0

    readonly property color itemColor: colors.colorWhite19
    property int highlightIndex: 0

    readonly property var letters: ["","A", "B", "C", "D"]

    property string pathStrings: "" // the complete path in one string given by QBrowser with separator " | "
    property var stringList: [""] // list of separated path elements (calculated in "updateStringList")
    property int stringListModelSize: 0 // nr of entries which can be displayed in the header ( calc in updateStringList)
    readonly property int maxTextWidth: 150 // if a single text path block is bigger than this: ElideMiddle
    readonly property int arrowContainerWidth: 18 // width of the graphical separator arrow. includes left / right spacing
    readonly property int fontSize: 13

    clip: true
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.top: parent.top
    height: 17 // set in state

    onPathStringsChanged: { updateStringList(textLengthDummy) }

  //--------------------------------------------------------------------------------------------------------------------
  // NOTE: text item used within the 'updateStringList' function to determine how many of the stringList items can be fit
  //       in the header!
  // IMPORTANT EXTRA NOTE: all texts in the header should have the same Capitalization and font size settings as the "dummy"
  //                       as the dummy is used to calculate the number of text blocks fitting into the header.
  //--------------------------------------------------------------------------------------------------------------------
    Text {
        id: textLengthDummy
        visible: false
        font.capitalization: Font.AllUppercase
        font.pixelSize: header.fontSize
    }

  // calculates the number of entries to be displayed in the header
    function updateStringList(dummy) {
        var sum   = 0
        var count = 0

        stringList = pathStrings.split(" | ")

        for (var i = 0; i < stringList.length; ++i) {
            dummy.text = header.stringList[stringList.length - i - 1]

            sum += (dummy.width) > maxTextWidth ? header.maxTextWidth : dummy.width
            sum += arrowContainerWidth

            if (sum > (textContainter.width - header.arrowContainerWidth)) {
                header.stringListModelSize = count
                return
            }
            count++
            }
        header.stringListModelSize = stringList.length;
    }

  //--------------------------------------------------------------------------------------------------------------------
  // background color
    Rectangle {
        id: browserHeaderBg
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 17
        color: colors.colorBrowserHeader //colors.colorGrey24
    }

  //--------------------------------------------------------------------------------------------------------------------

    Item {
        id: textContainter
        readonly property int spaceToDeckLetter: 20
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: deckLetter.left
        anchors.leftMargin: 3
        anchors.rightMargin: spaceToDeckLetter
        clip: true

    // dots appear at the left side of the browser in case the full path does not fit into the header anymore.
        Item {
            id: dots
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.leftMargin: (stringListModelSize < stringList.length) ? 0 : -width
            visible: (stringListModelSize < stringList.length)
            width: 30

            Text {
                anchors.left: parent.left
                anchors.top: parent.top
                text: "..."
                font.capitalization: Font.AllUppercase
                font.pixelSize: header.fontSize
                color: colors.colorFontBrowserHeader
            }
        }

    // the text flow
        Flow {
            id: textFlow
            layoutDirection: Qt.RightToLeft

            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.left: dots.right

            Repeater {
                model: stringListModelSize
                Item {
                    id: textContainer
                    property string displayTxt: (stringList[stringList.length - index - 1] == undefined) ? "" : stringList[stringList.length - index - 1]

                    width: headerPath.width + arrowContainerWidth
                    height: 20

          // arrows
          // the graphical separator between texts anchors on the left side of each text block. The space of "arrowContainerWidth" is reserved for that
          // Widgets.TextSeparatorArrow {
          //   color:               colors.colorGrey80
          //   visible:             true
          //   anchors.top:         parent.top
          //   anchors.right:       headerPath.left
          //   anchors.topMargin:   4
          //   anchors.rightMargin: 6 // left margin is set via "arrowContainerWidth"
          // }

                    Text {
                        id: dummy
            // NOTE: dummyTextPath is only used to get the displayWidth of the strings. (otherwise dynamic text sizes are hard/impossible)
                        text: displayTxt
                        visible: false
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: header.fontSize
                    }

                    Text {
                        id: headerPath
            // dummy.width is determined by the string contained in it and ceil to whole pixels (ceil instead of round to avoid unwanted elides)
                        width: (dummy.width > maxTextWidth) ? maxTextWidth : Math.ceil(dummy.width )
                        elide: Text.ElideMiddle
                        text: displayTxt
                        visible: true
                        color: (index == 0) ? colors.colorDeckBlueBright : colors.colorGrey88
                        font.capitalization: Font.AllUppercase
                        font.pixelSize: header.fontSize
                    }
                }
            }
        }
    }

  //--------------------------------------------------------------------------------------------------------------------

    Text {
        id: deckLetter
        anchors.right: parent.right
        anchors.top: parent.top
        height: parent.height
        width: parent.height

        text: header.letters[header.currentDeck]
        font.capitalization: Font.AllUppercase
        font.pixelSize: header.fontSize
        color: colors.colorDeckBlueBright
    }

  //--------------------------------------------------------------------------------------------------------------------
  // black border & shadow

    Rectangle {
        id: browserHeaderBlackBottomLine
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: browserHeaderBg.bottom
        height: 2
        color: colors.colorBlack
    }

    Rectangle {
        id: browserHeaderBottomGradient
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: browserHeaderBlackBottomLine.bottom
        height: 3
        gradient: Gradient {
            GradientStop { position: 0.0; color: colors.colorBlack38 }
            GradientStop { position: 1.0; color: colors.colorBlack0 }
        }
    }

 //--------------------------------------------------------------------------------------------------------------------

    state: "show"
    states: [
        State {
            name: "show"
            PropertyChanges {target: header; height: 15}
        },
        State {
            name: "hide"
            PropertyChanges {target: header; height: 0}
        }
    ]
}
