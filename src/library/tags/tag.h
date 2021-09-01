#pragma once

#include <QJsonValue>
#include <QVector>

#include "library/tags/label.h"
#include "library/tags/score.h"
#include "util/assert.h"
#include "util/macros.h"
#include "util/optional.h"

namespace mixxx {

namespace library {

namespace tags {

/// A plain, non-faceted tag with an optional label and a score.
///
/// The optional facet is usually provided by the outer context,
/// e.g. the key of a map that stores plain tags as values.
///
/// Mixxx uses these kind of tags as `Facets` to store extended
/// metadata. They should not be confused with file tags (ID3, MP4,
/// VorbisComment) for storing metadata in media files.
class Tag final {
    // Properties
    MIXXX_DECL_PROPERTY(Label, label, Label)
    MIXXX_DECL_PROPERTY(Score, score, Score)

  public:
    explicit Tag(
            const Label& label,
            Score score = Score{})
            : m_label(label),
              m_score(score) {
    }
    explicit Tag(
            Score score = Score{})
            : m_score(score) {
    }
    Tag(const Tag&) = default;
    Tag(Tag&&) = default;

    Tag& operator=(Tag&&) = default;
    Tag& operator=(const Tag&) = default;

    bool isValid() const {
        return getLabel().isValid() &&
                getScore().isValid();
    }

    /// Check if a label is present.
    ///
    /// Empty labels are considered as missing.
    bool hasLabel() const {
        return !m_label.isEmpty();
    }

    static std::optional<Tag> fromJsonValue(
            const QJsonValue& jsonValue);

    QJsonValue toJsonValue() const;
};

inline bool operator==(
        const Tag& lhs,
        const Tag& rhs) {
    return lhs.getLabel() == rhs.getLabel() &&
            lhs.getScore() == rhs.getScore();
}

inline bool operator!=(
        const Tag& lhs,
        const Tag& rhs) {
    return !(lhs == rhs);
}

QDebug operator<<(QDebug dbg, const Tag& arg);

typedef QVector<Tag> TagVector;

} // namespace tags

} // namespace library

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::library::tags::Tag)
Q_DECLARE_METATYPE(mixxx::library::tags::TagVector)
