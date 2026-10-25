#pragma once

#include <vector>

/// Auto DJ 2.0 plus Video Mixing: singing detection.
///
/// Where in a song someone sings, found from the song's vocal part (the
/// stem file made by Mixxx, via the deck's waveform with parts). The level
/// of the vocal part is smoothed; moments clearly louder than the song's
/// own quiet vocal background count as singing; breaths and short rests are
/// joined, tiny bits dropped. Pure maths, unit tested (vocalmap_test.cpp).
namespace vocalmap {

struct Section {
    double startSec = 0.0;
    double endSec = 0.0;
};

struct Settings {
    double smoothSec = 0.5;     ///< level averaged over this long
    double joinGapSec = 1.5;    ///< shorter rests (breaths) stay one section
    double minSectionSec = 1.0; ///< shorter bits are not singing
    /// Singing: above floor + level x (peak - floor) of the song's own
    /// vocal level (floor = quiet 20%, peak = loud 95%).
    double level = 0.3;
    double minPeak = 0.06;  ///< a vocal part never louder than this: no singing
    double leakRatio = 0.2; ///< vocal must reach this part of the band's level
};

/// @param vocal level of the vocal part (0..1), `rate` values per second
/// @param rest level of the other parts together (same length) or empty
/// @return the singing sections in seconds, in order
std::vector<Section> find(const std::vector<float>& vocal,
        const std::vector<float>& rest,
        double rate,
        const Settings& settings = Settings());

/// Does anyone sing somewhere between `fromSec` and `toSec`?
bool singsBetween(const std::vector<Section>& sections, double fromSec, double toSec);

/// The first moment between `fromSec` and `toSec` when nobody sings (the
/// end of a line), or -1 if they sing all the way.
double quietMoment(const std::vector<Section>& sections, double fromSec, double toSec);

/// When the singing starts after `fromSec` (-1 if it does not).
double nextSinging(const std::vector<Section>& sections, double fromSec);

} // namespace vocalmap
