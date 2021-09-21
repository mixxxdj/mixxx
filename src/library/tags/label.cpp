#include "library/tags/label.h"

namespace mixxx {

namespace library {

namespace tags {

//static
bool Label::isValidValue(
        const value_t& value) {
    if (value.isNull()) {
        return true;
    }
    if (value.isEmpty()) {
        // for disambiguation with null
        return false;
    }
    return value.trimmed() == value;
}

//static
Label::value_t Label::convertIntoValidValue(
        const value_t& value) {
    auto validValue = filterEmptyValue(value.trimmed());
    DEBUG_ASSERT(isValidValue(validValue));
    return validValue;
}

namespace {

LabelVector collectLabels(const QStringList& values) {
    LabelVector labels;
    labels.reserve(values.size());
    for (const auto& value : values) {
        labels.push_back(Label{Label::convertIntoValidValue(value)});
    }
    return labels;
}

} // anonymous namespace

LabelVector splitTextIntoLabels(const QString& text, QChar separator) {
    return collectLabels(text.split(separator,
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            Qt::SkipEmptyParts
#else
            QString::SkipEmptyParts
#endif
            ));
}

LabelVector splitTextIntoLabels(const QString& text, const QString& separator) {
    return collectLabels(text.split(separator,
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            Qt::SkipEmptyParts
#else
            QString::SkipEmptyParts
#endif
            ));
}

LabelVector splitTextIntoLabels(const QString& text, const QRegularExpression& pattern) {
    return collectLabels(text.split(pattern,
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
            Qt::SkipEmptyParts
#else
            QString::SkipEmptyParts
#endif
            ));
}

LabelVector splitTextIntoLabelsAtWhitespace(const QString& text) {
    static const auto kWhitespacePattern = QRegularExpression(QStringLiteral("\\s+"));
    return splitTextIntoLabels(text, kWhitespacePattern);
}

QString joinLabelsAsText(const LabelVector& labels, QChar separator) {
    QStringList values;
    values.reserve(labels.size());
    for (const auto& label : labels) {
        values.push_back(label);
    }
    return values.join(separator);
}

} // namespace tags

} // namespace library

} // namespace mixxx

const mixxx::library::tags::Label mixxx::library::tags::kLabelOrgMixxx =
        mixxx::library::tags::Label{QStringLiteral("org.mixxx")};
