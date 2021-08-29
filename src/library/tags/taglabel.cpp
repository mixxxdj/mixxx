#include "library/tags/taglabel.h"

namespace mixxx {

namespace library {

namespace tags {

//static
bool TagLabel::isValidValue(
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
TagLabel::value_t TagLabel::convertIntoValidValue(
        const value_t& value) {
    auto validValue = filterEmptyValue(value.trimmed());
    DEBUG_ASSERT(isValidValue(validValue));
    return validValue;
}

} // namespace tags

} // namespace library

} // namespace mixxx
