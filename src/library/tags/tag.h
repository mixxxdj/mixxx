#pragma once

#include <QJsonValue>
#include <QVector>

#include "util/assert.h"
#include "util/macros.h"
#include "util/math.h"
#include "util/optional.h"

namespace mixxx {

namespace library {

namespace tags {

/// A category for tags.
///
/// Facets are used for grouping/categorizing and providing context or meaning.
/// Their value serves as a symbolic, internal identifier that is not intended
/// to be displayed literally in the UI. This is also the reasons for the
/// restrictions on naming them.
///
/// Value constraints:
///   - lowercase ASCII
///   - and no whitespace
///
/// References:
///   - https://en.wikipedia.org/wiki/Faceted_classification
class TagFacet final {
  public:
    typedef QString value_t;

    static value_t defaultValue() {
        return value_t{};
    }

    static bool isValidValue(
            const value_t& value);

    /// Convert the given string into lowercase and then
    /// removes all whitespace and non-ASCII characters.
    static value_t convertIntoValidValue(
            const value_t& value);

    /// Ensure that empty values are always null
    static value_t filterEmptyValue(
            value_t value) {
        DEBUG_ASSERT(defaultValue().isEmpty());
        return value.isEmpty() ? defaultValue() : value;
    }

    explicit TagFacet(
            value_t value = defaultValue())
            : m_value(std::move(value)) {
        DEBUG_ASSERT(isValid());
    }
    TagFacet(const TagFacet&) = default;
    TagFacet(TagFacet&&) = default;

    TagFacet& operator=(const TagFacet&) = default;
    TagFacet& operator=(TagFacet&&) = default;

    bool isValid() const {
        return isValidValue(m_value);
    }

    bool isEmpty() const {
        DEBUG_ASSERT(isValid());
        return m_value.isEmpty();
    }

    const value_t& value() const {
        DEBUG_ASSERT(isValid());
        return m_value;
    }
    operator const value_t&() const {
        return value();
    }

  private:
    value_t m_value;
};

inline bool operator==(
        const TagFacet& lhs,
        const TagFacet& rhs) {
    return lhs.value() == rhs.value();
}

inline bool operator!=(
        const TagFacet& lhs,
        const TagFacet& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(
        const TagFacet& lhs,
        const TagFacet& rhs) {
    return lhs.value() < rhs.value();
}

inline bool operator>(
        const TagFacet& lhs,
        const TagFacet& rhs) {
    return !(lhs < rhs);
}

inline bool operator<=(
        const TagFacet& lhs,
        const TagFacet& rhs) {
    return !(lhs > rhs);
}

inline bool operator>=(
        const TagFacet& lhs,
        const TagFacet& rhs) {
    return !(lhs < rhs);
}

inline uint qHash(
        const TagFacet& facet,
        uint seed = 0) {
    return qHash(facet.value(), seed);
}

/// The displayable name or title of a tag.
///
/// The value contains arbitrary Unicode text that is supposed to
/// be displayed to the user.
///
/// Value constraints:
///   - no leading/trailing whitespace
class TagLabel final {
  public:
    typedef QString value_t;

    static const value_t defaultValue() {
        return value_t{};
    }

    static bool isValidValue(
            const value_t& value);

    /// Remove leading/trailing whitespace.
    static value_t convertIntoValidValue(
            const value_t& value);

    /// Ensure that empty values are always null
    static value_t filterEmptyValue(
            value_t value) {
        DEBUG_ASSERT(defaultValue().isEmpty());
        return value.isEmpty() ? defaultValue() : value;
    }

    explicit TagLabel(
            value_t value = defaultValue())
            : m_value(std::move(value)) {
        DEBUG_ASSERT(isValid());
    }
    TagLabel(const TagLabel&) = default;
    TagLabel(TagLabel&&) = default;

    TagLabel& operator=(const TagLabel&) = default;
    TagLabel& operator=(TagLabel&&) = default;

    bool isValid() const {
        return isValidValue(m_value);
    }

    bool isEmpty() const {
        DEBUG_ASSERT(isValid());
        return m_value.isEmpty();
    }

    const value_t& value() const {
        DEBUG_ASSERT(isValid());
        return m_value;
    }
    operator const value_t&() const {
        return value();
    }

  private:
    value_t m_value;
};

inline bool operator==(
        const TagLabel& lhs,
        const TagLabel& rhs) {
    return lhs.value() == rhs.value();
}

inline bool operator!=(
        const TagLabel& lhs,
        const TagLabel& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(
        const TagLabel& lhs,
        const TagLabel& rhs) {
    return lhs.value() < rhs.value();
}

inline bool operator>(
        const TagLabel& lhs,
        const TagLabel& rhs) {
    return !(lhs < rhs);
}

inline bool operator<=(
        const TagLabel& lhs,
        const TagLabel& rhs) {
    return !(lhs > rhs);
}

inline bool operator>=(
        const TagLabel& lhs,
        const TagLabel& rhs) {
    return !(lhs < rhs);
}

inline uint qHash(
        const TagLabel& label,
        uint seed = 0) {
    return qHash(label.value(), seed);
}

/// The score or weight of a tag relationship, defaulting to 1.0.
///
/// Constraints: Normalized to the unit interval [0.0, 1.0]
class TagScore final {
  public:
    typedef double value_t;

    static constexpr value_t kMinValue = 0.0;
    static constexpr value_t kMaxValue = 1.0;
    static constexpr value_t kDefaultValue = kMaxValue;

    static constexpr bool isValidValue(
            value_t value) {
        return value >= kMinValue && value <= kMaxValue;
    }

    /// Clamp the value to the valid range [kMinValue, kMaxValue].
    static constexpr value_t convertIntoValidValue(
            value_t value) {
        return math_min(kMaxValue, math_max(kMinValue, value));
    }

    explicit TagScore(
            value_t value = kDefaultValue)
            : m_value(value) {
        DEBUG_ASSERT(isValid());
    }

    constexpr bool isValid() const {
        return isValidValue(m_value);
    }

    constexpr value_t value() const {
        return m_value;
    }
    constexpr operator value_t() const {
        return value();
    }

  private:
    value_t m_value;
};

/// A plain tag with an optional label and a score.
///
/// Plain tags are not faceted. The facet is usually provided
/// by the outer context, e.g. the key of a map that stores
/// plain tags as values.
///
/// Mixxx uses these kind of tags as `CustomTags` to store extended
/// metadata. They should not be confused with file tags (ID3, MP4,
/// VorbisComment) for storing metadata in media files.
class Tag final {
    // Properties
    MIXXX_DECL_PROPERTY(TagLabel, label, Label)
    MIXXX_DECL_PROPERTY(TagScore, score, Score)

  public:
    explicit Tag(
            const TagLabel& label,
            TagScore score = TagScore{})
            : m_label(label),
              m_score(score) {
    }
    explicit Tag(
            TagScore score = TagScore{})
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

bool operator==(
        const Tag& lhs,
        const Tag& rhs);

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

Q_DECLARE_METATYPE(mixxx::library::tags::TagFacet)
Q_DECLARE_METATYPE(mixxx::library::tags::TagLabel)
Q_DECLARE_METATYPE(mixxx::library::tags::TagScore)
Q_DECLARE_METATYPE(mixxx::library::tags::Tag)
Q_DECLARE_METATYPE(mixxx::library::tags::TagVector)
