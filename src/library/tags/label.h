#pragma once

#include <QMetaType>
#include <QRegularExpression>
#include <QString>
#include <QVector>

#include "util/assert.h"

namespace mixxx {

namespace library {

namespace tags {

/// The displayable name or title of a tag.
///
/// The value contains arbitrary Unicode text that is supposed to
/// be displayed to the user.
///
/// Value constraints:
///   - charset: Unicode
///   - no leading/trailing whitespace
class Label final {
  public:
    typedef QString value_t;

    static bool isValidValue(
            const value_t& value);

    /// Remove leading/trailing whitespace.
    static value_t convertIntoValidValue(
            const value_t& value);

    /// Ensure that empty values are always null
    static value_t filterEmptyValue(
            value_t value) {
        // std::move() is required despite Return Value Optimization (RVO)
        // to avoid clazy warnings!
        return value.isEmpty() ? value_t{} : std::move(value);
    }

    explicit Label(
            value_t value = value_t{})
            : m_value(std::move(value)) {
        DEBUG_ASSERT(isValid());
    }
    Label(const Label&) = default;
    Label(Label&&) = default;

    Label& operator=(const Label&) = default;
    Label& operator=(Label&&) = default;

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
        const Label& lhs,
        const Label& rhs) {
    return lhs.value() == rhs.value();
}

inline bool operator!=(
        const Label& lhs,
        const Label& rhs) {
    return !(lhs == rhs);
}

inline bool operator<(
        const Label& lhs,
        const Label& rhs) {
    return lhs.value() < rhs.value();
}

inline bool operator>(
        const Label& lhs,
        const Label& rhs) {
    return !(lhs < rhs);
}

inline bool operator<=(
        const Label& lhs,
        const Label& rhs) {
    return !(lhs > rhs);
}

inline bool operator>=(
        const Label& lhs,
        const Label& rhs) {
    return !(lhs < rhs);
}

inline uint qHash(
        const Label& label,
        uint seed = 0) {
    return qHash(label.value(), seed);
}

typedef QVector<Label> LabelVector;

LabelVector splitTextIntoLabels(const QString& text, QChar separator);
LabelVector splitTextIntoLabels(const QString& text, const QString& separator);
LabelVector splitTextIntoLabels(const QString& text, const QRegularExpression& pattern);
LabelVector splitTextIntoLabelsAtWhitespace(const QString& text);

QString joinLabelsAsText(const LabelVector& labels, QChar separator = QChar(' '));

/// Predefined labels for identifying the source or owner of a score,
/// e.g. for custom ratings.
///
/// Naming convention: Reverse domain name notation
extern const Label kLabelOrgMixxx;

} // namespace tags

} // namespace library

} // namespace mixxx

Q_DECLARE_METATYPE(mixxx::library::tags::Label)
Q_DECLARE_METATYPE(mixxx::library::tags::LabelVector)
