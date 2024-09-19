import QtQuick 2.15

import '../Widgets' as Widgets
import '../Defines' as Defines

//------------------------------------------------------------------------------------------------------------------
//  LIST ITEM - DEFINES THE INFORMATION CONTAINED IN ONE LIST ITEM
//------------------------------------------------------------------------------------------------------------------

// the model contains the following roles:
//  dataType, nodeIconId, nodeName, nrOfSubnodes, coverUrl, artistName, trackName, bpm, key, keyIndex, rating, loadedInDeck, prevPlayed, prelisten

Item {
    id: contactDelegate

    Defines.Settings {id: settings}

    property string masterBPM:		""
    property string masterKey:		""
    property int keyIndex:			0
    property bool isPlaying:			false
    property bool adjacentKeys:		false
    property string newIndex:			keyIndex || ""
    property string masterKeyIndex:		keyIndex
    property color deckColor: qmlBrowser.focusColor
    property color textColor: !ListView.isCurrentItem ? "White" : deckColor
    property bool isCurrentItem: ListView.isCurrentItem
    readonly property int textTopMargin: 5 // centers text vertically
    // readonly property bool isLoaded: (dataType == "Track") ? model.loadedInDeck.length > 0 : false
    readonly property bool isLoaded: false
  // visible: !ListView.isCurrentItem
    readonly property string dataType:		"Track"
    readonly property string artistName:	model.artist
    readonly property string trackName:		model.title
    readonly property string key:		""
    readonly property string bpmText: "---"
    readonly property real bpm: 0

    readonly property string bpmMatch:	 tempoNeeded(masterBPM, bpm).toFixed(2).toString()

    property bool deck1: deckInfo.is1Playing
    property bool deck2: deckInfo.is2Playing
    property bool deck3: deckInfo.is3Playing
    property bool deck4: deckInfo.is4Playing

    readonly property bool deckPlaying: deck1 || deck2 || deck3 || deck4

    function tempoNeeded(master, current) {
        if (master > current) {
            return (1-(current/master))*100;
        }

        return ((master/current)-1)*100;
    }

    readonly property int key1m: 21
    readonly property int key2m: 16
    readonly property int key3m: 23
    readonly property int key4m: 18
    readonly property int key5m: 13
    readonly property int key6m: 20
    readonly property int key7m: 15
    readonly property int key8m: 22
    readonly property int key9m: 17
    readonly property int key10m: 12
    readonly property int key11m: 19
    readonly property int key12m: 14
    readonly property int key1d: 0
    readonly property int key2d: 7
    readonly property int key3d: 2
    readonly property int key4d: 9
    readonly property int key5d: 4
    readonly property int key6d: 11
    readonly property int key7d: 6
    readonly property int key8d: 1
    readonly property int key9d: 8
    readonly property int key10d: 3
    readonly property int key11d: 10
    readonly property int key12d: 5

    function colorKey(newKey,masterKey) {
        if (!contactDelegate.adjacentKeys) {return true}
        else if ((newKey == masterKey)) {return true}
        else if (masterKey == "") {return false}
        else if (masterKey == "21" && (newKey == "14" || newKey == "16" || newKey == "0")) {return true}
        else if (masterKey == "16" && (newKey == "21" || newKey == "23" || newKey == "7")) {return true}
        else if (masterKey == "23" && (newKey == "16" || newKey == "18" || newKey == "2")) {return true}
        else if (masterKey == "18" && (newKey == "23" || newKey == "13" || newKey == "9")) {return true}
        else if (masterKey == "13" && (newKey == "18" || newKey == "20" || newKey == "4")) {return true}
        else if (masterKey == "20" && (newKey == "13" || newKey == "15" || newKey == "11")) {return true}
        else if (masterKey == "15" && (newKey == "20" || newKey == "22" || newKey == "6")) {return true}
        else if (masterKey == "22" && (newKey == "15" || newKey == "17" || newKey == "1")) {return true}
        else if (masterKey == "17" && (newKey == "22" || newKey == "12" || newKey == "8")) {return true}
        else if (masterKey == "12" && (newKey == "17" || newKey == "19" || newKey == "3")) {return true}
        else if (masterKey == "19" && (newKey == "12" || newKey == "14" || newKey == "10")) {return true}
        else if (masterKey == "14" && (newKey == "19" || newKey == "21" || newKey == "5")) {return true}
        else if (masterKey == "0" && (newKey == "5" || newKey == "7" || newKey == "21")) {return true}
        else if (masterKey == "7" && (newKey == "0" || newKey == "2" || newKey == "16")) {return true}
        else if (masterKey == "2" && (newKey == "7" || newKey == "9" || newKey == "23")) {return true}
        else if (masterKey == "9" && (newKey == "2" || newKey == "4" || newKey == "18")) {return true}
        else if (masterKey == "4" && (newKey == "9" || newKey == "11" || newKey == "13")) {return true}
        else if (masterKey == "11" && (newKey == "4" || newKey == "6" || newKey == "20")) {return true}
        else if (masterKey == "6" && (newKey == "11" || newKey == "1" || newKey == "15")) {return true}
        else if (masterKey == "1" && (newKey == "6" || newKey == "8" || newKey == "22")) {return true}
        else if (masterKey == "8" && (newKey == "1" || newKey == "3" || newKey == "17")) {return true}
        else if (masterKey == "3" && (newKey == "8" || newKey == "10" || newKey == "12")) {return true}
        else if (masterKey == "10" && (newKey == "3" || newKey == "5" || newKey == "19")) {return true}
        else if (masterKey == "5" && (newKey == "10" || newKey == "0" || newKey == "14")) {return true}
        else {return false};
    }

    // MappingProperty { id: propShift1;     path: "mapping.state.left.shift" }
    QtObject {
        id: propShift1
        property string description: "Description"
        property var value: 0
    }
  // MappingProperty { id: propShift2;     path: "mapping.state.right.shift" }
    QtObject {
        id: propShift2
        property string description: "Description"
        property var value: 0
    }
    readonly property bool 	isShift: 	 propShift1.value || propShift2.value
    readonly property bool 	isShiftleft: 	 propShift1.value
    readonly property bool 	isShiftRight: 	 propShift2.value

    height: settings.browserFontSize*2
    anchors.left: parent.left
    anchors.right: parent.right

  // container for zebra & track infos
    Rectangle {
    // when changing colors here please remember to change it in the GridView in Templates/Browser.qml
        color: (index%2 == 0) ? colors.colorGrey32 : "Black"
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: settings.showTrackTitleColumn ? 3 : 0
        anchors.rightMargin: 3
        height: parent.height

	// folder name
        Text {
            id: firstFieldFolder
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.topMargin: contactDelegate.textTopMargin
            anchors.leftMargin: 37
            color: textColor
            clip: true
            text: (dataType == "Folder") ? model.nodeName : ""
            font.pixelSize: settings.browserFontSize
            elide: Text.ElideRight
            visible: (dataType != "Track")
            width: 190
        }

        // Image {
        //     id: prepListIcon
        //     visible: (dataType == "Track") ? model.prepared : false
        //     source: "../Images/PrepListIcon" + (!contactDelegate.isCurrentItem ? "White" : "Blue") + ".png"
        //     width: 10
        //     height: 17
        //     // anchors.left: firstFieldText.right
        //     anchors.top: parent.top
        //     anchors.topMargin: 6
        //     anchors.leftMargin: 5
        // }

	// track name
        Text {
            id: firstFieldTrack
            width: settings.swapArtistTitleColumns ? (settings.showArtistColumn ? (settings.hideBPM ? 110 : 77) + (settings.hideKey ? 30 : 0) : 0) + (!settings.showTrackTitleColumn ? 150 : 0) : !settings.showTrackTitleColumn ? 0 : (!settings.showArtistColumn && settings.hideBPM ? 280 : (settings.showArtistColumn ? 150 : 230)) + (!settings.showArtistColumn && settings.hideKey ? 30 : 0)
            visible: (dataType == "Track")
            anchors.top: parent.top
            anchors.topMargin: contactDelegate.textTopMargin
            anchors.left: parent.left
            anchors.leftMargin: model.prepared ? 10 : 0
            elide: Text.ElideRight
            text: settings.swapArtistTitleColumns ? ((dataType == "Track") ? artistName : "") : (settings.showArtistColumn && settings.showTrackTitleColumn ? ((dataType == "Track") ? trackName : "") : (!isShift && !settings.showArtistColumn && settings.showTrackTitleColumn ? ((dataType == "Track") ? trackName : "") : ((dataType == "Track") ? artistName : "")))
            font.pixelSize: settings.browserFontSize
            color: isLoaded ? "lime" : ((model.prevPlayed && !model.prelisten) ? "yellow" : (!bpm ? "red" : textColor))
        }

	// artist name
        Text {
            id: trackTitleField
            anchors.leftMargin: settings.showArtistColumn && settings.showTrackTitleColumn ? 4 : 0
            anchors.left: (dataType == "Track") ? firstFieldTrack.right : firstFieldFolder.right
            anchors.top: parent.top
            anchors.topMargin: contactDelegate.textTopMargin
            width: settings.swapArtistTitleColumns ? !settings.showTrackTitleColumn ? 0 : (!settings.showArtistColumn && settings.hideBPM ? 280 : (settings.showArtistColumn ? 150 : 230)) + (!settings.showArtistColumn && settings.hideKey ? 30 : 0) : (settings.showArtistColumn ? (settings.hideBPM ? 110 : 77) + (settings.hideKey ? 30 : 0) : 0) + (!settings.showTrackTitleColumn ? 150 : 0)
            color: isLoaded ? "lime" : ((model.prevPlayed && !model.prelisten) ? "yellow" : (!bpm ? "red" : textColor))
            clip: true
            text: settings.swapArtistTitleColumns ? (dataType == "Track") ? trackName : "" : (settings.showArtistColumn && settings.showTrackTitleColumn ? ((dataType == "Track") ? artistName : "") : (!isShift && settings.showArtistColumn && !settings.showTrackTitleColumn ? ((dataType == "Track") ? artistName : "") : (dataType == "Track") ? trackName : ""))
            font.pixelSize: settings.browserFontSize
            elide: Text.ElideRight
        }

    // bpm
        Text {
            id: bpmField
            anchors.right: keyField.left
            anchors.top: parent.top
            anchors.topMargin: contactDelegate.textTopMargin
            horizontalAlignment: Text.AlignLeft
            width: settings.hideBPM ? 0 :53
            color: settings.bpmBrowserTextColor ? (bpm == "0.00") ? "red" : (bpmMatch <= settings.browserBpmGreen) && (bpmMatch >= -(settings.browserBpmGreen)) ? "lime" : (!((bpmMatch >= settings.browserBpmRed) || (bpmMatch <= -(settings.browserBpmRed)) && (masterBPM != "0.00")) ? textColor : settings.accentColor) : textColor
            clip: true
            text: (dataType == "Track") ? bpmText : ""
            font.pixelSize: settings.browserFontSize
        }

        function colorForKey(keyIndex) {
            return colors.musicalKeyColors[keyIndex]
        }

    // key
        Text {
            id: keyField
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: contactDelegate.textTopMargin
            anchors.leftMargin: 5

            color: (dataType == "Track") ? (((key == "none") || (key == "None")) ? "White" : ((colorKey(contactDelegate.newIndex, contactDelegate.masterKeyIndex) && contactDelegate.deckPlaying) ? parent.colorForKey(keyIndex) : "White")) : "White"
            width: settings.hideKey ? 0 : 30
            clip: true
            text: (dataType == "Track") ? (((key == "none") || (key == "None")) ? "n.a." : (settings.camelotKey ? utils.camelotConvert(key) : key)) : ""
            font.pixelSize: settings.browserFontSize
        }

        ListHighlight {
            anchors.fill: parent
            visible: contactDelegate.isCurrentItem
            anchors.leftMargin: 0
            anchors.rightMargin: 0
        }

	  // folder icon
        Image {
            id: folderIcon
            source: (dataType == "Folder") ? ("image://icons/" + model.nodeIconId ) : ""
            width: 33
            height: 33
            fillMode: Image.PreserveAspectFit
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 3
            clip: true
            cache: false
            visible: (dataType == "Folder")
        }

	  // ColorOverlay {
		// id: folderIconColorOverlay
		// color: isCurrentItem == false ? colors.colorFontsListBrowser : contactDelegate.deckColor // unselected vs. selected
		// anchors.fill: folderIcon
		// source: folderIcon
	  // }
    }
}
