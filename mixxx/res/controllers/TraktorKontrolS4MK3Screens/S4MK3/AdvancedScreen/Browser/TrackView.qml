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
    property color deckColor: qmlBrowser.focusColor
    property color textColor: !ListView.isCurrentItem ? "White" : deckColor
    property bool isCurrentItem: ListView.isCurrentItem
    readonly property int textTopMargin: 5 // centers text vertically
    readonly property bool isLoaded: (model.dataType == "Track") ? model.loadedInDeck.length > 0 : false
    readonly property int rating: 				(model.dataType == "Track") ? ((model.rating == "") ? 0 : model.rating ) : 0
  // visible: !ListView.isCurrentItem
    readonly property string bpm: (model.bpm || 0).toFixed(2).toString()

    readonly property string bpmMatch:	 tempoNeeded(masterBPM, bpm).toFixed(2).toString()

    function tempoNeeded(master, current) {
        if (master > current) {

            return (1-(current/master))*100;

        } else if (master < current) {

            return ((master/current)-1)*100;
        }
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

    height: 240
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

	// track name
        Text {
            id: firstFieldTrack
            width: 300
            clip: true
            anchors.top: trackImage.bottom
            anchors.topMargin: contactDelegate.textTopMargin
            anchors.left: parent.left
            anchors.leftMargin: 5
            text: (model.dataType == "Track") ? model.trackName : ""
            font.pixelSize: 20
            color: isLoaded ? "lime" : ((model.prevPlayed && !model.prelisten) ? "yellow" : (((model.bpm || 0).toFixed(4) == "0.0000" ) ? "red" : textColor))
        }

	// artist name
        Text {
            id: trackTitleField
            anchors.top: firstFieldTrack.bottom
            anchors.topMargin: contactDelegate.textTopMargin
            anchors.left: parent.left
            anchors.leftMargin: 5
            width: 300
            color: isLoaded ? "lime" : ((model.prevPlayed && !model.prelisten) ? "yellow" : (((model.bpm || 0).toFixed(4) == "0.0000" ) ? "red" : textColor))
            clip: true
            text: (model.dataType == "Track") ? model.artistName : ""
            font.pixelSize: 20
        }

    //bpm text
        Text {
            id: bpmFieldText
            anchors.top: parent.top
            anchors.left: trackImage.right
            anchors.leftMargin: 5
            anchors.topMargin: 5
            horizontalAlignment: Text.AlignLeft

            color: "white"
            text: "BPM:"
            font.pixelSize: 28
        }

    //bpm
        Text {
            id: bpmField
            anchors.top: parent.top
            anchors.rightMargin: 0
            anchors.right: parent.right
            anchors.topMargin: 5
            horizontalAlignment: Text.AlignRight

            color: settings.bpmBrowserTextColor ? (bpm == "0.00") ? "red" : (bpmMatch <= settings.browserBpmGreen) && (bpmMatch >= -(settings.browserBpmGreen)) ? "lime" : (!((bpmMatch >= settings.browserBpmRed) || (bpmMatch <= -(settings.browserBpmRed)) && (masterBPM != "0.00")) ? textColor : settings.accentColor) : textColor
            clip: true
            text: (model.dataType == "Track") ? bpm : ""
            font.pixelSize: 30
        }

        function colorForKey(keyIndex) {
            return colors.musicalKeyColors[keyIndex]
        }

    // key text
        Text {
            id: keyFieldText
            anchors.top: bpmField.bottom
            anchors.left: trackImage.right
            anchors.topMargin: 8
            anchors.leftMargin: 5

            color: "white"
            clip: true
            text: "Key:"
            font.pixelSize: 30
        }

    // key
        Text {
            id: keyField
            anchors.top: bpmField.bottom
            anchors.right: parent.right
            anchors.topMargin: 8
            anchors.rightMargin: 0

            color: (model.dataType == "Track") ? (((model.key == "none") || (model.key == "None")) ? textColor : parent.colorForKey(model.keyIndex)) : textColor
            clip: true
            text: (model.dataType == "Track") ? (((model.key == "none") || (model.key == "None")) ? "n.a." : (settings.camelotKey ? utils.camelotConvert(model.key) : model.key)) : ""
            font.pixelSize: 30
        }

        Widgets.TrackRating {
            id: trackRating

            anchors.top: keyFieldText.bottom
            anchors.left: trackImage.right
            anchors.topMargin: 8
            anchors.leftMargin: 5
            rating: (model.dataType == "Track") ? ((model.rating == "") ? 0 : model.rating ) : 0
        }

        ListHighlight {
            anchors.fill: parent
            visible: contactDelegate.isCurrentItem
            anchors.leftMargin: 0
            anchors.rightMargin: 0
        }

        Rectangle {
            id: trackImage
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: 5
            anchors.topMargin: 10
            width: 125
            height: 125
            color: (model.coverUrl != "") ? "transparent" : ((contactDelegate.screenFocus < 2) ? colors.colorDeckBlueBright50Full : colors.colorGrey128 )
            visible: (model.dataType == "Track") && !settings.hideAlbumArt

            Image {
                id: cover
                anchors.fill: parent
                source: (model.dataType == "Track") ? ("image://covers/" + model.coverUrl ) : ""
                fillMode: Image.PreserveAspectFit
                clip: true
                cache: false
                sourceSize.width: width
                sourceSize.height: height
          // the image either provides the cover of the track, or if not available the traktor logo on colored background ( opacity == 0.3)
                opacity: (model.coverUrl != "") ? 1.0 : 0.3
            }
        }
    }
}
