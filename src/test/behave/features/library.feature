Feature: Library

  Background:
    Given a new library-ready profile
    And Mixxx is open and ready to operate

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
