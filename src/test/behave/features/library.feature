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

  # --- Library search (new search bar in the bottom right of the library) ---
  #
  # Requires objectNames on the search pane UI (res/qml/Library.qml):
  #   searchPane, searchField, clearButton, suggestionList, recentList
  # and new steps in steps/mixxx_steps.py:
  #   I activate the library search          (click the search bar / press Ctrl+F)
  #   I type "{text}" into the library search (inputText on searchField)
  #   I deactivate the library search        (focus outside the search bar)
  #   I clear the library search             (click clearButton)
  #   the library search suggestion "{text}" should be visible (suggestionList)
  #   the library recent search "{text}" should be visible     (recentList)
  Scenario: Search bar opens and closes without filtering
    When I activate the library search
    Then the library search bar should be visible
    When I deactivate the library search
    Then all tracks in the library should be shown without a search filter

  Scenario: Search filters the track results
    When I activate the library search
    And I type the title of the track at row 1 into the library search
    Then the track at row 1 should be visible in the results
    And no other track should be visible in the results

  Scenario: Clearing the search shows all tracks again
    When I activate the library search
    And I type the title of the track at row 1 into the library search
    Then the track at row 1 should be visible in the results
    When I clear the library search
    Then all tracks in the library should be shown again

  Scenario: Search suggests matching field names
    When I activate the library search
    And I type "art" into the library search
    Then the library search suggestion "artist:" should be visible

  Scenario: A field suggestion creates a search token
    When I activate the library search
    And I type "art" into the library search
    And I select the library search suggestion "artist:"
    Then a search token "artist:" should be shown in the search bar

  Scenario: A committed search appears in recent searches
    When I activate the library search
    And I type the title of the track at row 1 into the library search
    And I deactivate the library search
    When I activate the library search
    Then the library recent search "<title of the track at row 1>" should be visible

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

  Scenario: A search applies to both track lists in split view
    When I click the split view button
    And I activate the library search
    And I type the title of the track at row 1 into the library search
    Then the track at row 1 should be visible in the left track list
    And the track at row 1 should be visible in the right track list
