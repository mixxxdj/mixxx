import QtQuick 2.5

QtObject {

	// = comments

	//////////////////
	//EXTRA SETTINGS//
	//////////////////

	//show only decks A&B or C&D - SELECT ONLY ONE
    readonly property color 		accentColor: engine.getSetting('accentColor') || 'green'

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////
	//PAD TYPE SELECTION SETTINGS//
	///////////////////////////////

	//Please only use the values in the line below. Using other values could have unexpected effects.
	//0 = disabled, 4 = freeze, 5 = loop, 7 = roll,  8 = jump/move, 9 = fx1, 10 = fx2, 11 = tone
    readonly property int		recordButton: 8
    readonly property int		samplesButton: 4
    readonly property int		muteButton: 7
    readonly property int		stemsButton: 5
    readonly property int		cueButton: 11
    readonly property int		fxLeftButton: 9
    readonly property int		fxRightButton: 10

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////////
	//BPM/TEMPO DISPLAY SETTINGS//
	//////////////////////////////

	//Change to true to always show tempo/bpm info
    readonly property bool 		alwaysShowTempoInfo: engine.getSetting('alwaysShowTempoInfo') || false

	//amount of time the bpm overlay will stay on the screen in ms. 1000 = 1 second.
    readonly property int 		bpmOverlayTimer: parseInt(engine.getSetting('bpmOverlayTimer')) || 5000

	//0 = Hidden, 1 = Master BPM, 2 = BPM, 3 = Tempo, 4 = BPM Offset, 5 = Tempo Offset, 6 = Master Deck Letter, 7 = Tempo Range, 8 = Key, 9 = Original BPM
    readonly property int 		tempoDisplayLeft: parseInt(engine.getSetting('tempoDisplayLeft')) || 2
    readonly property int 		tempoDisplayCenter: parseInt(engine.getSetting('tempoDisplayCenter')) || 1
    readonly property int 		tempoDisplayRight: parseInt(engine.getSetting('tempoDisplayRight')) || 3
    readonly property int 		tempoDisplayLeftShift: parseInt(engine.getSetting('tempoDisplayLeftShift')) || 4
    readonly property int 		tempoDisplayCenterShift: parseInt(engine.getSetting('tempoDisplayCenterShift')) || 6
    readonly property int 		tempoDisplayRightShift: parseInt(engine.getSetting('tempoDisplayRightShift')) || 5

	//set to true to enable the text colour to aid with your mixing.
    readonly property bool 		enableBpmTextColor: engine.getSetting('enableBpmTextColor') || false
    readonly property bool 		enableMasterBpmTextColor: engine.getSetting('enableMasterBpmTextColor') || false
    readonly property bool 		enableTempoTextColor: engine.getSetting('enableTempoTextColor') || false
    readonly property bool 		enableBpmOffsetTextColor: engine.getSetting('enableBpmOffsetTextColor') || false
    readonly property bool 		enableTempoOffsetTextColor: engine.getSetting('enableTempoOffsetTextColor') || false
    readonly property bool 		enableMasterDeckTextColor: engine.getSetting('enableMasterDeckTextColor') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////
	//WAVEFORM SETTINGS//
	/////////////////////

	//change to true to disable the moving waveforms
    readonly property bool 		hideWaveforms: engine.getSetting('hideWaveforms') || false

	//Change to false to hide loop size indicator (after 10 seconds of loop inactivity)
    readonly property bool 		alwaysShowLoopSize: engine.getSetting('alwaysShowLoopSize') || false

	//amount of time the loop overlay will stay on the screen in ms. 1000 = 1 second.
    readonly property int 		loopOverlayTimer: engine.getSetting('loopOverlayTimer') || 10000

	//set to true to hide the beatgrid
    readonly property bool hideBeatgrid: engine.getSetting('hideBeatgrid') || false

	//this value is the visibility of the beatgrid lines in %. Values are 0 to 100
    readonly property int beatgridVisibility: engine.getSetting('beatgridVisibility') || 25

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////
	//BROWSER SETTINGS//
	////////////////////

	//set to false to disable browser view and pads
    readonly property bool 		enableBrowserMode: engine.getSetting('enableBrowserMode') || true

	//set to false to disable the adjacent key colouring and return to all keys coloured
    readonly property bool		adjacentKeys: engine.getSetting('adjacentKeys') || true

	//change to true to enable camelot key
    readonly property bool 		camelotKey:	engine.getSetting('camelotKey') || false

	//set to true to disable the preview player toggle button and change it back to hold
    readonly property bool 		disablePreviewPlayerToggle: engine.getSetting('disablePreviewPlayerToggle') || false

	//Set to false to disable browser on screen when pressing favorites button
    readonly property bool 		showBrowserOnFavourites: engine.getSetting('showBrowserOnFavourites') || true

	//set to true to swap the functions of the view and add to prep buttons
    readonly property bool 		swapViewButtons: engine.getSetting('swapViewButtons') || false

	//Set to false to disable browser on screen when open in full screen mode.
	//This will also revert the view and prep button functions back to default except opening the browser on the S4 instead of the laptop.
    readonly property bool 		showBrowserOnFullScreen: engine.getSetting('showBrowserOnFullScreen') || true

	//set to true to disable the led output on the browser sort buttons
    readonly property bool 		disableSortButtonOutput: engine.getSetting('disableSortButtonOutput') || false

	// 1 = "Sort By #", 2 = "Title", 3 = "Artist", 4 = "Time", 5 = "BPM", 6 = "Track #", 7 = "Release", 8 = "Label", 9 = "Genre", 10 = "Key Text", 11 = "Comment", 12 = "Lyrics", 13 = "Comment 2", 14 = "Path", 15 = "Analysed"
	// 16 = "Remixer", 17 = "Producer", 18 = "Mix", 19 = "CAT #", 20 = "Release Date", 21 = "Bitrate", 22 = "Rating", 23 = "Count", 24 = "Sort By #", 25 = "Cover Art", 26 = "Last Played", 27 = "Import Date", 28 = "Key", 29 = "Color"
    readonly property int 		hotcueButtonSort: parseInt(engine.getSetting('hotcueButtonSort')) || 2
    readonly property int 		recordButtonSort: parseInt(engine.getSetting('recordButtonSort')) || 3
    readonly property int 		samplesButtonSort: parseInt(engine.getSetting('samplesButtonSort')) || 5
    readonly property int 		muteButtonSort: parseInt(engine.getSetting('muteButtonSort')) || 28
    readonly property int 		stemsButtonSort: parseInt(engine.getSetting('stemsButtonSort')) || 22

	//Change this setting to true to change the browser encoder to a list scroll when holding shift.
    readonly property bool 		browserEncoderShiftScroll: engine.getSetting('browserEncoderShiftScroll') || false

	//This is the size of the page scroll.
    readonly property int 		scrollPageSize: parseInt(engine.getSetting('scrollPageSize')) || 6

	//change to false to disable the browser view displaying artist data whilst holding shift
    readonly property bool 		browserShift: engine.getSetting('browserShift') || true

	//only enable when both artist and title columns are shown
    readonly property bool 		swapArtistTitleColumns: engine.getSetting('swapArtistTitleColumns') || false

    readonly property bool 		hideBPM: engine.getSetting('hideBPM') || false
    readonly property bool 		hideKey: engine.getSetting('hideKey') || false
    readonly property bool		hideAlbumArt: engine.getSetting('hideAlbumArt') || false
    readonly property bool 		showArtistColumn: engine.getSetting('showArtistColumn') || false
    readonly property bool 		showTrackTitleColumn: engine.getSetting('showTrackTitleColumn') || true
    readonly property int 		browserFontSize: parseInt(engine.getSetting('browserFontSize')) || 15
    readonly property bool 		raiseBrowserFooter: engine.getSetting('raiseBrowserFooter') || false

	//change the values below to determine the bpm text colour in the browser
	//the number values represent the percentage difference of the master tempo and the selected song
    readonly property bool 		bpmBrowserTextColor: engine.getSetting('bpmBrowserTextColor') || true
    readonly property int 		browserBpmGreen: 3
    readonly property int 		browserBpmRed: 12

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////
	//STRIPE SETTINGS//
	///////////////////

	//change to true to hide stripe
    readonly property bool 		hideStripe: engine.getSetting('hideStripe') || false

	//change to false to disable the switching of stripes when holding shift
    readonly property bool 		shiftStripe: engine.getSetting('shiftStripe') || false

	//change to true to show the master deck's stripe when holding shift (shiftStripe must be set to true)
    readonly property bool 		shiftStripeMaster: engine.getSetting('shiftStripeMaster') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////
	//TIME/BEATS BOX SETTINGS//
	///////////////////////////

	// 0 = Remaining Time, 1 = Elapsed Time, 2 = Time To Cue, 3 = Beats (0.0.0), 4 = Beats Alt (0.0), 5 = Beats To Cue (0.0.0), 6 = Beats To Cue Alt (0.0)
    readonly property int 		timeBox: parseInt(engine.getSetting('timeBox')) || 0
    readonly property int 		timeBoxShift: parseInt(engine.getSetting('timeBoxShift')) || 1

	//set to true to have the time text change to black when the box is red.
    readonly property bool		timeTextColourChange: engine.getSetting('timeTextColourChange') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////
	//PHASE & PHRASE METER SETTINGS//
	/////////////////////////////////

	//0 = Red, 1 = Dark Orange, 2 = Light Orange, 3 = Default, 4 = Yellow, 5 = Lime, 6 = Green, 7 = Mint, 8 = Cyan, 9 = Turquoise, 10 = Blue, 11 = Plum, 12 = Violet, 13 = Purple, 14 = Magenta, 15 = Fuchsia, 16 = White, 17 = Warm Yellow
    readonly property int 		phaseAColour:	parseInt(engine.getSetting('phaseAColour')) || 3
    readonly property int 		phaseBColour:	parseInt(engine.getSetting('phaseBColour')) || 3
    readonly property int 		phaseCColour:	parseInt(engine.getSetting('phaseCColour')) || 3
    readonly property int 		phaseDColour:	parseInt(engine.getSetting('phaseDColour')) || 3

	//change to true to hide the phase meter
    readonly property bool 		hidePhase: engine.getSetting('hidePhase') || false

	//change to true to hide the phrase meter
    readonly property bool		hidePhrase: engine.getSetting('hidePhrase') || true

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////
	//GRID EDIT SETTINGS//
	//////////////////////

	//change to false to hide the bpm overlay when in grid adjust mode.
    readonly property bool 		showBPMGridAdjust: engine.getSetting('showBPMGridAdjust') || true
    readonly property bool 		rateAdjustTimer: engine.getSetting('rateAdjustTimer') || 2000

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////
	//HOTCUE SETTINGS//
	///////////////////

	//set to true to disable the hotcue overlay appearing
    readonly property bool 		hideHotcueOverlay: engine.getSetting('hideHotcueOverlay') || false

	//0 = Red, 1 = Dark Orange, 2 = Light Orange, 3 = White, 4 = Yellow, 5 = Lime, 6 = Green, 7 = Mint, 8 = Cyan, 9 = Turquoise, 10 = Blue, 11 = Plum, 12 = Violet, 13 = Purple, 14 = Magenta, 15 = Fuchsia, 16 = Warm Yellow
	//change these values to change default cue type colours.
	//This will change the cue markers and also the loop indicator.
    readonly property int 		cueCueColour: parseInt(engine.getSetting('cueCueColour')) || 10
    readonly property int 		cueLoopColour: parseInt(engine.getSetting('cueLoopColour')) || 6

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	////////////////////
	//EFFECTS SETTINGS//
	////////////////////

	//change to false to disable FX overlays
    readonly property bool 		fxOverlays: engine.getSetting('fxOverlays') || true

	//amount of time the fx overlay will stay on the screen in ms. 1000 = 1 second.
    readonly property int 		fxOverlayTimer: parseInt(engine.getSetting('fxOverlayTimer')) || 2000

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////
	//EFFECTS PAD 1 SETTINGS//
	//////////////////////////

	//set to true to disable the effects pads 1 overlay appearing
    readonly property bool		hideEffectsOverlay1: engine.getSetting('hideEffectsOverlay1') || false

	//The fx unit used by fx pads 1
    readonly property int 		fx1unit: 1

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//////////////////////////
	//EFFECTS PAD 2 SETTINGS//
	//////////////////////////

	//set to true to disable the effects pads 2 overlay appearing
    readonly property bool		hideEffectsOverlay2: engine.getSetting('hideEffectsOverlay2') || false

	//The fx unit used by fx pads 2
    readonly property int fx2unit: 2

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////
	//TONE PAD SETTINGS//
	/////////////////////

	//set to true to disable the tone pads overlay appearing
    readonly property bool		hideToneOverlay: engine.getSetting('hideToneOverlay') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////
	//JUMP PAD SETTINGS//
	/////////////////////

	//set to true to disable the tone pads overlay appearing
    readonly property bool		hideJumpOverlay: engine.getSetting('hideJumpOverlay') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////
	//LOOP PAD SETTINGS//
	/////////////////////

	//set to true to disable the loop pads overlay appearing
    readonly property bool		hideLoopOverlay: engine.getSetting('hideLoopOverlay') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////
	//ROLL PAD SETTINGS//
	/////////////////////

	//set to true to disable the tone pads overlay appearing
    readonly property bool		hideRollOverlay: engine.getSetting('hideRollOverlay') || false

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
