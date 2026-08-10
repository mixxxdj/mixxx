Feature: Library Settings
#
# E2E coverage for the "Library" preferences category
# (res/qml/Settings/Library.qml).
#
# Sections: Sources & music-directory integrations, Metadata, History,
# Search. No tabs: the category is a single scrollable column. The shared
# save/cancel/reset button behavior is covered in main.feature (component
# tests); this file covers Library-specific settings and responsiveness.
#

  Background:
    Given a fresh new empty profile
    And Mixxx is open and ready to operate
    And the settings popup is open
    And the "Library" category is selected

  # --- Integrations ----------------------------------------------------------

  Scenario: Music library integrations are shown
    Then the "Rhythmbox integration" setting should be "off"
    And the "Banshee integration" setting should be "off"
    And the "iTunes integration" setting should be "off"
    And the "Traktor integration" setting should be "off"
    And the "Rekordbox integration" setting should be "off"
    And the "Serato integration" setting should be "off"

  Scenario: An integration can be enabled and saved
    When I toggle the "Serato integration" setting to "on"
    And I click the save button
    Then the "Serato integration" setting should be "on"
    And the "Serato integration" should be saved as "on"

  # --- Metadata ---------------------------------------------------------------

  Scenario: Metadata synchronisation with files can be enabled
    When I toggle the "synchronise metadata with file" setting to "on"
    And I click the save button
    Then the "synchronise metadata with file" setting should be "on"
    And the "synchronise metadata with file" should be saved as "on"

  Scenario: Metadata synchronisation with Serato can be enabled
    When I toggle the "synchronise metadata with Serato library" setting to "on"
    Then the "synchronise metadata with Serato library" setting should be "on"

  Scenario: Relative path export can be preferred
    When I toggle the "prefer relative path on playlist export" setting to "on"
    And I click the save button
    Then the "prefer relative path on playlist export" setting should be "on"
    And the "prefer relative path on playlist export" should be saved as "on"

  # --- History ----------------------------------------------------------------

  Scenario: Track duplicate distance can be changed
    When I set the "track duplicate distance" setting to "5"
    And I click the save button
    Then the "track duplicate distance" setting should be "5"
    And the "track duplicate distance" should be saved as "5"

  Scenario: History cleanup threshold can be changed
    When I set the "delete history playlist with less than" setting to "10"
    And I click the save button
    Then the "delete history playlist with less than" setting should be "10"
    And the "delete history playlist with less than" should be saved as "10"

  # --- Search -----------------------------------------------------------------

  Scenario: Search completion can be disabled
    When I toggle the "library search completion" setting to "off"
    And I click the save button
    Then the "library search completion" setting should be "off"

  Scenario: Search history keyboard shortcuts can be disabled
    When I toggle the "library search history keyboard shortcuts" setting to "off"
    And I click the save button
    Then the "library search history keyboard shortcuts" setting should be "off"

  Scenario: Search-as-you-type timeout can be changed
    When I set the "search-as-you-type timeout" setting to "5" with the spinbox
    And I click the save button
    Then the "search-as-you-type timeout" setting should be "5"
    And the "search-as-you-type timeout" should be saved as "5"

  Scenario: Pitch slider fuzz BPM range can be changed
    # UI round-trip only: save() writes the raw 0-100 slider value to the
    # 0-100 "search_bpm_fuzzy_range" config key, which looks like a units bug
    # (config default is 0.06). Asserting persistence would enshrine it.
    When I set the "pitch slider for fuzz BPM search" setting to "50" with the spinbox
    Then the "pitch slider for fuzz BPM search" setting should be "50"

  Scenario: The music directory list is empty on a new profile
    Then the music directory list should be empty

  # --- Music directories (sources) -------------------------------------------
  #
  # Directories are seeded via the `library` RPC command; the last add runs a
  # blocking scan, whose completion refreshes the sources list in the already
  # open popup (scanner onRunningChanged -> loadSources()). Removal is driven
  # through the real UI (select row -> Remove -> keep/hide/purge -> Save).
  #
  # Adding directories is driven through the real UI too: the "Add" button is
  # routed to a test dialog mock (Library.qml testDialog) in test mode, which
  # emulates the native FolderDialog so the test can inject a folder path.

  Scenario: Newly added directories appear in the music directory list
    Given the following music directories
      | id | tracks | permission | dir |
      | 0  | 0      | read       | M1  |
      | 1  | 0      | read       | M2  |
      | 2  | 0      | read       | M3  |
    When I add the test music directory 0
    Then the music directory list should contain 1 source
    And the save button should be enabled
    When I add the test music directory 1
    And I add the test music directory 2
    Then the music directory list should contain 3 sources
    When I click the save button
    Then the music directory list should contain 3 sources
    And the save button should be disabled
    And the library state should match the following
      | key     | value |
      | sources | 3     |

  Scenario: A single directory with tracks can be added and saved
    Given a music directory
    When I add the test music directory 0
    Then the music directory list should contain 1 source
    And the save button should be enabled
    When I click the save button
    Then the music directory list should contain 1 source
    And the save button should be disabled
    And the library state should match the following
      | key     | value |
      | sources | 1     |

  Scenario: An empty music directory can be removed
    Given the library contains 2 music directories
    When I select the music directory at row 0
    Then the remove button of the music directory at row 0 should be visible
    When I click the remove button for the music directory at row 0
    Then the music directory list should contain 2 sources
    And the save button should be enabled
    When I click the save button
    Then the music directory list should contain 1 source
    And the library state should match the following
      | key     | value |
      | sources | 1     |

  Scenario: Cancelling a removal keeps the music directory
    Given the library contains 2 music directories
    When I select the music directory at row 0
    And I click the remove button for the music directory at row 0
    And I click the cancel button
    Then the music directory list should contain 2 sources
    And the save button should be disabled

  Scenario Outline: A music directory with tracks can be removed and its tracks <action>
    Given the tracks directory is in the library
    When I select the music directory at row 0
    And I click the remove button for the music directory at row 0
    And I choose to <action> the tracks of the music directory at row 0
    And I click the save button
    Then the music directory list should contain 0 sources
    And the library state should match the following
      | key               | value |
      | visibleTrackCount | <visible> |
      | hiddenTrackCount  | <hidden>  |

    Examples:
      | action | visible | hidden |
      | keep   | >0      | =0     |
      | hide   | =0      | >0     |
      | purge  | =0      | =0     |

  Scenario: Relink is available for a selected music directory
    Given the tracks directory is in the library
    When I select the music directory at row 0
    Then the relink button of the music directory at row 0 should be visible
    And the remove button of the music directory at row 0 should be visible

  # --- Responsiveness ----------------------------------------------------------
  #
  # The sources grid switches between 2 and 1 columns at 800px of content
  # width (integrations stack below the music directory pane), and the
  # metadata / history grids collapse their columns on narrow rows. On a
  # short window the category overflows its ScrollView and below-the-fold
  # settings must be reachable by scrolling.

  @category/responsiveness
  Scenario: Sources and integrations stack on a narrow window
    Given the window's width is 1792px
    Then the "sources" grid should be displayed in 2 columns
    When I resize the window's width to 700px
    Then the "sources" grid should be displayed in 1 column
    When I resize the window's width to 1792px

  @category/responsiveness
  Scenario: Metadata and history settings rearrange on a narrow window
    Given the window's width is 1792px
    Then the "metadata" grid should be displayed in 2 columns
    And the "history" grid should be displayed in 2 columns
    When I resize the window's width to 700px
    Then the "metadata" grid should be displayed in 1 column
    And the "history" grid should be displayed in 1 column
    When I resize the window's width to 1792px

  @category/responsiveness
  Scenario: Below-the-fold settings can be scrolled into view on a short window
    Given the window's width is 1792px
    When I resize the window's height to 500px
    Then the "pitch slider for fuzz BPM search" setting should not be visible on screen
    When I scroll down in the settings
    Then the "pitch slider for fuzz BPM search" setting should be visible on screen
    When I set the "pitch slider for fuzz BPM search" setting to "50" with the spinbox
    Then the "pitch slider for fuzz BPM search" setting should be "50"
    When I resize the window's height to 1008px

  @category/responsiveness
  Scenario: Integration options compact when the row is narrow
    Given the window's width is 1792px
    Then the "integrations" grid should be displayed in 1 column
    When I resize the window's width to 700px
    Then the "integrations" grid should be displayed in 2 columns
    And the "integrations grid" should be below the "library source pane"
    And the "integrations grid" should not overlap the "library source pane"
    When I resize the window's width to 1792px
