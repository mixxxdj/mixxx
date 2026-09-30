Feature: Interface Settings
#
# E2E coverage for the "Interface" preferences category
# (res/qml/Settings/Interface.qml).
#
# Tabs:
#   - "theme & color" : appearance, library and color settings
#   - "waveform"      : waveform type & preview options, rendering sliders,
#                       play marker hints and visual gain settings
#   - "decks"         : cue, track time, speed & key settings
#
# Step texts are behavior-oriented per the settings step DSL. The save, cancel
# and reset button steps are shared with sound.feature and resolve the button
# for whichever category is active. New/kind-aware setting steps will be
# implemented in steps/settings_steps.py (SETTING_MAP), backed by new QML
# objectNames in Interface.qml and new getConfigValue/setConfigValue commands.
#

  Background:
    Given a new library-ready profile
    And I have the following sound devices
      | name           | api  | outputChannels | inputChannels |
      | Built-in Audio | Mock | 4              | 4             |
    And Mixxx is open and ready to operate
    And the settings popup is open
    And the "Interface" category is selected

  # --- Tab navigation ---------------------------------------------------------

  Scenario: Interface tabs are visible
    Then the "theme & color" tab should be visible
    And the "waveform" tab should be visible
    And the "decks" tab should be visible

  Scenario: The theme & color tab is selected by default
    Then the "theme & color" tab should be selected

  Scenario: Switching to the waveform tab
    When I click the "waveform" tab
    Then the "waveform" tab should be selected

  Scenario: Switching to the decks tab
    When I click the "decks" tab
    Then the "decks" tab should be selected

  Scenario: Switching back to the theme & color tab
    Given the "decks" tab is selected
    When I click the "theme & color" tab
    Then the "theme & color" tab should be selected

  # --- Save / Cancel / Reset buttons -------------------------------------------

  Scenario: Save button is disabled when no changes
    Then the save button should be disabled
    And the cancel button should be disabled

  Scenario: Save and cancel enable when setting changes
    Given the "tool tips" setting is "all"
    When I toggle the "tool tips" setting to "off"
    Then the save button should be enabled
    And the cancel button should be enabled

  Scenario: Cancel reverts changes
    Given the "tool tips" setting is "all"
    When I toggle the "tool tips" setting to "off"
    And I click the cancel button
    Then the "tool tips" setting should be "all"
    And the save button should be disabled

  Scenario: Save applies changes
    When I toggle the "tool tips" setting to "off"
    And I click the save button
    Then the "tool tips" setting should be "off"
    And the save button should be disabled

  @xfail
  Scenario: Reset restores default settings
    Given the "tool tips" setting is "all"
    When I click the reset button
    Then the "tool tips" setting should be "library"

  # --- Theme & color tab: appearance ------------------------------------------

  Scenario: Skin, Color and Layout choices are displayed
    # Save is stubbed (skinInput/colorInput/layoutInput are commented out in
    # loadInterface/saveInterface), so only the display is asserted.
    Then the "skin" setting should be "Unnamed"
    And the "color" setting should be "Dark"
    And the "layout" setting should be "Performance"

  Scenario: Tool tips can be switched between all levels
    When I toggle the "tool tips" setting to "off"
    And I click the save button
    Then the "tool tips" setting should be "off"
    When I toggle the "tool tips" setting to "library"
    And I click the save button
    Then the "tool tips" setting should be "library"
    When I toggle the "tool tips" setting to "all"
    And I click the save button
    Then the "tool tips" setting should be "all"

  Scenario: Screen saver can be disabled
    When I toggle the "disable screen saver" setting to "no"
    And I click the save button
    Then the "disable screen saver" setting should be "no"
    And the "disable screen saver" should be saved as "no"

  Scenario: Screen saver can be limited to playback
    When I toggle the "disable screen saver" setting to "while playing"
    And I click the save button
    Then the "disable screen saver" setting should be "while playing"

  Scenario: Start in full-screen mode can be enabled
    When I toggle the "start in full-screen mode" setting to "on"
    And I click the save button
    Then the "start in full-screen mode" setting should be "on"

  Scenario: Auto-hide the menu bar can be enabled
    When I toggle the "auto-hide the menu bar" setting to "on"
    And I click the save button
    Then the "auto-hide the menu bar" setting should be "on"

  # --- Theme & color tab: library ----------------------------------------------

  Scenario: Search completion can be disabled
    When I toggle the "search completion" setting to "off"
    And I click the save button
    Then the "search completion" setting should be "off"

  Scenario: Search history keyboard shortcuts can be disabled
    When I toggle the "search history keyboard shortcuts" setting to "off"
    And I click the save button
    Then the "search history keyboard shortcuts" setting should be "off"

  Scenario: BPM display precision can be changed
    When I set the "bpm display precision" setting to "3"
    Then the "bpm display precision" setting should be "3"

  Scenario: Library row height can be changed
    When I set the "library row height" setting to "50" with a drag
    Then the "library row height" setting should be "50"

  Scenario: Search finds interface settings
    When I search for "Row Height"
    Then the search results should be visible

  # --- Theme & color tab: colors ----------------------------------------------

  Scenario: Key color palette is disabled when key color is off
    # Component state: keyPaletteComboBox.enabled is bound to key color == "on"
    When I toggle the "key color" setting to "off"
    And I click the save button
    Then the "key color palette" setting should be disabled
    When I toggle the "key color" setting to "on"
    And I click the save button
    Then the "key color palette" setting should be enabled

  Scenario Outline: Track palette can be changed
    # The second palette is beyond the popup's visible rows (popupMaxItem = 6)
    # and exercises the keyboard-selection fallback for off-screen options.
    # Persistence is asserted in the next scenario with a non-default palette:
    # Mixxx removes a config key that equals its default, so the default
    # palette ("Mixxx Track Colors") would read back as empty.
    When I set the "track palette" setting to "<palette>"
    And I click the save button
    Then the "track palette" setting should be "<palette>"

    Examples:
      | palette                |
      | Mixxx Track Colors     |
      | Rekordbox Track Colors |

  Scenario: Track palette selection is persisted
    When I set the "track palette" setting to "Rekordbox Track Colors"
    And I click the save button
    Then the "track palette" should be saved

  Scenario Outline: Hotcue palette can be changed
    When I set the "hotcue palette" setting to "<palette>"
    Then the "hotcue palette" setting should be "<palette>"

    Examples:
      | palette                |
      | Mixxx Hotcue Colors    |
      | Rekordbox COLD2 Hotcue Colors |

  Scenario: Hotcue default color can be changed
    When I set the "hotcue default color" setting to "3"
    Then the "hotcue default color" setting should be "3"

  Scenario: Loop default color can be changed
    When I set the "loop default color" setting to "4"
    Then the "loop default color" setting should be "4"

  # --- Decks tab ----------------------------------------------------------------

  Scenario: Cue mode can be changed
    Given the "decks" tab is selected
    When I set the "cue mode" setting to "Pioneer"
    And I click the save button
    Then the "cue mode" setting should be "Pioneer"

  Scenario: Intro start can be set to the main cue when analyzing
    Given the "decks" tab is selected
    When I toggle the "set intro start to main cue" setting to "off"
    And I click the save button
    Then the "set intro start to main cue" setting should be "off"

  Scenario: Track time display can be changed
    Given the "decks" tab is selected
    When I toggle the "track time display" setting to "remaining"
    And I click the save button
    Then the "track time display" setting should be "remaining"

  Scenario: Time format can be changed
    Given the "decks" tab is selected
    When I set the "time format" setting to "Seconds"
    Then the "time format" setting should be "Seconds"

  Scenario: Double-press load to clone can be disabled
    Given the "decks" tab is selected
    When I toggle the "double-press load to clone" setting to "off"
    And I click the save button
    Then the "double-press load to clone" setting should be "off"

  Scenario: Track load point can be changed
    Given the "decks" tab is selected
    When I set the "track load point" setting to "First hotcue"
    Then the "track load point" setting should be "First hotcue"

  Scenario: Loading a track when playing can be allowed
    Given the "decks" tab is selected
    When I toggle the "loading a track when playing" setting to "allow"
    And I click the save button
    Then the "loading a track when playing" setting should be "allow"

  Scenario: Speed reset on track load can be changed
    Given the "decks" tab is selected
    When I toggle the "reset on track load" setting to "tempo"
    And I click the save button
    Then the "reset on track load" setting should be "tempo"

  Scenario: Sync mode can be changed
    Given the "decks" tab is selected
    When I toggle the "sync mode" setting to "use steady"
    And I click the save button
    Then the "sync mode" setting should be "use steady"

  Scenario: Keylock mode can be changed
    Given the "decks" tab is selected
    When I toggle the "keylock mode" setting to "current key"
    And I click the save button
    Then the "keylock mode" setting should be "current key"

  Scenario: Keyunlock mode can be changed
    Given the "decks" tab is selected
    When I toggle the "keyunlock mode" setting to "keep key"
    And I click the save button
    Then the "keyunlock mode" setting should be "keep key"

  Scenario: Pitch bend behaviour can be changed
    Given the "decks" tab is selected
    When I toggle the "pitch bend behaviour" setting to "smooth ramping"
    And I click the save button
    Then the "pitch bend behaviour" setting should be "smooth ramping"

  Scenario: Adjustment button ranges can be changed
    Given the "decks" tab is selected
    When I set the "temporary coarse adjustment" setting to "4.02"
    And I set the "permanent fine adjustment" setting to "0.06"
    Then the "temporary coarse adjustment" setting should be "4.02"
    And the "permanent fine adjustment" setting should be "0.06"

  Scenario: Ramping sensitivity can be changed
    Given the "decks" tab is selected
    When I set the "ramping sensitivity" setting to "500" with the spinbox
    Then the "ramping sensitivity" setting should be "500"

  # --- Decks tab: ControlObject side-effects ------------------------------------

  Scenario: Slider range is applied to the deck rate range control
    Given the "decks" tab is selected
    When I set the "slider range" setting to "24%"
    And I click the save button
    Then the "slider range" setting should be "24%"
    And the "slider range" should be saved as "24%"
    And the "slider range" on deck 1 should be "24%"
    When I set the "slider range" setting to "16%"
    And I click the save button
    Then the "slider range" setting should be "16%"
    And the "slider range" should be saved as "16%"
    And the "slider range" on deck 1 should be "16%"

  Scenario: Slider orientation is applied to the deck rate direction control
    Given the "decks" tab is selected
    When I toggle the "slider orientation" setting to "down"
    And I click the save button
    Then the "slider orientation" setting should be "down"
    And the "slider orientation" should be saved as "down"
    And the "slider orientation" on deck 1 should be "down"
    When I toggle the "slider orientation" setting to "up"
    And I click the save button
    Then the "slider orientation" setting should be "up"
    And the "slider orientation" should be saved as "up"
    And the "slider orientation" on deck 1 should be "up"

  # --- Waveform tab ---------------------------------------------------------------
  #
  # These scenarios only cover the settings widgets and their backend config
  # effect: each change is persisted with the save button and its config key
  # is read back through the [Waveform] group. The visual side effects on the
  # live waveforms (preview and decks) are out of scope here and will be
  # covered by a dedicated feature file that presets the config keys and
  # asserts the rendered feedback.
  #
  # Defaults in this tab are read live from the config (Mixxx.Config), so a
  # value that equals its default (e.g. a reset gain of "100%") is stored as
  # the empty string and would read back flaky from [Waveform]. Scenarios
  # therefore save a non-default value and assert on it, like the decks tab
  # scenarios do.

  Scenario: Waveform tab has settings to display
    Given the settings popup is open
    And the "Interface" category is selected
    When I click the "waveform" tab
    Then the "waveform type" setting should be visible
    And the "end of track warning" setting should be visible
    And the "beat grid opacity" setting should be visible
    And the "default zoom level" setting should be visible
    And the "play marker position" setting should be visible
    And the "beats until next marker" setting should be visible
    And the "time until next marker" setting should be visible
    And the "marker hint placement" setting should be visible
    And the "marker hint font size" setting should be visible
    And the "global visual gain" setting should be visible
    And the "low visual gain" setting should be visible
    And the "medium visual gain" setting should be visible
    And the "high visual gain" setting should be visible

  Scenario: End of track warning can be reduced to zero
    # Zero disables the warning; a stored 0 is not the default (30), so the
    # config key round-trips.
    Given the "waveform" tab is selected
    When I set the "end of track warning" setting to "0" with the spinbox
    And I click the save button
    Then the "end of track warning" setting should be "0"
    And the "end of track warning" should be saved as "0"

  Scenario: End of track warning can be set to a marked value
    Given the "waveform" tab is selected
    When I set the "end of track warning" setting to "60" with the spinbox
    And I click the save button
    Then the "end of track warning" setting should be "60"
    And the "end of track warning" should be saved as "60"

  Scenario: Beat grid opacity can be changed
    Given the "waveform" tab is selected
    When I set the "beat grid opacity" setting to "50" with the spinbox
    And I click the save button
    Then the "beat grid opacity" setting should be "50"
    And the "beat grid opacity" should be saved as "50"
    When I set the "beat grid opacity" setting to "0" with the spinbox
    And I click the save button
    Then the "beat grid opacity" setting should be "0"
    And the "beat grid opacity" should be saved as "0"

  Scenario: Default zoom level can be changed
    # The slider displays zoom*10, so "50" is a zoom factor of 5.
    Given the "waveform" tab is selected
    When I set the "default zoom level" setting to "50" with the spinbox
    And I click the save button
    Then the "default zoom level" setting should be "50"
    And the "default zoom level" should be saved as "5"
    When I set the "default zoom level" setting to "10" with the spinbox
    And I click the save button
    Then the "default zoom level" setting should be "10"
    And the "default zoom level" should be saved as "1"

  Scenario: Zoom level is synchronised by default
    Given the "waveform" tab is selected
    Then the "synchronise zoom level across waveforms" setting should be "on"

  Scenario: Zoom synchronisation can be disabled
    Given the "waveform" tab is selected
    When I toggle the "synchronise zoom level across waveforms" setting to "off"
    And I click the save button
    Then the "synchronise zoom level across waveforms" setting should be "off"
    And the "synchronise zoom level across waveforms" should be saved as "off"

  Scenario: Overview normalisation can be disabled
    Given the "waveform" tab is selected
    When I toggle the "normalise waveform overview" setting to "off"
    And I click the save button
    Then the "normalise waveform overview" setting should be "off"
    And the "normalise waveform overview" should be saved as "off"

  Scenario: Play marker position can be changed
    Given the "waveform" tab is selected
    When I set the "play marker position" setting to "25" with the spinbox
    And I click the save button
    Then the "play marker position" setting should be "25"
    And the "play marker position" should be saved as "0.25"

  # --- Waveform type & options ----------------------------------------------------

  Scenario Outline: Waveform type can be selected
    # The ComboBox popup renders a preview of each type (more than six
    # entries would need the keyboard fallback), so all five stay clickable.
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "<type>"
    And I click the save button
    Then the "waveform type" setting should be "<type>"

    Examples:
      | type     |
      | Filtered |
      | HSV      |
      | RGB      |
      | Simple   |
      | Stacked  |

  Scenario: Waveform type selection is persisted
    # "should be saved as" reads back the [Waveform] WaveformType key and
    # translates the type name into its stored legacy WaveformWidgetType id
    # (translated in CONFIG_SAVE_MAP). The default RGB is removed from the
    # config (see the banner comment), so persistence is asserted with a
    # non-default type only.
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "Simple"
    And I click the save button
    Then the "waveform type" should be saved as "Simple"

  # The "Stereo split" and "High details" options shown under the preview
  # depend on the selected waveform type: they are only supported by some
  # renderers (RGB supports both, Filtered/Stacked support high details
  # only, HSV/Simple support none). The default type is RGB, so both are
  # visible out of the box.
  Scenario: The stereo split option is available on a supported waveform type
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "Simple"
    Then the "stereo split" setting should not be visible
    When I set the "waveform type" setting to "RGB"
    Then the "stereo split" setting should be visible

  Scenario: The high details option is available on a supported waveform type
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "HSV"
    Then the "high details" setting should not be visible
    When I set the "waveform type" setting to "RGB"
    Then the "high details" setting should be visible

  Scenario Outline: Waveform options can be toggled for the RGB type
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "RGB"
    And I toggle the "<option>" setting to "on"
    And I click the save button
    Then the "<option>" setting should be "on"
    And the "waveform options" should be saved as "<option>"
    When I toggle the "<option>" setting to "off"
    And I click the save button
    Then the "<option>" setting should be "off"
    And the "waveform options" should be saved as "none"

    Examples:
      | option        |
      | stereo split  |
      | high details  |

  Scenario: Beats until next marker can be shown
    Given the "waveform" tab is selected
    When I toggle the "beats until next marker" setting to "on"
    And I click the save button
    Then the "beats until next marker" setting should be "on"
    And the "beats until next marker" should be saved as "on"
    When I toggle the "beats until next marker" setting to "off"
    And I click the save button
    Then the "beats until next marker" setting should be "off"
    # "off" restores the default, so the config key is removed (see the
    # banner comment at the top of the section).
    And the "beats until next marker" config key should be unset

  Scenario: Time until next marker can be shown
    Given the "waveform" tab is selected
    When I toggle the "time until next marker" setting to "on"
    And I click the save button
    Then the "time until next marker" setting should be "on"
    And the "time until next marker" should be saved as "on"
    When I toggle the "time until next marker" setting to "off"
    And I click the save button
    Then the "time until next marker" setting should be "off"
    # "off" restores the default, so the config key is removed (see the
    # banner comment at the top of the section).
    And the "time until next marker" config key should be unset

  Scenario Outline: Marker hint placement can be changed
    Given the "waveform" tab is selected
    When I toggle the "marker hint placement" setting to "<placement>"
    And I click the save button
    Then the "marker hint placement" setting should be "<placement>"

    Examples:
      | placement |
      | top       |
      | center    |
      | bottom    |

  Scenario: Marker hint font size can be changed
    Given the "waveform" tab is selected
    When I set the "marker hint font size" setting to "32 pt"
    And I click the save button
    Then the "marker hint font size" setting should be "32 pt"
    And the "marker hint font size" should be saved as "32 pt"
    When I set the "marker hint font size" setting to "10 pt"
    And I click the save button
    Then the "marker hint font size" setting should be "10 pt"
    And the "marker hint font size" should be saved as "10 pt"

  Scenario Outline: Visual gain can be changed
    # Drags land within ~1-2px, so slider tolerance absorbs the drift and
    # the saved config (a gain factor = percent/100) is asserted exactly.
    # Defaults (100%) are excluded from the examples for the reason given
    # above.
    Given the "waveform" tab is selected
    When I set the "<gain>" setting to "<value>" with the spinbox
    And I click the save button
    Then the "<gain>" setting should be "<value>"
    And the "<gain>" should be saved as "<saved>"

    Examples:
      | gain               | value | saved |
      | global visual gain | 150   | 1.5   |
      | low visual gain    | 300   | 3     |
      | medium visual gain | 80    | 0.8   |
      | high visual gain   | 50    | 0.5   |

  # --- Waveform preview (embedded in the type combobox) ---------------------------
  #
  # The waveform type combobox embeds a live waveform preview whose renderers
  # are bound to the tab's controls, so every change made through a control
  # must show on the preview immediately (no save needed — the persistence of
  # the same settings is asserted by the scenarios above). The assertions read
  # back what the bound renderers actually consume (the preview's renderer
  # derived properties), not the inputs themselves.

  Scenario: The beat grid opacity reflects on the waveform preview
    Given the "waveform" tab is selected
    When I set the "beat grid opacity" setting to "30" with the spinbox
    Then the waveform preview beat grid should be 30% opaque
    When I set the "beat grid opacity" setting to "100" with the spinbox
    Then the waveform preview beat grid should be 100% opaque

  Scenario: The default zoom level reflects on the waveform preview
    Given the "waveform" tab is selected
    When I set the "default zoom level" setting to "50" with the spinbox
    Then the waveform preview should be at zoom 5
    When I set the "default zoom level" setting to "10" with the spinbox
    Then the waveform preview should be at zoom 1

  Scenario: The play marker position reflects on the waveform preview
    Given the "waveform" tab is selected
    When I set the "play marker position" setting to "25" with the spinbox
    Then the waveform preview play marker should sit at 25%
    When I set the "play marker position" setting to "75" with the spinbox
    Then the waveform preview play marker should sit at 75%

  Scenario: The until-marker hints reflect on the waveform preview
    Given the "waveform" tab is selected
    Then the waveform preview until-marker hint should not show beats
    And the waveform preview until-marker hint should not show the remaining time
    When I toggle the "beats until next marker" setting to "on"
    Then the waveform preview until-marker hint should show beats
    When I toggle the "time until next marker" setting to "on"
    Then the waveform preview until-marker hint should show the remaining time
    When I toggle the "time until next marker" setting to "off"
    Then the waveform preview until-marker hint should not show the remaining time

  Scenario Outline: The marker hint placement reflects on the waveform preview
    Given the "waveform" tab is selected
    When I toggle the "marker hint placement" setting to "<placement>"
    Then the waveform preview until-marker hint should be placed at the "<placement>"
    When I toggle the "marker hint placement" setting to "center"
    Then the waveform preview until-marker hint should be placed at the "center"

    Examples:
      | placement |
      | top       |
      | bottom    |

  Scenario Outline: The marker hint font size reflects on the waveform preview
    Given the "waveform" tab is selected
    When I set the "marker hint font size" setting to "<size> pt"
    Then the waveform preview until-marker hint should use a <size>pt font

    Examples:
      | size |
      | 10   |
      | 32   |

  Scenario Outline: The visual gains reflect on the waveform preview
    Given the "waveform" tab is selected
    When I set the "<gain>" setting to "<value>" with the spinbox
    Then the waveform preview should have the "<band>" band gain set to "<target>"
    When I set the "<gain>" setting to "100" with the spinbox
    Then the waveform preview should have the "<band>" band gain set to "1"

    Examples:
      | gain               | value | band   | target |
      | global visual gain | 150   | global | 1.5    |
      | low visual gain    | 300   | low    | 3      |
      | medium visual gain | 80    | middle | 0.8    |
      | high visual gain   | 50    | high   | 0.5    |

  Scenario: The waveform options reflect on the waveform preview
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "RGB"
    Then the waveform preview should render with the selected options
    When I toggle the "stereo split" setting to "on"
    Then the waveform preview should render with the selected options
    When I toggle the "high details" setting to "on"
    Then the waveform preview should render with the selected options

  @xfail
  Scenario: Reset restores default waveform settings
    Given the "waveform" tab is selected
    When I set the "beat grid opacity" setting to "40"
    And I click the reset button
    Then the "beat grid opacity" setting should be "90"
    And the "waveform type" setting should be "RGB"
    And the "marker hint font size" setting should be "24 pt"

  # --- Waveform colors --------------------------------------------------------------
  #
  # The low/mid/high swatches of the preview open a native ColorDialog,
  # which the harness cannot drive. They are routed to an invisible test
  # stand-in through the same pattern as the library category's folder
  # picker: while test mode is on, open() is counted, the color the test
  # set beforehand is accepted, and the picked swatch binds the color into
  # the preview renderers immediately (colors are preview-only state:
  # save() does not persist them, so no config or deck feedback is
  # asserted here).
  Scenario Outline: Waveform colors can be picked
    Given the "waveform" tab is selected
    When I set the "waveform type" setting to "<type>"
    And I pick the color "<color>" for the "<band>" waveform color
    Then the "<band>" color swatch should be "<color>"
    And the waveform preview should render the "<band>" band with "<color>"

    Examples:
      | type     | band | color     |
      | Filtered | low  | #f6de27 |
      | Filtered | mid  | #f07581 |
      | Filtered | high | #f95429 |
      | HSV      | all  | #d3a7f3 |
      | RGB      | low  | #b86ff5 |
      | RGB      | mid  | #69dc02 |
      | RGB      | high | #623b73 |
      | Simple   | all  | #ba7197 |
      | Stacked  | low  | #d3bf7d |
      | Stacked  | mid  | #d44685 |
      | Stacked  | high | #52122b |

  Scenario: Search finds waveform settings
    When I search for "visual gain"
    Then the search results should be visible

  # --- Responsiveness ------------------------------------------------------------
  #
  # On a short window the decks tab content overflows its ScrollView. A setting
  # below the fold must be reachable: scroll it into the on-screen area, then
  # prove it is interactable by changing its value.

  @category/responsiveness
  Scenario: Below-the-fold settings can be scrolled into view on a short window
    Given the "decks" tab is selected
    When I resize the window's height to 500px
    Then the "ramping sensitivity" setting should not be visible on screen
    When I scroll down in the settings
    Then the "ramping sensitivity" setting should be visible on screen
    When I set the "ramping sensitivity" setting to "600" with the spinbox
    Then the "ramping sensitivity" setting should be "600"
    When I resize the window's height to 1008px
