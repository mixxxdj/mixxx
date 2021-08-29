#pragma once

#include <QMetaType>

#include "util/assert.h"
#include "util/math.h"

namespace mixxx {

namespace library {

namespace tags {

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

} // namespace tags

} // namespace library

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::library::tags::TagScore)
