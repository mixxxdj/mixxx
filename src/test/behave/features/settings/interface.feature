Feature: Interface Settings
#
# E2E coverage for the "Interface" preferences category
# (res/qml/Settings/Interface.qml).
#
# Tabs:
#   - "theme & color" : appearance, library and color settings
#   - "waveform"      : placeholder (no options yet)
#   - "decks"         : cue, track time, speed & key settings
#
# Step texts are behavior-oriented per the settings step DSL. The save, cancel
# and reset button steps are shared with sound.feature and resolve the button
# for whichever category is active. New/kind-aware setting steps will be
# implemented in steps/settings_steps.py (SETTING_MAP), backed by new QML
# objectNames in Interface.qml and new getConfigValue/setConfigValue commands.
#

  Background:
    Given a new empty profile
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
