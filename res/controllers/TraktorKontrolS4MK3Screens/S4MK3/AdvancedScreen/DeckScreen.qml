import QtQuick 2.15
import './Defines' as Dfeines
import './Views' as Views
import './ViewModels' as ViewModels
import './Overlays' as Overlays

Item {
    id: deckscreen

    property int deckId: 1

    property bool active: true

  //--------------------------------------------------------------------------------------------------------------------
  // Deck Screen: show information for track, stem, remix decks
  //--------------------------------------------------------------------------------------------------------------------
    QtObject {
        id: deckType
        property string description: deckInfoModel.isStemsActive ? "Stem Deck" : "Track Deck"
        property var value: 1
    }

    QtObject {
        id: propShift1
        property var value: false
    }
    QtObject {
        id: propShift2
        property var value: false
    }
    readonly property bool 	isShift: 	 propShift1.value || propShift2.value

    property bool browser: settings.showBrowserOnFavorites ? ((deckInfoModel.viewButton) || (deckInfoModel.favorites)) : (deckInfoModel.viewButton)

    Component.onCompleted: {
        if (typeof engine.makeSharedDataConnection === "function") {
            engine.makeSharedDataConnection(deckscreen.onSharedDataUpdate)
            deckscreen.onSharedDataUpdate(engine.getSharedData())
        }
    }

    function isLeftScreen(deckId) {
        return deckId == 1 || deckId == 3;
    }

    function onSharedDataUpdate(data) {
        if (typeof data === "object" && typeof data.shift === "object") {
            propShift1.value = !!data.shift["leftdeck"]
            propShift2.value = !!data.shift["rightdeck"]
        }
    }

    ViewModels.DeckInfo {
        id: deckInfoModel
        deckId: deckscreen.deckId
    }

    Component {
        id: emptyDeckComponent;

        Views.EmptyDeck {
            anchors.fill: parent
            deckInfo: deckInfoModel
        }
    }

    Component {
        id: trackDeckComponent;
        Views.TrackDeck {
            id: trackDeck
            deckInfo: deckInfoModel
            deckId: deckscreen.deckId
            anchors.fill: parent
        }
    }

    Component {
        id: browserComponent;
        Views.BrowserView {
            id: browserView
            deckInfo: deckInfoModel
            anchors.fill: parent
            isActive: (loader.sourceComponent == browserComponent) && deckscreen.active
        }
    }

    Component {
        id: stemDeckComponent;

        Views.StemDeck {
            deckInfo: deckInfoModel
            anchors.fill: parent
        }
    }

    Loader {
        id: loader
        active: true
        visible: true
        anchors.fill: parent
        sourceComponent: trackDeckComponent
    }

    Item {
        id: content
        state: "Empty Deck"

        Component.onCompleted: {
            content.state = Qt.binding(function() {
                    return (browser && settings.enableBrowserMode) ? "Browser" : deckType.description });
        }

        states: [
            State {
                name: "Empty Deck"
                PropertyChanges { target: loader; sourceComponent: emptyDeckComponent }
            },
            State {
                name: "Track Deck"
                PropertyChanges { target: loader; sourceComponent: trackDeckComponent }
            },
            State {
                name: "Browser"
                PropertyChanges { target: loader; sourceComponent: browserComponent }
            },
            State {
                name: "Stem Deck"
                PropertyChanges { target: loader; sourceComponent: stemDeckComponent }
            }
        ]
    }

    Overlays.GridControls {
        id: grid
        deckId: deckInfoModel.deckId
        showHideState: !settings.hideGridOverlay && deckInfoModel.adjustEnabled && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.BankInfo {
        id: bank1;
        bank: 1
        showHideState: deckInfoModel.padsModeBank1 && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.BankInfo {
        id: bank2;
        bank: 2
        showHideState: deckInfoModel.padsModeBank2 && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.CueInfo {
        id: cue
        hotcue: deckInfoModel.hotcueId
        type: deckInfoModel.hotcueType
        name: deckInfoModel.hotcueName
        showHideState: !settings.hideHotcueOverlay && deckInfoModel.hotcueDisplay && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.JumpControls {
        id: jump;
        deckInfo: deckInfoModel
        showHideState: !settings.hideJumpOverlay && deckInfoModel.padsModeJump && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.LoopControls {
        id: loop;
        deckInfo: deckInfoModel
        deckId: deckInfoModel.deckId
        showHideState: !settings.hideLoopOverlay && deckInfoModel.padsModeLoop && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.RollControls {
        id: roll;
        deckInfo: deckInfoModel
        deckId: deckInfoModel.deckId
        showHideState: !settings.hideRollOverlay && deckInfoModel.padsModeRoll && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }

    Overlays.ToneControls {
        id: tone;
        deckId: deckInfoModel.deckId
        adjustVal: deckInfoModel.keyAdjustVal
        showHideState: !settings.hideToneOverlay && deckInfoModel.padsModeTone && !(loader.sourceComponent == browserComponent) ? "show" : "hide"
    }
}
