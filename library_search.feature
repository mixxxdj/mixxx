Feature: Library Search

  The Mixxx library provides a token-based, multi-criteria search bar. Instead of
  a single free-text field, the user builds a structured query out of field/value
  criteria (e.g. `Artist: art    Key: 10b`) rendered as blue pill-shaped tokens.
  The search bar is a floating, light-gray panel anchored to the bottom-right of
  the library pane; it expands vertically when activated to reveal a suggestion
  dropdown and recent searches. The feature is keyboard-first (Ctrl+F, arrows,
  Tab, Enter, Escape, Backspace, Delete, `=`) and touch-friendly (tappable tokens
  and suggestions, per-token remove affordance), and the Android soft keyboard is
  kept open across token edits.

  Background:
    Given a new library-ready profile
    And Mixxx is open and ready to operate
    And the library has the following tracks:
      | Artist           | Title                | BPM | Key |
      | A Super Artist   | An amazing track     | 100 | 10b |
      | A Super DJ       | Another amazing track | 110 | 8a  |
      | A Delicious Tart | A tasty track        | 120 | 4a  |
      | BLACKPINK        | JUMP                 | 130 | 10b |

  # ---------------------------------------------------------------------------
  # Search bar appearance and empty state
  # ---------------------------------------------------------------------------

  Scenario: Search bar is present in the library pane
    Then the search bar should be visible

  Scenario: Search bar is anchored to the bottom-right of the library pane
    Then the search bar should be aligned to the bottom-right of the library pane

  Scenario: Search bar has a light-gray panel background
    Then the search bar background should be the light-gray search panel color

  Scenario: Empty search bar shows the placeholder
    Then the search placeholder "Search..." should be visible

  Scenario: Empty search bar shows a search icon on the right
    Then the search icon should be visible at the right of the search bar

  Scenario: Focusing the search bar activates it
    When I focus the search bar
    Then the search bar should be activated
    And the search placeholder "Start typing to get suggestion" should be visible

  Scenario: Activating the search bar expands it
    When I focus the search bar
    Then the search bar should be expanded

  Scenario: Focusing away deactivates the search bar
    Given I focus the search bar
    When I focus the library results
    Then the search bar should not be activated
    And the search bar should be collapsed

  # ---------------------------------------------------------------------------
  # Recent searches (empty state)
  # ---------------------------------------------------------------------------

  Scenario: Recent searches are listed when the search bar is empty
    Given the following searches were recently performed:
      | Artist: A Super Artist                     |
      | Artist: super artist    BPM: 100           |
    When I focus the search bar
    Then the "Recent searches" section should be visible
    And the recent search "Artist: A Super Artist" should be visible
    And the recent search "Artist: super artist    BPM: 100" should be visible

  Scenario: Clicking a recent search restores the query
    Given the following searches were recently performed:
      | Artist: A Super Artist |
    When I focus the search bar
    And I click the recent search "Artist: A Super Artist"
    Then the search query should be "Artist: A Super Artist"
    And the criteria should include the field "Artist" with value "A Super Artist"

  Scenario: A recent search can be selected with the keyboard
    Given the following searches were recently performed:
      | Artist: A Super Artist |
      | BLACKPINK              |
    When I focus the search bar
    And I press the "Down" key
    And I press the "Enter" key
    Then the search query should be "Artist: A Super Artist"

  # ---------------------------------------------------------------------------
  # Field name typing and field autocomplete
  # ---------------------------------------------------------------------------

  Scenario: Typing a field name shows field suggestions
    When I focus the search bar
    And I type "art"
    Then the field suggestion "Artist" should be visible

  Scenario: Typing a field name shows the Tab hint
    When I focus the search bar
    And I type "art"
    Then the hint "Press Tab to search for an Artist" should be visible

  Scenario: Typing filters field suggestions
    When I focus the search bar
    And I type "art"
    Then the field suggestion "Artist" should be visible
    And the field suggestion "Album" should not be visible

  Scenario: Tab accepts the suggested field criterion
    When I focus the search bar
    And I type "art"
    And I press the "Tab" key
    Then the criteria should include the field "Artist"
    And the search input should be empty

  Scenario: Typing an unknown field shows no field suggestion
    When I focus the search bar
    And I type "zzz"
    Then no field suggestion should be visible

  # ---------------------------------------------------------------------------
  # Field selected, awaiting value
  # ---------------------------------------------------------------------------

  Scenario: Selecting a field creates an active token
    When I focus the search bar
    And I type "art"
    And I press the "Tab" key
    Then a token for the field "Artist" should be visible
    And the token for the field "Artist" should be active

  Scenario: Field token awaits its value
    When I focus the search bar
    And I type "art"
    And I press the "Tab" key
    Then the input focus should be in the value part of the "Artist" token

  Scenario: Exact-match hint is shown while awaiting a value
    When I focus the search bar
    And I type "art"
    And I press the "Tab" key
    Then the hint "Press = for an exact match" should be visible

  Scenario: Exact-match hint is visually distinct from the editable value
    When I focus the search bar
    And I type "art"
    And I press the "Tab" key
    Then the hint "Press = for an exact match" should be styled as a hint

  # ---------------------------------------------------------------------------
  # Value entry and value autocomplete
  # ---------------------------------------------------------------------------

  Scenario: Typing an artist value filters suggestions
    Given the field "Artist" is an active search criterion
    When I type "art"
    Then the suggestion "A Super Artist" should be visible
    And the suggestion "A Delicious Tart" should be visible

  Scenario: Selecting a suggestion applies it as the criterion value
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I select the suggestion "A Super Artist"
    Then the criteria should include the field "Artist" with value "A Super Artist"

  Scenario: Enter selects the highlighted suggestion
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I press the "Down" key
    And I press the "Enter" key
    Then the criteria should include the field "Artist" with value "A Super Artist"

  # ---------------------------------------------------------------------------
  # Adding more criteria with Tab
  # ---------------------------------------------------------------------------

  Scenario: Tab finalizes the current criterion and starts a new one
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I press the "Tab" key
    Then the criteria should include the field "Artist" with value "art"
    And a new empty criterion input should be active

  Scenario: Tab hint is shown after a value is entered
    Given the field "Artist" is an active search criterion
    When I type "art"
    Then the hint "Press Tab to add more criteria" should be visible

  Scenario: Multiple criteria can coexist and preserve insertion order
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I press the "Tab" key
    And I type "key"
    And I press the "Tab" key
    And I type "10b"
    Then the criteria should include the field "Artist" with value "art"
    And the criteria should include the field "Key" with value "10b"
    And the field "Artist" should appear before the field "Key"

  # ---------------------------------------------------------------------------
  # Key criterion and notation suggestions
  # ---------------------------------------------------------------------------

  Scenario: Typing a key value shows key suggestions with notation
    Given the field "Key" is an active search criterion
    When I type "B"
    Then the key suggestion "B" should be visible
    And the key suggestion "B" should show the notation "Traditional"

  Scenario: Typing a numeric key shows OpenKey and Lancelot suggestions
    Given the field "Key" is an active search criterion
    When I type "10"
    Then the key suggestion "10a" should be visible
    And the key suggestion "10b" should be visible
    And the key suggestion "10b" should show the notation "OpenKey"

  Scenario: Key suggestions are filtered by the typed input
    Given the field "Key" is an active search criterion
    When I type "10"
    Then the key suggestion "8a" should not be visible

  Scenario: Selecting a key suggestion applies it as the criterion value
    Given the field "Key" is an active search criterion
    When I type "10"
    And I select the key suggestion "10b"
    Then the criteria should include the field "Key" with value "10b"

  # ---------------------------------------------------------------------------
  # BPM criterion
  # ---------------------------------------------------------------------------

  Scenario: Typing a BPM value shows BPM suggestions
    Given the field "BPM" is an active search criterion
    When I type "10"
    Then the BPM suggestion "100" should be visible

  Scenario: Selecting a BPM suggestion applies it as the criterion value
    Given the field "BPM" is an active search criterion
    When I type "10"
    And I select the BPM suggestion "100"
    Then the criteria should include the field "BPM" with value "100"

  # ---------------------------------------------------------------------------
  # Exact match
  # ---------------------------------------------------------------------------

  Scenario: The equals prefix enables an exact match
    Given the field "Artist" is an active search criterion
    When I type "=A Super Artist"
    Then the criteria should include the field "Artist" with value "=A Super Artist"
    And the "Artist" criterion should be an exact match

  Scenario: A plain artist value is a partial match
    Given the field "Artist" is an active search criterion
    When I type "Super"
    Then the criteria should include the field "Artist" with value "Super"
    And the "Artist" criterion should not be an exact match

  # ---------------------------------------------------------------------------
  # Query execution and live filtering
  # ---------------------------------------------------------------------------

  Scenario: A single criterion filters the library results
    Given the field "Artist" is an active search criterion
    When I type "A Super Artist"
    And I wait for the search to settle
    Then the library results should only contain tracks where "Artist" contains "A Super Artist"

  Scenario: Multiple criteria are combined with AND
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I press the "Tab" key
    And I type "key"
    And I press the "Tab" key
    And I type "10b"
    And I wait for the search to settle
    Then the library results should only contain tracks where "Artist" contains "art" and "Key" matches "10b"

  Scenario: Results update as criteria are entered
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I wait for the search to settle
    Then the track "A Super Artist - An amazing track" should be visible in the results
    And the track "BLACKPINK - JUMP" should not be visible in the results

  Scenario: Multi-word values are quoted so they stay a single criterion
    Given the field "Artist" is an active search criterion
    When I type "A Super Artist"
    And I wait for the search to settle
    Then the criteria should include the field "Artist" with value "A Super Artist"

  # ---------------------------------------------------------------------------
  # Keyboard-first navigation
  # ---------------------------------------------------------------------------

  Scenario: Ctrl+F focuses the search bar
    When I press "Ctrl+F"
    Then the search bar should be activated
    And the input focus should be in the search bar

  Scenario: Arrow keys navigate the suggestions
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I press the "Down" key
    Then the suggestion "A Super Artist" should be highlighted
    When I press the "Down" key
    Then the suggestion "A Super DJ" should be highlighted
    When I press the "Up" key
    Then the suggestion "A Super Artist" should be highlighted

  Scenario: Escape closes the suggestion dropdown
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I press the "Escape" key
    Then the suggestion dropdown should not be visible

  Scenario: Escape moves focus back to the library results
    Given I focus the search bar
    When I press the "Escape" key
    Then the search bar should not be activated
    And the input focus should be in the library results

  Scenario: Left and Right arrows move between tokens
    Given the criteria include the field "Artist" with value "art"
    And the criteria include the field "Key" with value "10b"
    And the input focus is in the value part of the "Artist" token
    When I press the "Right" key
    Then the input focus should be in the value part of the "Key" token

  Scenario: Enter activates a token for editing
    Given the criteria include the field "Artist" with value "art"
    And the criteria include the field "Key" with value "10b"
    When I press the "Right" key
    And I press the "Enter" key
    Then the token for the field "Key" should be active

  # ---------------------------------------------------------------------------
  # Removing criteria
  # ---------------------------------------------------------------------------

  Scenario: Backspace on an empty value removes the previous token
    Given the field "Artist" is an active search criterion
    And I type "art"
    And I press the "Tab" key
    When I press the "Backspace" key
    Then the criteria should not include the field "Artist"

  Scenario: Delete removes the active token
    Given the criteria include the field "Artist" with value "art"
    And the token for the field "Artist" is active
    When I press the "Delete" key
    Then the criteria should not include the field "Artist"

  Scenario: A token can be removed with its remove affordance
    Given the criteria include the field "Artist" with value "art"
    When I click the remove button on the "Artist" token
    Then the criteria should not include the field "Artist"

  # ---------------------------------------------------------------------------
  # Save and clear
  # ---------------------------------------------------------------------------

  Scenario: The save icon is shown when a search is active
    Given the criteria include the field "Artist" with value "art"
    Then the save search button should be visible

  Scenario: Saving a search adds it to the recent searches
    Given the criteria include the field "Artist" with value "art"
    And the criteria include the field "Key" with value "10b"
    When I click the save search button
    Then the recent search "Artist: art    Key: 10b" should be visible

  Scenario: The close icon is shown when a search is active
    Given the criteria include the field "Artist" with value "art"
    Then the close search button should be visible

  Scenario: Closing a search clears the criteria and restores the library
    Given the criteria include the field "Artist" with value "art"
    And I wait for the search to settle
    When I click the close search button
    Then the search bar should be empty
    And the library results should not be filtered

  # ---------------------------------------------------------------------------
  # Touch-only experience
  # ---------------------------------------------------------------------------

  Scenario: Tapping a token activates it for editing
    Given the criteria include the field "Artist" with value "art"
    And the criteria include the field "Key" with value "10b"
    When I tap the "Key" token
    Then the token for the field "Key" should be active

  Scenario: Tapping a suggestion selects it
    Given the field "Artist" is an active search criterion
    When I type "art"
    And I tap the suggestion "A Super Artist"
    Then the criteria should include the field "Artist" with value "A Super Artist"

  # ---------------------------------------------------------------------------
  # Android soft keyboard
  # ---------------------------------------------------------------------------

  @android
  Scenario: The soft keyboard stays open across token edits
    Given the criteria include the field "Artist" with value "art"
    And I press the "Tab" key
    Then the soft keyboard should remain visible

  @android
  Scenario: The soft keyboard does not enter extracted-text mode
    When I focus the search bar
    And I type "art"
    Then the soft keyboard should not be in fullscreen extracted-text mode

  @android
  Scenario: The soft keyboard is dismissed when the search is closed
    Given the criteria include the field "Artist" with value "art"
    When I click the close search button
    Then the soft keyboard should be hidden
