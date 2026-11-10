#include "library/dao/trackschema.h"

namespace mixxx {
namespace trackschema {
QString tableForColumn(const QString& columnName) {
    if (columnName == TRACKLOCATIONSTABLE_FSDELETED ||
            columnName == TRACKLOCATIONSTABLE_LOCATION ||
            columnName == TRACKLOCATIONSTABLE_DIRECTORY) {
        return QStringLiteral(TRACKLOCATIONS_TABLE);
    }
    if (columnName == FINGERPRINT_TABLE_HASH ||
            columnName == FINGERPRINT_TABLE_SHA256 ||
            columnName == FINGERPRINT_TABLE_DURATION) {
        return FINGERPRINT_METADATA_TABLE;
    }
    if (columnName == CMRT_GROUPS_TABLE_ID ||
            columnName == CMRT_GROUPS_TABLE_HASH ||
            columnName == CMRT_GROUPS_TABLE_SHA256) {
        return CMRT_GROUPS_TABLE;
    }
    // This doesn't detect unknown columns, but that's not really important here.
    return QStringLiteral(LIBRARY_TABLE);
}
} // namespace trackschema
} // namespace mixxx
