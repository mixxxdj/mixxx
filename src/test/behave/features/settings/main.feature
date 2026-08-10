Feature: Settings

  Background:
    Given a new empty profile
    And Mixxx is open and ready to operate

  Scenario: Settings popup opens from toolbar gear button
    When I click the "PREFERENCES" button
    Then the settings popup should be visible

  Scenario: Settings popup closes with close button
    Given the settings popup is open
    When I click the settings close button
    Then the settings popup should not be visible

  Scenario: Default category is Sound hardware
    When I click the "PREFERENCES" button
    Then the "Sound hardware" category should be selected

  Scenario: Selecting a category changes the active view
    Given the settings popup is open
    When I select the "Library" category
    Then the "Library" category should be selected

  Scenario: Search filters settings
    Given the settings popup is open
    When I search for "Sample Rate"
    Then the search results should be visible

  # --- Save / Cancel buttons (shared component behavior) ----------------------
  # The popup opens on the default "Sound hardware" category. Saving the sound
  # category touches the audio config, so mock devices are registered per
  # scenario where a save/cancel is involved (the Background stays device-free).

  Scenario: Save button is disabled when no changes
    Given the settings popup is open
    Then the save button should be disabled
    And the cancel button should not be visible

  Scenario: Save and cancel enable when a setting changes
    Given the settings popup is open
    And I have the following sound devices
      | name           | api  | outputChannels | inputChannels |
      | Built-in Audio | Mock | 4              | 4             |
    And the "Main Mix" setting is "on"
    When I toggle the "Main Mix" setting to "off"
    Then the save button should be enabled
    And the cancel button should be visible

  Scenario: Cancel reverts changes
    Given the settings popup is open
    And I have the following sound devices
      | name           | api  | outputChannels | inputChannels |
      | Built-in Audio | Mock | 4              | 4             |
    And the "Main Mix" setting is "on"
    When I toggle the "Main Mix" setting to "off"
    And I click the cancel button
    Then the "Main Mix" setting should be "on"
    And the save button should be disabled

  Scenario: Save applies changes
    Given the settings popup is open
    And I have the following sound devices
      | name           | api  | outputChannels | inputChannels |
      | Built-in Audio | Mock | 4              | 4             |
    When I toggle the "Main Mix" setting to "off"
    And I click the save button
    Then the save button should be disabled
    And the cancel button should not be visible

  # --- Responsiveness -----------------------------------------------------------
  #
  # The settings popup collapses its category sidebar below `smallScreenWidth`
  # (1200 px) and compacts ratio options (pills -> prev/next spinbox) when the
  # row has too little room. These scenarios exercise the responsive layouts and
  # prove the settings stay usable on narrow and short windows.

  @category/responsiveness
  Scenario: Settings categories collapse to a button on a narrow window
    Given the settings popup is open
    When I resize the window's width to 1250px
    Then the settings categories should be visible
    When I resize the window's width to 1150px
    Then the settings categories should not be visible
    And the button to reveal the categories should be visible
    When I resize the window's width to 1792px

  @category/responsiveness
  Scenario: Settings categories can be shown on demand on a narrow window
    Given the settings popup is open
    And the window's width is 1150px
    Then the settings categories should not be visible
    And the button to reveal the categories should be visible
    When I click the button to reveal the categories
    Then the settings categories should be visible
    When I resize the window's width to 1792px

  @category/responsiveness
  Scenario: Ratio options stay usable when the popup is narrow
    Given the settings popup is open
    And the window's width is 1792px
    And the "Interface" category is selected
    And the "decks" tab is selected
    Then the "track time display" setting should be expanded
    When I resize the window's width to 680px
    Then the "track time display" setting should be compacted
    When I toggle the "track time display" setting to "remaining"
    Then the "track time display" setting should be "remaining"
    When I resize the window's width to 1792px

  @category/responsiveness
  Scenario: The settings popup stays usable when the window is short
    Given the settings popup is open
    And the window's width is 1792px
    And the "Interface" category is selected
    And the "decks" tab is selected
    When I resize the window's height to 500px
    Then the settings popup should be visible
    And the "track time display" setting should be visible
    When I toggle the "track time display" setting to "both"
    Then the "track time display" setting should be "both"
    When I resize the window's height to 1008px
