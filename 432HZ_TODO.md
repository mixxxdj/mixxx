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
