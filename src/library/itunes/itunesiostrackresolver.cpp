#include "library/itunes/itunesiostrackresolver.h"

#include <QStandardPaths>
#include <QString>
#include <QUrl>
#include <QtGlobal>

#include "library/itunes/itunesiosassetexporter.h"
#include "util/assert.h"

namespace mixxx {

QString resolveiPodLibraryTrack(const QUrl& url) {
    DEBUG_ASSERT(url.scheme() == "ipod-library");

    qInfo() << "Exporting" << url << "to file (if needed)";
    QString musicDir = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    ITunesIOSAssetExporter exporter(QDir(musicDir + "/Mixxx/iTunes Tracks"));
    return exporter.exportAsset(url);
}

}; // namespace mixxx
