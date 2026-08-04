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
