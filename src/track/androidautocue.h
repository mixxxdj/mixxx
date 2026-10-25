#pragma once

#include "track/track_decl.h"

class PlayerManager;
class Track;

namespace mixxx {
namespace android {

/// Moves the cue that the analysis put on the first sound of a track onto
/// the nearest beat.
///
/// The analysis puts the main cue (and the intro start) where the track
/// first gets louder than -60 dB. That is often a few milliseconds before
/// the first kick (a click, a fade-in, the attack of the kick), so a track
/// started from its cue begins slightly before the downbeat. If a beat of
/// the beatgrid is within a quarter beat of the first sound, the cues are
/// moved onto it. Cues that the user moved are left alone: only cues that
/// are still exactly on the first sound are changed. A beatgrid that is off
/// by more than a quarter beat is not followed; that has to be corrected in
/// the beatgrid panel.
///
/// Returns true if a cue was moved.
bool snapAutoCueToBeat(Track* pTrack);

/// Applies snapAutoCueToBeat() to every track that is loaded into a deck,
/// before the deck jumps to the main cue.
void snapAutoCuesOnLoad(PlayerManager* pPlayerManager);

} // namespace android
} // namespace mixxx
