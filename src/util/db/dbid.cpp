#include "util/db/dbid.h"


//static
const QVariant::Type DbId::kVariantType = DbId().toVariant().type();

//static
DbId::value_type DbId::valueOf(QVariant /*pass-by-value*/ variant) {
    // The parameter "variant" is passed by value since we need to use
    // the in place QVariant::convert(). Due to Qt's implicit sharing
    // the value is only copied if the type is different. The redundant
    // conversion inside value() is bypassed since the type already
    // matches. We cannot use value(), only because it returns the
    // valid id 0 in case of conversion errors.
    if (variant.convert(kVariantType)) {
        value_type value = variant.value<value_type>();
        if (isValidValue(value)) {
            return value;
        }
    }
    qCritical() << "Invalid database identifier value:"
            << variant;
    return kInvalidValue;
}
