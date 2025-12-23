#include "library/dao/trackschema.h"

namespace mixxx {
namespace trackschema {
QString tableForColumn(const QString& columnName) {
    if (columnName == TRACKLOCATIONSTABLE_FSDELETED || columnName == TRACKLOCATIONSTABLE_LOCATION) {
        return QStringLiteral(TRACKLOCATIONS_TABLE);
    }
    // This doesn't detect unknown columns, but that's not really important here.
    return QStringLiteral(LIBRARY_TABLE);
}
} // namespace trackschema
} // namespace mixxx
