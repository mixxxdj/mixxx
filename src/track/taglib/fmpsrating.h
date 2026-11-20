#pragma once

#include <QString>
#include <optional>

namespace mixxx {

namespace taglib {

/// Conversion helpers between Mixxx star ratings and rating values
/// stored in file tags.
///
/// FMPS ratings are decimal strings in the range [0.0, 1.0] as defined by
/// https://www.freedesktop.org/wiki/Specifications/free-media-player-specs/
/// Mixxx ratings are integers in the range
/// [TrackRecord::kMinRating, TrackRecord::kMaxRating] where
/// TrackRecord::kNoRating means unrated.

/// Format a Mixxx star rating as an FMPS rating string ("0.2".."1.0").
/// Returns std::nullopt for TrackRecord::kNoRating and out-of-range values.
std::optional<QString> formatFmpsRating(int rating);

/// Parse an FMPS rating string into a Mixxx rating.
/// Returns std::nullopt for malformed, non-finite, and out-of-range values.
/// Values close to 0.0 yield TrackRecord::kNoRating, i.e. explicitly unrated.
std::optional<int> parseFmpsRating(const QString& fmpsRating);

/// Convert an ID3v2 POPM (Popularimeter) rating to a Mixxx rating using
/// the bands established by common media players. POPM ratings originate
/// from a single byte (0-255), but the full int domain of TagLib's
/// rating() accessor is handled: values <= 0 map to TrackRecord::kNoRating
/// and values > 255 map to TrackRecord::kMaxRating.
int ratingFromPopm(int popmRating);

/// Convert a Mixxx rating to an ID3v2 POPM rating byte using the values
/// written by common media players (1, 64, 128, 196, 255). Returns 0 for
/// TrackRecord::kNoRating and out-of-range values.
int popmFromRating(int rating);

} // namespace taglib

} // namespace mixxx
