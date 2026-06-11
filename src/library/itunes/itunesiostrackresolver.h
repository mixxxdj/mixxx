#pragma once

#include <QString>
#include <QUrl>

namespace mixxx {

/// Resolves a track from the local iOS music library, identified by a URL of
/// the form `ipod-library://...`, to an on-disk file path. This may require
/// copying the underlying file to the Mixxx sandbox.
QString resolveiPodLibraryTrack(const QUrl& url);

}; // namespace mixxx
