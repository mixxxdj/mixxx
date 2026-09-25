import QtQuick 2.15

import Mixxx 1.0 as Mixxx

import '../Defines' as Defines
import '../Defines' as Defines
import '../ViewModels' as ViewModels

//------------------------------------------------------------------------------------------------------------------
//  LIST ITEM - DEFINES THE INFORMATION CONTAINED IN ONE LIST ITEM
//------------------------------------------------------------------------------------------------------------------
Rectangle {
    id: footer

    Defines.Colors { id: colors }

    required property var deckInfo

    property string propertiesPath: ""
    property real sortingKnobValue: 0.0
    property bool isContentList: qmlBrowser.isContentList
    property int maxCount:		 0
    property int count: 		 0

  // the given numbers are determined by the EContentListColumns in Traktor
    readonly property variant sortIds: [0 ,1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30]
    readonly property variant sortNames: ["Sort By #", "Sort By #", "Title", "Artist", "Time", "BPM", "Track #", "Label", "Release", "Label", "Key Text", "Comment", "Lyrics", "Comment 2", "Path", "Analysed", "Remixer", "Producer", "Mix", "CAT #", "Rel. Date", "Bitrate", "Rating", "Count", "Sort By #", "Cover Art", "Last Played", "Import Date", "Key", "Color", "File Name"]
    readonly property int selectedFooterId: (selectedFooterItem.value === undefined) ? 0 : ( ( selectedFooterItem.value % 2 === 1 ) ? 1 : 4 ) // selectedFooterItem.value takes values from 1 to 4.

    property real preSortingKnobValue: 0.0

  //--------------------------------------------------------------------------------------------------------------------

  // AppProperty { id: previewIsLoaded;     path : "app.traktor.browser.preview_player.is_loaded" }
    QtObject {
        id: previewIsLoaded
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: previewTrackLenght;  path : "app.traktor.browser.preview_content.track_length" }
    QtObject {
        id: previewTrackLenght
        property string description: "Description"
        property var value: 0
    }
  // AppProperty { id: previewTrackElapsed; path : "app.traktor.browser.preview_player.elapsed_time" }
    QtObject {
        id: previewTrackElapsed
        property string description: "Description"
        property var value: 0
    }

  // MappingProperty { id: overlayState;      path: propertiesPath + ".overlay" }
    QtObject {
        id: overlayState
        property string description: "Description"
        property var value: 0
    }
  // MappingProperty { id: isContentListProp; path: propertiesPath + ".browser.is_content_list" }
    QtObject {
        id: isContentListProp
        property string description: "Description"
        property var value: 0
    }
  // MappingProperty { id: selectedFooterItem;      path: propertiesPath + ".selected_footer_item" }
    QtObject {
        id: selectedFooterItem
        property string description: "Description"
        property var value: 0
    }

  //--------------------------------------------------------------------------------------------------------------------
  // Behavior on Sorting Changes (show/hide sorting widget, select next allowed sorting)
  //--------------------------------------------------------------------------------------------------------------------

    onIsContentListChanged: {
    // We need this to be able do disable mappings (e.g. sorting ascend/descend)
        isContentListProp.value = isContentList;
    }

    onSortingKnobValueChanged: {
        if (!footer.isContentList)
            return;

        overlayState.value = Overlay.sorting;
        sortingOverlayTimer.restart();

        var val = clamp(footer.sortingKnobValue - footer.preSortingKnobValue, -1, 1);
        val     = parseInt(val);
        if (val != 0) {
            qmlBrowser.sortingId   = getSortingIdWithDelta( val );
            footer.preSortingKnobValue = footer.sortingKnobValue;
        }
    }

    Timer {
        id: sortingOverlayTimer
        interval: 800 // duration of the scrollbar opacity
        repeat: false

        onTriggered: overlayState.value = Overlay.none;
    }

  //--------------------------------------------------------------------------------------------------------------------
  // View
  //--------------------------------------------------------------------------------------------------------------------

    clip: false
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    height: 36 + (settings.raiseBrowserFooter ? 4 : 0) // set in state
    color: "transparent"

  // background color
    Rectangle {
        id: browserFooterBg
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 30 + (settings.raiseBrowserFooter ? 4 : 0)
        color: colors.colorBrowserHeader // footer background color
    }

    Row {
        id: sortingRow
        anchors.left: browserFooterBg.left
        anchors.leftMargin: 1
        anchors.top: browserFooterBg.top

        Item {
            width: 100
            height: 30 + (settings.raiseBrowserFooter ? 4 : 0)

            Text {
                font.pixelSize: fonts.scale(15)
                anchors.left: parent.left
                anchors.leftMargin: 3
                anchors.top: parent.top
                anchors.topMargin: 3
                font.capitalization: Font.AllUppercase
                color: selectedFooterId == 1 ? "white" : colors.colorFontBrowserHeader
                text: getSortingNameForSortId(qmlBrowser.sortingId)
                visible: qmlBrowser.isContentList
            }
      // Arrow (Sorting Direction Indicator)
            Triangle {
                id: sortDirArrow
                width: 15
                height: 15
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 4
                anchors.rightMargin: 6
                antialiasing: false
                visible: qmlBrowser.sortingId > 0
                color: colors.colorGrey80
                rotation: ((qmlBrowser.sortingDirection == 1) ? 0 : 180)
            }
            Rectangle {
                id: divider
                height: 30
                width: 1
                color: colors.colorGrey40 // footer divider color
                anchors.right: parent.right
            }
        }

    // Preview Player footer
        Item {
            width: 150
            height: 30

            Text {
                font.pixelSize: fonts.scale(18)
                anchors.left: parent.left
                anchors.leftMargin: 1
                anchors.top: parent.top
                anchors.topMargin: 3
                font.capitalization: Font.AllUppercase
                visible: !previewIsLoaded.value
                color: selectedFooterId == 4 ? "white" : "green"
                text: deckInfo.masterDeckLetter
            }

            Text {
                font.pixelSize: fonts.scale(18)
                anchors.left: parent.left
                anchors.leftMargin: 15
                anchors.top: parent.top
                anchors.topMargin: 3
                font.capitalization: Font.AllUppercase
                visible: !previewIsLoaded.value
                color: selectedFooterId == 4 ? "white" : colors.colorFontBrowserHeader
                text: deckInfo.masterBPMFooter2
            }

            Text {
                font.pixelSize: fonts.scale(18)
                anchors.right: parent.right
                anchors.rightMargin: 2
                anchors.top: parent.top
                anchors.topMargin: 3
                font.capitalization: Font.AllUppercase
                visible: !previewIsLoaded.value
                color: selectedFooterId == 4 ? "white" : colors.musicalKeyColorsDark[deckInfo.masterKeyIndex]
                text: settings.camelotKey ? utils.camelotConvert(deckInfo.masterKey) : deckInfo.masterKey
            }

            Text {
                font.pixelSize: fonts.scale(16)
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.top: parent.top
                anchors.topMargin: 3
                font.capitalization: Font.AllUppercase
                visible: previewIsLoaded.value
                color: selectedFooterId == 4 ? "white" : colors.colorFontBrowserHeader
                text: "Preview"
            }

            // Image {
            //     anchors.top: parent.top
            //     anchors.right: parent.right
            //     anchors.topMargin: 3
            //     anchors.rightMargin: 49
            //     visible: previewIsLoaded.value
            //     antialiasing: false
            //     source: "../Images/PreviewIcon_Small.png"
            //     fillMode: Image.Pad
            //     clip: true
            //     cache: false
            //     width: 20
            //     height: 20
            // }
            Text {
                width: 40
                clip: true
                horizontalAlignment: Text.AlignRight
                visible: previewIsLoaded.value
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.topMargin: 2
                anchors.rightMargin: 7
                font.pixelSize: fonts.scale(18)
                font.capitalization: Font.AllUppercase
                font.family: "Pragmatica"
                color: colors.browser.prelisten
                text: utils.convertToTimeString(previewTrackElapsed.value)
            }
            Rectangle {
                id: divider2
                height: 30
                width: 1
                color: colors.colorGrey40 // footer divider color
                anchors.right: parent.right
            }
        }

        Item {

            width: 150
            height: 30

            Text {
                Text {
                    font.pixelSize: fonts.scale(20)
                    anchors.left: parent.right
                    anchors.leftMargin: 3
                    anchors.top: parent.top
                    anchors.topMargin: 3
                    font.capitalization: Font.AllUppercase
                    visible: true
                    color: colors.colorFontBrowserHeader
                    text: count
                }
            }
        }
    }

  //--------------------------------------------------------------------------------------------------------------------
  // black border & shadow
  //--------------------------------------------------------------------------------------------------------------------

    Rectangle {
        id: browserHeaderBottomGradient
        height: 3
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: browserHeaderBlackBottomLine.top
        gradient: Gradient {
            GradientStop { position: 0.0; color: colors.colorBlack0 }
            GradientStop { position: 1.0; color: colors.colorBlack38 }
        }
    }

    Rectangle {
        id: browserHeaderBlackBottomLine
        height: 2
        color: colors.colorBlack
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: browserFooterBg.top
    }

  //------------------------------------------------------------------------------------------------------------------

    state: "show"
    states: [
        State {
            name: "show"
            PropertyChanges { target: footer; height: 36 + (settings.raiseBrowserFooter ? 4 : 0) }
        },
        State {
            name: "hide"
            PropertyChanges { target: footer; height: 0 }
        }
    ]

    //--------------------------------------------------------------------------------------------------------------------
    // Necessary Functions
    //--------------------------------------------------------------------------------------------------------------------

    function getSortingIdWithDelta( delta ) {
        var curPos = getPosForSortId( qmlBrowser.sortingId );
        var pos    = curPos + delta;
        var count  = sortIds.length;

        pos = (pos < 0) ? count-1 : pos;
        pos = (pos >= count) ? 0 : pos;

        return sortIds[pos];
    }

    function getPosForSortId(id) {
        if (id == -1) return 0; // -1 is a special case which should be interpreted as "0"
        for (var i=0; i<sortIds.length; i++) {
            if (sortIds[i] == id) return i;
        }
        return -1;
    }

    function getSortingNameForSortId(id) {
        var pos = getPosForSortId(id);
        if (pos >= 0 && pos < sortNames.length)
            return sortNames[pos];
        return "SORTED";
    }

    function clamp(val, min, max) {
        return Math.max( Math.min(val, max) , min );
    }
}
