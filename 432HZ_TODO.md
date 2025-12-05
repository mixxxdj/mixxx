# 432Hz Feature - Next Improvements

## Branch: feature-432hzdetect

### Current Limitations
The current implementation has a fixed detection and pitch adjustment logic:
- Detection: Only checks 440Hz vs 432Hz
- Pitch adjustment: Fixed -0.31767 semitones (440Hz → 432Hz)

### Problem
Real-world tracks can have varying tuning frequencies:
- Example: A track tuned to 447Hz would need a different correction
- Current logic doesn't detect or adjust for frequencies other than 440Hz/432Hz

### Proposed Solution
Make detection and pitch adjustment dynamic:

1. **Flexible Detection**
   - Detect actual tuning frequency (not just 440Hz/432Hz binary)
   - Store detected frequency in track metadata
   - Support range: e.g., 427Hz - 447Hz

2. **Dynamic Pitch Adjustment**
   - Calculate adjustment based on detected frequency
   - Formula: log₂(432 / detected_frequency) × 12 semitones
   - Examples:
     - 440Hz → 432Hz: -0.31767 semitones (current)
     - 447Hz → 432Hz: -0.58 semitones (needs more correction)
     - 435Hz → 432Hz: -0.12 semitones (slight correction)

3. **Implementation Ideas**
   - Extend Queen Mary analyzer to detect tuning frequency
   - Add `detected_tuning_hz` field to keys.proto
   - Update KeyControl to calculate adjustment dynamically
   - UI: Display actual detected frequency in library (optional)

### Technical Considerations
- Detection accuracy vs. computational cost
- How to handle tracks with pitch variation (live recordings)
- Backward compatibility with existing 432Hz metadata
- User preference: strict 432Hz vs. flexible tuning tolerance

### Status
- Current implementation: ✅ Complete (binary 440Hz/432Hz)
- Next step: ⏳ Flexible frequency detection and adjustment

---

## UI/UX Improvements

### Issue 1: Spinny/Vinyl Display "Shadow" Effect
**Problem:**
- When a track is pitched from 440Hz to 432Hz, a "shadow" appears behind the playback position indicator on the vinyl/spinny widget
- The visual playback position runs behind the actual position
- This is caused by the pitch adjustment affecting playback speed but not visual sync

**User Perspective:**
- ✅ Praktisch: Shows user that correction is active
- ⚠️ Potenziell irritierend: Looks like a visual bug

**Proposed Solution:**
- Fix visual position calculation to account for pitch_adjust
- Ensure spinny/vinyl display stays synchronized with actual playback
- Investigate: `src/widget/wspinny.cpp` or waveform renderers

**Commit Category:** 🐛 Bug-Fix

---

### Issue 2: Library Column Symbols for Tuning
**Current:**
- ✧ (sparkle) only shows for exactly 432Hz tracks

**Proposed Enhancement:**
Add visual indicators for all tuning variations:
- **↓ (arrow down)**: Tracks tuned **below** 432Hz (e.g., 427Hz, 430Hz)
- **✧ (sparkle)**: Tracks tuned **at** 432Hz (current, keep this!)
- **↑ (arrow up)**: Tracks tuned **above** 432Hz (e.g., 440Hz, 447Hz)

**Rationale:**
- User can immediately see if a track needs pitch correction
- Visual consistency: arrows indicate direction of required pitch shift
- Sparkle symbol remains special for exact 432Hz tracks

**Implementation:**
- Extend `KeyDelegate::paintItem()` in `src/library/tabledelegates/keydelegate.cpp`
- Add detection logic based on stored tuning frequency
- Unicode symbols:
  - ↓ = `\u2193` or `\u25BC` (down arrow / down triangle)
  - ↑ = `\u2191` or `\u25B2` (up arrow / up triangle)

**Commit Category:** 🎨 Library UI Enhancement

---

### Issue 3: Deck UI - 432Hz/440Hz Status Display
**Proposed:**
Display active tuning mode directly in the deck UI, next to the key/note symbol

**Design Options:**
1. Text indicator: "432Hz" or "440Hz" (depends on pitch lock setting)
2. Badge/pill style with color coding
3. Icon + text combination

**Placement:**
- Next to existing key display (note symbol)
- Should be visible without cluttering the deck

**Behavior:**
- Shows "432Hz" when 432Hz Pitch Lock is **enabled**
- Shows "440Hz" when 432Hz Pitch Lock is **disabled**
- Could also show actual target frequency from settings

**Implementation:**
- Affects multiple skins (Deere, LateNight, Shade, etc.)
- Add new CO: `[ChannelN],432hz_pitch_lock` (already exists!)
- Update skin XMLs to display this control
- Consider: New widget type or extend existing key display widget?

**Files to modify:**
- Skin XML files: `res/skins/*/deck.xml` or similar
- Widget: `src/widget/wkey.cpp/h` or create `src/widget/wtuningdisplay.cpp/h`

**Commit Category:** 🎨 Deck UI Enhancement

**Note:** This will require changes across multiple skins, so it might be better to:
1. First implement for one reference skin (e.g., Deere)
2. Then port to other skins in follow-up commits

---

## Proposed Commit Structure

### Completed ✅
- **Commit 1**: Core 432Hz Detection & Pitch Lock Feature

### Planned ⏳
- **Commit 2**: Library UI - Tuning symbols (↓/✧/↑) in key column
- **Commit 3**: Deck UI - 432Hz/440Hz status display (reference skin)
- **Commit 4**: Deck UI - Port to additional skins
- **Commit 5**: Bug-Fix - Spinny/Vinyl display synchronization
- **Commit 6**: Core - Dynamic frequency detection (427Hz-447Hz range)
- **Commit 7**: Core - Dynamic pitch adjustment based on detected frequency

### Rationale for Separation:
- **UI vs. Logic**: Design changes separated from functional changes
- **Reviewability**: Each commit has clear, focused scope
- **Skin Maintenance**: Skin-specific changes can be reviewed by skin maintainers
- **Bug Tracking**: Visual bugs fixed separately
- **Backwards Compatibility**: Core changes can be tested independently
