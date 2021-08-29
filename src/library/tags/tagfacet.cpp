#include "library/tags/tagfacet.h"

#include <QRegularExpression>

namespace {

const QRegularExpression kLowercaseAsciiNotEmpty(
        QStringLiteral("^[\\x{0021}-\\x{0040}\\x{005B}-\\x{007E}]+"));
const QRegularExpression kInverseLowercaseAsciiNotEmpty(
        QStringLiteral("[^\\x{0021}-\\x{0040}\\x{005B}-\\x{007E}]+"));

} // anonymous namespace

namespace mixxx {

namespace library {

namespace tags {

//static
bool TagFacet::isValidValue(
        const value_t& value) {
    if (value.isNull()) {
        return true;
    }
    if (value.isEmpty()) {
        // for disambiguation with null
        return false;
    }
    const auto match = kLowercaseAsciiNotEmpty.match(value);
    DEBUG_ASSERT(match.isValid());
    DEBUG_ASSERT(value.length() > 0);
    // match = exact match
    return match.capturedLength() == value.length();
}

//static
TagFacet::value_t TagFacet::convertIntoValidValue(
        const value_t& value) {
    auto validValue = filterEmptyValue(value.toLower().remove(kInverseLowercaseAsciiNotEmpty));
    DEBUG_ASSERT(isValidValue(validValue));
    return validValue;
}

} // namespace tags

} // namespace library

} // namespace mixxx
