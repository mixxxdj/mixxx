#include "track/trackref.h"


bool TrackRef::verifyConsistency() const {
    DEBUG_ASSERT_AND_HANDLE(hasLocation() || !hasCanonicalLocation()) {
        return false;
    }
    DEBUG_ASSERT_AND_HANDLE(hasLocation() || !hasId()) {
        return false;
    }
    return true;
}

std::ostream& operator<<(std::ostream& os, const TrackRef& trackRef) {
    return os << '[' << trackRef.getLocation().toStdString()
            << " | " << trackRef.getCanonicalLocation().toStdString()
            << " | " << trackRef.getId()
            << ']';

}

QDebug operator<<(QDebug debug, const TrackRef& trackRef) {
    debug.nospace() << '[' << trackRef.getLocation()
                    << " | " << trackRef.getCanonicalLocation()
                    << " | " << trackRef.getId()
                    << ']';
    return debug.space();
}
