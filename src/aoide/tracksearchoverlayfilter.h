#pragma once

#include <QStringList>

#include "library/tags/label.h"
#include "track/bpm.h"

namespace aoide {

struct TrackSearchOverlayFilter {
    mixxx::Bpm minBpm;
    mixxx::Bpm maxBpm;
    mixxx::library::tags::LabelVector anyGenreLabels;
    mixxx::library::tags::LabelVector allCommentTerms;

    bool isEmpty() const {
        return !minBpm.isValid() &&
                !maxBpm.isValid() &&
                anyGenreLabels.isEmpty() &&
                allCommentTerms.isEmpty();
    }
};

inline bool operator==(const TrackSearchOverlayFilter& lhs, const TrackSearchOverlayFilter& rhs) {
    return lhs.minBpm == rhs.minBpm &&
            lhs.maxBpm == rhs.maxBpm &&
            lhs.anyGenreLabels == rhs.anyGenreLabels &&
            lhs.allCommentTerms == rhs.allCommentTerms;
}

inline bool operator!=(const TrackSearchOverlayFilter& lhs, const TrackSearchOverlayFilter& rhs) {
    return !(lhs == rhs);
}

} // namespace aoide

Q_DECLARE_METATYPE(aoide::TrackSearchOverlayFilter)
