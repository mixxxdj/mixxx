#pragma once

#include <QMetaType>
#include <QString>

#include "util/assert.h"

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

} // namespace tags

} // namespace library

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::library::tags::TagFacet)
