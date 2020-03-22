#include "track/trackfile.h"

QString TrackFile::freshCanonicalLocation() {
    // Note: We return here the cached value, that was calculated just after
    // init this TrackFile object. This will avoid repeated use of the time
    // consuming file IO.
    QString loc = canonicalLocation();
    if (loc.isEmpty()) {
        // Refresh the cached file info and try again
        m_fileInfo.refresh();
        loc = canonicalLocation();
    }
    return loc;
}
