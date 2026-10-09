Feature: Library

  Background:
    Given a new library-ready profile
    And Mixxx is open and ready to operate
    And the window size is default
    And the library columns are in their default state

  Scenario: Library toggle button is visible
    When I check on the button "LIBRARY" in the main toolbar
    Then the library is shown for less than 75% of the Window's height

  Scenario: Maximize library with toggle
    When I click the "LIBRARY" button
    Then the library is shown for more than 75% of the Window's height

  Scenario: Library columns adapt to available width
    When I resize the window's width to 1500px
    Then only the column ",Preview,Title,Artist,Album,Year,BPM,Key,File Type,Bitrate" are shown
    When I resize the window's width to 1200px
    Then only the column ",Preview,Title,Artist,Album,Year,BPM,Key,File Type" are shown
    When I resize the window's width to 800px
    Then only the column "Title,Artist,BPM,Key" are shown

  Scenario: Library columns can be re-ordered
    When I drag the column "Artist" before the column "Title"
    Then the column "Artist" should appear before the column "Title"

  Scenario: Library columns can be shown when auto-hidden
    When I resize the window's width to 800px
    Then the column "Preview" should not be visible
    When I open the column picker menu
    And I toggle the column "Preview" in the column picker
    Then the column "Preview" should be visible

  Scenario: Library columns can be hidden
    When I resize the window's width to 1200px
    Then the column "Artist" should be visible
    When I open the column picker menu
    And I toggle the column "Artist" in the column picker
    Then the column "Artist" should not be visible

  Scenario: Library columns can be used to sort results
    When I click the column header "Title"
    Then the results should be sorted by "Title" in "ascending" order

  Scenario: Track can be selected
    When I click the track at row 1
    Then the track at row 1 should be selected

  Scenario: Track below the fold can be scrolled into view
    Given the library is maximized
    And the window's height is 500px
    When I click a track below the fold
    Then the track below the fold should be visible on screen

  Scenario: Track context menu can be shown on right click
    When I right-click the track at row 1
    Then the track context menu should be visible

  Scenario: Track context menu can be shown on long press
    When I long-press the track at row 1
    Then the track context menu should be visible

  # Flaky test on Xcb
  @xpass
  Scenario: Track can be loaded via double click
    Given no track is loaded on deck 1
    When I double-click the track at row 1
    Then a track is loaded on deck 1

  Scenario: Track can be loaded via context menu
    Given no track is loaded on deck 1
    When I right-click the track at row 1
    And I select "Load to" > "Deck" > "Deck 1" on the track menu
    Then a track is loaded on deck 1

  @category/responsiveness
  Scenario: Library collapses when the window is too short
    Given the library is not maximized
    When I resize the window's height to 650px
    Then the library should not be visible
    When I resize the window's height to 800px
    Then the library should be visible

  # --- Library search (floating search bar in the bottom right of the library) ---
  #
  # Requires objectNames on the search pane UI (res/qml/Library.qml):
  #   searchPane, searchField, searchClearButton, searchSuggestionFieldList
  #   (field suggestions, e.g. "Artist:"), searchSuggestionList (value
  #   suggestions), searchRecentList, searchPlaceholder,
  #   searchCollapsedPlaceholder, searchTabHint and the read-only properties
  #   activated, activeTokenIndex, criteriaCount, criteriaFields, activeQuery,
  #   suggestionTexts (value-suggestion texts, JSON list).
  # At the track list (res/qml/Library/TrackList.qml):
  #   trackTitleForRow(row), trackDataForRow(row)
  #
  # Search state must be declared per scenario: "no search is currently
  # active" resets leftover criteria/query, "the library search criteria
  # include the field ..." builds a deterministic token search.

  Scenario: Search bar opens and closes without filtering
    Given no search is currently active
    When I activate the library search
    Then the library search bar should be visible
    When I deactivate the library search
    Then all tracks in the library should be shown without a search filter

  Scenario: Search filters the track results
    Given no search is currently active
    And a track available in the library with a unique title
    When I activate the library search
    And I type "title:" into the library search
    And I type the title of this track into the library search prefixed with "="
    Then this track should be visible in the results
    And no other track should be visible in the results

  Scenario: Clearing the search shows all tracks again
    Given no search is currently active
    And a track available in the library
    When I activate the library search
    And I type the title of this track into the library search
    Then this track should be visible in the results
    When I clear the library search
    Then all tracks in the library should be shown again

  Scenario: Search suggests matching field names
    Given no search is currently active
    When I activate the library search
    And I type "art" into the library search
    Then the library search suggestion "Artist:" should be visible

  Scenario: A field suggestion creates a search token
    Given no search is currently active
    When I activate the library search
    And I type "art" into the library search
    And I press the "Down" key in the library search
    And I press the "Enter" key in the library search
    Then a search token "Artist" should be shown in the search bar

  Scenario: A committed search appears in recent searches
    Given no search is currently active
    And a track available in the library
    When I activate the library search
    And I type the title of this track into the library search
    And I deactivate the library search
    And I clear the library search
    When I activate the library search
    Then the library recent search "<title of this track>" should be visible

  Scenario: Search bar is present in the library pane
    Given no search is currently active
    Then the library search bar should be present
    And the search placeholder "Search..." should be visible

  Scenario: Activating the search bar expands it
    Given no search is currently active
    When I activate the library search
    Then the library search bar should be visible
    And the library search bar should be expanded

  Scenario: An activated search bar shows a suggestion placeholder
    Given no search is currently active
    When I activate the library search
    Then the search placeholder "Start typing to get suggestion" should be visible

  Scenario: Typing a field name shows the matching field suggestion and the Tab hint
    Given no search is currently active
    When I activate the library search
    And I type "art" into the library search
    Then the library search suggestion "Artist:" should be visible
    And the search hint "Press "Tab" to search for an Artist" should be visible

  Scenario: Typing filters the field suggestions
    Given no search is currently active
    When I activate the library search
    And I type "alb" into the library search
    Then the library search suggestion "Album:" should be visible
    And the library search suggestion "Artist:" should not be visible

  Scenario: Typing an unknown field shows no field suggestion
    Given no search is currently active
    When I activate the library search
    And I type "zzz" into the library search
    Then the library search suggestion "zzz:" should not be visible

  Scenario: Tab accepts the suggested field criterion and awaits its value
    Given no search is currently active
    When I activate the library search
    And I type "art" into the library search
    And I press the "Tab" key in the library search
    Then a search token "Artist" should be shown in the search bar
    And the search token "Artist" should be active
    And the search placeholder "Type "=" for an exact match" should be visible

  Scenario: A typed field-and-value query becomes a criterion token
    Given no search is currently active
    When I activate the library search
    And I type "artist:Rockot" into the library search
    Then a search token "Artist" should be shown in the search bar
    And the library search query should be "artist:Rockot"
    And the search hint "Press "Tab" to add more criteria" should be visible

  Scenario: Multiple criteria can coexist and preserve the insertion order
    Given no search is currently active
    When I activate the library search
    And I type "artist:Rockot" into the library search
    And I press the "Tab" key in the library search
    And I type "title:Drive" into the library search
    Then a search token "Artist" should be shown in the search bar
    And a search token "Title" should be shown in the search bar
    And the library search query should be "artist:Rockot title:Drive"

  Scenario: A pasted multi-criteria query becomes exact-match criteria
    Given no search is currently active
    And a track available in the library with a unique title
    When I paste "artist:="<artist of this track>" title:"<title of this track>"" into the library search
    Then the library search criteria should be "Artist,Title"
    And the library search free text should be empty

  Scenario: Pasting a query keeps its non-criteria words as free text
    Given no search is currently active
    And a track available in the library with a unique title
    When I paste "artist:"<artist of this track>" <title of this track>" into the library search
    Then the library search criteria should be "Artist"
    And the library search free text should be "<title of this track>"

  # Known gap: the chip search dialect cannot represent the OR operator yet;
  # a query mixing criteria and a bare "Or" word drops all criteria instead
  # of keeping them (will be fixed by the advanced raw-query editor).
  @xfail
  Scenario: A pasted query with an OR word keeps its criteria
    Given no search is currently active
    And a track available in the library with a unique title and with search operator in its metadata
    When I paste "artist:Rockot Baby Mandala | Nepalese Drill Music" into the library search
    Then the library search criteria should be "Artist"
    And the library search free text should be "Baby Mandala | Nepalese Drill Music"

  Scenario: The equals prefix enables an exact match
    Given no search is currently active
    When I activate the library search
    And I type "artist:" into the library search
    And I type "=Rockot" into the library search
    Then the library search query should be "artist:=Rockot"

  Scenario: Multi-word values are quoted so they stay a single criterion
    Given no search is currently active
    Given the library search criteria include the field "Artist" with the value "Please Calm My Mind"
    Then the library search query should be "artist:"Please Calm My Mind""

  Scenario: Typing an artist value shows matching value suggestions
    Given no search is currently active
    Given a track available in the library with a unique title
    When I activate the library search
    And I type "artist:" into the library search
    And I type the artist of this track into the library search
    Then the library search suggestion showing the artist of this track should be visible

  Scenario: Enter selects the highlighted value suggestion
    Given no search is currently active
    Given a track available in the library with a unique title
    When I activate the library search
    And I type "artist:" into the library search
    And I type the artist of this track into the library search
    And I press the "Down" key in the library search
    And I press the "Enter" key in the library search
    Then a search token "Artist" should be shown in the search bar
    And the library search query should contain the artist of this track

  Scenario: A key suggestion is shown with its notation and applies as a key criterion
    Given no search is currently active
    When I activate the library search
    And I type "key:" into the library search
    And I type "10" into the library search
    Then the library search suggestion "10B" should be visible
    And the library search suggestion "10d" should be visible
    And I press the "Down" key in the library search
    And I press the "Enter" key in the library search
    Then a search token "Key" should be shown in the search bar

  # BPM value suggestions read the library table, whose analyzed BPM is
  # persisted lazily (on track eviction) and is therefore not deterministic
  # through UI actions in an E2E profile. The criterion application mechanism
  # is covered by "Enter selects the highlighted value suggestion", so BPM is
  # covered by the typed field:value path instead.
  Scenario: A typed BPM value applies as a BPM criterion
    Given no search is currently active
    When I activate the library search
    And I type "bpm:1" into the library search
    Then a search token "BPM" should be shown in the search bar
    And the library search query should be "bpm:1"

  Scenario: Clicking a recent search restores the query
    Given no search is currently active
    And a track available in the library
    When I activate the library search
    And I type the title of this track into the library search
    And I deactivate the library search
    And I clear the library search
    When I activate the library search
    And I click the recent library search "<title of this track>"
    Then the library search query should be "<title of this track>"

  Scenario: A recent search can be applied with the keyboard
    Given no search is currently active
    And a track available in the library
    When I activate the library search
    And I type the title of this track into the library search
    And I deactivate the library search
    And I clear the library search
    When I activate the library search
    And I press the "Down" key in the library search
    And I press the "Enter" key in the library search
    Then the library search query should be "<title of this track>"

  Scenario: A single criterion filters the library results
    Given no search is currently active
    And a track available in the library with a unique title
    When I activate the library search
    And I type the title of this track into the library search
    And I wait for the search to settle
    Then this track should be visible in the results
    And only this track should be visible in the results

  Scenario: An artist criterion filters the library results
    Given no search is currently active
    And a track available in the library with a unique title
    When I activate the library search
    And I type "artist:" into the library search
    And I type the artist of this track into the library search
    And I wait for the search to settle
    Then this track should be visible in the results
    And not all tracks should be visible in the results

  Scenario: Multiple criteria are combined with AND
    Given no search is currently active
    And a track available in the library with a unique title
    When I activate the library search
    And I type "artist:" into the library search
    And I type the artist of this track into the library search
    And I press the "Tab" key in the library search
    And I type the first word of the title of this track into the library search
    And I wait for the search to settle
    Then this track should be visible in the results
    And not all tracks should be visible in the results

  Scenario: Ctrl+F activates the search bar
    Given no search is currently active
    When I press the "Ctrl+F" key in the library search
    Then the library search bar should be visible

  Scenario: The Right arrow moves the focus between criteria tokens
    Given no search is currently active
    When I activate the library search
    And I type "artist:Rockot" into the library search
    And I press the "Tab" key in the library search
    And I type "key:74" into the library search
    And I press the "Left" key in the library search
    And I press the "Left" key in the library search
    And I press the "Left" key in the library search
    Then the search token "Artist" should be active
    When I press the "Right" key in the library search
    Then the search token "Key" should be active

  Scenario: Backspace on an empty value removes the previous token
    Given no search is currently active
    When I activate the library search
    And I type "artist:" into the library search
    And I press the "Backspace" key in the library search
    Then the library search bar should be empty

  Scenario: Delete removes the active criterion token
    Given no search is currently active
    When I activate the library search
    And I type "artist:Rockot" into the library search
    And I press the "Delete" key in the library search
    Then the library search bar should be empty

  Scenario: The search bar is anchored to the bottom-right of the library pane
    Given no search is currently active
    Then the library search bar should be anchored to the bottom-right of the library pane

  # --- Right side menu & split view (tracklist toolbar on the right edge) ---
  #
  # Requires objectNames on the tracklist right menu (res/qml/Library.qml):
  #   splitViewButton, rightTrackListLoader (the right-side Loader's TrackList)
  # and new steps:
  #   I click the split view button
  #   I click the track at row {row} on the right track list
  Scenario: Split view opens a second track list
    When I click the split view button
    Then the track list on the right should be visible
    When I click the track at row 1 on the right track list
    Then the track at row 1 on the right track list should be selected
    When I click the split view button
    Then the track list on the right should not be visible

  # TODO: This scenario is invalid: we need to verify that search is independent per splits, and gets restored when the focus changes. Currently needs backend work so LibraryTableModel is specific to the view and not shared.
  @xfail
  Scenario: A search applies to both track lists in split view
    Given a track available in the library
    When I click the split view button
    And I activate the library search
    And I type the title of this track into the library search
    Then this track should be visible in the left track list
    And this track should be visible in the right track list
