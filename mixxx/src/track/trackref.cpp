#include "track/trackref.h"

#include <QDebugStateSaver>

bool TrackRef::verifyConsistency() const {
    // Class invariant: The location can only be set together with
    // at least one of the other members!
    VERIFY_OR_DEBUG_ASSERT(!hasCanonicalLocation() || hasLocation()) {
        // Condition violated: hasCanonicalLocation() => hasLocation()
        return false;
    }
    VERIFY_OR_DEBUG_ASSERT(!hasId() || hasLocation()) {
        // Condition violated: hasId() => hasLocation()
        return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const TrackRef& trackRef) {
    return os
            << "TrackRef{"
            << trackRef.getLocation().toStdString()
            << ','
            << trackRef.getCanonicalLocation().toStdString()
            << ','
            << trackRef.getId()
            << '}';
}

QDebug operator<<(QDebug dbg, const TrackRef& trackRef) {
    const QDebugStateSaver saver(dbg);
    dbg = dbg.maybeSpace() << "TrackRef";
    return dbg.nospace()
            << '{'
            << trackRef.getLocation()
            << ','
            << trackRef.getCanonicalLocation()
            << ','
            << trackRef.getId()
            << '}';
}
