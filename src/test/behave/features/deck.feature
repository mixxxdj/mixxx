Feature: Deck

  Background:
    Given a new empty profile
    And Mixxx is open and ready to operate

  Scenario: Show or hide all 4 decks
    When I click the "4DECKS" button
    Then the deck for "[Channel3]" should be visible
    And the deck for "[Channel4]" should be visible
    When I click the "4DECKS" button
    Then the deck for "[Channel3]" should not be visible
    And the deck for "[Channel4]" should not be visible

  Scenario: Deck play button toggles playback
    Given a track is loaded on deck 1
    When I click the "play" button on deck 1
    Then the play button on deck 1 should be pressed
    And the deck 1 should be playing
    When I click the "play" button on deck 1
    Then the play button on deck 1 should be released
    And the deck 1 should be stopped

  # The test appears flaky, cue_point seems to occasionally be set to a very low value. It could be a genuine bug tho!
  @xpass
  Scenario: Deck cue button can be triggered
    Given a track is loaded on deck 1
    When I seek to 0.05 in deck 1
    And I click the "cue" button on deck 1
    Then the cue point should be set on deck 1

  @xfail
  Scenario: Deck hotcue can be set and cleared
    Given a track is loaded on deck 1
    When I set hotcue 1 on deck 1
    Then hotcue 1 should be set on deck 1
    When I clear hotcue 1 on deck 1
    Then hotcue 1 should not be set on deck 1

  Scenario: Beatjump moves playback position forward
    Given a track is loaded on deck 1
    When I click the "play" button on deck 1
    And I remember the "playback position" of deck 1
    When I click the "beatjump_forward" button on deck 1
    Then the "playback position" of deck 1 should have changed

  Scenario: Loop can be set and enabled
    Given a track is loaded on deck 1
    # This is needed because with quantize=1, if loop in and out end up on the same beat, no loop will be created
    # Alternatively, we could wait X to ensure more than one beat goes by between in and out
    And the quantize on deck 1 is set to 0.0
    When I click the "play" button on deck 1
    And I click the "loop_in" button on deck 1
    And I click the "loop_out" button on deck 1
    Then the loop should be enabled on deck 1

  @xfail
  Scenario: Loop size can be halved and doubled
    Given a track is loaded on deck 1
    When I remember the "beatloop size" of deck 1
    When I halve the "beatloop" size on deck 1
    Then the "beatloop size" of deck 1 should have changed
    When I double the "beatloop" size on deck 1
    Then the "beatloop size" of deck 1 should have changed

  Scenario: Loop can be recalled after exit
    Given a track is loaded on deck 1
    When I click the "play" button on deck 1
    And I click the "loop_in" button on deck 1
    # This is needed as if loop in and out end up on the same beat, no loop will be created
    # Alternatively, we could set quantize=0
    And I wait for 1 second
    And I click the "loop_out" button on deck 1
    Then the loop should be enabled on deck 1
    When I click the "reloop_toggle" button on deck 1
    Then the loop should not be enabled on deck 1
    When I click the "reloop_toggle" button on deck 1
    Then the loop should be enabled on deck 1

  Scenario: Sync can be toggled on a deck
    Given a track is loaded on deck 1
    When I click the "sync" button on deck 1
    Then sync should be enabled on deck 1
    When I click the "sync" button on deck 1
    Then sync should be disabled on deck 1

  @xfail
  Scenario: Other deck's BPM can be copied with long press
    Given a track is loaded on deck 1
    And a track is loaded on deck 2
    When I long-press the "sync" button on deck 1
    Then deck 1 BPM should be equal to deck 2

  Scenario: Range can be cycled on a deck
    Given a track is loaded on deck 1
    When I remember the "rate range" of deck 1
    When I click the "range" button on deck 1
    Then the "rate range" of deck 1 should have changed

  Scenario: Tempo fader changes playback rate
    Given a track is loaded on deck 1
    And the sync_on deck 1 is disabled
    And the tempo rate on deck 1 is set to 0.0
    When I set the rate of deck 1 to 5%
    Then the rate of deck 1 should be near 0.05
    And the rate ratio of deck 1 should be near 1.004

  Scenario: Edit mode can be toggled
    When I click the "EDIT" button
    Then edit mode should be enabled
    When I click the "EDIT" button
    Then edit mode should not be enabled

  Scenario: Edit mode overlays are visible on deck components
    When I click the "EDIT" button
    Then the edit overlay should be visible on the "play" component in deck 1
    When I click the "EDIT" button
    Then the edit overlay should not be visible on the "play" component in deck 1

  Scenario: Edit mode: single component can be repositioned
    When I click the "EDIT" button
    And I move the "play" component in deck 1 under the "cue" component
    And I click the "EDIT" button
    Then the "play" component should appear under the "cue" component in deck 1

  Scenario: Edit mode: component group can be moved
    When I click the "EDIT" button
    And I select the group containing "play" in deck 1 with a ctrl+click
    # Select the parent's parent
    And I select the group containing "play" in deck 1 with a long press
    # Select the parent's parent's parent
    And I select the group containing "play" in deck 1 with a ctrl+click
    And I move the selected group in deck 1 after the "rateSlider" component
    And I click the "EDIT" button
    Then the "play" component should appear after the "rateSlider" component in deck 1

  @category/responsiveness
  Scenario: The spinny is hidden when the deck gets too narrow
    When I resize the window's width to 1792px
    Then the "spinny" component should be visible in deck 1
    When I resize the window's width to 1500px
    Then the "spinny" component should not be visible in deck 1

  @category/responsiveness
  Scenario: The beatjump is hidden when the deck gets too narrow
    When I resize the window's width to 1792px
    Then the "beatjump" component should be visible in deck 1
    When I resize the window's width to 1400px
    Then the "beatjump" component should not be visible in deck 1

  @category/responsiveness
  Scenario: The hotcue is hidden when the deck gets too narrow
    When I resize the window's width to 1792px
    Then the "hotcue" component should be visible in deck 1
    When I resize the window's width to 1200px
    Then the "hotcue" component should not be visible in deck 1

  @category/responsiveness
  Scenario: The tempo is hidden when the deck gets too narrow
    When I resize the window's width to 1792px
    Then the "tempo" component should be visible in deck 1
    When I resize the window's width to 700px
    Then the "tempo" component should not be visible in deck 1
    And the "play" component should be visible in deck 1

  @category/responsiveness
  Scenario: 4 decks button hides when the window is too short
    When I resize the window's height to 500px
    Then the "4DECKS" button in the main toolbar should not be visible
    When I resize the window's height to 600px
    Then the "4DECKS" button in the main toolbar should be visible
