#include "library/tags/tagfacetid.h"

#include <QRegularExpression>

namespace {

const QRegularExpression kValidFacetStringNotEmpty(
        QStringLiteral("^[\\+\\-\\./0-9@a-z\\[\\]_]+"));
const QRegularExpression kInversekValidFacetStringNotEmpty(
        QStringLiteral("[^\\+\\-\\./0-9@a-z\\[\\]_]+"));

} // anonymous namespace

namespace mixxx {

namespace library {

namespace tags {

//static
bool TagFacetId::isValidValue(
        const value_t& value) {
    if (value.isNull()) {
        return true;
    }
    if (value.isEmpty()) {
        // for disambiguation with null
        return false;
    }
    const auto match = kValidFacetStringNotEmpty.match(value);
    DEBUG_ASSERT(match.isValid());
    DEBUG_ASSERT(value.length() > 0);
    // match = exact match
    return match.capturedLength() == value.length();
}

//static
TagFacetId::value_t TagFacetId::convertIntoValidValue(
        const value_t& value) {
    auto validValue = filterEmptyValue(value.toLower().remove(kInversekValidFacetStringNotEmpty));
    DEBUG_ASSERT(isValidValue(validValue));
    return validValue;
}

} // namespace tags

} // namespace library

} // namespace mixxx
