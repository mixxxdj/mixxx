#pragma once

#include <QString>
#include <optional>

namespace mixxx {

namespace taglib {

/// Conversion helpers for the FMPS_Playcount tag defined by
/// https://www.freedesktop.org/wiki/Specifications/free-media-player-specs/
///
/// The specification stores the play count as a decimal number. Media
/// players write whole numbers, but fractional values are legal and
/// are truncated on import.

/// Format a play count as an FMPS_Playcount string.
/// Returns std::nullopt for negative values.
std::optional<QString> formatFmpsPlayCount(int playCount);

/// Parse an FMPS_Playcount string.
/// Returns std::nullopt for malformed, negative, and non-finite values.
/// Values beyond the range of int are clamped.
std::optional<int> parseFmpsPlayCount(const QString& fmpsPlayCount);

} // namespace taglib

} // namespace mixxx
