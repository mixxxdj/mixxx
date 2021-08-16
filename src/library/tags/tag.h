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
/// Constraints: Lowercase ASCII, no whitespace characters
class TagFacet final {
  public:
    typedef QString value_t;

    static bool isValidValue(
            const value_t& value);

    /// Converts the given string into lowercase and then
    /// removes all whitespace and non-ASCII characters.
    static value_t clampValue(
            const value_t& value);

    /// Ensure that empty values are always null
    static value_t filterEmptyValue(
            const value_t& value) {
        return value.isEmpty() ? value_t() : value;
    }

    explicit TagFacet(
            const value_t& value = value_t())
            : m_value(value) {
        // Full validation not possible due to static constants with
        // regular expressions that are inaccessible when creating
        // static constants of this type!
        DEBUG_ASSERT(filterEmptyValue(m_value) == m_value);
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

/// The name/title of a tag.
///
/// Constraints: No leading/trailing whitespace
class TagLabel final {
  public:
    typedef QString value_t;

    static bool isValidValue(
            const value_t& value);

    static value_t clampValue(
            const value_t& value);

    /// Ensure that empty values are always null
    static value_t filterEmptyValue(
            const value_t& value) {
        return value.isEmpty() ? value_t() : value;
    }

    explicit TagLabel(
            const value_t& value = value_t())
            : m_value(value) {
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
        return m_value.isEmpty();
    }

    constexpr const value_t& value() const {
        return m_value;
    }
    constexpr operator const value_t&() const {
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

    static value_t clampValue(
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

// A plain tag with label and score.
class Tag final {
    // Properties
    MIXXX_DECL_PROPERTY(TagLabel, label, Label)
    MIXXX_DECL_PROPERTY(TagScore, score, Score)

  public:
    explicit Tag(
            const TagLabel& label,
            TagScore score = TagScore())
            : m_label(label),
              m_score(score) {
    }
    explicit Tag(
            TagScore score = TagScore())
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
