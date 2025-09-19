import QtQuick 2.5
import '../Defines' as Defines

import Mixxx 1.0 as Mixxx

//----------------------------------------------------------------------------------------------------------------------
//  Track Deck Model - provide data for the track deck view
//----------------------------------------------------------------------------------------------------------------------

Item {
    id: viewModel

    property string group: `[Channel${viewModel.deckId}]`
    readonly property var deckPlayer: Mixxx.PlayerManager.getPlayer(viewModel.group)
    readonly property var currentPlayer: viewModel.deckPlayer?.currentTrack
    readonly property string screenName: isLeftScreen(viewModel.deckId) ? "leftdeck" : "rightdeck"

    function onSharedDataUpdate(data) {
        if (typeof data !== "object") {
            return;
        }
        if (typeof data.group[screenName] === "string") {
            viewModel.group = data.group[screenName]
            console.log(`Changed group for screen ${screenName} to ${viewModel.group}`);
        }
        if (typeof data.shift === "object") {
            propShift.value = !!data.shift[screenName]
        }
        if (typeof data.padsMode === "object") {
            propPadsMode.value = data.padsMode[viewModel.group]
            console.log(`Changed padsMode for screen ${screenName} to ${propPadsMode.value}`);
        }
        if (typeof data.selectedQuickFX !== "undefined") {
            propSelectedQuickFX.value = data.selectedQuickFX
            console.log(`Changed selectedQuickFX to ${propSelectedQuickFX.value}`);
        }
        if (typeof data.selectedStems === "object") {
            let firstSelected = (data.selectedStems[viewModel.group] || []).findIndex(x => !!x);
            propStemSelected.active = firstSelected >= 0;
            if (propStemSelected.active) {
                propStemSelected.idx = firstSelected;
            }
            console.log(`Changed selectedStems for screen ${screenName} to ${propStemSelected.idx}`);
        }
        if (typeof data.selectedHotcue === "object") {
            let hotcue = data.selectedHotcue[viewModel.group];

            if (hotcue) {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(hotcue - 1);
                viewModel.hotcueId = hotcue;
                viewModel.hotcuePressed = true;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            } else {
                viewModel.hotcuePressed = false;
            }

            console.log(`Changed selectedHotcue for screen ${screenName} to ${hotcue}`);
        }
        if (typeof data.deckColor === "object") {
            propDeckColors.a = data.deckColor["[Channel1]"]
            propDeckColors.b = data.deckColor["[Channel2]"]
            propDeckColors.c = data.deckColor["[Channel3]"]
            propDeckColors.d = data.deckColor["[Channel4]"]
        }
        if (typeof data.rollpadSize === "object") {
            for (let i = 0; i < 8; i++) {
                switch (`${data.rollpadSize[i]}`.toLowerCase()) {
                    case "double":
                        propRollSizePad[`pad${i+1}`] = "x2"
                        break;
                    case "half":
                        propRollSizePad[`pad${i+1}`] = "/2"
                        break;
                    default:
                        propRollSizePad[`pad${i+1}`] = parseFloat(data.rollpadSize[i]) < 1 ? `1/${1/parseFloat(data.rollpadSize[i])}` : data.rollpadSize[i]
                }
            }
        }
        if (typeof data.beatjumpSize === "object") {
            for (let i = 0; i < 8; i++) {
                switch (`${data.beatjumpSize[i]}`.toLowerCase()) {
                    case "double":
                        propJumpSizePad[`pad${i+1}`] = "x2"
                        break;
                    case "half":
                        propJumpSizePad[`pad${i+1}`] = "/2"
                        break;
                    case "beatjump":
                        propJumpSizePad[`pad${i+1}`] = "??"
                        break;
                    default:
                        propJumpSizePad[`pad${i+1}`] = parseFloat(data.beatjumpSize[i]) < 1 ? `1/${1/parseFloat(data.beatjumpSize[i])}` : data.beatjumpSize[i]
                }
            }
        }
    }
    Component.onCompleted: {
        if (typeof engine.makeSharedDataConnection === "function") {
            engine.makeSharedDataConnection(viewModel.onSharedDataUpdate)
            viewModel.onSharedDataUpdate(engine.getSharedData())
        }
    }

    function isLeftScreen(deckId) {
        return deckId == 1 || deckId == 3;
    }

    function deckLetter(deckId) {
        switch (deckId) {
            case 1: return "A";
            case 2: return "B";
            case 3: return "C";
            default:
                console.error(`Unknown deck ${deckId}. Defaulting to D`);
            case 4:
                return "D";
        }
    }

    function tempoNeeded(master, current) {
        if (master > current) {
            return (1-(current/master))*100;
        }
        return (master/current)*100;
    }

    function toInt_round(val) { return parseInt(val+0.5); }

    function computeBeatCounterStringFromPosition(beat) {
        var phraseLen = 4;
        var curBeat  = parseInt(beat);

        if (beat < 0.0)
            curBeat = curBeat*-1;

        var value1 = parseInt(((curBeat/4)/phraseLen)+1);
        var value2 = parseInt(((curBeat/4)%phraseLen)+1);
        var value3 = parseInt( (curBeat%4)+1);

        if (beat < 0.0)
            return "-" + value1.toString() + "." + value2.toString() + "." + value3.toString();

        return value1.toString() + "." + value2.toString() + "." + value3.toString();
    }

    function computeBeatCounterStringFromPositionSingle(beat) {
        var phraseLen = 4;
        var curBeat  = parseInt(beat);

        if (beat < 0.0)
            curBeat = curBeat*-1;

        var value3 = parseInt( (curBeat%4)+1);

        return value3.toString();
    }

    function computeBeatCounterStringFromPositionAlt(beat) {
        var phraseLen = 4;
        var curBeat  = parseInt(beat);

        if (beat < 0.0)
            curBeat = curBeat*-1;

        var value1 = parseInt(((curBeat)/phraseLen)+1);
        var value2 = parseInt( (curBeat%4)+1);

        if (beat < 0.0)
            return "-" + value1.toString() + "." + value2.toString();

        return value1.toString() + "." + value2.toString();
    }

  ////////////////////////////////////
  ////// Global info properties //////
  ////////////////////////////////////
    QtObject {
        id: propDeckColors
        property int a: 10
        property int b: 10
        property int c: 2
        property int d: 2
    }
    QtObject {
        id: propRollSizePad
        property var pad1: 1/32
        property var pad2: 1/16
        property var pad3: 1/8
        property var pad4: 1/4
        property var pad5: 1/2
        property var pad6: 1
        property var pad7: 2
        property var pad8: 4
    }
    QtObject {
        id: propJumpSizePad
        property var pad1: 0.5
        property var pad2: 1
        property var pad3: 2
        property var pad4: 4
        property var pad5: 8
        property var pad6: 16
        property var pad7: 32
        property var pad8: 64
    }

    readonly property int 		deckAColor:	propDeckColors.a
    readonly property int 		deckBColor:	propDeckColors.b
    readonly property int 		deckCColor:	propDeckColors.c
    readonly property int 		deckDColor:	propDeckColors.d

  ////////////////////////////////////
  /////// Track info properties //////
  ////////////////////////////////////

    property int deckId: 1
    readonly property bool trackEndWarning: propTrackEndWarning.value
    readonly property bool shift: propShift.value
    readonly property string artistString: isLoaded ? propArtist.value : "Mixxx"
    readonly property string bpmString: isLoaded ? propBPM.value.toFixed(2).toString() : "0.00"
    readonly property string beats: 	 computeBeatCounterStringFromPosition(((propElapsedTime.value*1000-propGridOffset.value)*propMixerBpm.value)/60000.0)
    readonly property string beatSingle: computeBeatCounterStringFromPositionSingle(((propElapsedTime.value*1000-propGridOffset.value)*propMixerBpm.value)/60000.0)
    readonly property string beatsAlt: computeBeatCounterStringFromPositionAlt(((propElapsedTime.value*1000-propGridOffset.value)*propMixerBpm.value)/60000.0)
    readonly property string 	masterDeckLetter: 	leaderGroup.replace('[Channel', '').substr(0, 1)
        readonly property string masterBPM: isLoaded ? propMasterBPM.value : 0.00
        readonly property string masterBPMShort: isLoaded ? propMasterBPM.value.toFixed(2).toString() : 0.00
        readonly property string masterBPMFooter: isLoaded ? propMasterBPM.value.toFixed(2).toString() + " BPM" : ""
        readonly property string masterBPMFooter2: isLoaded ? propMasterBPM.value.toFixed(2).toString() + "BPM" : ""
        readonly property string bpmOffset: isLoaded ? (bpmString - masterBPM).toFixed(2).toString() : "0.00"
        readonly property string tempoString:		 isLoaded ? (propTempo.value).toFixed(2).toString() : "0.00"
        readonly property string tempoRange:		 	 toInt_round(propTempoRange.value*100).toString() + "%"
        readonly property string tempoStringPer:		 tempoString+'%'
        readonly property string tempoNeededVal:	 tempoNeeded(masterBPMShort, bpmString).toFixed(2).toString()
        readonly property string tempoNeededString:	 isLoaded ? (tempoNeededVal == 0) ? "0.00" : (tempoNeededVal < 0) ? tempoNeededVal + "%" : "+" + tempoNeededVal + "%" : "0.00"
        readonly property string songBPM: propSongBPM.value.toFixed(2).toString()
        readonly property bool hightlightLoop: !shift
        readonly property bool hightlightKey: shift
        readonly property int isLoaded: (propTrackLength.value > 0)
        readonly property bool	showLogo:			 propTrackLength.value == 0 ? true : false
        readonly property string keyString: propKeyForDisplay.value
        readonly property string masterKey: propMasterKey.value
        readonly property int keyIndex: propFinalKeyId.value
        readonly property int masterKeyIndex: propMasterKeyId.value
        readonly property bool hasKey: isLoaded && keyIndex >= 0
        readonly property bool hasTempo: isLoaded && !!propTempo.value
        readonly property bool isKeyLockOn: propKeyLockOn.value
        readonly property bool isSyncOn: 	 propIsInSync.value
        readonly property bool isStemDeck: (propIsStemDeck.value >= 2) ? true : false
        readonly property bool loopActive: propLoopActive.value
        readonly property string loopSizeString: propLoopSize.value < 1 ? `1/${1 / propLoopSize.value}` : `${propLoopSize.value}`
        readonly property string loopSizeInt: propLoopSize.value
        readonly property string remainingTimeString: (!isLoaded) ? "00:00" : utils.computeRemainingTimeString(propTrackLength.value, propElapsedTime.value)
        readonly property string elapsedTimeString: (!isLoaded) ? "00:00" : utils.convertToTimeString(Math.floor(propElapsedTime.value))
        readonly property string titleString: isLoaded ? propTitle.value : "Load a Track to Deck " + deckLetter(deckId)
        readonly property real phase: isPlaying && leaderGroup != group ? propPhase.value : 0
        readonly property bool touchKey: false // TODO map shift encoder touch event
        readonly property bool touchTime: false // TODO map shift encoder touch event
        readonly property bool touchLoop: false // TODO map shift encoder touch event
        readonly property int deckType: propDeckType.value
        readonly property string keyAdjustString: (keyAdjustVal < 0 ? "" : "+") + (keyAdjustVal).toFixed(0).toString()
        readonly property real keyAdjustVal: propKeyAdjust.value*12
        readonly property variant loopSizeText: ["1/32", "1/16", "1/8", "1/4", "1/2", "1", "2", "4", "8", "16", "32"]
        readonly property bool 	slicerEnabled: 	 propEnabled.value
        readonly property int 	slicerNo: 	 	 propSlicerNo.value
        readonly property int		slicerSize:		 	 propSlicerSize.value

        readonly property bool headerEnabled: propHeaderEnabled.value
        readonly property string headerText: propHeaderText.value
        readonly property string headerTextLong: propHeaderTextLong.value
        readonly property int		sampleRate:			propSampleRate.value

        readonly property bool		isPlaying:			propIsPlaying.value

        readonly property bool		is1Playing:			propIs1Playing.value
        readonly property bool		is2Playing:			propIs2Playing.value
        readonly property bool		is3Playing:			propIs3Playing.value
        readonly property bool		is4Playing:			propIs4Playing.value

        Mixxx.ControlProxy {
            group: viewModel.group
            key: "track_samplerate"
            id: propSampleRate
        }
        Mixxx.ControlProxy {
            group: viewModel.leaderGroup
            key: "track_samplerate"
            id: propLeaderSampleRate
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            id: propTempoRange
            key: "rateRange"
        }
        QtObject {
            id: propEnabled
            property var value: 0
        }
        QtObject {
            id: propSlicerNo
            property var value: 0
        }
        QtObject {
            id: propSlicerSize
            property var value: 0
        }
        QtObject {
            id: propDeckType
            property var value: 0
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "play"
            id: propIsPlaying
        }

        Mixxx.ControlProxy {
            group: "[Channel1]"
            id: propIs1Leader
            key: "sync_mode"
        }

        Mixxx.ControlProxy {
            group: "[Channel2]"
            id: propIs2Leader
            key: "sync_mode"
        }

        Mixxx.ControlProxy {
            group: "[Channel3]"
            id: propIs3Leader
            key: "sync_mode"
        }

        Mixxx.ControlProxy {
            group: "[Channel4]"
            id: propIs4Leader
            key: "sync_mode"
        }

        readonly property string leaderGroup: propIs1Leader.value >= 2 ? `[Channel1]` : propIs2Leader.value >= 2 ? `[Channel2]` : propIs3Leader.value >= 2 ? `[Channel3]` : propIs4Leader.value >= 2 ? `[Channel4]` : viewModel.group

        Mixxx.ControlProxy {
            group: "[Channel1]"
            key: "play"
            id: propIs1Playing
        }
        Mixxx.ControlProxy {
            group: "[Channel2]"
            key: "play"
            id: propIs2Playing
        }
        Mixxx.ControlProxy {
            group: "[Channel3]"
            key: "play"
            id: propIs3Playing
        }
        Mixxx.ControlProxy {
            group: "[Channel4]"
            key: "play"
            id: propIs4Playing
        }

        QtObject {
            id: propTitle
            property var value: viewModel.currentPlayer?.title || "Unknown"
        }
        QtObject {
            id: propArtist
            property var value: viewModel.currentPlayer?.artist || "Unknown"
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            id: propSongBPM
            key: "file_bpm"
        }

        Mixxx.ControlProxy {
            group: viewModel.group
            id: propKey
            key: "key"
        }
        QtObject {
            id: propKeyForDisplay
            property var value: [
                                 "No key",
                                 "1d",
                                 "8d",
                                 "3d",
                                 "10d",
                                 "5d",
                                 "12d",
                                 "7d",
                                 "2d",
                                 "9d",
                                 "4d",
                                 "11d",
                                 "6d",
                                 "10m",
                                 "5m",
                                 "12m",
                                 "7m",
                                 "2m",
                                 "9m",
                                 "4m",
                                 "11m",
                                 "6m",
                                 "1m",
                                 "8m",
                                 "3m"
                                 ][propKey.value]
        }
        QtObject {
            id: propMasterKey
            property var value: 0
        }
        QtObject {
            id: propMixerBpm
            property var value: 0
        }
        QtObject {
            id: propMixerBpmMaster
            property var value: 160
        }
        QtObject {
            id: propFinalKeyId
            property var value: propKey.value
        }
        Mixxx.ControlProxy {
            group: viewModel.leaderGroup
            id: propMasterKeyId
            key: "key"
        }
        QtObject {
            id: propKeyAdjust
            property var value: 0
        }
        QtObject {
            id: propGridOffset
            property var value: 0
        }
        QtObject {
            id: propGridOffsetMaster
            property var value: 10000
        }

        Mixxx.ControlProxy {
            group: viewModel.group
            id: propKeyLockOn
            key: "keylock"
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "bpm"
            id: propBPM
        }
        Mixxx.ControlProxy {
            group: '[InternalClock]'
            key: "bpm"
            id: propMasterBPM
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "visual_bpm"
            id: propTempo
        }
        QtObject {
            id: propTempoAbsolute
            property var value: 0
        }

        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beat_closest"
            id: propBeatClosest
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "track_samples"
            id: propSample
        }
        QtObject {
            id: propBeatSample
            property var value: (propSampleRate.value * 60) / propBPM.value
        }
        QtObject {
            id: propBeatSampleOffset
            property var value: propBeatClosest.value % propBeatSample.value
        }
        QtObject {
            id: propBeat
            property var value: (propTrackPosition.value * propSample.value / 2) / propBeatSample.value
        }
        Mixxx.ControlProxy {
            group: viewModel.leaderGroup
            key: "beat_closest"
            id: propLeaderBeatClosest
        }
        Mixxx.ControlProxy {
            group: viewModel.leaderGroup
            key: "track_samples"
            id: propLeaderSample
        }
        QtObject {
            id: propLeaderBeatSample
            property var value: (propLeaderSampleRate.value * 60) / propMasterBPM.value
        }
        QtObject {
            id: propLeaderBeatSampleOffset
            property var value: propLeaderBeatClosest.value % propLeaderBeatSample.value
        }
        QtObject {
            id: propLeaderBeat
            property var value: (propLeaderTrackPosition.value * propLeaderSample.value / 2) / propLeaderBeatSample.value
        }
        QtObject {
            id: propPhase
            property var value: (propLeaderBeat.value-propBeat.value - 0.5) % 1 - 0.5
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beatloop_size"
            id: propLoopSize
        }
        Mixxx.ControlProxy {
            id: propLoopActive
            group: viewModel.group
            key: "loop_enabled"
        }
        QtObject {
            id: proploopActive
            property var value: 0
        }
        Mixxx.ControlProxy {
            id: propTrackLength
            group: viewModel.group
            key: "duration"
        }
        Mixxx.ControlProxy {
            id: propTrackPosition
            group: viewModel.group
            key: "playposition"
        }
        Mixxx.ControlProxy {
            id: propLeaderTrackPosition
            group: viewModel.leaderGroup
            key: "playposition"
        }
        QtObject {
            id: propElapsedTime
            property var value: parseInt(propTrackPosition.value * propTrackLength.value)
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `end_of_track`
            id: propTrackEndWarning
        }

        QtObject {
            id: propHeaderEnabled
            property var value: false
        }
        QtObject {
            id: propHeaderText
            property var value: "HeaderText"
        }
        QtObject {
            id: propHeaderTextLong
            property var value: "HeaderTextLong"
        }

        Mixxx.ControlProxy {
            group: viewModel.group
            key: "stem_count"
            id: propIsStemDeck
        }

        Timer {
            id: loopAdjust
            property bool show: false

            triggeredOnStart: true
            interval: settings.loopOverlayTimer
            repeat: false
            running: false

            onTriggered: {
                show = !show
            }
        }

        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beats_translate_curpos"
            id: propBeatsTranslateCurpos
            onValueChanged: {
                loopAdjust.running = true
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beats_adjust_faster"
            id: propBeatsAdjustFaster
            onValueChanged: {
                loopAdjust.running = true
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beats_adjust_slower"
            id: propBeatsAdjustSlower
            onValueChanged: {
                loopAdjust.running = true
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beats_translate_later"
            id: propBeatsTranslateLater
            onValueChanged: {
                loopAdjust.running = true
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "beats_translate_earlier"
            id: propBeatsTranslateEarlier
            onValueChanged: {
                loopAdjust.running = true
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: "rateRange"
            id: propRateRange
            onValueChanged: {
                loopAdjust.running = true
            }
        }

        readonly property bool adjustEnabled: settings.showBPMGridAdjust ? loopAdjust.show : false

        QtObject {
            id: propPadsMode
            property var value: 0
        }
        QtObject {
            id: propSelectedQuickFX
            property var value: null
        }
        readonly property var quickFXSelected: propSelectedQuickFX.value
        property bool padsModeJump: propPadsMode.value == 1
        property bool padsModeLoop: propPadsMode.value == 5
        property bool padsModeRoll: propPadsMode.value == 3
        property bool padsModeTone: propPadsMode.value == 11
        property bool padsModeBank1: propPadsMode.value == 12
        property bool padsModeBank2: propPadsMode.value == 13

        Mixxx.ControlProxy {
            id: propIsInSync
            group: root.group
            key: "sync_enabled"
        }

        Mixxx.ControlProxy {
            id: propBrowser
            group: "[Skin]"
            key: "show_maximized_library"
        }
        readonly property bool 	isInBrowserMode: 	 propBrowser.value

        QtObject {
            id: propShift
            property bool value: false
        }

        Mixxx.ControlProxy {
            id: propZoom

            group: root.group
            key: "waveform_zoom"
            onValueChanged: {
                loopAdjust.running = true
            }
        }

        readonly property int zoomLevel: propZoom.value

  //fx and overlays
        property var fxModel: Mixxx.EffectsManager.visibleEffectsModel

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: "mix_mode"
            id: propFx1Type
        }
        readonly property int fx1Type:	propFx1Type.value

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: "mix_mode"
            id: propFx2Type
        }
        readonly property int fx2Type:	propFx2Type.value

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: "mix_mode"
            id: propFx3Type
        }
        readonly property int fx3Type:	propFx3Type.value

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: "mix_mode"
            id: propFx4Type
        }
        readonly property int fx4Type:	propFx4Type.value

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: "mix"
            id: propFx1DryWet
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: "mix"
            id: propFx2DryWet
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect1]"
            key: `meta`
            id: propFx1Knob1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect2]"
            key: `meta`
            id: propFx1Knob2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect3]"
            key: `meta`
            id: propFx1Knob3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2_Effect1]"
            key: `meta`
            id: propFx2Knob1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2_Effect2]"
            key: `meta`
            id: propFx2Knob2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2_Effect3]"
            key: `meta`
            id: propFx2Knob3
        }

        Mixxx.ControlProxy {
            id: propFx1Knob1Name
            group: "[EffectRack1_EffectUnit1_Effect1]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect2]"
            key: "loaded_effect"
            id: propFx1Knob2Name
        }
        Mixxx.ControlProxy {
            id: propFx1Knob3Name
            group: "[EffectRack1_EffectUnit1_Effect3]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx2Knob1Name
            group: "[EffectRack1_EffectUnit2_Effect1]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx2Knob2Name
            group: "[EffectRack1_EffectUnit2_Effect2]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx2Knob3Name
            group: "[EffectRack1_EffectUnit2_Effect3]"
            key: "loaded_effect"
        }

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: "enabled"
            id: propFx1Enabled
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: `enabled`
            id: propFx2Enabled
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect1]"
            key: `enabled`
            id: propFx1Button1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect2]"
            key: `enabled`
            id: propFx1Button2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1_Effect3]"
            key: `enabled`
            id: propFx1Button3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2_Effect1]"
            key: `enabled`
            id: propFx2Button1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2_Effect2]"
            key: `enabled`
            id: propFx2Button2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2_Effect3]"
            key: `enabled`
            id: propFx2Button3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: `group_[Channel1]_enable`
            id: propFx1Ch1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: `group_[Channel2]_enable`
            id: propFx1Ch2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: `group_[Channel3]_enable`
            id: propFx1Ch3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: `group_[Channel4]_enable`
            id: propFx1Ch4
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: `group_[Channel1]_enable`
            id: propFx2Ch1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: `group_[Channel2]_enable`
            id: propFx2Ch2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: `group_[Channel3]_enable`
            id: propFx2Ch3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: `group_[Channel4]_enable`
            id: propFx2Ch4
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: `group_[Channel1]_enable`
            id: propFx3Ch1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: `group_[Channel2]_enable`
            id: propFx3Ch2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: `group_[Channel3]_enable`
            id: propFx3Ch3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: `group_[Channel4]_enable`
            id: propFx3Ch4
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: `group_[Channel1]_enable`
            id: propFx4Ch1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: `group_[Channel2]_enable`
            id: propFx4Ch2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: `group_[Channel3]_enable`
            id: propFx4Ch3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: `group_[Channel4]_enable`
            id: propFx4Ch4
        }

        readonly property real fx1DryWet: propFx1DryWet.value
        readonly property real fx2DryWet: propFx2DryWet.value
        readonly property real fx1Knob1: propFx1Knob1.value
        readonly property real fx1Knob2: propFx1Knob2.value
        readonly property real fx1Knob3: propFx1Knob3.value
        readonly property real fx2Knob1: propFx2Knob1.value
        readonly property real fx2Knob2: propFx2Knob2.value
        readonly property real fx2Knob3: propFx2Knob3.value

        readonly property string fx1Knob1Name: viewModel.fxModel.get(propFx1Knob1Name.value).display
        readonly property string fx1Knob2Name: viewModel.fxModel.get(propFx1Knob2Name.value).display
        readonly property string fx1Knob3Name: viewModel.fxModel.get(propFx1Knob3Name.value).display
        readonly property string fx2Knob1Name: viewModel.fxModel.get(propFx2Knob1Name.value).display
        readonly property string fx2Knob2Name: viewModel.fxModel.get(propFx2Knob2Name.value).display
        readonly property string fx2Knob3Name: viewModel.fxModel.get(propFx2Knob3Name.value).display

        readonly property bool fx1Enabled:	propFx1Enabled.value
        readonly property bool fx2Enabled:	propFx2Enabled.value
        readonly property bool fx1Button1:	propFx1Button1.value
        readonly property bool fx1Button2:	propFx1Button2.value
        readonly property bool fx1Button3:	propFx1Button3.value
        readonly property bool fx2Button1:	propFx2Button1.value
        readonly property bool fx2Button2:	propFx2Button2.value
        readonly property bool fx2Button3:	propFx2Button3.value

        readonly property bool fx1Ch1:	propFx1Ch1.value
        readonly property bool fx1Ch2:	propFx1Ch2.value
        readonly property bool fx1Ch3:	propFx1Ch3.value
        readonly property bool fx1Ch4:	propFx1Ch4.value
        readonly property bool fx2Ch1:	propFx2Ch1.value
        readonly property bool fx2Ch2:	propFx2Ch2.value
        readonly property bool fx2Ch3:	propFx2Ch3.value
        readonly property bool fx2Ch4:	propFx2Ch4.value

        onFx1DryWetChanged: {fx1Timer.running = true}
        onFx2DryWetChanged: {fx2Timer.running = true}
        onFx1Knob1Changed: {fx1Timer.running = true}
        onFx1Knob2Changed: {fx1Timer.running = true}
        onFx1Knob3Changed: {fx1Timer.running = true}
        onFx2Knob1Changed: {fx2Timer.running = true}
        onFx2Knob2Changed: {fx2Timer.running = true}
        onFx2Knob3Changed: {fx2Timer.running = true}
        onFx1EnabledChanged: {fx1Timer.running = true}
        onFx2EnabledChanged: {fx2Timer.running = true}
        onFx1Button1Changed: {fx1Timer.running = true}
        onFx1Button2Changed: {fx1Timer.running = true}
        onFx1Button3Changed: {fx1Timer.running = true}
        onFx2Button1Changed: {fx2Timer.running = true}
        onFx2Button2Changed: {fx2Timer.running = true}
        onFx2Button3Changed: {fx2Timer.running = true}
        onFx1Ch1Changed: {fx1Timer.running = true}
        onFx1Ch2Changed: {fx1Timer.running = true}
        onFx1Ch3Changed: {fx1Timer.running = true}
        onFx1Ch4Changed: {fx1Timer.running = true}
        onFx2Ch1Changed: {fx2Timer.running = true}
        onFx2Ch2Changed: {fx2Timer.running = true}
        onFx2Ch3Changed: {fx2Timer.running = true}
        onFx2Ch4Changed: {fx2Timer.running = true}
        onFx1Knob1NameChanged: {fx1Timer.running = true}
        onFx1Knob2NameChanged: {fx1Timer.running = true}
        onFx1Knob3NameChanged: {fx1Timer.running = true}
        onFx2Knob1NameChanged: {fx2Timer.running = true}
        onFx2Knob2NameChanged: {fx2Timer.running = true}
        onFx2Knob3NameChanged: {fx2Timer.running = true}

        onLoopSizeStringChanged: {loopTimer.running = true}
        onLoopActiveChanged: {loopTimer.running = true}

        Timer {
            id: loopTimer
            property bool showLoop: false

            triggeredOnStart: true
            interval: settings.loopOverlayTimer
            repeat: false
            running: false

            onTriggered: {
                showLoop = !showLoop
            }
        }

        property bool showLoopInfo:	 loopTimer.showLoop

        onBpmStringChanged: {bpmTimer.running = true}

        Timer {
            id: bpmTimer
            property bool showBPM: false

            triggeredOnStart: true
            interval: settings.bpmOverlayTimer
            repeat: false
            running: false

            onTriggered: {
                showBPM = !showBPM
            }
        }

        property bool showBPMInfo:	 bpmTimer.showBPM && bpmTimer.running

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: "mix"
            id: propFx3DryWet
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: "mix"
            id: propFx4DryWet
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3_Effect1]"
            key: `meta`
            id: propFx3Knob1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3_Effect2]"
            key: `meta`
            id: propFx3Knob2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3_Effect3]"
            key: `meta`
            id: propFx3Knob3
        }
        Mixxx.ControlProxy {
            id: propFx4Knob1
            group: "[EffectRack1_EffectUnit4_Effect1]"
            key: `meta`
        }
        Mixxx.ControlProxy {
            id: propFx4Knob2
            group: "[EffectRack1_EffectUnit4_Effect2]"
            key: `meta`
        }
        Mixxx.ControlProxy {
            id: propFx4Knob3
            group: "[EffectRack1_EffectUnit4_Effect3]"
            key: `meta`
        }

        Mixxx.ControlProxy {
            id: propFx3Knob1Name
            group: "[EffectRack1_EffectUnit3_Effect1]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx3Knob2Name
            group: "[EffectRack1_EffectUnit3_Effect2]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx3Knob3Name
            group: "[EffectRack1_EffectUnit3_Effect3]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx4Knob1Name
            group: "[EffectRack1_EffectUnit4_Effect1]"
            key: `loaded_effect`
        }
        Mixxx.ControlProxy {
            id: propFx4Knob2Name
            group: "[EffectRack1_EffectUnit4_Effect2]"
            key: "loaded_effect"
        }
        Mixxx.ControlProxy {
            id: propFx4Knob3Name
            group: "[EffectRack1_EffectUnit4_Effect3]"
            key: `loaded_effect`
        }

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: "enabled"
            id: propFx3Enabled
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: "enabled"
            id: propFx4Enabled
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3_Effect1]"
            key: `enabled`
            id: propFx3Button1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3_Effect2]"
            key: `enabled`
            id: propFx3Button2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3_Effect3]"
            key: `enabled`
            id: propFx3Button3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4_Effect1]"
            key: `enabled`
            id: propFx4Button1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4_Effect2]"
            key: `enabled`
            id: propFx4Button2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4_Effect3]"
            key: `enabled`
            id: propFx4Button3
        }

        readonly property real fx3DryWet: propFx3DryWet.value
        readonly property real fx4DryWet: propFx3DryWet.value
        readonly property real fx3Knob1: propFx3Knob1.value
        readonly property real fx3Knob2: propFx3Knob2.value
        readonly property real fx3Knob3: propFx3Knob3.value
        readonly property real fx4Knob1: propFx4Knob1.value
        readonly property real fx4Knob2: propFx4Knob2.value
        readonly property real fx4Knob3: propFx4Knob3.value

        readonly property string fx3Knob1Name: viewModel.fxModel.get(propFx3Knob1Name.value).display
        readonly property string fx3Knob2Name: viewModel.fxModel.get(propFx3Knob2Name.value).display
        readonly property string fx3Knob3Name: viewModel.fxModel.get(propFx3Knob3Name.value).display
        readonly property string fx4Knob1Name: viewModel.fxModel.get(propFx4Knob1Name.value).display
        readonly property string fx4Knob2Name: viewModel.fxModel.get(propFx4Knob2Name.value).display
        readonly property string fx4Knob3Name: viewModel.fxModel.get(propFx4Knob3Name.value).display

        readonly property bool fx3Enabled:	propFx3Enabled.value
        readonly property bool fx4Enabled:	propFx4Enabled.value
        readonly property bool fx3Button1:	propFx3Button1.value
        readonly property bool fx3Button2:	propFx3Button2.value
        readonly property bool fx3Button3:	propFx3Button3.value
        readonly property bool fx4Button1:	propFx4Button1.value
        readonly property bool fx4Button2:	propFx4Button2.value
        readonly property bool fx4Button3:	propFx4Button3.value

        readonly property bool fx3Ch1:	propFx3Ch1.value
        readonly property bool fx3Ch2:	propFx3Ch2.value
        readonly property bool fx3Ch3:	propFx3Ch3.value
        readonly property bool fx3Ch4:	propFx3Ch4.value
        readonly property bool fx4Ch1:	propFx4Ch1.value
        readonly property bool fx4Ch2:	propFx4Ch2.value
        readonly property bool fx4Ch3:	propFx4Ch3.value
        readonly property bool fx4Ch4:	propFx4Ch4.value

        onFx3DryWetChanged: {fx3Timer.running = true}
        onFx4DryWetChanged: {fx4Timer.running = true}
        onFx3Knob1Changed: {fx3Timer.running = true}
        onFx3Knob2Changed: {fx3Timer.running = true}
        onFx3Knob3Changed: {fx3Timer.running = true}
        onFx4Knob1Changed: {fx4Timer.running = true}
        onFx4Knob2Changed: {fx4Timer.running = true}
        onFx4Knob3Changed: {fx4Timer.running = true}
        onFx3EnabledChanged: {fx3Timer.running = true}
        onFx4EnabledChanged: {fx4Timer.running = true}
        onFx3Button1Changed: {fx3Timer.running = true}
        onFx3Button2Changed: {fx3Timer.running = true}
        onFx3Button3Changed: {fx3Timer.running = true}
        onFx4Button1Changed: {fx4Timer.running = true}
        onFx4Button2Changed: {fx4Timer.running = true}
        onFx4Button3Changed: {fx4Timer.running = true}
        onFx3Ch1Changed: {fx3Timer.running = true}
        onFx3Ch2Changed: {fx3Timer.running = true}
        onFx3Ch3Changed: {fx3Timer.running = true}
        onFx3Ch4Changed: {fx3Timer.running = true}
        onFx4Ch1Changed: {fx4Timer.running = true}
        onFx4Ch2Changed: {fx4Timer.running = true}
        onFx4Ch3Changed: {fx4Timer.running = true}
        onFx4Ch4Changed: {fx4Timer.running = true}
        onFx3Knob1NameChanged: {fx3Timer.running = true}
        onFx3Knob2NameChanged: {fx3Timer.running = true}
        onFx3Knob3NameChanged: {fx3Timer.running = true}
        onFx4Knob1NameChanged: {fx4Timer.running = true}
        onFx4Knob2NameChanged: {fx4Timer.running = true}
        onFx4Knob3NameChanged: {fx4Timer.running = true}

        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit1]"
            key: `group_${viewModel.group}_enable`
            id: propfx1
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit2]"
            key: `group_${viewModel.group}_enable`
            id: propfx2
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit3]"
            key: `group_${viewModel.group}_enable`
            id: propfx3
        }
        Mixxx.ControlProxy {
            group: "[EffectRack1_EffectUnit4]"
            key: `group_${viewModel.group}_enable`
            id: propfx4
        }

        readonly property bool fx1On: propfx1.value
        readonly property bool fx2On: propfx2.value
        readonly property bool fx3On: propfx3.value
        readonly property bool fx4On: propfx4.value

        Timer {
            id: fx1Timer
            property bool blink: false

            triggeredOnStart: true
            interval: settings.fxOverlayTimer
            repeat: false
            running: fx1On

            onTriggered: {
                blink = !blink
            }
        }

        Timer {
            id: fx2Timer
            property bool blink: false

            triggeredOnStart: true
            interval: settings.fxOverlayTimer
            repeat: false
            running: fx2On

            onTriggered: {
                blink = !blink
            }
        }

        Timer {
            id: fx3Timer
            property bool blink: false

            triggeredOnStart: true
            interval: settings.fxOverlayTimer
            repeat: false
            running: fx3On

            onTriggered: {
                blink = !blink
            }
        }

        Timer {
            id: fx4Timer
            property bool blink: false

            triggeredOnStart: true
            interval: settings.fxOverlayTimer
            repeat: false
            running: fx4On

            onTriggered: {
                blink = !blink
            }
        }

        readonly property bool showFx1: fx1On && fx1Timer.blink
        readonly property bool showFx2: fx2On && fx2Timer.blink
        readonly property bool showFx3: fx3On && fx3Timer.blink
        readonly property bool showFx4: fx4On && fx4Timer.blink

        Mixxx.ControlProxy {
            id: propView
            group: "[Skin]"
            key: "show_maximized_library"
        }

        readonly property bool viewButton: propView.value && false

        property int hotcueId: 0
        readonly property bool hotcueDisplay: hotcuePressed || cueTimer.running
        property string hotcueName: ""
        property int hotcueType: 0

        property bool hotcuePressed: false
        onHotcuePressedChanged: {hotcuePressed == false ? cueTimer.restart() : hotcuePressed = hotcuePressed }

        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_1_activate`
            id: propHotcue1Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(0);
                viewModel.hotcueId = 1;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_2_activate`
            id: propHotcue2Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(1);
                viewModel.hotcueId = 2;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_3_activate`
            id: propHotcue3Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(2);
                viewModel.hotcueId = 3;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_4_activate`
            id: propHotcue4Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(3);
                viewModel.hotcueId = 4;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_5_activate`
            id: propHotcue5Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(4);
                viewModel.hotcueId = 5;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_6_activate`
            id: propHotcue6Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(5);
                viewModel.hotcueId = 6;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_7_activate`
            id: propHotcue7Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(6);
                viewModel.hotcueId = 7;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }
        Mixxx.ControlProxy {
            group: viewModel.group
            key: `hotcue_8_activate`
            id: propHotcue8Activated
            onValueChanged: {
                let model = viewModel.currentPlayer?.hotcuesModel?.get(7);
                viewModel.hotcueId = 8;
                viewModel.hotcuePressed = value;
                viewModel.hotcueName = model?.label || "Unnamed cue";
                viewModel.hotcueType = model?.isLoop ? 5 : 0;
            }
        }

        Timer {
            id: cueTimer
            property bool blink: false

            triggeredOnStart: true
            interval: 1000
            repeat: false
            running: false
        }

  ///////////////////////////////////////////////////
  /////// Stem Deck properties //////////////////////
  ///////////////////////////////////////////////////

        Mixxx.ControlProxy {
            group: viewModel.group
            key: `stem_count`
            id: propStemCount
        }

        readonly property bool isStemsActive: propStemCount.value > 0
        readonly property int stemCount: propStemCount.value

        QtObject {
            id: propStemSelected
            property var idx: 0
            property bool active: false
        }
        readonly property bool stemSelected: propStemSelected.active
        readonly property var stemSelectedIdx: propStemSelected.idx

        readonly property string stemSelectedName: viewModel.currentPlayer?.stemsModel.get(viewModel.stemSelectedIdx).label || "Unknown"
        readonly property real stemSelectedVolume: isStemsActive ? [propStem1Volume,propStem2Volume,propStem3Volume,propStem4Volume][viewModel.stemSelectedIdx].value : 0.0
        readonly property bool stemSelectedMuted: isStemsActive ? [propStem1Muted,propStem2Muted,propStem3Muted,propStem4Muted][viewModel.stemSelectedIdx].value : false
        readonly property int stemSelectedQuickFXId: isStemsActive ? [propStem1FX,propStem2FX,propStem3FX,propStem4FX][viewModel.stemSelectedIdx].value : 0
        readonly property real stemSelectedQuickFXValue: isStemsActive ? [propStem1FXValue,propStem2FXValue,propStem3FXValue,propStem4FXValue][viewModel.stemSelectedIdx].value : 0.0
        readonly property bool stemSelectedQuickFXOn: isStemsActive ? [propStem1FXOn,propStem2FXOn,propStem3FXOn,propStem4FXOn][viewModel.stemSelectedIdx].value : false
        readonly property string stemSelectedQuickFXName: Mixxx.EffectsManager.quickChainPresetModel.get(viewModel.stemSelectedQuickFXId).display || "---"
        readonly property color stemSelectedBrightColor: viewModel.currentPlayer?.stemsModel.get(viewModel.stemSelectedIdx).color ?? "grey"
        readonly property color stemSelectedMidColor: isStemsActive ? stemSelectedBrightColor : "black"

        Mixxx.ControlProxy {
            id: propStem1Volume
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem1]`
            key: `volume`
        }
        Mixxx.ControlProxy {
            id: propStem1Muted
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem1]`
            key: `mute`
        }
        Mixxx.ControlProxy {
            id: propStem1FX
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem1]]`
            key: `loaded_chain_preset`
        }
        Mixxx.ControlProxy {
            id: propStem1FXOn
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem1]]`
            key: `enabled`
        }
        Mixxx.ControlProxy {
            id: propStem1FXValue
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem1]]`
            key: `super1`
        }
        Mixxx.ControlProxy {
            id: propStem2Volume
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem2]`
            key: `volume`
        }
        Mixxx.ControlProxy {
            id: propStem2Muted
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem2]`
            key: `mute`
        }
        Mixxx.ControlProxy {
            id: propStem2FX
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem2]]`
            key: `loaded_chain_preset`
        }
        Mixxx.ControlProxy {
            id: propStem2FXOn
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem2]]`
            key: `enabled`
        }
        Mixxx.ControlProxy {
            id: propStem2FXValue
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem2]]`
            key: `super1`
        }
        Mixxx.ControlProxy {
            id: propStem3Volume
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem3]`
            key: `volume`
        }
        Mixxx.ControlProxy {
            id: propStem3Muted
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem3]`
            key: `mute`
        }
        Mixxx.ControlProxy {
            id: propStem3FX
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem3]]`
            key: `loaded_chain_preset`
        }
        Mixxx.ControlProxy {
            id: propStem3FXOn
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem3]]`
            key: `enabled`
        }
        Mixxx.ControlProxy {
            id: propStem3FXValue
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem3]]`
            key: `super1`
        }
        Mixxx.ControlProxy {
            id: propStem4Volume
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem4]`
            key: `volume`
        }
        Mixxx.ControlProxy {
            id: propStem4Muted
            group: `${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem4]`
            key: `mute`
        }
        Mixxx.ControlProxy {
            id: propStem4FX
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem4]]`
            key: `loaded_chain_preset`
        }
        Mixxx.ControlProxy {
            id: propStem4FXOn
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem4]]`
            key: `enabled`
        }
        Mixxx.ControlProxy {
            id: propStem4FXValue
            group: `[QuickEffectRack1_${viewModel.group.substr(0, viewModel.group.length - 1)}_Stem4]]`
            key: `super1`
        }

  ///////////////////////////////////////////////////
  /////// Stripe properties /////////////////////////
  ///////////////////////////////////////////////////

        readonly property var hotcues: viewModel.currentPlayer?.hotcuesModel

  /// Loop size
        readonly property var	loopSizePad1: "1/4"
        readonly property var	loopSizePad2: "1/2"
        readonly property var	loopSizePad3: "1"
        readonly property var	loopSizePad4: "2"
        readonly property var	loopSizePad5: "4"
        readonly property var	loopSizePad6: "8"
        readonly property var	loopSizePad7: "16"
        readonly property var	loopSizePad8: "32"

        readonly property var jumpSizePad1: propJumpSizePad.pad1
        readonly property var jumpSizePad2: propJumpSizePad.pad2
        readonly property var jumpSizePad3: propJumpSizePad.pad3
        readonly property var jumpSizePad4: propJumpSizePad.pad4
        readonly property var jumpSizePad5: propJumpSizePad.pad5
        readonly property var jumpSizePad6: propJumpSizePad.pad6
        readonly property var jumpSizePad7: propJumpSizePad.pad7
        readonly property var jumpSizePad8: propJumpSizePad.pad8

        readonly property var rollSizePad1: propRollSizePad.pad1
        readonly property var rollSizePad2: propRollSizePad.pad2
        readonly property var rollSizePad3: propRollSizePad.pad3
        readonly property var rollSizePad4: propRollSizePad.pad4
        readonly property var rollSizePad5: propRollSizePad.pad5
        readonly property var rollSizePad6: propRollSizePad.pad6
        readonly property var rollSizePad7: propRollSizePad.pad7
        readonly property var rollSizePad8: propRollSizePad.pad8
    }
