import QtQuick 2.15

import Mixxx 1.0 as Mixxx

import '../Browser' as BrowserView
import '../Widgets' as Widgets

//----------------------------------------------------------------------------------------------------------------------
//                                            BROWSER VIEW
//
//  The Browser View is connected to traktors QBrowser from which it receives its data model. The navigation through the
//  data is done by calling funcrtions invoked from QBrowser.
//----------------------------------------------------------------------------------------------------------------------

Item {
    id: qmlBrowser
    required property var deckInfo
    property string propertiesPath: ""
    property bool isActive: false
    property bool enterNode: false
    property bool exitNode: false
    property int increment: 0
    property color focusColor: colors.colorDeckBlueBright
    property int speed: 150
    property real sortingKnobValue: 0
    property int pageSize: 10
    property int fastScrollCenter: 3
    property bool leftScreen: deckInfo.isLeftScreen(deckInfo.deckId)

    readonly property int maxItemsOnScreen: 8

  // This is used by the footer to change/display the sorting!
    property alias sortingId: browser.sorting
    property alias sortingDirection: browser.sortingDirection
    property alias isContentList: browser.isContentList

    anchors.fill: parent

    enum WidgetKind {
        None,
        Searchbar,
        Sidebar,
        LibraryView
    }

    Mixxx.ControlProxy {
        id: focusWidget

        group: "[Library]"
        key: "focused_widget"
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectTrackKnob"
        onValueChanged: (value) => {
            console.log("SelectTrackKnob", value)
            if (value != 0) {
                focusWidget.value = BrowserView.WidgetKind.LibraryView;
                moveSelectionVertical(value);
            }
        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectPrevTrack"
        onValueChanged: (value) => {
            console.log("SelectPrevTrack", value)
            if (value != 0) {
                focusWidget.value = BrowserView.WidgetKind.LibraryView;
                moveSelectionVertical(-1);
            }
        }
    }

    Mixxx.ControlProxy {
        group: "[Playlist]"
        key: "SelectNextTrack"
        onValueChanged: (value) => {
            console.log("SelectNextTrack", value)
            if (value != 0) {
                focusWidget.value = BrowserView.WidgetKind.LibraryView;
                moveSelectionVertical(1);
            }
        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveVertical"
        onValueChanged: (value) => {
            console.log("MoveVertical", value, focusWidget.value == BrowserView.WidgetKind.LibraryView)
            // if (value != 0 && focusWidget.value == BrowserView.WidgetKind.LibraryView)
            moveSelectionVertical(value);
        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveUp"
        onValueChanged: (value) => {
            console.log("MoveUp", value)
            if (value != 0 && focusWidget.value == BrowserView.WidgetKind.LibraryView)
                moveSelectionVertical(-1);
        }
    }

    Mixxx.ControlProxy {
        group: "[Library]"
        key: "MoveDown"
        onValueChanged: (value) => {
            console.log("MoveDown", value)
            if (value != 0 && focusWidget.value == BrowserView.WidgetKind.LibraryView)
                moveSelectionVertical(1);
        }
    }

    function moveSelectionVertical(value) {
        if (value == 0)
            return ;

        const rowCount = browser.dataSet.rowCount();
        if (rowCount == 0)
            return ;

        browser.currentIndex = Mixxx.MathUtils.positiveModulo(browser.currentIndex + value, rowCount);
    }

  //--------------------------------------------------------------------------------------------------------------------

    onIncrementChanged: {
        if (qmlBrowser.increment != 0) {
            var newValue = clamp(browser.currentIndex + qmlBrowser.increment, 0, contentList.count - 1);

      // center selection if user is _fast scrolling_ but we're at the _beginning_ or _end_ of the list
            if (qmlBrowser.increment >= pageSize) {
                var centerTop = fastScrollCenter;

                if (browser.currentIndex < centerTop) {
                    newValue = centerTop;
                }
            }
            if (qmlBrowser.increment <= (-pageSize)) {
                var centerBottom = contentList.count - 1 - fastScrollCenter;

                if (browser.currentIndex > centerBottom) {
                    newValue = centerBottom;
                }
            }

            browser.changeCurrentIndex(newValue);
            qmlBrowser.increment = 0;
        }
    }

    onExitNodeChanged: {
        if (qmlBrowser.exitNode) {
            browser.exitNode()
        }

        qmlBrowser.exitNode = false;
    }

  //--------------------------------------------------------------------------------------------------------------------

    onEnterNodeChanged: {
        if (qmlBrowser.enterNode) {
            var movedDown = browser.enterNode(screen.focusDeckId, contentList.currentIndex);
            if (movedDown) {
                browser.relocateCurrentIndex()
            }
        }

        qmlBrowser.enterNode = false;
    }

    function clamp(val, min, max) {
        return Math.max(min, Math.min(val, max));
    }

  // Traktor.Browser
  // {
  //   id: browser;
  //   isActive: qmlBrowser.isActive
  // }
    Item {
        id: browser;
        property bool changeCurrentIndex: false
        property int currentIndex: 0
        property bool currentPath: false
        property var dataSet: Mixxx.Library.model
        property bool enterNode: false
        property bool exitNode: false
        property bool iconId: false
        property bool isContentList: false
        property bool relocateCurrentIndex: false
        property bool sorting: false
        property bool sortingDirection: false
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: "black"
    }

   //--------------------------------------------------------------------------------------------------------------------
  //  LIST VIEW -- NEEDS A MODEL CONTAINING THE LIST OF ITEMS TO SHOW AND A DELEGATE TO DEFINE HOW ONE ITEM LOOKS LIKE
  //-------------------------------------------------------------------------------------------------------------------

  // zebra filling up the rest of the list if smaller than maxItemsOnScreen (= 8 entries)
    Grid {
        anchors.top: contentList.top
        anchors.topMargin: contentList.topMargin + contentList.contentHeight + 1 // +1 = for spacing
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.leftMargin: 3
        columns: 1
        spacing: 1

        Repeater {
            model: (contentList.count < qmlBrowser.maxItemsOnScreen) ? (qmlBrowser.maxItemsOnScreen - contentList.count) : 0
            Rectangle {
                color: ( (contentList.count + index)%2 == 0) ? colors.colorGrey32 : "Black"
                width: qmlBrowser.width;
                height: settings.browserFontSize*2 }
        }
    }

  //--------------------------------------------------------------------------------------------------------------------

    ListView {
        id: contentList
        anchors.fill: parent
        verticalLayoutDirection: ListView.TopToBottom
    // the top/bottom margins are applied only at the beginning/end of the list in order to show half entries while scrolling
    // and keep the list delegates in the same position always.

    // the commented out margins caused browser anchor problems leading to a disappearing browser! check later !?
        anchors.topMargin: 17 // ( (contentList.count < qmlBrowser.maxItemsOnScreen ) || (currentIndex < 4 )) ? 17 : 0
        anchors.bottomMargin: 18 // ( (contentList.count >= qmlBrowser.maxItemsOnScreen) && (currentIndex >= contentList.count - 4)) ? 18 : 0
        clip: false
        spacing: 1
        preferredHighlightBegin: 119 - 17 // -17 because of the reduced height due to the topMargin
        preferredHighlightEnd: 152 - 17 // -17 because of the reduced height due to the topMargin
        highlightRangeMode: ListView.ApplyRange
        highlightMoveDuration: 0
        delegate: BrowserView.ListDelegate {id: listDelegate; masterBPM: deckInfo.masterBPM; masterKey: deckInfo.masterKey; keyIndex: deckInfo.keyIndex; isPlaying: deckInfo.isPlaying; adjacentKeys: settings.adjacentKeys;}
        model: browser.dataSet
        currentIndex: browser.currentIndex
        focus: true
        cacheBuffer: 10
        visible: settings.showBrowserOnFullScreen ? ((deckInfo.isInBrowserMode && leftScreen) || (deckInfo.viewButton && !deckInfo.isInBrowserMode) || deckInfo.favorites) : true
    }

    ListView {
        id: contentListRight
        anchors.fill: parent
        verticalLayoutDirection: ListView.TopToBottom
    // the top/bottom margins are applied only at the beginning/end of the list in order to show half entries while scrolling
    // and keep the list delegates in the same position always.

    // the commented out margins caused browser anchor problems leading to a disappearing browser! check later !?
        anchors.topMargin: 0 // ( (contentList.count < qmlBrowser.maxItemsOnScreen ) || (currentIndex < 4 )) ? 17 : 0
        anchors.bottomMargin: 0 // ( (contentList.count >= qmlBrowser.maxItemsOnScreen) && (currentIndex >= contentList.count - 4)) ? 18 : 0
        clip: false
        spacing: 0
        preferredHighlightBegin: 0 // -17 because of the reduced height due to the topMargin
        preferredHighlightEnd: 240 // -17 because of the reduced height due to the topMargin
        highlightRangeMode: ListView.ApplyRange
        highlightMoveDuration: 0
        delegate: BrowserView.TrackView {id: trackView; masterBPM: deckInfo.masterBPM;}
        model: browser.dataSet
        currentIndex: browser.currentIndex
        focus: true
        cacheBuffer: 10
        visible: settings.showBrowserOnFullScreen ? (deckInfo.isInBrowserMode && !leftScreen) : false
    }

    BrowserView.BrowserHeader {
        id: browserHeader
        nodeIconId: browser.iconId
        currentDeck: deckInfo.deckId
        state: "show"
        pathStrings: browser.currentPath

        Behavior on height { NumberAnimation { duration: speed; } }

        visible: settings.showBrowserOnFullScreen ? !(deckInfo.isInBrowserMode && !leftScreen) : true
    }

  //--------------------------------------------------------------------------------------------------------------------

    BrowserView.BrowserFooter {
        id: browserFooter
        state: "show"
        propertiesPath: qmlBrowser.propertiesPath
        sortingKnobValue: qmlBrowser.sortingKnobValue
        maxCount:		contentList.count
        count:			browser.currentIndex + 1
        deckInfo: qmlBrowser.deckInfo

        Behavior on height { NumberAnimation { duration: speed; } }

        visible: settings.showBrowserOnFullScreen ? !(deckInfo.isInBrowserMode && !leftScreen) : true
    }

    BrowserView.TrackFooter {
        id: trackFooter
        state: "show"
        propertiesPath: qmlBrowser.propertiesPath
        sortingKnobValue: qmlBrowser.sortingKnobValue
        maxCount:		contentList.count
        count:			browser.currentIndex + 1
        deckInfo: qmlBrowser.deckInfo

        Behavior on height { NumberAnimation { duration: speed; } }

        visible: settings.showBrowserOnFullScreen ? (deckInfo.isInBrowserMode && !leftScreen) : false
    }
}
