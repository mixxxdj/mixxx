#include "track/steminfo.h"

#include <QDebugStateSaver>

QDebug operator<<(QDebug dbg, const StemInfo& stemInfo) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "StemInfo";
    return dbg.nospace()
            << '{'
            << stemInfo.getLabel()
            << ','
            << stemInfo.getColor().name()
            << '}';
}

bool operator==(
        const StemInfo& lhs,
        const StemInfo& rhs) {
    return lhs.getLabel() == rhs.getLabel() && lhs.getColor() == rhs.getColor();
}
