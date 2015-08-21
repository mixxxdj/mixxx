#include "util/dbid.h"


const QVariant::Type DbId::kVariantType = DbId().toVariant().type();

//static
DbId::value_type DbId::valueOf(QVariant /*pass-by-value*/ variant) {
    if (variant.canConvert<value_type>() &&
            variant.convert(kVariantType)) {
        value_type value = variant.value<value_type>();
        if (isValidValue(value)) {
            return value;
        }
    }
    return kInvalidValue;
}
