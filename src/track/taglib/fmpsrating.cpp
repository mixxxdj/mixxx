#include "track/taglib/fmpsrating.h"

#include <QRegularExpression>
#include <cmath>

#include "track/trackrecord.h"
#include "util/assert.h"

namespace mixxx {

namespace taglib {

std::optional<QString> formatFmpsRating(int rating) {
    if (!TrackRecord::isValidRating(rating) ||
            rating == TrackRecord::kNoRating) {
        return std::nullopt;
    }
    return QString::number(
            static_cast<double>(rating) / TrackRecord::kMaxRating, 'f', 1);
}

std::optional<int> parseFmpsRating(const QString& fmpsRating) {
    // FMPS ratings are plain, unsigned decimal numbers. Validate the
    // format on the string level before converting: QString::toDouble()
    // also accepts "nan" and "inf", and floating-point classification
    // functions like std::isfinite() cannot be used to reject those
    // afterwards, because Mixxx is compiled with -ffast-math.
    static const QRegularExpression kFmpsRatingFormat(
            QStringLiteral("^\\d+(\\.\\d+)?$"));
    const QString trimmed = fmpsRating.trimmed();
    if (!kFmpsRatingFormat.match(trimmed).hasMatch()) {
        return std::nullopt;
    }
    bool ok = false;
    const double value = trimmed.toDouble(&ok);
    if (!ok || value > 1.0) {
        return std::nullopt;
    }
    // Rounding to the nearest star matches the exported values 0.2,
    // 0.4, ..., 1.0 exactly and maps intermediate values from other
    // applications to the nearest band, with values below 0.1
    // becoming TrackRecord::kNoRating.
    const int rating = static_cast<int>(
            std::lround(value * TrackRecord::kMaxRating));
    DEBUG_ASSERT(TrackRecord::isValidRating(rating));
    return rating;
}

int popmFromRating(int rating) {
    switch (rating) {
    case 1:
        return 1;
    case 2:
        return 64;
    case 3:
        return 128;
    case 4:
        return 196;
    case 5:
        return 255;
    default:
        return 0;
    }
}

int ratingFromPopm(int popmRating) {
    if (popmRating <= 0) {
        return TrackRecord::kNoRating;
    } else if (popmRating <= 31) {
        return 1;
    } else if (popmRating <= 95) {
        return 2;
    } else if (popmRating <= 159) {
        return 3;
    } else if (popmRating <= 223) {
        return 4;
    } else {
        return 5;
    }
}

} // namespace taglib

} // namespace mixxx
