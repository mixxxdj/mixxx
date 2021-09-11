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

} // namespace tags

} // namespace library

} // namespace mixxx

const mixxx::library::tags::Label mixxx::library::tags::kLabelOrgMixxx =
        mixxx::library::tags::Label{QStringLiteral("org.mixxx")};
