#include "util/db/dbid.h"

namespace {
/// The variant type has to be obtained from a valid value,
/// because invalid values are mapped to QVariant::Invalid!
const QVariant::Type kVariantType = DbId{DbId::kMinValue}.toVariant().type();
} // namespace

//static
DbId::value_type DbId::valueOf(QVariant variant) {
    // The parameter "variant" is passed by value since we need to use
    // the in place QVariant::convert(). Due to Qt's implicit sharing
    // the value is only copied if the type is different. The redundant
    // conversion inside value() is bypassed since the type already
    // matches. We cannot use value(), only because it returns the
    // valid id 0 in case of conversion errors.
    if (variant.isNull()) {
        return kInvalidValue;
    }
    DEBUG_ASSERT(kVariantType != QVariant::Invalid);
    if (variant.convert(kVariantType)) {
        const auto value = variant.value<value_type>();
        if (isValidValue(value)) {
            return value;
        }
    }
    qCritical() << "Invalid database identifier value:"
                << variant;
    return kInvalidValue;
}
